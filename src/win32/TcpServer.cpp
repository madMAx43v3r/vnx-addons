/*
 * TcpServer.cpp
 *
 *  Created on: Dec 18, 2021
 *      Author: mad
 */

#include <vnx/addons/TcpServer.h>

#include <winsock2.h>


namespace vnx {
namespace addons {


int TcpServer::set_socket_nonblocking(int fd){
	u_long mode = 1;
	const auto res = ioctlsocket(fd, FIONBIO, &mode);
	if(res != 0){
		log(WARN) << "ioctlsocket() failed with: " << WSAGetLastError();
	}
	return res;
}

std::string TcpServer::get_socket_error_text()
{
	return std::to_string(WSAGetLastError());
}


} // addons
} // vnx
