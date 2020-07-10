/*
 * HttpServer.h
 *
 *  Created on: Jul 8, 2020
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_HTTPSERVER_H_
#define INCLUDE_VNX_ADDONS_HTTPSERVER_H_

#include <vnx/addons/HttpServerBase.hxx>


namespace vnx {
namespace addons {

HttpServerBase* new_HttpServer(const std::string& name);


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_HTTPSERVER_H_ */
