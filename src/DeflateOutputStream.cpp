/*
 * DeflateOutputStream.cpp
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#include <vnx/addons/DeflateOutputStream.h>

#include <limits>

namespace vnx {
namespace addons {

static constexpr size_t DEFAULT_CHUNK_SIZE = 262144;

DeflateOutputStream::DeflateOutputStream(Buffer* buffer, int level, int chunk_size)
	:	buffer(buffer),
		chunk_size(chunk_size > 0 ? std::max(chunk_size, 1024) : DEFAULT_CHUNK_SIZE)
{
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if(::deflateInit(&strm, level) != Z_OK) {
		throw std::runtime_error("DeflateOutputStream: deflateInit() failed!");
	}
}

DeflateOutputStream::~DeflateOutputStream()
{
	::deflateEnd(&strm);
}

void DeflateOutputStream::set_output(Buffer* buffer_)
{
	buffer = buffer_;
}

void DeflateOutputStream::write(const void* buf, size_t len)
{
	if(is_finished) {
		throw std::logic_error("DeflateOutputStream::write(): already finished stream");
	}
	if(len > std::numeric_limits<uint32_t>::max()) {
		throw std::runtime_error("DeflateOutputStream::write(): len > UINT_MAX");
	}
	strm.avail_in = len;
	strm.next_in = (::Bytef*)buf;
	while(strm.avail_in) {
		if(compress_loop(Z_NO_FLUSH) != Z_OK) {
			if(strm.avail_in) {
				throw std::runtime_error("DeflateOutputStream::write(): output error");
			}
		}
	}
}

void DeflateOutputStream::flush()
{
	bool done = false;
	while(!done) {
		switch(compress_loop(Z_SYNC_FLUSH)) {
			case Z_OK: continue;
			case Z_STREAM_END:
				done = true;
				break;
			case Z_BUF_ERROR:
				if(strm.avail_in == 0) {
					done = true;
					break;
				}
				/* no break */
			default:
				throw std::runtime_error("DeflateOutputStream::flush(): output error");
		}
	}
	strm.avail_out = 0;
	strm.next_out = 0;
}

void DeflateOutputStream::finish()
{
	bool done = false;
	while(!done) {
		switch(compress_loop(Z_FINISH)) {
			case Z_OK: continue;
			case Z_STREAM_END:
				done = true;
				break;
			default:
				throw std::runtime_error("DeflateOutputStream::finish(): output error");
		}
	}
	strm.avail_out = 0;
	strm.next_out = 0;
	is_finished = true;
}

int DeflateOutputStream::compress_loop(int flag)
{
	const auto offset = buffer->size();
	if(buffer->capacity() < offset + chunk_size) {
		buffer->reserve(std::max(offset + chunk_size, buffer->capacity() * 2));
	}
	const auto avail = buffer->capacity() - offset;
	strm.avail_out = avail;
	strm.next_out = (::Bytef*)buffer->data(offset);
	const auto ret = ::deflate(&strm, flag);
	buffer->resize(offset + (avail - strm.avail_out));
	return ret;
}


} // addons
} // vnx
