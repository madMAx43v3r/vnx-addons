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

#include <cstdlib>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif


#ifndef _WIN32
inline void closesocket(int fd) {
	::close(fd);
}
#endif

// trim from left
inline
std::string ltrim(const std::string& str, const char* t = " \t\n\r\f\v")
{
	std::string out(str);
	out.erase(0, out.find_first_not_of(t));
	return out;
}

// trim from right
inline
std::string rtrim(const std::string& str, const char* t = " \t\n\r\f\v")
{
	std::string out(str);
	out.erase(out.find_last_not_of(t) + 1);
	return out;
}

// trim from left & right
inline
std::string trim(const std::string& str, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(str, t), t);
}

inline
::sockaddr_in get_sockaddr_byname(const std::string& endpoint, int port)
{
	::sockaddr_in addr;
	::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(port);
	::hostent* host = ::gethostbyname(endpoint.c_str());
	if(!host) {
		throw std::runtime_error("could not resolve: '" + endpoint + "'");
	}
	::memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	return addr;
}

inline
std::vector<std::pair<std::string, std::string>> parse_query_string(const std::string& query)
{
	std::vector<std::pair<std::string, std::string>> out;
	for(const auto& entry : vnx::string_split(query, '&', true)) {
		const auto pos = entry.find('=');
		if(pos != std::string::npos) {
			out.emplace_back(entry.substr(0, pos), entry.substr(pos + 1));
		} else {
			out.emplace_back(entry, "");
		}
	}
	return out;
}

inline
std::vector<std::pair<std::string, std::string>> parse_cookie_header(const std::string& value)
{
	std::vector<std::pair<std::string, std::string>> out;
	for(const auto& entry : vnx::string_split(value, ';', true)) {
		const auto pos = entry.find('=');
		if(pos != std::string::npos) {
			out.emplace_back(trim(entry.substr(0, pos)), trim(entry.substr(pos + 1)));
		}
	}
	return out;
}

inline
std::string get_http_status_string(const ::int32_t& code) {
	switch(code) {
		case 200: return "OK";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 429: return "Timeout";
		case 500: return "Internal Server Error";
		case 503: return "Overload";
		default: return "?";
	}
}


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

void HttpServer::notify(std::shared_ptr<vnx::Pipe> pipe)
{
	Node::notify(pipe);

	// trigger poll() to wake up
	char dummy = 0;
	if(::write(m_notify_pipe[1], &dummy, 1) != 1) {
		// file a bug report
	}
}

