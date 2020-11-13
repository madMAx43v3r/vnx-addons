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
		value->message.resize(16 * 1024 * 1024 + 1337);
		
		for(auto& v : value->message) {
			if(::rand() % 8 == 0) {
				v = ::rand();
			}
		}

		const auto time_begin = vnx::get_wall_time_micros();
		auto deflated = addons::DeflatedValue::compress(value);
		std::cout << "took " << (vnx::get_wall_time_micros() - time_begin) << " usec" << std::endl;
		std::cout << "deflated size = " << deflated->data.size() << " bytes" << std::endl;
//		std::cout << vnx::to_string(deflated->data) << std::endl;
		
		auto value2 = std::dynamic_pointer_cast<const LogMsg>(deflated->decompress());
		if(value2->level != value->level) {
			throw std::logic_error("value2->level != value->level");
		}
		if(value2->message != value->message) {
			throw std::logic_error("value2->message != value->message");
		}
	}
}

