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

std::shared_ptr<const HttpResponse> HttpResponse::from_text(const std::string& content)
{
	return from_text_ex(content, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_text_ex(const std::string& content, const int32_t& status)
{
	return from_string_ex(content, "text/plain", status);
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
	result->data = content;
	if(status >= 400) {
		result->error_text = content;
	}
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_string_json(const std::string& content)
{
	return from_string_json_ex(content, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_string_json_ex(const std::string& content, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = "application/json";
	result->data = content;
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_variant_json(const vnx::Variant& value)
{
	return from_variant_json_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_variant_json_ex(const vnx::Variant& value, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = "application/json";
	result->data = vnx::to_string(value);
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_object_json(const vnx::Object& value)
{
	return from_object_json_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_object_json_ex(const vnx::Object& value, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = "application/json";
	result->data = vnx::to_string(value);
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_json(std::shared_ptr<const vnx::Value> value)
{
	return from_value_json_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_json_ex(std::shared_ptr<const vnx::Value> value, const int32_t& status)
{
	auto result = HttpResponse::create();
	result->status = status;
	result->content_type = "application/json";
	result->data = vnx::to_string(value);
	return result;
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_binary(std::shared_ptr<const vnx::Value> value)
{
	return from_value_binary_ex(value, 200);
}

std::shared_ptr<const HttpResponse> HttpResponse::from_value_binary_ex(std::shared_ptr<const vnx::Value> value, const int32_t& status)
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
		result->data = tmp;
	}
	return result;
}


} // addons
} // vnx

