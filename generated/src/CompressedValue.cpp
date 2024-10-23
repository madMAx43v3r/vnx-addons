
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/CompressedValue.hxx>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 CompressedValue::VNX_TYPE_HASH(0x50c46e244bd9765dull);
const vnx::Hash64 CompressedValue::VNX_CODE_HASH(0x83f41ad21fe492c6ull);

vnx::Hash64 CompressedValue::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string CompressedValue::get_type_name() const {
	return "vnx.addons.CompressedValue";
}

const vnx::TypeCode* CompressedValue::get_type_code() const {
	return vnx::addons::vnx_native_type_code_CompressedValue;
}

std::shared_ptr<CompressedValue> CompressedValue::create() {
	return std::make_shared<CompressedValue>();
}

std::shared_ptr<vnx::Value> CompressedValue::clone() const {
	return std::make_shared<CompressedValue>(*this);
}

void CompressedValue::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void CompressedValue::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void CompressedValue::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_CompressedValue;
	_visitor.type_begin(*_type_code);
	_visitor.type_end(*_type_code);
}

void CompressedValue::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.CompressedValue\"";
	_out << "}";
}

void CompressedValue::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object CompressedValue::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.CompressedValue";
	return _object;
}

void CompressedValue::from_object(const vnx::Object& _object) {
}

vnx::Variant CompressedValue::get_field(const std::string& _name) const {
	return vnx::Variant();
}

void CompressedValue::set_field(const std::string& _name, const vnx::Variant& _value) {
}

/// \private
std::ostream& operator<<(std::ostream& _out, const CompressedValue& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, CompressedValue& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* CompressedValue::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> CompressedValue::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.CompressedValue";
	type_code->type_hash = vnx::Hash64(0x50c46e244bd9765dull);
	type_code->code_hash = vnx::Hash64(0x83f41ad21fe492c6ull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->native_size = sizeof(::vnx::addons::CompressedValue);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<CompressedValue>(); };
	type_code->build();
	return type_code;
}

std::shared_ptr<vnx::Value> CompressedValue::vnx_call_switch(std::shared_ptr<const vnx::Value> _method) {
	switch(_method->get_type_hash()) {
	}
	return nullptr;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::CompressedValue& value, const TypeCode* type_code, const uint16_t* code) {
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
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::CompressedValue& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_CompressedValue;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::CompressedValue>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
}

void read(std::istream& in, ::vnx::addons::CompressedValue& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::CompressedValue& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::CompressedValue& value) {
	value.accept(visitor);
}

} // vnx
