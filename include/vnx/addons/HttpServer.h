/*
 * HttpServer.h
 *
 *  Created on: Jul 8, 2020
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_HTTPSERVER_H_
#define INCLUDE_VNX_ADDONS_HTTPSERVER_H_

#include <vnx/addons/HttpServerBase.hxx>
#include <vnx/addons/HttpComponentAsyncClient.hxx>


namespace vnx {
namespace addons {

class HttpServer : public HttpServerBase {
public:
	HttpServer(const std::string& _vnx_name);

protected:
	struct state_t {
		bool is_parsed = false;
		bool is_blocked = false;
		bool keep_alive = true;
		vnx::Memory payload;
		std::string sub_path;
		std::shared_ptr<vnx::Pipe> pipe;
		std::shared_ptr<vnx::Stream> stream;
		std::shared_ptr<HttpRequest> request;
		std::shared_ptr<HttpComponentAsyncClient> module;
		std::list<std::pair<std::shared_ptr<const HttpChunk>, size_t>> write_queue;
	};

	enum poll_bits_e : int {
		POLL_READ = 1,
		POLL_WRITE = 2
	};

	void init() override;

	void main() override;

	void handle(std::shared_ptr<const HttpChunk> value) override;

	void http_request_async(std::shared_ptr<const HttpRequest> request,
							const std::string& sub_path,
							const vnx::request_id_t& request_id) const override;

	void http_request_chunk_async(std::shared_ptr<const HttpRequest> request,
							const std::string& sub_path,
							const int64_t& offset,
							const int64_t& max_bytes,
							const vnx::request_id_t& _request_id) const override;

private:
	std::shared_ptr<HttpSession> create_session() const;

	void add_session(std::shared_ptr<HttpSession> session) const;

	void remove_session(const std::string& http_sid) const;

	std::string get_session_cookie(std::shared_ptr<const HttpSession> session) const;

	void update();

	std::shared_ptr<state_t> find_state(int fd) const;

	void on_connect(int fd);

	void on_read(int fd);

	void on_write(int fd);

	void close(int fd);

	void poll_reset();

	void poll_loop() noexcept;

private:
	int m_socket = -1;
	int m_signal_pipe[2] = {-1, -1};
	uint64_t m_next_id = 1;

	std::unordered_map<uint64_t, int> m_request_map;											// [request id => fd]
	std::unordered_map<int, std::shared_ptr<state_t>> m_state_map;								// [fd => state]
	std::map<std::string, std::shared_ptr<HttpComponentAsyncClient>> m_client_map;				// [url path => clients]

	std::shared_ptr<const HttpSession> m_default_session;
	mutable std::unordered_map<std::string, std::shared_ptr<const HttpSession>> m_session_map;	// [http session id => session]
	mutable std::multimap<int64_t, std::string> m_session_timeout_queue;						// [deadline => http session id]

	mutable size_t m_error_counter = 0;
	mutable size_t m_request_counter = 0;
	mutable std::map<int, size_t> m_error_map;

	std::mutex m_poll_mutex;
	std::unordered_map<int, poll_bits_e> m_poll_map;		// [fd => bits]

};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_HTTPSERVER_H_ */
