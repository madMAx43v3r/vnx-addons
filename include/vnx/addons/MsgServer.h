/*
 * MsgServer.h
 *
 *  Created on: Jan 22, 2022
 *      Author: mad
 */

#ifndef INCLUDE_VNX_ADDONS_MSGSERVER_H_
#define INCLUDE_VNX_ADDONS_MSGSERVER_H_

#include <vnx/addons/MsgServerBase.hxx>

#include <vnx/Input.hpp>
#include <vnx/Output.hpp>

#include <zstd.h>


namespace vnx {
namespace addons {

class MsgServer : public MsgServerBase {
public:
	MsgServer(const std::string& _vnx_name);

protected:
	struct peer_t {
		uint32_t msg_size = 0;
		uint32_t msg_type = 0;
		uint64_t client = 0;
		uint64_t bytes_send = 0;
		uint64_t bytes_recv = 0;
		uint64_t write_queue_size = 0;

		std::vector<uint8_t> data;
		std::vector<uint8_t> buffer;
		vnx::VectorInputStream in_stream;
		vnx::VectorOutputStream out_stream;
		vnx::TypeInput in;
		vnx::TypeOutput out;

		ZSTD_DCtx* zstd_in = nullptr;
		ZSTD_CCtx* zstd_out = nullptr;

		peer_t();
		~peer_t();
	};

	bool send_to(std::shared_ptr<peer_t> peer, std::shared_ptr<const vnx::Value> msg);

	virtual std::shared_ptr<peer_t> get_peer_base(uint64_t client) const = 0;

	virtual void on_msg(uint64_t client, std::shared_ptr<const vnx::Value> msg) = 0;

private:
	void on_buffer(uint64_t client, void*& buffer, size_t& max_bytes) override;

	void on_read(uint64_t client, size_t num_bytes) override;

	void on_write(uint64_t client, size_t num_bytes) override;

};


} // addons
} // vnx

#endif /* INCLUDE_VNX_ADDONS_MSGSERVER_H_ */
