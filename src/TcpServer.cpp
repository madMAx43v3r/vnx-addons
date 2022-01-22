/*
 * TcpServer.cpp
 *
 *  Created on: Dec 17, 2021
 *      Author: mad
 */

#include <vnx/addons/TcpServer.h>
#include <vnx/vnx.h>

#include <cstdlib>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#endif


namespace vnx {
namespace addons {

TcpServer::TcpServer(const std::string& _vnx_name)
	:	TcpServerBase(_vnx_name)
{
}

void TcpServer::notify(std::shared_ptr<vnx::Pipe> pipe)
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

void TcpServer::init()
{
	vnx::open_pipe(vnx_name, this, 500);

	// create notify pipe
#ifdef _WIN32
	m_notify_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_notify_socket == -1){
		throw std::runtime_error("socket() failed with: " + get_socket_error_text());
	}
	set_socket_nonblocking(m_notify_socket);
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
	set_socket_nonblocking(m_notify_pipe[0]);
#endif
}

void TcpServer::main()
{
	// create server socket
	endpoint = TcpEndpoint::create();
	endpoint->port = port;
	endpoint->host_name = host;
	endpoint->non_blocking = true;
	endpoint->tcp_no_delay = tcp_no_delay;
	endpoint->tcp_keepalive = tcp_keepalive;
	endpoint->send_buffer_size = send_buffer_size;
	endpoint->receive_buffer_size = receive_buffer_size;
	endpoint->listen_queue_size = listen_queue_size;

	m_socket = endpoint->open();
	endpoint->bind(m_socket);
	endpoint->listen(m_socket);

	log(INFO) << "Running on " << host << ":" << port;

	if(stats_interval_ms > 0) {
		set_timer_millis(stats_interval_ms, std::bind(&TcpServer::print_stats, this));
	}

	while(true) {
		const auto timeout_us = vnx_process(false);
		if(!vnx_do_run()) {
			break;
		}
		do_poll(timeout_us >= 0 ? timeout_us / 1000 : 1000);
	}

	// close all sockets
	for(const auto& entry : m_state_map) {
		endpoint->close(entry.first);
	}
	m_state_map.clear();

	endpoint->close(m_socket);
#ifdef _WIN32
	endpoint->close(m_notify_socket);
#else
	endpoint->close(m_notify_pipe[0]);
	endpoint->close(m_notify_pipe[1]);
#endif
}

bool TcpServer::pause(uint64_t client)
{
	if(auto state = find_state_by_id(client)) {
		state->poll_bits &= ~POLL_BIT_READ;
		return true;
	}
	return false;
}

bool TcpServer::resume(uint64_t client)
{
	if(auto state = find_state_by_id(client)) {
		state->poll_bits |= POLL_BIT_READ;
		return true;
	}
	return false;
}

bool TcpServer::disconnect(uint64_t client)
{
	if(auto state = find_state_by_id(client)) {
		on_disconnect(state);
		return true;
	}
	return false;
}

bool TcpServer::send_to(uint64_t client, std::shared_ptr<vnx::Buffer> data)
{
	if(auto state = find_state_by_id(client)) {
		state->write_queue.emplace_back(data, 0);
		on_write(state);
		return true;
	}
	return false;
}

uint64_t TcpServer::add_client(int fd, const std::string& address)
{
	endpoint->set_options(fd);

	if(auto state = on_connect(fd, address)) {
		return state->id;
	}
	throw std::runtime_error("failed to add client");
}

void TcpServer::print_stats()
{
	log(INFO) << m_state_map.size() << " clients, " << m_error_counter << " failed, "
			<< m_timeout_counter << " timeout, " << m_refused_counter << " refused";
}

std::shared_ptr<TcpServer::state_t> TcpServer::find_state_by_id(uint64_t id) const
{
	auto iter = m_client_map.find(id);
	if(iter != m_client_map.end()) {
		return iter->second;
	}
	return nullptr;
}

std::shared_ptr<TcpServer::state_t> TcpServer::find_state_by_socket(int fd) const
{
	auto iter = m_state_map.find(fd);
	if(iter != m_state_map.end()) {
		return iter->second;
	}
	return nullptr;
}

std::shared_ptr<TcpServer::state_t> TcpServer::on_connect(int fd, const std::string& address)
{
	auto state = std::make_shared<state_t>();
	state->fd = fd;
	state->id = m_next_id++;
	state->poll_bits = POLL_BIT_READ;

	m_state_map[fd] = state;
	m_client_map[state->id] = state;
	m_connect_counter++;

	try {
		on_connect(state->id, address);
	} catch(...) {
		on_disconnect(state);
	}
	return state;
}

void TcpServer::on_read(std::shared_ptr<state_t> state)
{
	void* buffer = nullptr;
	size_t max_bytes = 0;
	try {
		on_buffer(state->id, buffer, max_bytes);
	}
	catch(const std::exception& ex) {
		if(show_warnings) {
			log(WARN) << "on_buffer() failed with: " << ex.what();
		}
		m_error_counter++;
		on_disconnect(state);		// buffer error
		return;
	}

	const auto num_bytes = ::recv(state->fd, (char *)buffer, max_bytes, 0);
	if(num_bytes < 0) {
#ifdef _WIN32
		if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
		if(errno != EAGAIN && errno != EWOULDBLOCK)
#endif
		{
			m_error_counter++;
			on_disconnect(state);	// broken connection
			return;
		}
	}
	else if(num_bytes > 0) {
		state->waiting_since = -1;
	}
	else if(max_bytes > 0) {
		on_disconnect(state);		// normal disconnect
		return;
	}
	try {
		if(on_read(state->id, num_bytes)) {
			state->poll_bits |= POLL_BIT_READ;
		}
	}
	catch(const std::exception& ex) {
		if(show_warnings) {
			log(WARN) << "on_read() failed with: " << ex.what();
		}
		m_error_counter++;
		on_disconnect(state);		// parse error
	}
}

void TcpServer::on_write(std::shared_ptr<state_t> state)
{
	bool is_eof = false;
	bool is_blocked = false;
	while(!is_blocked && !state->write_queue.empty())
	{
		const auto iter = state->write_queue.begin();
		const auto chunk = iter->first;
		if(!chunk) {
			is_eof = true;
			state->write_queue.erase(iter);
			break;
		}
		const void* data = chunk->data(iter->second);
		const size_t num_bytes = chunk->size() - iter->second;
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
					log(WARN) << "send() failed with: " << get_socket_error_text();
				}
				m_error_counter++;
				on_disconnect(state);	// broken connection
				return;
			}
		}
	}
	if(is_blocked && !state->is_blocked) {
		try {
			on_pause(state->id);
		} catch(...) {
			on_disconnect(state);
		}
		if(state->pipe) {
			state->pipe->pause();
		}
	}
	if(!is_blocked && state->is_blocked) {
		if(state->pipe) {
			state->pipe->resume();
		}
		try {
			on_resume(state->id);
		} catch(...) {
			on_disconnect(state);
		}
	}
	state->is_blocked = is_blocked;

	if(is_blocked) {
		state->poll_bits |= POLL_BIT_WRITE;
	}
	else if(is_eof) {
		on_disconnect(state);
	}
}

