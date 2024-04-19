/*
 * zstd_utils.h
 *
 *  Created on: Apr 19, 2024
 *      Author: mad
 */

#ifndef VNX_ADDONS_INCLUDE_VNX_ADDONS_ZSTD_UTILS_H_
#define VNX_ADDONS_INCLUDE_VNX_ADDONS_ZSTD_UTILS_H_

#include <vector>
#include <cstdint>
#include <stdexcept>

#include <zstd.h>


namespace vnx {
namespace addons {

inline
std::vector<uint8_t> zstd_compress(ZSTD_CCtx* cctx, const std::vector<uint8_t>& in)
{
	static const auto CHUNK_SIZE = ZSTD_CStreamOutSize();

	size_t offset = 0;
	std::vector<uint8_t> out(CHUNK_SIZE);

	ZSTD_inBuffer input = {in.data(), in.size(), 0};
	while(true) {
		out.resize(offset + CHUNK_SIZE);

		ZSTD_outBuffer output = {out.data(), out.size(), offset};

		const size_t remaining = ZSTD_compressStream2(cctx, &output , &input, ZSTD_e_end);

		offset = output.pos;

		if(remaining == 0) {
			break;
		}
	}
	out.resize(offset);
	return out;
}

inline
std::vector<uint8_t> zstd_decompress(ZSTD_DCtx* dctx, const std::vector<uint8_t>& in, const size_t in_offset = 0)
{
	static const auto CHUNK_SIZE = ZSTD_DStreamOutSize();

	size_t offset = 0;
	std::vector<uint8_t> out(CHUNK_SIZE);

	ZSTD_inBuffer input = {in.data(), in.size(), in_offset};
	while (input.pos < input.size)
	{
		out.resize(offset + CHUNK_SIZE);

		ZSTD_outBuffer output = {out.data(), out.size(), offset};

		const size_t ret = ZSTD_decompressStream(dctx, &output , &input);

		if(ZSTD_isError(ret)) {
			throw std::runtime_error("ZSTD_decompressStream() failed with: " + std::string(ZSTD_getErrorName(ret)));
		}
		offset = output.pos;
	}
	out.resize(offset);
	return out;
}







} // addons
} // vnx

#endif /* VNX_ADDONS_INCLUDE_VNX_ADDONS_ZSTD_UTILS_H_ */
