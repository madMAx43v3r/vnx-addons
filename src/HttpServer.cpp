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
#include <vnx/addons/DeflateOutputStream.h>

#include <vnx/OverflowException.hxx>
#include <vnx/PermissionDenied.hxx>
#include <vnx/SHA256.h>
#include <vnx/vnx.h>

#include <url.h>

#include <cstdlib>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <poll.h>
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
	addr.sin_port = htons(port);
	{
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);

		::hostent* host = ::gethostbyname(endpoint.c_str());
		if(!host) {
			throw std::runtime_error("could not resolve: '" + endpoint + "'");
		}
		memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	}
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
	do_compress.insert("text");
	do_compress.insert("application/json");
	do_compress.insert("application/javascript");

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
#ifdef _WIN32
	if(::send(m_notify_socket, &dummy, 1, 0) != 1)
#else
	if(::write(m_notify_pipe[1], &dummy, 1) != 1)
#endif
	{
		// file a bug report
	}
}

void HttpServer::init()
{
	vnx::open_pipe(vnx_name, this, 500);

	// create notify pipe
#ifdef _WIN32
	m_notify_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_notify_socket == -1){
		throw std::runtime_error("socket() failed with: " + get_socket_error_text());
	}
	if(set_socket_nonblocking(m_notify_socket) < 0) {
		throw std::runtime_error("set_socket_nonblocking() failed with: " + get_socket_error_text());
	}

	{
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		if(bind(m_notify_socket, (sockaddr *)&addr, sizeof(addr)) == -1){
			throw std::runtime_error("bind() failed with: " + get_socket_error_text());
		}
	}

	{
		sockaddr_in addr = {};
		int length = sizeof(addr);
		if(getsockname(m_notify_socket, (sockaddr*)&addr, &length) == -1){
			throw std::runtime_error("getsockname() failed with: " + get_socket_error_text());
		}
		if(connect(m_notify_socket, (sockaddr*)&addr, length) < 0){
			throw std::runtime_error("connect() failed with: " + get_socket_error_text());
		}
	}
#else
	if(::pipe(m_notify_pipe) < 0) {
		throw std::runtime_error("pipe() failed with: " + get_socket_error_text());
	}
	if(set_socket_nonblocking(m_notify_pipe[0]) < 0) {
		throw std::runtime_error("set_socket_nonblocking() failed with: " + get_socket_error_text());
	}
#endif
}

void HttpServer::main()
{
	if(num_threads <= 0) {
		throw std::logic_error("num_threads <= 0");
	}
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
		throw std::runtime_error("socket() failed with: " + get_socket_error_text());
	}
	if(set_socket_nonblocking(m_socket) < 0) {
		throw std::runtime_error("set_socket_nonblocking() failed with: " + get_socket_error_text());
	}
	{
		int enable = 1;
		if(::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int)) < 0) {
			log(WARN) << "setsockopt(SO_REUSEADDR) failed with: " << get_socket_error_text();
		}
	}
	{
		::sockaddr_in addr = get_sockaddr_byname(host, port);
		if(::bind(m_socket, (::sockaddr*)&addr, sizeof(addr)) < 0) {
			throw std::runtime_error("bind() failed with: " + get_socket_error_text());
		}
	}
	if(::listen(m_socket, listen_queue_size) < 0) {
		throw std::runtime_error("listen() failed with: " + get_socket_error_text());
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
	for(int i = 0; i < num_threads; ++i) {
		m_threads.push_back(std::make_shared<vnx::ThreadPool>(1, 10));
	}

	while(true) {
		const auto timeout_us = vnx_process(false);
		if(!vnx_do_run()) {
			break;
		}
		do_poll(timeout_us >= 0 ? timeout_us / 1000 : 1000);
	}

	// stop all threads
	for(auto thread : m_threads) {
		thread->close();
	}

	// close all sockets
	for(const auto& entry : m_state_map) {
		closesocket(entry.first);
	}
	m_state_map.clear();

	closesocket(m_socket);
#ifdef _WIN32
	closesocket(m_notify_socket);
#else
	closesocket(m_notify_pipe[0]);
	closesocket(m_notify_pipe[1]);
#endif

	if(m_default_session) {
		vnx::get_auth_server()->logout(m_default_session->vsid);
	}
}

