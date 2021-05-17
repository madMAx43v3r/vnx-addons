/*
 * HttpServer.cpp
 *
 *  Created on: May 12, 2021
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>

#include <chrono>

#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>


namespace vnx {
namespace addons {

void HttpServer::do_poll(int timeout_ms) noexcept
{
	std::vector<::pollfd> fds;
	std::vector<std::shared_ptr<state_t>> states;
	fds.reserve(2 + m_state_map.size());
	states.reserve(fds.capacity());
	{
		::pollfd set = {};
		set.fd = m_notify_pipe[0];
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

	for(const auto& entry : m_state_map)
	{
		::pollfd set = {};
		const auto& state = entry.second;
		if(state->poll_bits & POLL_BIT_READ) {
			set.events |= POLLIN;
		}
		if(state->poll_bits & POLL_BIT_WRITE) {
			set.events |= POLLOUT;
		}
		if(set.events) {
			set.fd = entry.first;
			fds.push_back(set);
			states.push_back(state);
		}
	}
	if(::poll(fds.data(), fds.size(), timeout_ms) < 0) {
		log(ERROR) << "poll() failed with: " << std::strerror(errno);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(fds[0].revents & POLLIN) {
		char buf[1024];
		while(::read(m_notify_pipe[0], buf, sizeof(buf)) >= sizeof(buf));
	}
	if(fds[1].revents & POLLIN) {
		while(true) {
			const int fd = ::accept(m_socket, 0, 0);
			if(fd >= 0) {
				if(set_socket_nonblocking(fd) < 0) {
					::close(fd);
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
}


} // addons
} // vnx