void TcpServer::on_timeout(std::shared_ptr<state_t> state)
{
	if(state->do_timeout) {
		m_timeout_counter++;
		on_disconnect(state);
	}
}

void TcpServer::on_disconnect(std::shared_ptr<state_t> state)
{
	if(auto pipe = state->pipe) {
		pipe->close();
	}
	m_state_map.erase(state->fd);
	m_client_map.erase(state->id);

	endpoint->close(state->fd);
	state->fd = -1;

	try {
		on_disconnect(state->id);
	} catch(...) {
		// ignore
	}
}

void TcpServer::do_poll(int timeout_ms) noexcept
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
	if(WSAPoll(fds.data(), fds.size(), timeout_ms) == SOCKET_ERROR) {
		log(WARN) << "WSAPoll() failed with: " << WSAGetLastError();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
#else
	if(::poll(fds.data(), fds.size(), timeout_ms) < 0) {
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
			try {
				const auto fd = endpoint->accept(m_socket);
				if(fd >= 0) {
					::sockaddr_in sock_addr = {};
					::socklen_t addr_len = sizeof(sock_addr);
					::getpeername(fd, (::sockaddr*)&sock_addr, &addr_len);

					char address[INET_ADDRSTRLEN] = {};
					::inet_ntop(AF_INET, &sock_addr.sin_addr, address, INET_ADDRSTRLEN);

					if(m_state_map.size() < size_t(max_connections)) {
						on_connect(fd, std::string(address));
					} else {
						if(show_warnings) {
							log(WARN) << "Refused connection due to limit at " << max_connections;
						}
						endpoint->close(fd);
						m_refused_counter++;
					}
				} else {
					break;
				}
			} catch(...) {
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


} // addons
} // vnx

