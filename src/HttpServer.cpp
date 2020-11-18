/*
 * HttpServer.cpp
 *
 *  Created on: Jul 8, 2020
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/addons/HttpResponse.hxx>
#include <vnx/addons/HttpComponentClient.hxx>
#include <vnx/addons/HttpComponentAsyncClient.hxx>
#include <vnx/vnx.h>

#include <atomic>

#include <url.h>
#include <microhttpd.h>

#if MHD_VERSION < 0x00097002
typedef int MHD_Result;
#endif


namespace vnx {
namespace addons {

class HttpServer : public HttpServerBase {
public:
	HttpServer(const std::string& _vnx_name);

protected:
	struct request_state_t {
		MHD_Connection* connection = nullptr;
		std::shared_ptr<HttpRequest> request;
		std::shared_ptr<const HttpResponse> response;
		std::shared_ptr<HttpComponentClient> httpclient;
		std::string sub_path;
	};

	void init() override;

	void main() override;

	void http_request_async(std::shared_ptr<const HttpRequest> request,
							const std::string& sub_path,
							const vnx::request_id_t& request_id) const override;

	void http_request_chunk_async(std::shared_ptr<const HttpRequest> request,
							const std::string& sub_path,
							const int64_t& offset,
							const int64_t& max_bytes,
							const vnx::request_id_t& _request_id) const override;

private:
	void process(request_state_t* state);

	void reply(	request_state_t* state,
				std::shared_ptr<const HttpResponse> result);

	void reply_error(	request_state_t* state,
						const std::exception& ex);

	static MHD_Result
	access_handler_callback(void* cls,
							MHD_Connection* connection,
							const char* url,
							const char* method,
							const char* version,
							const char* upload_data,
							size_t* upload_data_size,
							void** con_cls);

	static MHD_Result
	http_header_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value);

	static MHD_Result
	query_params_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value);

	static void
	request_completed_callback(	void* cls,
								MHD_Connection* connection,
								void** con_cls,
								MHD_RequestTerminationCode term_code);

	static ssize_t chunked_transfer_callback(void *userdata, uint64_t offset, char *dest, size_t length);

private:
	struct HttpClients{
		std::shared_ptr<HttpComponentClient> sync_client;
		std::shared_ptr<HttpComponentAsyncClient> async_client;
	};
	MHD_Daemon* m_daemon = nullptr;
	std::atomic<uint64_t> m_next_id {1};

	std::map<std::string, HttpClients> m_client_map;

};


HttpServer::HttpServer(const std::string& _vnx_name)
	:	HttpServerBase(_vnx_name)
{
	vnx_clean_exit = true;		// process remaining requests on exit
}

void HttpServer::init()
{
	vnx::open_pipe(vnx_name, this, 500);
}

void HttpServer::main()
{
	if(show_info) {
		show_warnings = true;
	}

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

	for(const auto& entry : components)
	{
		const auto& path = entry.first;
		const auto& module = entry.second;
		if(path.empty() || path.back() != '/') {
			log(ERROR) << "Path needs to end with a '/': '" << path << "'";
			continue;
		}
		auto async_client = std::make_shared<HttpComponentAsyncClient>(module);
		async_client->vnx_set_non_blocking(non_blocking);
		add_async_client(async_client);
		auto sync_client = std::make_shared<HttpComponentClient>(module);
		m_client_map[path].sync_client = sync_client;
		m_client_map[path].async_client = async_client;
		log(INFO) << "Got component '" << module << "' for path '" << path << "'";
	}

	Super::main();

	if(m_daemon) {
		MHD_stop_daemon(m_daemon);
	}
}

void HttpServer::http_request_async(std::shared_ptr<const HttpRequest> request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	throw std::logic_error("not implemented");
}

void HttpServer::http_request_chunk_async(std::shared_ptr<const HttpRequest> request,
									const std::string& sub_path,
									const int64_t& offset,
									const int64_t& max_bytes,
									const vnx::request_id_t& _request_id) const
{
	throw std::logic_error("not implemented");
}

void HttpServer::process(request_state_t* state)
{
	if(output_request) {
		publish(state->request, output_request);
	}
	if(state->request->method == "OPTIONS")
	{
		auto result = HttpResponse::create();
		result->status = 204;
		result->headers.emplace_back("Allow", "OPTIONS, GET, HEAD, POST, PUT, DELETE");
		result->headers.emplace_back("Access-Control-Allow-Methods", "DELETE, PUT, POST, GET, OPTIONS");
		result->headers.emplace_back("Access-Control-Allow-Headers", "Content-Type");
		result->headers.emplace_back("Access-Control-Max-Age", "86400");
		reply(state, result);
		return;
	}
	const auto& path = state->request->path;

	std::string prefix;
	std::string sub_path;
	size_t best_match_length = 0;
	HttpClients clients;
	for(const auto& entry : m_client_map) {
		const auto entry_size = entry.first.size();
		if(path.size() >= entry_size
			&& entry_size > best_match_length
			&& path.substr(0, entry_size) == entry.first)
		{
			prefix = entry.first;
			sub_path = "/" + path.substr(entry_size);
			best_match_length = entry_size;
			clients = entry.second;
		}
	}
	if(clients.async_client) {
		if(show_info) {
			log(INFO) << state->request->method << " '" << state->request->path
					<< "' => '" << components[prefix] << sub_path << "'";
		}
		state->sub_path = sub_path;
		state->httpclient = clients.sync_client;
		clients.async_client->http_request(state->request, sub_path,
				std::bind(&HttpServer::reply, this, state, std::placeholders::_1),
				std::bind(&HttpServer::reply_error, this, state, std::placeholders::_1));
	} else {
		if(show_warnings) {
			log(WARN) << state->request->method << " '" << state->request->path << "' failed with: 404 (not found)";
		}
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

	MHD_Response *response;
	if(result->is_chunked){
		response = MHD_create_response_from_callback(result->chunked_total_size, chunk_size, chunked_transfer_callback, state, NULL);
	}else{
		response = MHD_create_response_from_buffer(result->payload.size(), (void*)result->payload.data(), MHD_RESPMEM_PERSISTENT);
	}
	MHD_add_response_header(response, "Server", "vnx.addons.HttpServer");
	if(!access_control_allow_origin.empty()) {
		MHD_add_response_header(response, "Access-Control-Allow-Origin", access_control_allow_origin.c_str());
	}
	if(!result->content_type.empty()) {
		MHD_add_response_header(response, "Content-Type", result->content_type.c_str());
	}
	for(const auto& entry : result->headers) {
		MHD_add_response_header(response, entry.first.c_str(), entry.second.c_str());
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
	if(show_warnings) {
		log(WARN) << state->request->method << " '" << state->request->path << "' failed with: " << ex.what();
	}
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
		try {
			// convert relative paths to absolute
			request->path = Url::Url(request->url).abspath().path();
		} catch(...) {
			return MHD_NO;
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

		if(self->add_task(std::bind(&HttpServer::process, self, state))) {
			MHD_suspend_connection(connection);
		} else {
			return MHD_NO;
		}
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

ssize_t HttpServer::chunked_transfer_callback(void *userdata, uint64_t offset, char *dest, size_t length){
	request_state_t *state = (request_state_t *)userdata;
	std::shared_ptr<const HttpResponse> response = state->httpclient->http_request_chunk(state->request, state->sub_path, offset, length);
	size_t size = response->payload.size();
	if(size == 0){
		return MHD_CONTENT_READER_END_OF_STREAM;
	}

	::memcpy(dest, response->payload.data(), size);
	return size;
}

HttpServerBase* new_HttpServer(const std::string& name) {
	return new HttpServer(name);
}


} // addons
} // vnx
