/*
 * HttpClient.cpp
 *
 *  Created on: Sep 19, 2024
 *      Author: mad
 */

#include <vnx/addons/HttpClient.h>
#include <vnx/vnx.h>

#include <url.h>
#include <cstdlib>


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


namespace vnx {
namespace addons {

HttpClient::HttpClient(const std::string& _vnx_name)
	:	HttpClientBase(_vnx_name)
{
	// setup parser
	llhttp_settings_init(&m_settings);
	m_settings.on_header_field = &HttpClient::on_header_field;
	m_settings.on_header_field_complete = &HttpClient::on_header_field_complete;
	m_settings.on_header_value = &HttpClient::on_header_value;
	m_settings.on_header_value_complete = &HttpClient::on_header_value_complete;
	m_settings.on_headers_complete = &HttpClient::on_headers_complete;
	m_settings.on_body = &HttpClient::on_body;
	m_settings.on_message_complete = &HttpClient::on_message_complete;
}

void HttpClient::init()
{
	Super::init();

	vnx::open_pipe(vnx_name, this, 10000);
	vnx::open_pipe(vnx_get_id(), this, 10000);
}

std::shared_ptr<HttpClient::state_t> HttpClient::new_state(
		const std::string& method, const std::string& url, const http_request_options_t& options,
		const vnx::request_id_t& request_id) const
{
	auto state = std::make_shared<state_t>();
	state->module = (HttpClient*)this;
	state->method = method;
	state->options = options;
	state->request_id = request_id;

	llhttp_init(&state->parser, HTTP_RESPONSE, &m_settings);
	state->parser.data = state.get();

	auto response = HttpResponse::create();

	auto new_url = url;
	bool has_query = url.find('?') != std::string::npos;
	for(const auto& entry : options.query) {
		new_url += (has_query ? "&" : "?") + entry.first + "=" + entry.second;
		has_query = true;
	}
	{
		Url::Url url(new_url);
		url.escape();
		state->scheme = url.scheme();
		state->host = url.host();
		state->port = url.port();
		state->path = url.fullpath();
		response->url = url.str();
	}
	state->response = response;
	return state;
}

void HttpClient::connect(std::shared_ptr<state_t> state)
{
	if(state->scheme.empty() || state->host.empty()) {
		throw std::logic_error("invalid url: " + state->response->url);
	}
	if(state->scheme != "http") {
		throw std::logic_error("unsupported scheme: " + state->scheme);
	}
	if(state->port <= 0) {
		state->port = 80;
	}
	TcpEndpoint peer;
	peer.host_name = state->host;
	peer.port = state->port;

	state->client = connect_client(peer);

	m_state_map[state->client] = state;
}

void HttpClient::get_async(const std::string& url, const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("GET", url, options, request_id);

	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		get_async_return(request_id, response);
	};

	((HttpClient*)this)->connect(state);
}

void HttpClient::get_json_async(const std::string& url, const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("GET", url, options, request_id);

	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		vnx::Variant value;
		vnx::from_string(response->data.as_string(), value);
		get_json_async_return(request_id, value);
	};

	((HttpClient*)this)->connect(state);
}

void HttpClient::get_text_async(const std::string& url, const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("GET", url, options, request_id);

	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		get_text_async_return(request_id, response->data.as_string());
	};

	((HttpClient*)this)->connect(state);
}

void HttpClient::post_async(
		const std::string& url, const ::vnx::Buffer& data,
		const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("POST", url, options, request_id);

	state->request_body = std::make_shared<vnx::Buffer>(data);

	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		post_async_return(request_id, response);
	};

	((HttpClient*)this)->connect(state);
}

void HttpClient::post_json_async(
		const std::string& url, const std::string& data,
		const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("POST", url, options, request_id);

	state->request_body = std::make_shared<vnx::Buffer>(data);

	if(!state->options.content_type) {
		state->options.content_type = "application/json; charset=utf-8";
	}
	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		if(response->status == 200) {
			post_json_async_return(request_id, response);
		} else {
			vnx_async_return_ex_what(request_id, "HTTP status " + std::to_string(response->status));
		}
	};

	((HttpClient*)this)->connect(state);
}

void HttpClient::post_text_async(
		const std::string& url, const std::string& data,
		const http_request_options_t& options, const vnx::request_id_t& request_id) const
{
	const auto state = new_state("POST", url, options, request_id);

	state->request_body = std::make_shared<vnx::Buffer>(data);

	if(!state->options.content_type) {
		state->options.content_type = "text/plain; charset=utf-8";
	}
	state->async_return = [this, request_id](std::shared_ptr<HttpResponse> response) {
		if(response->status == 200) {
			post_text_async_return(request_id, response);
		} else {
			vnx_async_return_ex_what(request_id, "HTTP status " + std::to_string(response->status));
		}
	};

	((HttpClient*)this)->connect(state);
}

