/*
 * test_zstd.cpp
 *
 *  Created on: Apr 20, 2024
 *      Author: mad
 */

#include <vnx/addons/zstd_utils.h>

#include <vector>
#include <cstring>
#include <iostream>


int main(int argc, char** argv)
{
	const int level = argc > 1 ? ::atoi(argv[1]) : 0;
	const uint32_t count = argc > 2 ? ::atoi(argv[2]) : 10000;

	std::cout << "level = " << level << std::endl;
	std::cout << "count = " << count << std::endl;

	auto zstd_in = ZSTD_createDCtx();
	auto zstd_out = ZSTD_createCCtx();

	if(ZSTD_isError(ZSTD_CCtx_setParameter(zstd_out, ZSTD_c_compressionLevel, level))) {
		throw std::logic_error("invalid level");
	}

	std::vector<uint8_t> data(count * 4);

	for(uint32_t i = 0; i < count; ++i) {
		::memcpy(data.data() + i * 4, &i, 4);
	}
	std::cout << "data bytes = " << data.size() << std::endl;

	for(int iter = 0; iter < 3; ++iter)
	{
		const auto cdata = vnx::addons::zstd_compress(zstd_out, data);

		std::cout << "[" << iter << "] compressed bytes = " << cdata.size() << " -> " << double(cdata.size()) / data.size() << std::endl;

		const auto res = vnx::addons::zstd_decompress(zstd_in, cdata);

		if(res != data) {
			throw std::logic_error("res != data");
		}
	}
	ZSTD_freeDCtx(zstd_in);
	ZSTD_freeCCtx(zstd_out);

	return 0;
}


