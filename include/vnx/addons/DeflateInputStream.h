/*
 * DeflateInputStream.h
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_DEFLATEINPUTSTREAM_H_
#define INCLUDE_VNX_ADDONS_DEFLATEINPUTSTREAM_H_

#include <vnx/InputStream.h>
#include <vnx/Buffer.h>

#include <zlib.h>


namespace vnx {
namespace addons {

class DeflateInputStream : public InputStream {
public:
	DeflateInputStream(const Buffer* buffer_);
	
	~DeflateInputStream();
	
	size_t read(void* buf, size_t len) override;
	
	int64_t get_input_pos() const override {
		return buffer->size() - strm.avail_in;
	}
	
private:
	const Buffer* buffer = 0;
	z_stream strm = {};
	
};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_DEFLATEINPUTSTREAM_H_ */
