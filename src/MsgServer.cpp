/*
 * MsgServer.cpp
 *
 *  Created on: Jan 22, 2022
 *      Author: mad
 */

#include <vnx/addons/MsgServer.h>
#include <vnx/addons/DeflatedValue.hxx>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {

MsgServer::MsgServer(const std::string& _vnx_name)
	:	MsgServerBase(_vnx_name)
{
}

bool MsgServer::send_to(std::shared_ptr<peer_t> peer, std::shared_ptr<const vnx::Value> msg)
{
	if(peer->write_queue_size >= max_write_queue) {
		return false;
	}
	if(compress_level > 0) {
		if(!peer->deflate_out_stream) {
			peer->deflate_out_stream = std::make_shared<DeflateOutputStream>(nullptr, compress_level);
		}
		auto value = DeflatedValue::create();
		peer->deflate_out_stream->set_output(&value->data);

		if(!peer->deflate_out) {
			peer->deflate_out = std::make_shared<vnx::TypeOutput>(peer->deflate_out_stream.get());
		}
		vnx::write(*peer->deflate_out, msg);
		peer->deflate_out->flush();
		peer->deflate_out_stream->flush();
		msg = value;
	}
	vnx::write(peer->out, uint16_t(vnx::CODE_UINT32));
	vnx::write(peer->out, uint32_t(0));
	vnx::write(peer->out, msg);
	peer->out.flush();

	auto buffer = std::make_shared<vnx::Buffer>(peer->data);
	peer->data.clear();

	if(buffer->size() > max_msg_size) {
		return false;
	}
	*((uint32_t*)buffer->data(2)) = buffer->size() - 6;

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
	const auto offset = peer->buffer.size();
	if(peer->msg_size == 0) {
		if(offset > 6) {
			throw std::logic_error("offset > 6");
		}
		peer->buffer.reserve(6);
		max_bytes = 6 - offset;
	} else {
		max_bytes = (6 + peer->msg_size) - offset;
	}
	buffer = peer->buffer.data(offset);
}

void MsgServer::on_read(uint64_t client, size_t num_bytes)
{
	const auto peer = get_peer_base(client);
	if(!peer) {
		throw std::logic_error("!peer");
	}
	peer->in.max_list_size = max_list_size;
	peer->deflate_in.max_list_size = max_msg_size;
	peer->bytes_recv += num_bytes;
	peer->buffer.resize(peer->buffer.size() + num_bytes);

	if(peer->msg_size == 0) {
		if(peer->buffer.size() >= 6) {
			uint16_t code = 0;
			vnx::read_value(peer->buffer.data(), code);
			vnx::read_value(peer->buffer.data(2), peer->msg_size, &code);
			if(peer->msg_size > max_msg_size) {
				throw std::logic_error("message too large");
			}
			if(peer->msg_size > 0) {
				peer->buffer.reserve(6 + peer->msg_size);
			} else {
				peer->buffer.clear();
			}
		}
	}
	else if(peer->buffer.size() == 6 + peer->msg_size)
	{
		peer->in.read(6);
		if(auto value = vnx::read(peer->in)) {
			if(auto deflated = std::dynamic_pointer_cast<const DeflatedValue>(value)) {
				try {
					peer->deflate_in_stream.set_input(&deflated->data);
					peer->deflate_in.reset();
					value = vnx::read(peer->deflate_in);
				}
				catch(const std::exception& ex) {
					if(show_warnings) {
						log(WARN) << "deflate failed with: " << ex.what();
					}
				}
			}
			try {
				on_msg(client, value);
			}
			catch(const std::exception& ex) {
				if(show_warnings) {
					log(WARN) << "on_msg() failed with: " << ex.what();
				}
			}
		}
		peer->buffer.clear();
		peer->in_stream.reset();
		peer->msg_size = 0;
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
