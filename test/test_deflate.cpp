/*
 * test_deflate.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/vnx.h>
#include <vnx/LogMsg.hxx>
#include <vnx/addons/DeflatedValue.hxx>

using namespace vnx;


int main()
{
	{
		auto value = LogMsg::create();
		value->level = 1337;
		value->message.resize(1 * 1024 * 1024 + 1337);
		
		auto deflated = addons::DeflatedValue::compress(value);
		std::cout << "deflated size = " << deflated->data.size() << " bytes" << std::endl;
		std::cout << vnx::to_string(deflated->data) << std::endl;
		
		auto value2 = std::dynamic_pointer_cast<const LogMsg>(deflated->decompress());
		if(value2->level != value->level) {
			throw std::logic_error("value2->level != value->level");
		}
		if(value2->message.size() != value->message.size()) {
			throw std::logic_error("value2->message.size() != value->message.size()");
		}
	}
}

