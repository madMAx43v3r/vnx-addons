
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/HttpComponent_http_request.hxx>
#include <vnx/Value.h>
#include <vnx/addons/HttpComponent_http_request_return.hxx>
#include <vnx/addons/HttpRequest.hxx>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 HttpComponent_http_request::VNX_TYPE_HASH(0xe0b6c38f619bad92ull);
const vnx::Hash64 HttpComponent_http_request::VNX_CODE_HASH(0xfb2ef6c722ec93a1ull);

vnx::Hash64 HttpComponent_http_request::get_type_hash() const {
	return VNX_TYPE_HASH;
}

const char* HttpComponent_http_request::get_type_name() const {
	return "vnx.addons.HttpComponent.http_request";
}
const vnx::TypeCode* HttpComponent_http_request::get_type_code() const {
	return vnx::addons::vnx_native_type_code_HttpComponent_http_request;
}

std::shared_ptr<HttpComponent_http_request> HttpComponent_http_request::create() {
	return std::make_shared<HttpComponent_http_request>();
}

std::shared_ptr<vnx::Value> HttpComponent_http_request::clone() const {
	return std::make_shared<HttpComponent_http_request>(*this);
}

void HttpComponent_http_request::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void HttpComponent_http_request::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void HttpComponent_http_request::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_HttpComponent_http_request;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, request);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, sub_path);
	_visitor.type_end(*_type_code);
}

void HttpComponent_http_request::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.HttpComponent.http_request\"";
	_out << ", \"request\": "; vnx::write(_out, request);
	_out << ", \"sub_path\": "; vnx::write(_out, sub_path);
	_out << "}";
}

void HttpComponent_http_request::read(std::istream& _in) {
	std::map<std::string, std::string> _object;
	vnx::read_object(_in, _object);
	for(const auto& _entry : _object) {
		if(_entry.first == "request") {
			vnx::from_string(_entry.second, request);
		} else if(_entry.first == "sub_path") {
			vnx::from_string(_entry.second, sub_path);
		}
	}
}

vnx::Object HttpComponent_http_request::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.HttpComponent.http_request";
	_object["request"] = request;
	_object["sub_path"] = sub_path;
	return _object;
}

void HttpComponent_http_request::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "request") {
			_entry.second.to(request);
		} else if(_entry.first == "sub_path") {
			_entry.second.to(sub_path);
		}
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const HttpComponent_http_request& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, HttpComponent_http_request& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* HttpComponent_http_request::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> HttpComponent_http_request::static_create_type_code() {
	std::shared_ptr<vnx::TypeCode> type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.HttpComponent.http_request";
	type_code->type_hash = vnx::Hash64(0xe0b6c38f619bad92ull);
	type_code->code_hash = vnx::Hash64(0xfb2ef6c722ec93a1ull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_method = true;
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<HttpComponent_http_request>(); };
	type_code->return_type = ::vnx::addons::HttpComponent_http_request_return::static_get_type_code();
	type_code->fields.resize(2);
	{
		vnx::TypeField& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "request";
		field.code = {16};
	}
	{
		vnx::TypeField& field = type_code->fields[1];
		field.is_extended = true;
		field.name = "sub_path";
		field.code = {12, 5};
	}
	type_code->build();
	return type_code;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::HttpComponent_http_request& value, const TypeCode* type_code, const uint16_t* code) {
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
		throw std::logic_error("read(): type_code == 0");
	}
	if(code) {
		switch(code[0]) {
			case CODE_STRUCT: type_code = type_code->depends[code[1]]; break;
			case CODE_ALT_STRUCT: type_code = type_code->depends[vnx::flip_bytes(code[1])]; break;
			default: vnx::skip(in, type_code, code); return;
		}
	}
	if(type_code->is_matched) {
	}
	for(const vnx::TypeField* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.request, type_code, _field->code.data()); break;
			case 1: vnx::read(in, value.sub_path, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::HttpComponent_http_request& value, const TypeCode* type_code, const uint16_t* code) {
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_HttpComponent_http_request;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::HttpComponent_http_request>(out);
	}
	if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	vnx::write(out, value.request, type_code, type_code->fields[0].code.data());
	vnx::write(out, value.sub_path, type_code, type_code->fields[1].code.data());
}

void read(std::istream& in, ::vnx::addons::HttpComponent_http_request& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::HttpComponent_http_request& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::HttpComponent_http_request& value) {
	value.accept(visitor);
}

} // vnx
