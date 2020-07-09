/*
 * HttpServer.cpp
 *
 *  Created on: Jul 8, 2020
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

HttpServer::HttpServer(const std::string& _vnx_name)
	:	HttpServerBase(_vnx_name)
{
}

void HttpServer::main()
{
	open_pipe(vnx_name, this, UNLIMITED);		// TODO: this should not be UNLIMITED

	m_daemon = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_SUSPEND_RESUME | (use_epoll ? MHD_USE_EPOLL_LINUX_ONLY : MHD_USE_POLL),
			port, NULL, NULL,
			&HttpServer::access_handler_callback, this,
			MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::request_completed_callback, this,
			MHD_OPTION_END);

	if(!m_daemon) {
		log(ERROR) << "Failed to start MHD daemon!";
		return;
	}
	log(INFO) << "Running on port " << port;

	for(const auto& entry : component_map)
	{
		const auto& path = entry.first;
		const auto& module = entry.second;
		if(path.empty() || path.back() != '/') {
			log(ERROR) << "Path needs to end with a '/': '" << path << "'";
			continue;
		}
		auto client = std::make_shared<HttpComponentAsyncClient>(module);
		m_client_map[path] = client;
		add_async_client(client);
		log(INFO) << "Got component '" << module << "' for path '" << path << "'";
	}

	Super::main();

	if(m_daemon) {
		MHD_stop_daemon(m_daemon);
	}
}

void HttpServer::http_request_async(const std::shared_ptr<const HttpRequest>& request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	throw std::logic_error("not implemented");
}

void HttpServer::process(request_state_t* state)
{
	if(output_request) {
		publish(state->request, output_request);
	}
	const auto& path = state->request->path;

	size_t best_match_length = 0;
	std::string sub_path;
	std::shared_ptr<HttpComponentAsyncClient> client;
	for(const auto& entry : m_client_map) {
		const auto entry_size = entry.first.size();
		if(path.size() >= entry_size
			&& entry_size > best_match_length
			&& path.substr(0, entry_size) == entry.first)
		{
			client = entry.second;
			sub_path = "/" + path.substr(entry_size);
			best_match_length = entry_size;
		}
	}
	if(client) {
		client->http_request(state->request, sub_path,
				std::bind(&HttpServer::reply, this, state, std::placeholders::_1),
				std::bind(&HttpServer::reply_error, this, state, std::placeholders::_1));
	} else {
		reply(state, HttpResponse::from_status(404));
	}
}

void HttpServer::reply(	request_state_t* state,
						std::shared_ptr<const HttpResponse> result)
{
	if(output_response) {
		publish(result, output_response);
	}
	state->response = result;
	MHD_Response* response = MHD_create_response_from_buffer(result->payload.size(), (void*)result->payload.data(), MHD_RESPMEM_PERSISTENT);
	if(!result->content_type.empty()) {
		MHD_add_response_header(response, "Content-Type", result->content_type.c_str());
	}
	const auto ret = MHD_queue_response(state->connection, result->status, response);
	if(ret != MHD_YES) {
		log(WARN) << "Failed to queue MHD response!";
	}
	MHD_resume_connection(state->connection);	// after this, 'state' may be deleted at any time
	MHD_destroy_response(response);
}

void HttpServer::reply_error(	request_state_t* state,
								const std::exception& ex)
{
	log(WARN) << state->request->method << " '" << state->request->path << "' failed: " << ex.what();
	reply(state, HttpResponse::from_status(500));
}

MHD_Result HttpServer::access_handler_callback(	void* cls,
												MHD_Connection* connection,
												const char* url,
												const char* method,
												const char* version,
												const char* upload_data,
												size_t* upload_data_size,
												void** con_cls)
{
	HttpServer* self = (HttpServer*)cls;
	request_state_t* state = (request_state_t*)(*con_cls);
	if(!state) {
		state = new request_state_t();
		state->connection = connection;
		auto request = HttpRequest::create();
		request->id = self->m_next_id++;
		request->url = std::string(url);
		request->method = std::string(method);
		{
			const std::string tmp(url);
			const auto pos = tmp.find_first_of("?;");
			if(pos != std::string::npos) {
				request->path = tmp.substr(0, pos);
			} else {
				request->path = tmp;
			}
		}
		state->request = request;
		*con_cls = state;
		return MHD_YES;
	}
	const size_t length = *upload_data_size;

	if(length) {
		auto& payload = state->request->payload;
		const size_t offset = payload.size();
		if(self->max_payload_size >= 0 && offset + length > self->max_payload_size) {
			return MHD_NO;
		}
		payload.resize(offset + length);
		::memcpy(payload.data(offset), upload_data, length);
		*upload_data_size = 0;
	} else {
		MHD_get_connection_values(connection, MHD_HEADER_KIND, &HttpServer::http_header_callback, state);
		MHD_get_connection_values(connection, MHD_ValueKind(MHD_POSTDATA_KIND | MHD_GET_ARGUMENT_KIND), &HttpServer::query_params_callback, state);
		MHD_suspend_connection(connection);
		self->add_task(std::bind(&HttpServer::process, self, state));
	}
	return MHD_YES;
}

MHD_Result HttpServer::http_header_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value)
{
	request_state_t* state = (request_state_t*)cls;
	if(key && value) {
		const auto key_ = ascii_tolower(std::string(key));
		if(key_ == "content-type") {
			state->request->content_type = ascii_tolower(std::string(value));
		}
		state->request->headers.emplace_back(key_, std::string(value));
	}
	return MHD_YES;
}

MHD_Result HttpServer::query_params_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value)
{
	request_state_t* state = (request_state_t*)cls;
	if(key && value) {
		state->request->query_params[std::string(key)] = std::string(value);
	}
	return MHD_YES;
}

void HttpServer::request_completed_callback(void* cls,
											MHD_Connection* connection,
											void** con_cls,
											MHD_RequestTerminationCode term_code)
{
	request_state_t* state = (request_state_t*)(*con_cls);
	if(state) {
		delete state;
	}
}


} // addons
} // vnx
