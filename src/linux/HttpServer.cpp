/*
 * HttpServer.cpp
 *
 *  Created on: May 12, 2021
 *      Author: mad
 */

#include <vnx/addons/HttpServer.h>

#include <fcntl.h>


namespace vnx {
namespace addons {


int HttpServer::set_socket_nonblocking(int fd)
{
	const auto res = ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	if(res < 0 && show_warnings) {
		log(WARN) << "fcntl() failed with: " << strerror(errno);
	}
	return res;
}

std::string HttpServer::get_socket_error_text()
{
	return std::string(std::strerror(errno)) + " (" + std::to_string(errno) + ")";
}


} // addons
} // vnx
