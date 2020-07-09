
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_vnx_addons_HttpComponent_ASYNC_CLIENT_HXX_
#define INCLUDE_vnx_addons_HttpComponent_ASYNC_CLIENT_HXX_

#include <vnx/AsyncClient.h>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/addons/HttpResponse.hxx>


namespace vnx {
namespace addons {

class HttpComponentAsyncClient : public vnx::AsyncClient {
public:
	HttpComponentAsyncClient(const std::string& service_name);
	
	HttpComponentAsyncClient(vnx::Hash64 service_addr);
	
	uint64_t http_request(const std::shared_ptr<const ::vnx::addons::HttpRequest>& request, const std::string& sub_path, 
			const std::function<void(std::shared_ptr<const ::vnx::addons::HttpResponse>)>& _callback = std::function<void(std::shared_ptr<const ::vnx::addons::HttpResponse>)>(),
			const std::function<void(const std::exception&)>& _error_callback = std::function<void(const std::exception&)>());
	
	std::vector<uint64_t> vnx_get_pending_ids() const override;
	
protected:
	void vnx_purge_request(uint64_t _request_id, const std::exception& _ex) override;
	
	void vnx_callback_switch(uint64_t _request_id, std::shared_ptr<const vnx::Value> _value) override;
	
private:
	std::map<uint64_t, std::pair<std::function<void(std::shared_ptr<const ::vnx::addons::HttpResponse>)>, std::function<void(const std::exception&)>>> vnx_queue_http_request;
	
};


} // namespace vnx
} // namespace addons

#endif // INCLUDE_vnx_addons_HttpComponent_ASYNC_CLIENT_HXX_