void HttpServer::init()
{
	vnx::open_pipe(vnx_name, this, 500);

	// create notify pipe
	if(::pipe(m_notify_pipe) < 0) {
		throw std::runtime_error("pipe() failed with: " + std::string(strerror(errno)));
	}
	if(set_socket_nonblocking(m_notify_pipe[0]) < 0) {
		throw std::runtime_error("set_socket_nonblocking() failed with: " + std::string(strerror(errno)));
	}
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

	// setup parser
	llhttp_settings_init(&m_settings);
	m_settings.on_url = &HttpServer::on_url;
	m_settings.on_header_field = &HttpServer::on_header_field;
	m_settings.on_header_field_complete = &HttpServer::on_header_field_complete;
	m_settings.on_header_value = &HttpServer::on_header_value;
	m_settings.on_header_value_complete = &HttpServer::on_header_value_complete;
	m_settings.on_headers_complete = &HttpServer::on_headers_complete;
	m_settings.on_body = &HttpServer::on_body;
	m_settings.on_message_complete = &HttpServer::on_message_complete;

	// create server socket
	m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket < 0) {
		throw std::runtime_error("socket() failed with: " + std::string(strerror(errno)));
	}
	if(set_socket_nonblocking(m_socket) < 0) {
		throw std::runtime_error("set_socket_nonblocking() failed with: " + std::string(strerror(errno)));
	}
	{
		int enable = 1;
		if(::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int)) < 0) {
			log(WARN) << "setsockopt(SO_REUSEADDR) failed with: " << strerror(errno);
		}
	}
	{
		::sockaddr_in addr = get_sockaddr_byname(host, port);
		if(::bind(m_socket, (::sockaddr*)&addr, sizeof(addr)) < 0) {
			throw std::runtime_error("bind() failed with: " + std::string(strerror(errno)));
		}
	}
	if(::listen(m_socket, listen_queue_size) < 0) {
		throw std::runtime_error("listen() failed with: " + std::string(strerror(errno)));
	}
	log(INFO) << "Running on " << host << ":" << port;

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
			auto client = std::make_shared<HttpComponentAsyncClient>(module);
			client->vnx_set_non_blocking(is_ourself ? true : non_blocking);
			add_async_client(client);
			m_client_map[path] = client;
		}
		log(INFO) << "Got component '" << module << "' for path '" << path << "'";
	}

	if(stats_interval_ms > 0) {
		set_timer_millis(stats_interval_ms, std::bind(&HttpServer::update, this));
	}

	while(vnx_do_run())
	{
		const auto timeout_us = vnx_process(false);
		do_poll(timeout_us >= 0 ? timeout_us / 1000 : 1000);
	}

	// close all sockets
	for(const auto& entry : m_state_map) {
		closesocket(entry.first);
	}
	m_state_map.clear();

	closesocket(m_socket);
	closesocket(m_notify_pipe[0]);
	closesocket(m_notify_pipe[1]);

	if(m_default_session) {
		vnx::get_auth_server()->logout(m_default_session->vsid);
	}
}

void HttpServer::handle(std::shared_ptr<const HttpChunk> value)
{
	if(auto state = find_state_by_id(value->id)) {
		state->write_queue.emplace_back(value, 0);
	}
	else if(show_warnings) {
		log(WARN) << "Invalid chunk id: " << value->id;
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
		std::ostringstream out;
		out << "<html><body><ul><li>/?"
				<< login_path << "</li><li>/?"
				<< logout_path << "</li><li>/?"
				<< session_path << "</li></ul></body></html>";
		response = HttpResponse::create();
		response->status = 404;
		response->data = out.str();
		response->content_type = "text/html";
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

int HttpServer::on_url(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	state->request->url += std::string(at, length);
	return 0;
}

int HttpServer::on_header_field(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	state->header.key += ascii_tolower(std::string(at, length));
	return 0;
}

int HttpServer::on_header_field_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	state->header.value.clear();
	return 0;
}

int HttpServer::on_header_value(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	state->header.value += std::string(at, length);
	return 0;
}

int HttpServer::on_header_value_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	const auto key = state->header.key;
	const auto value = trim(state->header.value);
	if(key == "cookie") {
		for(const auto& entry : parse_cookie_header(value)) {
			state->request->cookies[entry.first] = entry.second;
		}
	}
	if(key == "content-type") {
		state->request->content_type = value;
	}
	state->request->headers.emplace_back(key, value);
	state->header.key.clear();
	state->header.value.clear();
	return 0;
}

int HttpServer::on_headers_complete(llhttp_t* parser)
{
	if(parser->flags & F_TRANSFER_ENCODING) {
		return -1;
	}
	auto state = (state_t*)parser->data;
	auto self = state->server;
	auto request = state->request;

	// url parsing
	request->method = llhttp_method_name(llhttp_method(parser->method));
	try {
		Url::Url parsed(request->url);
		parsed.abspath();
		request->path = parsed.path();
		for(const auto& entry : parse_query_string(parsed.query())) {
			request->query_params[entry.first] = entry.second;
		}
	}
	catch(...) {
		return -1;
	}

	// session handling
	request->session = self->m_default_session;
	{
		auto cookie = request->cookies.find(self->session_coookie_name);
		if(cookie != request->cookies.end()) {
			auto iter = self->m_session_map.find(cookie->second);
			if(iter != self->m_session_map.end()) {
				request->session = iter->second;
			}
		}
	}

	// TODO: handle streams
	if(false) {
		const auto dst_mac = Hash64::rand();
		state->stream = std::make_shared<Stream>(dst_mac);
		state->request->stream = dst_mac;
		self->process(state);
		return HPE_PAUSED;
	}
	return 0;
}