void HttpServer::handle(std::shared_ptr<const HttpChunk> value)
{
	on_write_data(value->id, value);
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
		response = HttpResponse::create();
		response->status = 404;
		response->data = vnx::to_string(std::vector<std::string>{login_path, logout_path, session_path});
		response->content_type = "application/json";
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
		state->is_pending = true;
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
	state->is_pending = false;
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
	headers.emplace_back("Date", vnx::get_date_string_ex("%a, %d %b %Y %H:%M:%S %Z", true));
	headers.insert(headers.end(), add_headers.begin(), add_headers.end());
	headers.insert(headers.end(), response->headers.begin(), response->headers.end());

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
	const bool skip_body = state->request->method == "HEAD";

	std::shared_ptr<const HttpData> payload;
	if(response->stream) {
		auto pipe = vnx::get_pipe(response->stream);
		if(!pipe) {
			log(WARN) << "Response stream not found!";
		}
		if(pipe && !skip_body) {
			if(state->stream) {
				try {
					state->stream->open();
					on_resume(state);
				} catch(const std::exception& ex) {
					log(WARN) << ex.what();
				}
			}
			vnx::connect(pipe, this, 100, 100);
			pipe->resume();
			state->pipe = pipe;
			state->is_chunked_transfer = true;
			headers.emplace_back("Connection", "keep-alive");
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

	bool do_compress_ = false;
	if(!payload || payload->data.size() > min_compress_size)
	{
		auto content_type = response->content_type;
		{
			const auto pos = content_type.find(';');
			if(pos != std::string::npos) {
				content_type = content_type.substr(0, pos);
			}
		}
		for(const auto& type : do_compress) {
			if(content_type == type
				|| (content_type.size() > type.size()
					&& content_type[type.size()] == '/'
					&& content_type.substr(0, type.size()) == type))
			{
				do_compress_ = true;
				break;
			}
		}
	}
	if(do_compress_) {
		for(const auto& entry : vnx::string_split(state->request->get_header_value("Accept-Encoding"), ',', true)) {
			const auto encoding = trim(entry);
			if(enable_deflate && encoding == "deflate") {
				state->output_encoding = DEFLATE;
			}
		}
	}
	switch(state->output_encoding) {
		case DEFLATE:
			state->is_chunked_transfer = true;
			headers.emplace_back("Content-Encoding", "deflate");
			break;
		case IDENTITY:
			if(response->is_chunked) {
				if(response->total_size >= 0) {
					headers.emplace_back("Content-Length", std::to_string(response->total_size));
				} else {
					state->is_chunked_transfer = true;
				}
			}
			else if(payload) {
				headers.emplace_back("Content-Length", std::to_string(payload->data.size()));
			}
			break;
		default:
			on_disconnect(state);
			return;
	}
	if(state->is_chunked_transfer) {
		headers.emplace_back("Transfer-Encoding", "chunked");
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
		data->is_eof = skip_body;
		state->write_queue.emplace_back(data, 0);
	}
	if(payload && !skip_body) {
		do_write_data(state, payload);
	} else {
		on_write(state);
	}
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
	session->session_timeout = session_timeout;
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
	log(INFO) << m_request_counter << " requests, " << m_error_counter << " failed, " << m_timeout_counter << " timeout, "
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
#ifdef _WIN32
		if(WSAGetLastError() != WSAEWOULDBLOCK) {
#else
		if(errno != EAGAIN && errno != EWOULDBLOCK) {
#endif
			on_disconnect(state);	// broken connection
			return;
		}
	}
	else if(num_bytes > 0) {
		state->offset += num_bytes;
		state->waiting_since = -1;
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
	while(!is_blocked && !state->write_queue.empty())
	{
		const auto iter = state->write_queue.begin();
		const auto chunk = iter->first;
		const void* data = chunk->data.data(iter->second);
		const size_t num_bytes = chunk->data.size() - iter->second;
#ifdef MSG_NOSIGNAL
			ssize_t res = ::send(state->fd, (const char*)data, num_bytes, MSG_NOSIGNAL);
#else
			ssize_t res = ::send(state->fd, (const char*)data, num_bytes, 0);
#endif
		if(res >= 0) {
			if(res > 0) {
				state->waiting_since = -1;
			}
			if(size_t(res) >= num_bytes) {
				state->write_queue.erase(iter);
				if(chunk->is_eof) {
					is_eof = true;
					break;
				}
			} else {
				iter->second += res;
				is_blocked = true;
			}
		} else {
#ifdef _WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK)
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK)
#endif
			{
				is_blocked = true;
			} else {
				if(show_warnings) {
					log(WARN) << "Error when writing to socket: " << get_socket_error_text();
				}
				on_disconnect(state);	// broken connection
				return;
			}
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
	else if(state->is_chunked_reply && !state->is_chunked_reply_pending) {
		if(auto client = state->module) {
			client->http_request_chunk(state->request, state->sub_path, state->payload_size, max_chunk_size,
					std::bind(&HttpServer::on_write_data, this, state->request->id, std::placeholders::_1, true),
					std::bind(&HttpServer::on_write_error, this, state->request->id, std::placeholders::_1));
			state->is_chunked_reply_pending = true;
		} else {
			log(WARN) << "Unable to make client request!";
			on_disconnect(state);
			return;
		}
	}
}

void HttpServer::on_write_data(uint64_t id, std::shared_ptr<const HttpData> chunk, bool encode)
{
	if(auto state = find_state_by_id(id)) {
		do_write_data(state, chunk, encode);
	}
	else if(show_warnings) {
		log(WARN) << "Invalid request id: " << id;
	}
}

void HttpServer::do_write_data(std::shared_ptr<state_t> state, std::shared_ptr<const HttpData> chunk, bool encode)
{
	if(encode) {
		if(state->is_chunked_reply) {
			publish(chunk, output_response);
			state->is_chunked_reply_pending = false;
		}
		state->payload_size += chunk->data.size();
		if(auto response = state->response) {
			if(response->total_size >= 0) {
				if(state->payload_size > size_t(response->total_size)) {
					log(WARN) << "Response payload overflow, payload_size=" << state->payload_size << ", total_size=" << response->total_size;
					on_disconnect(state);
					return;
				}
				if(chunk->is_eof && state->payload_size != size_t(response->total_size)) {
					log(WARN) << "Response payload underflow, payload_size=" << state->payload_size << ", total_size=" << response->total_size;
					on_disconnect(state);
					return;
				}
			}
		}
		switch(state->output_encoding) {
			case DEFLATE:
				if(!state->deflate) {
					state->deflate = std::make_shared<DeflateOutputStream>(nullptr, 6, 32768);
				}
				m_threads[state->fd % m_threads.size()]->add_task(std::bind(&HttpServer::deflate_write_task, this, state->request->id, state->deflate, chunk));
				return;
			case IDENTITY:
				break;
			default:
				log(WARN) << "Invalid output encoding: " << state->output_encoding;
				on_disconnect(state);
				return;
		}
	}
	if(state->is_chunked_transfer) {
		auto data = HttpData::create();
		std::ostringstream out;
		out << std::hex << chunk->data.size() << "\r\n";
		out.write((const char*)chunk->data.data(), chunk->data.size());
		out << "\r\n";
		if(chunk->is_eof) {
			out << "0\r\n\r\n";
		}
		data->data = out.str();
		data->is_eof = chunk->is_eof;
		chunk = data;
	}
	state->write_queue.emplace_back(chunk, 0);
	on_write(state);
}

void HttpServer::on_write_error(uint64_t id, const vnx::exception& ex)
{
	if(show_warnings) {
		log(WARN) << "http_request_chunk() failed with: " << ex.what();
	}
	if(auto state = find_state_by_id(id)) {
		if(state->is_chunked_reply) {
			state->is_chunked_reply_pending = false;
		}
		on_disconnect(state);
	}
}

void HttpServer::on_finish(std::shared_ptr<state_t> state)
{
	if(auto request = state->request) {
		m_request_map.erase(request->id);
	}
	if(auto pipe = state->pipe) {
		pipe->close();
	}
	llhttp_reset(&state->parser);

	const auto fd = state->fd;
	*state = state_t();
	state->fd = fd;
	state->server = this;
}

void HttpServer::on_timeout(std::shared_ptr<state_t> state)
{
	if(state->do_timeout) {
		m_timeout_counter++;
		on_disconnect(state);
	}
}

void HttpServer::on_disconnect(std::shared_ptr<state_t> state)
{
	on_finish(state);
	m_state_map.erase(state->fd);
	closesocket(state->fd);
	state->fd = -1;
}

void HttpServer::do_poll(int timeout_ms) noexcept
{
	const auto now = vnx::get_wall_time_micros();

	std::vector<::pollfd> fds;
	std::vector<std::shared_ptr<state_t>> states;
	fds.reserve(2 + m_state_map.size());
	states.reserve(fds.capacity());
	{
		::pollfd set = {};
#ifdef _WIN32
		set.fd = m_notify_socket;
#else
		set.fd = m_notify_pipe[0];
#endif
		set.events = POLLIN;
		fds.push_back(set);
		states.push_back(nullptr);
	}
	{
		::pollfd set = {};
		set.fd = m_socket;
		set.events = POLLIN;
		fds.push_back(set);
		states.push_back(nullptr);
	}

	std::vector<std::shared_ptr<state_t>> timeout_list;
	for(const auto& entry : m_state_map)
	{
		const auto& state = entry.second;
		if(state->waiting_since < 0) {
			state->waiting_since = now;
		}
		else if(connection_timeout_ms > 0) {
			const auto delta = now - state->waiting_since;
			if(delta / 1000 > connection_timeout_ms) {
				timeout_list.push_back(state);
			}
		}
		if(state->fd >= 0) {
			::pollfd set = {};
			if(state->poll_bits & POLL_BIT_READ) {
				set.events |= POLLIN;
			}
			if(state->poll_bits & POLL_BIT_WRITE) {
				set.events |= POLLOUT;
			}
			if(set.events) {
				set.fd = state->fd;
				fds.push_back(set);
				states.push_back(state);
			}
		}
	}
#ifdef _WIN32
	if(WSAPoll(fds.data(), fds.size(), std::min(timeout_ms, 1000)) == SOCKET_ERROR) {
		log(WARN) << "WSAPoll() failed with: " << WSAGetLastError();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
#else
	if(::poll(fds.data(), fds.size(), std::min(timeout_ms, 1000)) < 0) {
		log(WARN) << "poll() failed with: " << get_socket_error_text();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
#endif
	if(fds[0].revents & POLLIN) {
		char buf[1024];
#ifdef _WIN32
		while(::recv(m_notify_socket, buf, sizeof(buf), 0) >= sizeof(buf));
#else
		while(::read(m_notify_pipe[0], buf, sizeof(buf)) >= sizeof(buf));
#endif
	}
	if(fds[1].revents & POLLIN) {
		while(true) {
			const int fd = ::accept(m_socket, 0, 0);
			if(fd >= 0) {
				if(set_socket_nonblocking(fd) < 0) {
					closesocket(fd);
					continue;
				}
				on_connect(fd);
			} else {
				break;
			}
		}
	}
	for(size_t i = 2; i < fds.size(); ++i) {
		const auto& set = fds[i];
		const auto& state = states[i];
		if(set.revents & POLLIN) {
			// reset poll bit first
			state->poll_bits &= ~POLL_BIT_READ;
			on_read(state);
		}
		if(set.revents & POLLOUT) {
			// reset poll bit first
			state->poll_bits &= ~POLL_BIT_WRITE;
			on_write(state);
		}
	}
	for(const auto& state : timeout_list) {
		on_timeout(state);
	}
}

void HttpServer::deflate_write_task(const uint64_t id,
									std::shared_ptr<DeflateOutputStream> stream,
									std::shared_ptr<const HttpData> chunk) noexcept
{
	auto data = HttpData::create();
	stream->set_output(&data->data);
	stream->write(chunk->data.data(), chunk->data.size());
	if(chunk->is_eof) {
		stream->finish();
	} else {
		stream->flush();
	}
	data->is_eof = chunk->is_eof;
	add_task(std::bind(&HttpServer::on_write_data, this, id, data, false));
}


} // addons
} // vnx
