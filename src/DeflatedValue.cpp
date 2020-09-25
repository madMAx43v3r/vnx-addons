/*
 * DeflatedValue.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/addons/DeflatedValue.hxx>
#include <vnx/addons/DeflateInputStream.h>
#include <vnx/addons/DeflateOutputStream.h>

#include <vnx/Input.hpp>
#include <vnx/Output.hpp>


namespace vnx {
namespace addons {

std::shared_ptr<const Value> DeflatedValue::decompress() const
{
	DeflateInputStream stream(&data);
	TypeInput in(&stream);
	return vnx::read(in);
}

std::shared_ptr<const DeflatedValue> DeflatedValue::compress(std::shared_ptr<const Value> value)
{
	return compress_ex(value, 6);
}

std::shared_ptr<const DeflatedValue> DeflatedValue::compress_ex(std::shared_ptr<const Value> value, const int32_t& level)
{
	if(!value) {
		return nullptr;
	}
	auto out = DeflatedValue::create();
	{
		DeflateOutputStream stream(&out->data, level);
		TypeOutput out(&stream);
		vnx::write(out, value);
		out.flush();
		stream.flush();
	}
	return out;
}


} // addons
} // vnx
