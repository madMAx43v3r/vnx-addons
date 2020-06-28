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

std::shared_ptr<const DeflatedValue> DeflatedValue::compress(const std::shared_ptr<const Value>& value)
{
	auto out = DeflatedValue::create();
	{
		DeflateOutputStream stream(&out->data);
		TypeOutput out(&stream);
		vnx::write(out, value);
		out.flush();
		stream.flush();
	}
	return out;
}


} // addons
} // vnx
