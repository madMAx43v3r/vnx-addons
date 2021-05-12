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
		const auto ret = ::poll(fds.data(), fds.size(), poll_timeout_ms);

		if(ret < 0) {
			log(ERROR) << "poll() failed with: " << errno;
			break;
		}
		if(ret > 0) {
			if(fds[1].revents & POLLIN) {
				while(true) {
					const int fd = ::accept(m_socket, 0, 0);
					if(fd >= 0) {
						::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
						add_task(std::bind(&HttpServer::on_connect, this, fd));
					} else {
						break;
					}
				}
			}
			for(size_t i = 2; i < fds.size(); ++i)
			{
				if(fds[i].revents & POLLIN) {
					add_task(std::bind(&HttpServer::on_read, this, fds[i].fd));
				}
				if(fds[i].revents & POLLOUT) {
					add_task(std::bind(&HttpServer::on_write, this, fds[i].fd));
				}
			}
		}
	}
}


} // addons
} // vnx
