/*
 * DeflateOutputStream.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/addons/DeflateOutputStream.h>


namespace vnx {
namespace addons {

static const size_t CHUNK_SIZE = 262144;

DeflateOutputStream::DeflateOutputStream(Buffer* buffer_)
	:	buffer(buffer_)
{
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if(::deflateInit(&strm, 9) != Z_OK) {
		throw std::runtime_error("deflateInit() failed!");
	}
}

DeflateOutputStream::~DeflateOutputStream()
{
	::deflateEnd(&strm);
}

void DeflateOutputStream::write(const void* buf, size_t len)
{
	if(len > UINT_MAX) {
		throw std::runtime_error("DeflateOutputStream: len > UINT_MAX");
	}
	strm.avail_in = len;
	strm.next_in = (::Bytef*)buf;
	while(strm.avail_in) {
		compress_loop(false);
	}
}

void DeflateOutputStream::flush()
{
	while(compress_loop(true) == Z_OK);
	strm.avail_out = 0;
	strm.next_out = 0;
}

int DeflateOutputStream::compress_loop(bool flush)
{
	const auto offset = buffer->size();
	buffer->reserve(offset + CHUNK_SIZE);
	strm.avail_out = CHUNK_SIZE;
	strm.next_out = (::Bytef*)buffer->data(offset);
	const auto ret = ::deflate(&strm, flush ? Z_FINISH : Z_NO_FLUSH);
	buffer->resize(offset + (CHUNK_SIZE - strm.avail_out));
	return ret;
}


} // addons
} // vnx
