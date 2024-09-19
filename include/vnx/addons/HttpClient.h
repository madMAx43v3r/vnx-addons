/*
 * HttpClient.h
 *
 *  Created on: Sep 19, 2024
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_HTTPCLIENT_H_
#define INCLUDE_VNX_ADDONS_HTTPCLIENT_H_

#include <vnx/addons/HttpClientBase.hxx>

#include <llhttp.h>


namespace vnx {
namespace addons {

class HttpClient : public HttpClientBase {
public:
	HttpClient(const std::string& _vnx_name);

protected:
	struct state_t {
		bool is_blocked = false;
		bool is_pending = true;
		bool do_keep_alive = false;
		bool do_timeout = true;
		size_t payload_size = 0;
		uint64_t client = 0;
		HttpClient* module = nullptr;
		llhttp_t parser = {};
		char buffer[16384];
		uint32_t offset = 0;					// offset into buffer
		struct {
			std::string key;
			std::string value;
		} header;
		int port = -1;
		std::string host;
		std::string scheme;
		std::string method;
		std::string path;
		http_request_options_t options;
		std::shared_ptr<vnx::Buffer> request_body;
		std::shared_ptr<vnx::Memory> payload;
		std::shared_ptr<HttpResponse> response;
		std::string ex_what;
		vnx::request_id_t request_id;
		std::function<void(std::shared_ptr<HttpResponse>)> async_return;
	};

	void init() override;

	void get_async(const std::string& url, const http_request_options_t& options,
			const vnx::request_id_t& request_id) const override;

	void post_async(const std::string& url, const ::vnx::Buffer& data,
			const http_request_options_t& options, const vnx::request_id_t& request_id) const override;

	void post_json_async(const std::string& url, const std::string& data,
			const http_request_options_t& options, const vnx::request_id_t& request_id) const override;

	void post_text_async(const std::string& url, const std::string& data,
			const http_request_options_t& options, const vnx::request_id_t& request_id) const override;

private:
	void process(state_t* state);

	std::shared_ptr<state_t> new_state(
			const std::string& method, const std::string& url, const http_request_options_t& options,
			const vnx::request_id_t& request_id) const;

	void connect(std::shared_ptr<state_t> state);

	void send_request(std::shared_ptr<state_t> state);

	void on_buffer(uint64_t client, void*& buffer, size_t& max_bytes) override;

	void on_read(uint64_t client, size_t num_bytes) override;

	void on_write(uint64_t client, size_t num_bytes) override;

	void on_connect(uint64_t client, const std::string& address) override;

	void on_disconnect(uint64_t client, const std::string& address) override;

	std::shared_ptr<state_t> find_state_by_id(uint64_t id) const;

	static int on_header_field(llhttp_t* parser, const char* at, size_t length);
	static int on_header_field_complete(llhttp_t* parser);
	static int on_header_value(llhttp_t* parser, const char* at, size_t length);
	static int on_header_value_complete(llhttp_t* parser);
	static int on_headers_complete(llhttp_t* parser);
	static int on_body(llhttp_t* parser, const char* at, size_t length);
	static int on_message_complete(llhttp_t* parser);

private:
	llhttp_settings_t m_settings = {};

	std::unordered_map<uint64_t, std::shared_ptr<state_t>> m_state_map;

	mutable size_t m_error_counter = 0;
	mutable size_t m_timeout_counter = 0;
	mutable size_t m_request_counter = 0;
	mutable size_t m_connect_counter = 0;
	mutable size_t m_total_sent = 0;
	mutable size_t m_total_received = 0;
	mutable std::map<int, size_t> m_error_map;

};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_HTTPCLIENT_H_ */
