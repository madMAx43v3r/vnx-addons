
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/HttpSession.hxx>
#include <vnx/Hash64.hpp>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 HttpSession::VNX_TYPE_HASH(0xaf1b568d83351450ull);
const vnx::Hash64 HttpSession::VNX_CODE_HASH(0x16912460af18199cull);

vnx::Hash64 HttpSession::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string HttpSession::get_type_name() const {
	return "vnx.addons.HttpSession";
}

const vnx::TypeCode* HttpSession::get_type_code() const {
	return vnx::addons::vnx_native_type_code_HttpSession;
}

std::shared_ptr<HttpSession> HttpSession::create() {
	return std::make_shared<HttpSession>();
}

std::shared_ptr<vnx::Value> HttpSession::clone() const {
	return std::make_shared<HttpSession>(*this);
}

void HttpSession::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void HttpSession::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void HttpSession::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_HttpSession;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, user);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, hsid);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, vsid);
	_visitor.type_field(_type_code->fields[3], 3); vnx::accept(_visitor, login_time);
	_visitor.type_field(_type_code->fields[4], 4); vnx::accept(_visitor, session_timeout);
	_visitor.type_end(*_type_code);
}

void HttpSession::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.HttpSession\"";
	_out << ", \"user\": "; vnx::write(_out, user);
	_out << ", \"hsid\": "; vnx::write(_out, hsid);
	_out << ", \"vsid\": "; vnx::write(_out, vsid);
	_out << ", \"login_time\": "; vnx::write(_out, login_time);
	_out << ", \"session_timeout\": "; vnx::write(_out, session_timeout);
	_out << "}";
}

void HttpSession::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object HttpSession::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.HttpSession";
	_object["user"] = user;
	_object["hsid"] = hsid;
	_object["vsid"] = vsid;
	_object["login_time"] = login_time;
	_object["session_timeout"] = session_timeout;
	return _object;
}

void HttpSession::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "hsid") {
			_entry.second.to(hsid);
		} else if(_entry.first == "login_time") {
			_entry.second.to(login_time);
		} else if(_entry.first == "session_timeout") {
			_entry.second.to(session_timeout);
		} else if(_entry.first == "user") {
			_entry.second.to(user);
		} else if(_entry.first == "vsid") {
			_entry.second.to(vsid);
		}
	}
}

vnx::Variant HttpSession::get_field(const std::string& _name) const {
	if(_name == "user") {
		return vnx::Variant(user);
	}
	if(_name == "hsid") {
		return vnx::Variant(hsid);
	}
	if(_name == "vsid") {
		return vnx::Variant(vsid);
	}
	if(_name == "login_time") {
		return vnx::Variant(login_time);
	}
	if(_name == "session_timeout") {
		return vnx::Variant(session_timeout);
	}
	return vnx::Variant();
}

void HttpSession::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "user") {
		_value.to(user);
	} else if(_name == "hsid") {
		_value.to(hsid);
	} else if(_name == "vsid") {
		_value.to(vsid);
	} else if(_name == "login_time") {
		_value.to(login_time);
	} else if(_name == "session_timeout") {
		_value.to(session_timeout);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const HttpSession& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, HttpSession& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* HttpSession::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> HttpSession::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.HttpSession";
	type_code->type_hash = vnx::Hash64(0xaf1b568d83351450ull);
	type_code->code_hash = vnx::Hash64(0x16912460af18199cull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->native_size = sizeof(::vnx::addons::HttpSession);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<HttpSession>(); };
	type_code->fields.resize(5);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "user";
		field.code = {32};
	}
	{
		auto& field = type_code->fields[1];
		field.is_extended = true;
		field.name = "hsid";
		field.code = {32};
	}
	{
		auto& field = type_code->fields[2];
		field.is_extended = true;
		field.name = "vsid";
		field.code = {4};
	}
	{
		auto& field = type_code->fields[3];
		field.data_size = 8;
		field.name = "login_time";
		field.code = {8};
	}
	{
		auto& field = type_code->fields[4];
		field.data_size = 8;
		field.name = "session_timeout";
		field.code = {8};
	}
	type_code->build();
	return type_code;
}

std::shared_ptr<vnx::Value> HttpSession::vnx_call_switch(std::shared_ptr<const vnx::Value> _method) {
	switch(_method->get_type_hash()) {
	}
	return nullptr;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::HttpSession& value, const TypeCode* type_code, const uint16_t* code) {
	if(code) {
		switch(code[0]) {
			case CODE_OBJECT:
			case CODE_ALT_OBJECT: {
				Object tmp;
				vnx::read(in, tmp, type_code, code);
				value.from_object(tmp);
				return;
			}
			case CODE_DYNAMIC:
			case CODE_ALT_DYNAMIC:
				vnx::read_dynamic(in, value);
				return;
		}
	}
	if(!type_code) {
		vnx::skip(in, type_code, code);
		return;
	}
	if(code) {
		switch(code[0]) {
			case CODE_STRUCT: type_code = type_code->depends[code[1]]; break;
			case CODE_ALT_STRUCT: type_code = type_code->depends[vnx::flip_bytes(code[1])]; break;
			default: {
				vnx::skip(in, type_code, code);
				return;
			}
		}
	}
	const auto* const _buf = in.read(type_code->total_field_size);
	if(type_code->is_matched) {
		if(const auto* const _field = type_code->field_map[3]) {
			vnx::read_value(_buf + _field->offset, value.login_time, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[4]) {
			vnx::read_value(_buf + _field->offset, value.session_timeout, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.user, type_code, _field->code.data()); break;
			case 1: vnx::read(in, value.hsid, type_code, _field->code.data()); break;
			case 2: vnx::read(in, value.vsid, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::HttpSession& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_HttpSession;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::HttpSession>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	auto* const _buf = out.write(16);
	vnx::write_value(_buf + 0, value.login_time);
	vnx::write_value(_buf + 8, value.session_timeout);
	vnx::write(out, value.user, type_code, type_code->fields[0].code.data());
	vnx::write(out, value.hsid, type_code, type_code->fields[1].code.data());
	vnx::write(out, value.vsid, type_code, type_code->fields[2].code.data());
}

void read(std::istream& in, ::vnx::addons::HttpSession& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::HttpSession& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::HttpSession& value) {
	value.accept(visitor);
}

} // vnx
