/*
 * HttpBalancer.cpp
 *
 *  Created on: Jun 30, 2024
 *      Author: mad
 */

#include <vnx/addons/HttpBalancer.h>


namespace vnx {
namespace addons {

HttpBalancer::HttpBalancer(const std::string& _vnx_name)
	:	HttpBalancerBase(_vnx_name)
{
}

void HttpBalancer::init()
{
	vnx::open_pipe(vnx_name, this, 1000);
	vnx::open_pipe(vnx_get_id(), this, 1000);
}

void HttpBalancer::main()
{
	for(const auto& addr : backend) {
		const auto client = std::make_shared<HttpComponentAsyncClient>(addr);
		client->vnx_set_non_blocking(non_blocking);
		add_async_client(client);
		clients.push_back(client);
	}
	if(clients.empty()) {
		log(WARN) << "Got no backend servers!";
	} else {
		log(INFO) << "Got " << clients.size() << " backend servers";
	}
	Super::main();

	clients.clear();
}

void HttpBalancer::http_request_async(
		std::shared_ptr<const HttpRequest> request, const std::string& sub_path, const vnx::request_id_t& request_id) const
{
	if(clients.empty()) {
		throw std::logic_error("got no backend servers");
	}
	size_t least_pending = 0;
	std::shared_ptr<HttpComponentAsyncClient> selected;

	for(const auto& client : clients) {
		const auto pending = client->vnx_get_num_pending();
		if(!selected || pending < least_pending) {
			selected = client;
			least_pending = pending;
		}
		if(pending == 0) {
			break;
		}
	}
	selected->http_request(request, sub_path,
		[this, request_id](std::shared_ptr<const HttpResponse> response) {
			http_request_async_return(request_id, response);
		},
		std::bind(&HttpBalancer::vnx_async_return_ex, this, request_id, std::placeholders::_1));
}


} // addons
} // vnx
