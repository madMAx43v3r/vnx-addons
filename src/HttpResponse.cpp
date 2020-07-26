/*
 * HttpResponse.cpp
 *
 *  Created on: Jul 9, 2020
 *      Author: mad
 */

#include <vnx/addons/HttpResponse.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

std::shared_ptr<const HttpResponse> HttpResponse::from_status(const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_string(const std::string& content, const std::string& mime_type)
{
	return from_string_ex(content, mime_type, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_string_ex(const std::string& content, const std::string& mime_type, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = mime_type;
	result->payload = content;
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_json(const std::shared_ptr<const vnx::Value>& value)
{
	return from_value_json_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_json_ex(const std::shared_ptr<const vnx::Value>& value, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = "application/json";
	result->payload = value->to_string();
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_binary(const std::shared_ptr<const vnx::Value>& value)
{
	return from_value_binary_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_binary_ex(const std::shared_ptr<const vnx::Value>& value, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = 200;
	result->content_type = "application/octet-stream";
	{
		Memory tmp;
		MemoryOutputStream stream(&tmp);
		TypeOutput out(&stream);
		vnx::write(out, value);
		out.flush();
		result->payload = tmp;
	}
	return result;
}


} // addons
} // vnx