int HttpClient::on_header_field(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	state->header.key += ascii_tolower(std::string(at, length));
	return 0;
}

int HttpClient::on_header_field_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	state->header.value.clear();
	return 0;
}

int HttpClient::on_header_value(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	state->header.value += std::string(at, length);
	return 0;
}

int HttpClient::on_header_value_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;

	const auto key = state->header.key;
	const auto value = trim(state->header.value);
	if(key == "content-type") {
		state->response->content_type = value;
	}
	state->response->headers.emplace_back(key, value);
	state->header.key.clear();
	state->header.value.clear();
	return 0;
}

int HttpClient::on_headers_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	state->response->status = parser->status_code;
	state->response->total_size = parser->content_length;
	return 0;
}

int HttpClient::on_body(llhttp_t* parser, const char* at, size_t length)
{
	auto state = (state_t*)parser->data;
	auto self = state->module;

	if(self->max_payload_size >= 0 && state->payload_size + length > self->max_payload_size) {
		state->ex_what = "max_payload_size exceeded";
		return -1;
	}
	if(!state->payload) {
		state->payload = std::make_shared<vnx::Memory>();
	}
	char* chunk = state->payload->add_chunk(length);
	::memcpy(chunk, at, length);
	state->payload_size += length;
	return 0;
}

int HttpClient::on_message_complete(llhttp_t* parser)
{
	auto state = (state_t*)parser->data;
	auto self = state->module;

	if(auto payload = state->payload) {
		state->response->data = *payload;
	}
	state->payload = nullptr;
	state->payload_size = 0;
	state->do_keep_alive = llhttp_should_keep_alive(parser);

	self->process(state);
	// Note: state could be deleted already here
	return 0;
}

void HttpClient::process(state_t* state)
{
	m_request_counter++;

	publish(state->response, output_response);

	if(state->is_pending) {
		state->async_return(state->response);
		state->is_pending = false;
	}
	disconnect(state->client);		// TODO: connection reuse?
}

void HttpClient::send_request(std::shared_ptr<state_t> state)
{
	const auto& options = state->options;

	std::ostringstream ss;
	ss << state->method << " " << state->path << " HTTP/1.1\r\n";
	ss << "Host: " << state->host << "\r\n";

	for(const auto& entry : options.headers) {
		ss << entry.first << ": " << entry.second << "\r\n";
	}
	if(state->request_body) {
		std::string content_type = "application/octet-stream";
		if(auto type = options.content_type) {
			content_type = *type;
		}
		ss << "Content-Type: " << content_type << "\r\n";
		ss << "Content-Length: " << state->request_body->size() << "\r\n";
	}
	ss << "\r\n";

	send_to(state->client, std::make_shared<vnx::Buffer>(ss.str()));

	if(state->request_body) {
		send_to(state->client, state->request_body);
	}
}

void HttpClient::on_buffer(uint64_t client, void*& buffer, size_t& max_bytes)
{
	if(auto state = find_state_by_id(client)) {
		buffer = state->buffer + state->offset;
		max_bytes = sizeof(state->buffer) - state->offset;
	} else {
		disconnect(client);
	}
}

void HttpClient::on_read(uint64_t client, size_t num_bytes)
{
	auto state = find_state_by_id(client);
	if(!state) {
		disconnect(client);
		return;
	}
	state->offset += num_bytes;

	const auto ret = llhttp_execute(&state->parser, state->buffer, state->offset);
	switch(ret) {
		case HPE_OK:
			state->offset = 0;
			break;
		case HPE_PAUSED:
			break;
		default:
			state->ex_what = "HTTP parsing failed with: " + std::string(llhttp_errno_name(ret));
			disconnect(client);
	}
	m_total_received += num_bytes;
}

void HttpClient::on_write(uint64_t client, size_t num_bytes)
{
	m_total_sent += num_bytes;
}

void HttpClient::on_connect(uint64_t client, const std::string& address)
{
	if(auto state = find_state_by_id(client)) {
		send_request(state);
		m_connect_counter++;
	} else {
		disconnect(client);
	}
}

void HttpClient::on_disconnect(uint64_t client, const std::string& address)
{
	if(auto state = find_state_by_id(client)) {
		if(state->is_pending) {
			vnx_async_return_ex_what(
					state->request_id, state->ex_what.empty() ? "connection failed" : state->ex_what);
			state->is_pending = false;
			m_error_counter++;
		}
		llhttp_reset(&state->parser);
	}
	m_state_map.erase(client);
}

std::shared_ptr<HttpClient::state_t> HttpClient::find_state_by_id(uint64_t id) const
{
	auto iter = m_state_map.find(id);
	if(iter != m_state_map.end()) {
		return iter->second;
	}
	return nullptr;
}


} // addons
} // vnx
