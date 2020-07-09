/*
 * HttpServer.h
 *
 *  Created on: Jul 8, 2020
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_HTTPSERVER_H_
#define INCLUDE_VNX_ADDONS_HTTPSERVER_H_

#include <vnx/addons/HttpServerBase.hxx>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/addons/HttpResponse.hxx>
#include <vnx/addons/HttpComponentAsyncClient.hxx>

#include <microhttpd.h>

#include <atomic>

#if MHD_VERSION < 0x00097002
typedef int MHD_Result;
#endif


namespace vnx {
namespace addons {

class HttpServer : public HttpServerBase {
public:
	HttpServer(const std::string& _vnx_name);

protected:
	struct request_state_t {
		MHD_Connection* connection = nullptr;
		std::shared_ptr<HttpRequest> request;
		std::shared_ptr<const HttpResponse> response;
	};

	void main() override;

	void http_request_async(const std::shared_ptr<const HttpRequest>& request,
							const std::string& sub_path,
							const vnx::request_id_t& request_id) const override;

private:
	void process(request_state_t* state);

	void reply(	request_state_t* state,
				std::shared_ptr<const HttpResponse> result);

	void reply_error(	request_state_t* state,
						const std::exception& ex);

	static MHD_Result
	access_handler_callback(void* cls,
							MHD_Connection* connection,
							const char* url,
							const char* method,
							const char* version,
							const char* upload_data,
							size_t* upload_data_size,
							void** con_cls);

	static MHD_Result
	http_header_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value);

	static MHD_Result
	query_params_callback(void* cls, MHD_ValueKind kind, const char* key, const char* value);

	static void
	request_completed_callback(	void* cls,
								MHD_Connection* connection,
								void** con_cls,
								MHD_RequestTerminationCode term_code);

private:
	MHD_Daemon* m_daemon = nullptr;
	std::atomic<uint64_t> m_next_id {1};

	std::map<std::string, std::shared_ptr<HttpComponentAsyncClient>> m_client_map;

};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_HTTPSERVER_H_ */
