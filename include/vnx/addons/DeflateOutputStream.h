/*
 * DeflateOutputStream.h
 *
 *  Created on: Jun 28, 2020
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_DEFLATEOUTPUTSTREAM_H_
#define INCLUDE_VNX_ADDONS_DEFLATEOUTPUTSTREAM_H_

#include <vnx/OutputStream.h>
#include <vnx/Buffer.h>

#include <zlib.h>


namespace vnx {
namespace addons {

class DeflateOutputStream : public OutputStream {
public:
	DeflateOutputStream(Buffer* buffer_, int level = 9);
	
	~DeflateOutputStream();
	
	void write(const void* buf, size_t len) override;
	
	void flush();
	
private:
	int compress_loop(bool flush);
	
private:
	Buffer* buffer = 0;
	z_stream strm = {};
	
};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_DEFLATEOUTPUTSTREAM_H_ */
