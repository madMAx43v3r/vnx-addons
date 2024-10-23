
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/FileServer_get_file_info.hxx>
#include <vnx/Value.h>
#include <vnx/addons/FileServer_get_file_info_return.hxx>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 FileServer_get_file_info::VNX_TYPE_HASH(0xd0dde082f276b7f1ull);
const vnx::Hash64 FileServer_get_file_info::VNX_CODE_HASH(0x56f1c0245432d2d1ull);

vnx::Hash64 FileServer_get_file_info::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string FileServer_get_file_info::get_type_name() const {
	return "vnx.addons.FileServer.get_file_info";
}

const vnx::TypeCode* FileServer_get_file_info::get_type_code() const {
	return vnx::addons::vnx_native_type_code_FileServer_get_file_info;
}

std::shared_ptr<FileServer_get_file_info> FileServer_get_file_info::create() {
	return std::make_shared<FileServer_get_file_info>();
}

std::shared_ptr<vnx::Value> FileServer_get_file_info::clone() const {
	return std::make_shared<FileServer_get_file_info>(*this);
}

void FileServer_get_file_info::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void FileServer_get_file_info::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void FileServer_get_file_info::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_FileServer_get_file_info;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, path);
	_visitor.type_end(*_type_code);
}

void FileServer_get_file_info::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.FileServer.get_file_info\"";
	_out << ", \"path\": "; vnx::write(_out, path);
	_out << "}";
}

void FileServer_get_file_info::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object FileServer_get_file_info::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.FileServer.get_file_info";
	_object["path"] = path;
	return _object;
}

void FileServer_get_file_info::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "path") {
			_entry.second.to(path);
		}
	}
}

vnx::Variant FileServer_get_file_info::get_field(const std::string& _name) const {
	if(_name == "path") {
		return vnx::Variant(path);
	}
	return vnx::Variant();
}

void FileServer_get_file_info::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "path") {
		_value.to(path);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const FileServer_get_file_info& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, FileServer_get_file_info& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* FileServer_get_file_info::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> FileServer_get_file_info::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.FileServer.get_file_info";
	type_code->type_hash = vnx::Hash64(0xd0dde082f276b7f1ull);
	type_code->code_hash = vnx::Hash64(0x56f1c0245432d2d1ull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_method = true;
	type_code->native_size = sizeof(::vnx::addons::FileServer_get_file_info);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<FileServer_get_file_info>(); };
	type_code->is_const = true;
	type_code->return_type = ::vnx::addons::FileServer_get_file_info_return::static_get_type_code();
	type_code->fields.resize(1);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "path";
		field.code = {32};
	}
	type_code->build();
	return type_code;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::FileServer_get_file_info& value, const TypeCode* type_code, const uint16_t* code) {
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
			case 0: vnx::read(in, value.path, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::FileServer_get_file_info& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_FileServer_get_file_info;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::FileServer_get_file_info>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	vnx::write(out, value.path, type_code, type_code->fields[0].code.data());
}

void read(std::istream& in, ::vnx::addons::FileServer_get_file_info& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::FileServer_get_file_info& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::FileServer_get_file_info& value) {
	value.accept(visitor);
}

} // vnx
