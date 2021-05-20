/*
 * HttpRequest.cpp
 *
 *  Created on: Jul 9, 2020
 *      Author: mad
 */

#include <vnx/addons/HttpRequest.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

std::string HttpRequest::get_header_value(const std::string& key) const
{
	const auto key_ = ascii_tolower(key);
	for(const auto& entry : headers) {
		if(entry.first == key_) {
			return entry.second;
		}
	}
	return std::string();
}


} // addons
} // vnx
