/*
 * HttpBalancer.h
 *
 *  Created on: Jun 30, 2024
 *      Author: mad
 */

#ifndef VNX_ADDONS_HTTPBALANCER_H_
#define VNX_ADDONS_HTTPBALANCER_H_

#include <vnx/addons/HttpBalancerBase.hxx>
#include <vnx/addons/HttpComponentAsyncClient.hxx>


namespace vnx {
namespace addons {

class HttpBalancer : public HttpBalancerBase {
public:
	HttpBalancer(const std::string& _vnx_name);

protected:
	void init() override;

	void main() override;

	void http_request_async(
			std::shared_ptr<const HttpRequest> request, const std::string& sub_path,
			const vnx::request_id_t& _request_id) const override;

	void http_request_chunk_async(
			std::shared_ptr<const HttpRequest> request, const std::string& sub_path,
			const int64_t& offset, const int64_t& max_bytes, const vnx::request_id_t& _request_id) const override
	{
		throw std::logic_error("not supported");
	}

private:
	std::vector<std::shared_ptr<HttpComponentAsyncClient>> clients;

};


} // addons
} // vnx

#endif /* VNX_ADDONS_HTTPBALANCER_H_ */
