/*
 * CompressedValue.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/addons/CompressedValue.hxx>


namespace vnx {
namespace addons {

std::shared_ptr<const Value> CompressedValue::decompress() const {
	return nullptr;
}

std::shared_ptr<const Value> CompressedValue::vnx_decompress() const {
	return decompress();
}


} // addons
} // vnx