int HttpServer::on_body(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	auto self = state->server;
	if(state->payload_size + length > self->max_payload_size) {
		if(self->show_warnings) {
			self->log(WARN) << "Maximum payload size of " << self->max_payload_size << " bytes exceeded!";
		}
		return -1;
	}
	if(state->stream) {
		auto chunk = HttpChunk::create();
		chunk->id = state->request->id;
		chunk->data.resize(length);
		chunk->is_eof = false;
		::memcpy(chunk->data.data(), at, length);
		state->stream->send(chunk);
	} else {
		char* chunk = state->payload.add_chunk(length);
		::memcpy(chunk, at, length);
	}
	state->payload_size += length;
	return 0;
}

int HttpServer::on_message_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	auto self = state->server;
	if(state->stream) {
		auto chunk = HttpChunk::create();
		chunk->id = state->request->id;
		chunk->is_eof = true;
		state->stream->send(chunk);
		state->stream = nullptr;
	}
	else {
		auto request = state->request;
		request->payload = vnx::Buffer(state->payload);
		if(request->content_type == "application/x-www-form-urlencoded") {
			for(const auto& entry : parse_query_string(request->payload.as_string())) {
				request->query_params[entry.first] = entry.second;
			}
		}
		self->process(state);
	}
	state->payload.clear();
	state->payload_size = 0;
	state->do_keep_alive = llhttp_should_keep_alive(parser);
	return HPE_PAUSED;
}

void HttpServer::process(state_t* state)
{
	m_request_counter++;
	auto request = state->request;
	publish(request, output_request);

	if(request->method == "OPTIONS")
	{
		auto result = HttpResponse::create();
		result->status = 204;
		result->headers.emplace_back("Allow", "OPTIONS, GET, HEAD, POST, PUT, DELETE");
		result->headers.emplace_back("Access-Control-Allow-Methods", "DELETE, PUT, POST, GET, OPTIONS");
		result->headers.emplace_back("Access-Control-Allow-Headers", "Content-Type");
		result->headers.emplace_back("Access-Control-Max-Age", "86400");
		reply(request->id, result);
		return;
	}
	std::string prefix;
	std::string sub_path;
	size_t best_match_length = 0;
	std::shared_ptr<HttpComponentAsyncClient> client;
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
			client = entry.second;
		}
	}
	if(client) {
		if(show_info) {
			log(INFO) << request->method << " '" << request->path << "' => '" << components[prefix] << sub_path << "'";
		}
		state->module = client;
		state->sub_path = sub_path;
		client->http_request(request, sub_path,
				std::bind(&HttpServer::reply, this, request->id, std::placeholders::_1),
				std::bind(&HttpServer::reply_error, this, request->id, std::placeholders::_1));
	} else {
		reply(request->id, HttpResponse::from_status(404));
	}
}

