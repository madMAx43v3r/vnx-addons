/*
 * DeflateInputStream.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/addons/DeflateInputStream.h>

#include <limits>


namespace vnx {
namespace addons {

DeflateInputStream::DeflateInputStream(const Buffer* buffer_)
	:	buffer(buffer_)
{
	if(buffer->size() > std::numeric_limits<uint32_t>::max()) {
		throw std::runtime_error("DeflateInputStream: buffer->size() > UINT_MAX");
	}
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if(::inflateInit(&strm) != Z_OK) {
		throw std::runtime_error("inflateInit() failed!");
	}
	strm.avail_in = buffer->size();
	strm.next_in = (::Bytef*)buffer->data();
}

DeflateInputStream::~DeflateInputStream()
{
	::inflateEnd(&strm);
}

size_t DeflateInputStream::read(void* buf, size_t len)
{
	if(len > std::numeric_limits<uint32_t>::max()) {
		throw std::runtime_error("DeflateInputStream: len > UINT_MAX");
	}
	strm.avail_out = len;
	strm.next_out = (::Bytef*)buf;
	::inflate(&strm, Z_FINISH);
	return len - strm.avail_out;
}


} // addons
} // vnx
