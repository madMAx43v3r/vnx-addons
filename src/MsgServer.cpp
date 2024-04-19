/*
 * MsgServer.cpp
 *
 *  Created on: Jan 22, 2022
 *      Author: mad
 */

#include <vnx/addons/MsgServer.h>
#include <vnx/addons/zstd_utils.h>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {

static const uint32_t HEADER_SIZE = 8;
static const uint32_t MSG_TYPE_ZSTD = 0x4B277B06;
static const uint32_t MSG_TYPE_ZSTD_ALT = 0x067B274B;

MsgServer::peer_t::peer_t()
	:	in_stream(nullptr), out_stream(&data), in(&in_stream), out(&out_stream)
{
	in.safe_read = true;
	zstd_in = ZSTD_createDCtx();
}

MsgServer::peer_t::~peer_t()
{
	if(zstd_out) {
		ZSTD_freeCCtx(zstd_out);
	}
	ZSTD_freeDCtx(zstd_in);
}

MsgServer::MsgServer(const std::string& _vnx_name)
	:	MsgServerBase(_vnx_name)
{
}

bool MsgServer::send_to(std::shared_ptr<peer_t> peer, std::shared_ptr<const vnx::Value> msg)
{
	if(peer->write_queue_size >= max_write_queue) {
		return false;
	}
	if(!peer->zstd_out) {
		auto ctx = ZSTD_createCCtx();
		ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compress_level);
		peer->zstd_out = ctx;
	}
	vnx::write(peer->out, msg);
	peer->out.flush();

	const auto tmp = zstd_compress(peer->zstd_out, peer->data);

	peer->data.clear();

	auto buffer = std::make_shared<vnx::Buffer>();
	buffer->resize(HEADER_SIZE + tmp.size());

	const uint32_t msg_size = tmp.size();
	::memcpy(buffer->data(0), &msg_size, 4);
	::memcpy(buffer->data(4), &MSG_TYPE_ZSTD, 4);
	::memcpy(buffer->data(HEADER_SIZE), tmp.data(), tmp.size());

	peer->bytes_send += buffer->size();
	peer->write_queue_size += buffer->size();
	return Super::send_to(peer->client, buffer);
}

void MsgServer::on_buffer(uint64_t client, void*& buffer, size_t& max_bytes)
{
	const auto peer = get_peer_base(client);
	if(!peer) {
		throw std::logic_error("!peer");
	}
	if(peer->msg_size == 0) {
		peer->buffer.resize(HEADER_SIZE);
	}
	const auto offset = peer->buffer_size;
	if(offset > peer->buffer.size()) {
		throw std::logic_error("on_buffer(): offset > buffer.size()");
	}
	buffer = peer->buffer.data() + offset;
	max_bytes = peer->buffer.size() - offset;
}

void MsgServer::on_read(uint64_t client, size_t num_bytes)
{
	const auto peer = get_peer_base(client);
	if(!peer) {
		throw std::logic_error("!peer");
	}
	peer->bytes_recv += num_bytes;
	peer->buffer_size += num_bytes;

	if(peer->msg_size == 0) {
		if(peer->buffer_size >= HEADER_SIZE) {
			::memcpy(&peer->msg_size, peer->buffer.data(), 4);
			::memcpy(&peer->msg_type, peer->buffer.data() + 4, 4);

			switch(peer->msg_type) {
				case MSG_TYPE_ZSTD: break;
				case MSG_TYPE_ZSTD_ALT:
					peer->msg_size = vnx::flip_bytes(peer->msg_size);
					peer->msg_type = vnx::flip_bytes(peer->msg_type);
					break;
				default:
					throw std::logic_error("on_read(): invalid message type: 0x" + vnx::to_hex_string(peer->msg_type));
			}
			if(peer->msg_size == 0) {
				throw std::logic_error("on_read(): zero length message");
			}
			if(peer->msg_size > max_msg_size) {
				throw std::logic_error("on_read(): message too large: " + std::to_string(peer->msg_size) + " bytes");
			}
			peer->buffer.resize(HEADER_SIZE + peer->msg_size);
		}
	}
	else if(peer->buffer_size == HEADER_SIZE + peer->msg_size)
	{
		try {
			const auto tmp = zstd_decompress(peer->zstd_in, peer->buffer, HEADER_SIZE);

			peer->in_stream.reset(&tmp);
			peer->in.reset();

			const auto value = vnx::read(peer->in);
			try {
				if(value) {
					on_msg(client, value);
				}
			}
			catch(const std::exception& ex) {
				if(show_warnings) {
					log(WARN) << "on_msg() failed with: " << ex.what();
				}
			}
		} catch(const std::exception& ex) {
			if(show_warnings) {
				log(WARN) << ex.what();
			}
		}
		peer->buffer.clear();
		peer->buffer_size = 0;
		peer->msg_size = 0;
		peer->msg_type = 0;
	}
}

void MsgServer::on_write(uint64_t client, size_t num_bytes)
{
	if(auto peer = get_peer_base(client)) {
		peer->write_queue_size -= std::min(num_bytes, peer->write_queue_size);
	}
}


} // addons
} // vnx