void HttpServer::reply(uint64_t id, std::shared_ptr<const HttpResponse> response)
{
	publish(response, output_response);

	const auto state = find_state_by_id(id);
	if(!state) {
		if(show_warnings) {
			log(WARN) << "Invalid response id: " << id;
		}
		return;
	}
	state->response = response;

	if(response->status >= 400) {
		m_error_counter++;
		m_error_map[response->status]++;
		if(show_warnings) {
			log(WARN) << state->request->method << " '" << state->request->path << "' failed with: HTTP "
					<< response->status << " (" << response->error_text << ")";
		}
	}

	if(state->stream) {
		// input stream is supported only when response is also a stream
		if(response->stream) {
			llhttp_resume(&state->parser);
		} else {
			if(show_warnings) {
				log(WARN) << "Input stream of type '" << state->request->content_type << "' was not accepted!";
			}
			on_disconnect(state);
			return;
		}
	}

	std::vector<std::pair<std::string, std::string>> headers;
	headers.emplace_back("Server", "vnx.addons.HttpServer");

	if(!access_control_allow_origin.empty()) {
		headers.emplace_back("Access-Control-Allow-Origin", access_control_allow_origin);
	}
	if(!content_security_policy.empty()) {
		headers.emplace_back("Content-Security-Policy", content_security_policy);
	}
	if(!response->content_type.empty())
	{
		auto content_type = response->content_type;
		if(content_type.find("charset=") == std::string::npos) {
			auto iter = charset.find(content_type);
			if(iter != charset.end()) {
				content_type += "; charset=" + iter->second;
			}
		}
		headers.emplace_back("Content-Type", content_type.c_str());
	}
	if(auto_session && state->request->session == m_default_session)
	{
		auto session = create_session();
		session->vsid = m_default_session->vsid;
		add_session(session);
		headers.emplace_back("Set-Cookie", get_session_cookie(session));
	}

	std::shared_ptr<const HttpData> payload;
	if(response->stream) {
		if(auto pipe = vnx::get_pipe(response->stream)) {
			vnx::connect(pipe, this, 100, 100);
			state->pipe = pipe;
			on_resume(state);		// resume reading & parsing body
		}
		else {
			payload = response;
		}
	}
	else if(response->is_chunked) {
		state->payload_size = 0;	// reset offset
		state->is_chunked_reply = true;
	}
	else {
		payload = response;
	}

	if(payload) {
		headers.emplace_back("Content-Length", std::to_string(payload->data.size()));
	}
	std::ostringstream out;
	out << "HTTP/1.1 " << response->status << " " << get_http_status_string(response->status) << "\r\n";
	for(const auto& entry : headers) {
		out << entry.first << ": " << entry.second << "\r\n";
	}
	out << "\r\n";
	{
		auto data = HttpData::create();
		data->data = out.str();
		data->is_eof = false;
		state->write_queue.emplace_back(data, 0);
	}
	if(payload) {
		state->write_queue.emplace_back(payload, 0);
	}
	on_write(state);
}

void HttpServer::reply_error(uint64_t id, const vnx::exception& ex)
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
		response->content_type = "text/plain";
		response->data = response->error_text;
	}
	reply(id, response);
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

std::shared_ptr<HttpServer::state_t> HttpServer::find_state_by_id(uint64_t id) const
{
	auto iter = m_request_map.find(id);
	if(iter != m_request_map.end()) {
		return iter->second;
	}
	return nullptr;
}

std::shared_ptr<HttpServer::state_t> HttpServer::find_state_by_socket(int fd) const
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
	state->fd = fd;
	state->server = this;
	on_request(state);

	m_state_map[fd] = state;
	m_connect_counter++;
}

void HttpServer::on_request(std::shared_ptr<state_t> state)
{
	state->poll_bits = POLL_BIT_READ;
	state->request = HttpRequest::create();
	state->request->id = m_next_id++;

	llhttp_init(&state->parser, HTTP_REQUEST, &m_settings);
	state->parser.data = state.get();

	m_request_map[state->request->id] = state;
}

void HttpServer::on_resume(std::shared_ptr<state_t> state)
{
	llhttp_resume(&state->parser);
	on_parse(state);
}

void HttpServer::on_parse(std::shared_ptr<state_t> state)
{
	const auto ret = llhttp_execute(&state->parser, state->buffer, state->offset);
	switch(ret) {
		case HPE_OK:
			state->offset = 0;
			state->poll_bits |= POLL_BIT_READ;
			break;
		case HPE_PAUSED:
			break;
		default:
			if(show_warnings) {
				log(WARN) << "HTTP parsing failed with: " << llhttp_errno_name(ret);
			}
			on_disconnect(state);
	}
}

