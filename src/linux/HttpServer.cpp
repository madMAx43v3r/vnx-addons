/*
 * HttpServer.cpp
 *
 *  Created on: May 12, 2021
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>

#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>


namespace vnx {
namespace addons {

void HttpServer::poll_loop() noexcept
{
	while(vnx_do_run())
	{
		std::vector<::pollfd> fds;
		{
			std::lock_guard lock(m_poll_mutex);

			fds.reserve(2 + m_poll_map.size());
			{
				::pollfd set = {};
				set.fd = m_signal_pipe[0];
				set.events = POLLIN;
				fds.push_back(set);
			}
			{
				::pollfd set = {};
				set.fd = m_socket;
				set.events = POLLIN;
				fds.push_back(set);
			}
			for(const auto& entry : m_poll_map)
			{
				::pollfd set = {};
				set.fd = entry.first;
				if(entry.second & POLL_READ) {
					set.events |= POLLIN;
				}
				if(entry.second & POLL_WRITE) {
					set.events |= POLLOUT;
				}
				if(set.events) {
					fds.push_back(set);
				}
			}
		}
		if(::poll(fds.data(), fds.size(), poll_timeout_ms) < 0) {
			log(ERROR) << "poll() failed with: " << std::strerror(errno);
			break;
		}
		std::lock_guard lock(m_poll_mutex);

		if(fds[1].revents & POLLIN) {
			while(true) {
				const int fd = ::accept(m_socket, 0, 0);
				if(fd >= 0) {
					m_poll_map[fd] = POLL_READ;
					::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);		// set O_NONBLOCK
					add_task(std::bind(&HttpServer::on_connect, this, fd));
				} else {
					break;
				}
			}
		}
		for(size_t i = 2; i < fds.size(); ++i)
		{
			const auto& set = fds[i];
			if(set.revents & POLLIN) {
				{
					// reset poll bit first
					auto iter = m_poll_map.find(set.fd);
					if(iter != m_poll_map.end()) {
						iter->second &= ~POLL_READ;
					}
				}
				add_task(std::bind(&HttpServer::on_read, this, set.fd));
			}
			if(set.revents & POLLOUT) {
				{
					// reset poll bit first
					auto iter = m_poll_map.find(set.fd);
					if(iter != m_poll_map.end()) {
						iter->second &= ~POLL_WRITE;
					}
				}
				add_task(std::bind(&HttpServer::on_write, this, set.fd));
			}
		}
	}
}


} // addons
} // vnx
