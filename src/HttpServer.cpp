/*
 * HttpServer.cpp
 *
 *  Created on: May 12, 2021
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>
#include <vnx/addons/HttpChunk.hxx>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/addons/HttpResponse.hxx>
#include <vnx/OverflowException.hxx>
#include <vnx/PermissionDenied.hxx>
#include <vnx/SHA256.h>
#include <vnx/vnx.h>

#include <url.h>


namespace vnx {
namespace addons {

HttpServer::HttpServer(const std::string& _vnx_name)
	:	HttpServerBase(_vnx_name)
{
	vnx_clean_exit = true;		// process remaining requests on exit

	charset.emplace("text/html", "utf-8");
	charset.emplace("text/css", "utf-8");
	charset.emplace("text/plain", "utf-8");
	charset.emplace("text/x-tex", "utf-8");
	charset.emplace("text/x-lua", "utf-8");
	charset.emplace("text/x-shellscript", "utf-8");
	charset.emplace("application/json", "utf-8");
	charset.emplace("application/javascript", "utf-8");
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
	{
		auto session = HttpSession::create();
		session->vsid = vnx::get_auth_server()->login_anonymous(default_access)->id;
		m_default_session = session;
	}
	m_daemon = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_SUSPEND_RESUME | (use_epoll ? MHD_USE_EPOLL_LINUX_ONLY : MHD_USE_POLL),
			port, NULL, NULL,
			&HttpServer::access_handler_callback, this,
			MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::request_completed_callback, this,
			MHD_OPTION_URI_LOG_CALLBACK, &HttpServer::uri_log_callback, this,
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
		const bool is_ourself = module == vnx_name;
		{
			auto async_client = std::make_shared<HttpComponentAsyncClient>(module);
			async_client->vnx_set_non_blocking(is_ourself ? true : non_blocking);
			add_async_client(async_client);
			m_client_map[path].async_client = async_client;
		}
		if(!is_ourself) {
			auto sync_client = std::make_shared<HttpComponentClient>(module);
			m_client_map[path].sync_client = sync_client;
		}
		log(INFO) << "Got component '" << module << "' for path '" << path << "'";
	}

	set_timer_millis(10 * 1000, std::bind(&HttpServer::update, this));

	Super::main();

	if(m_daemon) {
		MHD_stop_daemon(m_daemon);
	}
	if(m_default_session) {
		vnx::get_auth_server()->logout(m_default_session->vsid);
	}
}

void HttpServer::http_request_async(std::shared_ptr<const HttpRequest> request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	std::shared_ptr<HttpResponse> response;

	if(sub_path == login_path)
	{
		auto session = create_session();

		auto user = request->query_params.find("user");
		if(user != request->query_params.end())
		{
			auto passwd_hex = request->query_params.find("passwd_hex");
			auto passwd_plain = request->query_params.find("passwd_plain");

			std::string passwd;
			if(passwd_hex != request->query_params.end()) {
				passwd = passwd_hex->second;
			}
			else if(passwd_plain != request->query_params.end()) {
				passwd = vnx::sha256_str(passwd_plain->second);
			}
			else {
				// no password = no bueno
				http_request_async_return(request_id, HttpResponse::from_status(403));
				return;
			}
			auto vnx_session = vnx::get_auth_server()->login(user->second, passwd);
			if(!vnx_session || vnx_session->user != user->second) {
				// login failed
				http_request_async_return(request_id, HttpResponse::from_status(403));
				return;
			}
			session->user = user->second;
			session->vsid = vnx_session->id;
			log(INFO) << "User '" << user->second << "' logged in successfully.";
		} else {
			// anonymous login
			session->vsid = m_default_session->vsid;
			log(INFO) << "Anonymous user logged in successfully.";
		}
		add_session(session);

		vnx::Object result = session->to_object();
		if(auto vnx_session = vnx::get_session(session->vsid)) {
			result["permissions"] = vnx_session->permissions;
		}
		response = vnx::clone(HttpResponse::from_object_json(result));
		response->headers.emplace_back("Set-Cookie", get_session_cookie(session));
	}
	else if(sub_path == logout_path)
	{
		if(auto session = request->session) {
			remove_session(session->hsid);
			if(session->user.empty()) {
				log(INFO) << "Anonymous user logged out.";
			} else {
				log(INFO) << "User '" << session->user << "' logged out.";
			}
		}
		response = HttpResponse::create();
		response->status = 200;
		response->headers.emplace_back("Set-Cookie", session_coookie_name + "=null; Path=/; " + cookie_policy);
	}
	else if(sub_path == session_path)
	{
		vnx::Object result = request->session->to_object();
		if(auto vnx_session = vnx::get_session(request->session->vsid)) {
			result["permissions"] = vnx_session->permissions;
		}
		http_request_async_return(request_id, HttpResponse::from_object_json(result));
		return;
	}
	else {
		throw std::logic_error("invalid request");
	}

	if(response) {
		auto redirect = request->query_params.find("redirect");
		if(redirect != request->query_params.end()) {
			response->status = 303;
			response->headers.emplace_back("Location", redirect->second);
		}
		http_request_async_return(request_id, response);
	}
}

void HttpServer::http_request_chunk_async(	std::shared_ptr<const HttpRequest> request,
											const std::string& sub_path,
											const int64_t& offset,
											const int64_t& max_bytes,
											const vnx::request_id_t& _request_id) const
{
	throw std::logic_error("invalid request");
}

void HttpServer::process(request_state_t* state)
{
	m_request_counter++;
	auto request = state->request;
	request->session = m_default_session;
	{
		auto cookie = request->cookies.find(session_coookie_name);
		if(cookie != request->cookies.end()) {
			auto iter = m_session_map.find(cookie->second);
			if(iter != m_session_map.end()) {
				request->session = iter->second;
			}
		}
	}
	if(output_request) {
		publish(request, output_request);
	}
	if(request->method == "OPTIONS")
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
	std::string prefix;
	std::string sub_path;
	size_t best_match_length = 0;
	http_clients_t clients;
	for(const auto& entry : m_client_map)
	{
		const auto entry_size = entry.first.size();
		if(request->path.size() >= entry_size
			&& entry_size > best_match_length
			&& request->path.substr(0, entry_size) == entry.first)
		{
			prefix = entry.first;
			sub_path = "/" + request->path.substr(entry_size);
			best_match_length = entry_size;
			clients = entry.second;
		}
	}
	if(clients.async_client) {
		if(show_info) {
			log(INFO) << request->method << " '" << request->path
					<< "' => '" << components[prefix] << sub_path << "'";
		}
		state->sub_path = sub_path;
		state->http_client = clients.sync_client;
		clients.async_client->http_request(request, sub_path,
				std::bind(&HttpServer::reply, this, state, std::placeholders::_1),
				std::bind(&HttpServer::reply_error, this, state, std::placeholders::_1));
	} else {
		reply(state, HttpResponse::from_status(404));
	}
}

void HttpServer::reply(	request_state_t* state,
						std::shared_ptr<const HttpResponse> result)
{
	auto request = state->request;
	if(result->status >= 400) {
		m_error_counter++;
		m_error_map[result->status]++;
		if(show_warnings) {
			log(WARN) << request->method << " '" << request->path << "' failed with: HTTP " << result->status << " (" << result->error_text << ")";
		}
	}
	publish(result, output_response);
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
	if(!content_security_policy.empty()) {
		MHD_add_response_header(response, "Content-Security-Policy", content_security_policy.c_str());
	}
	if(!result->content_type.empty()) {
		auto content_type = result->content_type;
		if(content_type.find("charset=") == std::string::npos) {
			auto iter = charset.find(content_type);
			if(iter != charset.end()) {
				content_type += "; charset=" + iter->second;
			}
		}
		MHD_add_response_header(response, "Content-Type", content_type.c_str());
	}
	for(const auto& entry : result->headers) {
		MHD_add_response_header(response, entry.first.c_str(), entry.second.c_str());
	}
	if(auto_session && request->session == m_default_session)
	{
		auto session = create_session();
		session->vsid = m_default_session->vsid;
		add_session(session);
		MHD_add_response_header(response, "Set-Cookie", get_session_cookie(session).c_str());
	}
	const auto ret = MHD_queue_response(state->connection, result->status, response);
	if(ret != MHD_YES) {
		log(WARN) << "Failed to queue MHD response!";
	}
	MHD_resume_connection(state->connection);	// after this, 'state' may be deleted at any time
	MHD_destroy_response(response);
}

void HttpServer::reply_error(	request_state_t* state,
								const vnx::exception& ex)
{
	auto response = HttpResponse::create();
	if(std::dynamic_pointer_cast<const OverflowException>(ex.value())) {
		response->status = 503;
	} else if(std::dynamic_pointer_cast<const PermissionDenied>(ex.value())) {
		response->status = 403;
	} else {
		response->status = 500;
	}
	response->error_text = ex.what();
	if(error_payload) {
		response->content_type = "text/plain; charset=utf-8";
		response->payload = response->error_text;
	}
	reply(state, response);
}

std::shared_ptr<HttpSession> HttpServer::create_session() const
{
	auto session = HttpSession::create();
	session->login_time = vnx::get_time_seconds();
	for(int i = 0; i < session_size; ++i) {
		if(i) { session->hsid += "-"; }
		session->hsid += vnx::to_hex_string(vnx::rand64());
	}
	return session;
}

void HttpServer::add_session(std::shared_ptr<HttpSession> session) const
{
	if(session->hsid.empty()) {
		throw std::logic_error("http_session_id.empty()");
	}
	if(!session->login_time) {
		throw std::logic_error("login_time == 0");
	}
	if(m_session_map.emplace(session->hsid, session).second) {
		if(session_timeout >= 0) {
			m_session_timeout_queue.emplace(session->login_time + session_timeout, session->hsid);
		}
	} else {
		throw std::logic_error("session already exists");
	}
}

void HttpServer::remove_session(const std::string& http_sid) const
{
	auto iter = m_session_map.find(http_sid);
	if(iter == m_session_map.end()) {
		return;
	}
	if(auto session = iter->second) {
		vnx::get_auth_server()->logout(session->vsid);
	}
	for(auto iter = m_session_timeout_queue.begin(); iter != m_session_timeout_queue.end(); ++iter) {
		if(iter->second == http_sid) {
			m_session_timeout_queue.erase(iter);
			break;
		}
	}
	m_session_map.erase(iter);
}

std::string HttpServer::get_session_cookie(std::shared_ptr<const HttpSession> session) const
{
	return session_coookie_name + "=" + session->hsid + "; Path=/; "
			+ (session_timeout >= 0 ? "Max-Age=" + std::to_string(session_timeout) + "; " : "") + cookie_policy;
}

void HttpServer::update()
{
	const auto now = vnx::get_time_seconds();
	while(!m_session_timeout_queue.empty()) {
		auto iter = m_session_timeout_queue.begin();
		if(now > iter->first) {
			const auto http_sid = iter->second;
			log(INFO) << "Session '" << http_sid << "' timed out.";
			remove_session(http_sid);
		} else {
			break;
		}
	}
	log(INFO) << m_request_counter << " requests, " << m_error_counter << " failed, "
			<< m_session_map.size() << " sessions, errors=" << vnx::to_string(m_error_map);
}

void* HttpServer::uri_log_callback(void* cls, const char* uri)
{
	auto request = HttpRequest::create();
	if(uri) {
		request->url = std::string(uri);
	}
	auto* state = new request_state_t();
	state->request = request;
	return state;
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
	if(!state->connection) {
		state->connection = connection;
		auto request = state->request;
		request->id = self->m_next_id++;
		if(method) {
			request->method = std::string(method);
		}
		try {
			// convert relative paths to absolute
			request->path = Url::Url(request->url).abspath().path();
		} catch(...) {
			return MHD_NO;
		}
		state->request = request;
		*con_cls = state;
		MHD_get_connection_values(connection, MHD_HEADER_KIND, &HttpServer::http_header_callback, state);
		MHD_get_connection_values(connection, MHD_COOKIE_KIND, &HttpServer::cookie_callback, state);
		MHD_get_connection_values(connection, MHD_ValueKind(MHD_POSTDATA_KIND | MHD_GET_ARGUMENT_KIND), &HttpServer::query_params_callback, state);
		return MHD_YES;
	}
	const auto request = state->request;

	if(const auto length = *upload_data_size)
	{
		if(state->post_processor || request->content_type == "application/x-www-form-urlencoded") {
			if(!state->post_processor) {
				state->post_processor = MHD_create_post_processor(connection, 65536, &HttpServer::post_data_iterator, state);
			}
			MHD_post_process(state->post_processor, upload_data, length);
		}
		auto& payload = request->payload;
		const size_t offset = payload.size();
		if(self->max_payload_size >= 0 && offset + length > self->max_payload_size) {
			return MHD_NO;
		}
		payload.resize(offset + length);
		::memcpy(payload.data(offset), upload_data, length);
		*upload_data_size = 0;
	} else {
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
	auto request = state->request;
	if(key) {
		const auto key_ = ascii_tolower(std::string(key));
		if(key_ == "content-type" && value) {
			request->content_type = ascii_tolower(std::string(value));
		}
		request->headers.emplace_back(key_, value ? std::string(value) : std::string());
	}
	return MHD_YES;
}

MHD_Result HttpServer::cookie_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value)
{
	request_state_t* state = (request_state_t*)cls;
	if(key && value) {
		state->request->cookies[std::string(key)] = std::string(value);
	}
	return MHD_YES;
}

MHD_Result HttpServer::query_params_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value)
{
	request_state_t* state = (request_state_t*)cls;
	if(key) {
		state->request->query_params[std::string(key)] = value ? std::string(value) : std::string();
	}
	return MHD_YES;
}

MHD_Result
HttpServer::post_data_iterator(	void* cls, MHD_ValueKind kind, const char* key, const char* filename, const char* content_type,
								const char* transfer_encoding, const char* data, uint64_t off, size_t size)
{
	request_state_t* state = (request_state_t*)cls;
	if(key && off == 0) {
		state->request->query_params[std::string(key)] = data ? std::string(data, size) : std::string();
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
		if(state->post_processor) {
			MHD_destroy_post_processor(state->post_processor);
		}
		delete state;
	}
}

ssize_t HttpServer::chunked_transfer_callback(void *userdata, uint64_t offset, char *dest, size_t length)
{
	request_state_t *state = (request_state_t *)userdata;
	if(!state->http_client) {
		return MHD_CONTENT_READER_END_OF_STREAM;
	}
	std::shared_ptr<const HttpResponse> response = state->http_client->http_request_chunk(state->request, state->sub_path, offset, length);
	const size_t size = response->payload.size();
	if(size == 0){
		return MHD_CONTENT_READER_END_OF_STREAM;
	}
	::memcpy(dest, response->payload.data(), size);
	return size;
}

std::shared_ptr<HttpServer::state_t> HttpServer::find_state(int fd) const
{
	auto iter = m_state_map.find(fd);
	if(iter != m_state_map.end()) {
		return iter->second;
	}
	return nullptr;
}

void HttpServer::on_connect(int fd)
{
	auto state = std::make_shared<state_t>();
	m_state_map[fd] = state;
}

void HttpServer::on_read(int fd)
{
	if(auto state = find_state(fd))
	{
		if(!state->is_parsed) {
			// TODO: parse
		}
	}
}

void HttpServer::on_write(int fd)
{
	if(auto state = find_state(fd))
	{
		// TODO
	}
}

void HttpServer::close(int fd)
{
	auto iter = m_state_map.find(fd);
	if(iter != m_state_map.end()) {
		auto state = iter->second;
		if(auto request = state->request) {
			m_request_map.erase(request->id);
		}
		m_state_map.erase(iter);
	}
	{
		std::lock_guard lock(m_poll_mutex);
		m_poll_map.erase(fd);
	}
}


} // addons
} // vnx