void HttpServer::on_read(std::shared_ptr<state_t> state)
{
	const auto max_bytes = sizeof(state->buffer) - state->offset;
	const auto num_bytes = ::recv(state->fd, state->buffer + state->offset, max_bytes, 0);
	if(num_bytes < 0) {
		if(errno != EAGAIN) {
			on_disconnect(state);	// broken connection
			return;
		}
	}
	else if(num_bytes > 0) {
		state->offset += num_bytes;
	}
	else if(max_bytes > 0) {
		on_disconnect(state);		// normal disconnect
		return;
	}
	on_parse(state);
}

void HttpServer::on_write(std::shared_ptr<state_t> state)
{
	bool is_eof = false;
	bool is_blocked = false;
	while(!state->write_queue.empty()) {
		const auto iter = state->write_queue.begin();
		const auto chunk = iter->first;
		const void* data = chunk->data.data(iter->second);
		const size_t num_bytes = chunk->data.size() - iter->second;
#ifdef MSG_NOSIGNAL
			ssize_t res = ::send(state->fd, data, num_bytes, MSG_NOSIGNAL);
#else
			ssize_t res = ::send(state->fd, data, num_bytes, 0);
#endif
		if(res >= 0) {
			if(size_t(res) >= num_bytes) {
				state->write_queue.erase(iter);
				if(chunk->is_eof) {
					is_eof = true;
					break;
				}
			} else {
				iter->second += res;
				is_blocked = true;
				break;
			}
		} else {
			break;	// cannot write to socket, wait for on_read() to close connection
		}
	}
	if(state->pipe) {
		if(is_blocked && !state->is_blocked) {
			state->pipe->pause();
		}
		if(!is_blocked && state->is_blocked) {
			state->pipe->resume();
		}
	}
	state->is_blocked = is_blocked;

	if(is_blocked) {
		state->poll_bits |= POLL_BIT_WRITE;
	}
	else if(is_eof) {
		if(state->do_keep_alive) {
			on_finish(state);
			on_request(state);
		} else {
			on_disconnect(state);
		}
	}
	else if(state->is_chunked_reply) {
		if(auto client = state->module) {
			client->http_request_chunk(state->request, state->sub_path, state->payload_size, chunk_size,
					std::bind(&HttpServer::on_write_data, this, state->request->id, std::placeholders::_1),
					std::bind(&HttpServer::on_write_error, this, state->request->id, std::placeholders::_1));
		}
		else {
			on_disconnect(state);
		}
	}
}

void HttpServer::on_write_data(uint64_t id, std::shared_ptr<const HttpData> chunk)
{
	if(auto state = find_state_by_id(id)) {
		state->payload_size += chunk->data.size();
		state->write_queue.emplace_back(chunk, 0);
		on_write(state);
	}
}

void HttpServer::on_write_error(uint64_t id, const vnx::exception& ex)
{
	if(show_warnings) {
		log(WARN) << "http_request_chunk() failed with: " << ex.what();
	}
	if(auto state = find_state_by_id(id)) {
		on_disconnect(state);
	}
}

void HttpServer::on_finish(std::shared_ptr<state_t> state)
{
	if(auto request = state->request) {
		m_request_map.erase(request->id);
	}
	llhttp_reset(&state->parser);

	const auto fd = state->fd;
	*state = state_t();
	state->fd = fd;
	state->server = this;
}

void HttpServer::on_disconnect(std::shared_ptr<state_t> state)
{
	on_finish(state);
	m_state_map.erase(state->fd);
	closesocket(state->fd);
}

int HttpServer::set_socket_nonblocking(int fd)
{
	const auto res = ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	if(res < 0 && show_warnings) {
		log(WARN) << "fcntl() failed with: " << strerror(errno);
	}
	return res;
}


} // addons
} // vnx
