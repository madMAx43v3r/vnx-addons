/*
 * HttpServer.cpp
 *
 *  Created on: May 19, 2021
 *      Author: jaw
 */

#include <vnx/addons/HttpServer.h>

#include <winsock2.h>
#include <win32-compat/poll.h>



int poll(pollfd *fds, size_t num, int timeout){
	fd_set read, write, except;
	FD_ZERO(&read);
	FD_ZERO(&write);
	FD_ZERO(&except);

	int max = -1;
	for (size_t i=0; i<num; i++){
		if(fds[i].fd < 0) continue;
		if(fds[i].events & POLLIN)  FD_SET(fds[i].fd, &read);
		if(fds[i].events & POLLOUT) FD_SET(fds[i].fd, &write);
		if(fds[i].events & POLLERR) FD_SET(fds[i].fd, &except);

		if(fds[i].fd > max) max = fds[i].fd;
	}
	if(max == -1) return 0;

	int ret;
	if(timeout < 0){
		ret = select(max+1, &read, &write, &except, NULL);
	}else{
		timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = 1000 * (timeout % 1000);
		ret = select(max+1, &read, &write, &except, &tv);
	}

	if(ret >= 0){
		for(size_t i=0; i<num; i++){
			fds[i].revents = 0;
			if(FD_ISSET(fds[i].fd, &read))   fds[i].revents |= POLLIN;
			if(FD_ISSET(fds[i].fd, &write))  fds[i].revents |= POLLOUT;
			if(FD_ISSET(fds[i].fd, &except)) fds[i].revents |= POLLERR;
		}
	}

	return ret;
}


namespace vnx {
namespace addons {


int HttpServer::set_socket_nonblocking(int fd){
	u_long mode = 1;
	const auto res = ioctlsocket(fd, FIONBIO, &mode);
	if(res != 0 && show_warnings){
		log(WARN) << "ioctlsocket() failed with: " << WSAGetLastError();
	}
	return res;
}


} // addons
} // vnx
