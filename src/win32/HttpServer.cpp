/*
 * HttpServer.cpp
 *
 *  Created on: May 19, 2021
 *      Author: jaw
 */

#include <vnx/addons/HttpServer.h>

#include <winsock2.h>


namespace vnx {
namespace addons {


int HttpServer::set_socket_nonblocking(int fd){
	u_long mode = 1;
	const auto res = ioctlsocket(fd, FIONBIO, &mode);
	if(res != 0){
		log(WARN) << "ioctlsocket() failed with: " << WSAGetLastError();
	}
	return res;
}

std::string HttpServer::get_socket_error_text()
{
	return std::to_string(WSAGetLastError());
}


} // addons
} // vnx
