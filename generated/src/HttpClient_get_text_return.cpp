
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/HttpClient_get_text_return.hxx>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 HttpClient_get_text_return::VNX_TYPE_HASH(0x88ca50b90580467eull);
const vnx::Hash64 HttpClient_get_text_return::VNX_CODE_HASH(0x294f5a9809c9d36aull);

vnx::Hash64 HttpClient_get_text_return::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string HttpClient_get_text_return::get_type_name() const {
	return "vnx.addons.HttpClient.get_text.return";
}

const vnx::TypeCode* HttpClient_get_text_return::get_type_code() const {
	return vnx::addons::vnx_native_type_code_HttpClient_get_text_return;
}

std::shared_ptr<HttpClient_get_text_return> HttpClient_get_text_return::create() {
	return std::make_shared<HttpClient_get_text_return>();
}

std::shared_ptr<vnx::Value> HttpClient_get_text_return::clone() const {
	return std::make_shared<HttpClient_get_text_return>(*this);
}

void HttpClient_get_text_return::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void HttpClient_get_text_return::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void HttpClient_get_text_return::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_HttpClient_get_text_return;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, _ret_0);
	_visitor.type_end(*_type_code);
}

void HttpClient_get_text_return::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.HttpClient.get_text.return\"";
	_out << ", \"_ret_0\": "; vnx::write(_out, _ret_0);
	_out << "}";
}

void HttpClient_get_text_return::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object HttpClient_get_text_return::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.HttpClient.get_text.return";
	_object["_ret_0"] = _ret_0;
	return _object;
}

void HttpClient_get_text_return::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "_ret_0") {
			_entry.second.to(_ret_0);
		}
	}
}

vnx::Variant HttpClient_get_text_return::get_field(const std::string& _name) const {
	if(_name == "_ret_0") {
		return vnx::Variant(_ret_0);
	}
	return vnx::Variant();
}

void HttpClient_get_text_return::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "_ret_0") {
		_value.to(_ret_0);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const HttpClient_get_text_return& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, HttpClient_get_text_return& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* HttpClient_get_text_return::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> HttpClient_get_text_return::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.HttpClient.get_text.return";
	type_code->type_hash = vnx::Hash64(0x88ca50b90580467eull);
	type_code->code_hash = vnx::Hash64(0x294f5a9809c9d36aull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_return = true;
	type_code->native_size = sizeof(::vnx::addons::HttpClient_get_text_return);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<HttpClient_get_text_return>(); };
	type_code->fields.resize(1);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "_ret_0";
		field.code = {32};
	}
	type_code->build();
	return type_code;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::HttpClient_get_text_return& value, const TypeCode* type_code, const uint16_t* code) {
	TypeInput::recursion_t tag(in);
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
	in.read(type_code->total_field_size);
	if(type_code->is_matched) {
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value._ret_0, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::HttpClient_get_text_return& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_HttpClient_get_text_return;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::HttpClient_get_text_return>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	vnx::write(out, value._ret_0, type_code, type_code->fields[0].code.data());
}

void read(std::istream& in, ::vnx::addons::HttpClient_get_text_return& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::HttpClient_get_text_return& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::HttpClient_get_text_return& value) {
	value.accept(visitor);
}

} // vnx
