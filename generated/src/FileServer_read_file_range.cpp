
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/FileServer_read_file_range.hxx>
#include <vnx/Value.h>
#include <vnx/addons/FileServer_read_file_range_return.hxx>

#include <vnx/vnx.h>


namespace vnx {
namespace addons {


const vnx::Hash64 FileServer_read_file_range::VNX_TYPE_HASH(0x1294e2e1e6847490ull);
const vnx::Hash64 FileServer_read_file_range::VNX_CODE_HASH(0x9c188b5e58046f0eull);

vnx::Hash64 FileServer_read_file_range::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string FileServer_read_file_range::get_type_name() const {
	return "vnx.addons.FileServer.read_file_range";
}

const vnx::TypeCode* FileServer_read_file_range::get_type_code() const {
	return vnx::addons::vnx_native_type_code_FileServer_read_file_range;
}

std::shared_ptr<FileServer_read_file_range> FileServer_read_file_range::create() {
	return std::make_shared<FileServer_read_file_range>();
}

std::shared_ptr<vnx::Value> FileServer_read_file_range::clone() const {
	return std::make_shared<FileServer_read_file_range>(*this);
}

void FileServer_read_file_range::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void FileServer_read_file_range::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void FileServer_read_file_range::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = vnx::addons::vnx_native_type_code_FileServer_read_file_range;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, path);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, offset);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, length);
	_visitor.type_end(*_type_code);
}

void FileServer_read_file_range::write(std::ostream& _out) const {
	_out << "{\"__type\": \"vnx.addons.FileServer.read_file_range\"";
	_out << ", \"path\": "; vnx::write(_out, path);
	_out << ", \"offset\": "; vnx::write(_out, offset);
	_out << ", \"length\": "; vnx::write(_out, length);
	_out << "}";
}

void FileServer_read_file_range::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object FileServer_read_file_range::to_object() const {
	vnx::Object _object;
	_object["__type"] = "vnx.addons.FileServer.read_file_range";
	_object["path"] = path;
	_object["offset"] = offset;
	_object["length"] = length;
	return _object;
}

void FileServer_read_file_range::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "length") {
			_entry.second.to(length);
		} else if(_entry.first == "offset") {
			_entry.second.to(offset);
		} else if(_entry.first == "path") {
			_entry.second.to(path);
		}
	}
}

vnx::Variant FileServer_read_file_range::get_field(const std::string& _name) const {
	if(_name == "path") {
		return vnx::Variant(path);
	}
	if(_name == "offset") {
		return vnx::Variant(offset);
	}
	if(_name == "length") {
		return vnx::Variant(length);
	}
	return vnx::Variant();
}

void FileServer_read_file_range::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "path") {
		_value.to(path);
	} else if(_name == "offset") {
		_value.to(offset);
	} else if(_name == "length") {
		_value.to(length);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const FileServer_read_file_range& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, FileServer_read_file_range& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* FileServer_read_file_range::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> FileServer_read_file_range::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "vnx.addons.FileServer.read_file_range";
	type_code->type_hash = vnx::Hash64(0x1294e2e1e6847490ull);
	type_code->code_hash = vnx::Hash64(0x9c188b5e58046f0eull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_method = true;
	type_code->native_size = sizeof(::vnx::addons::FileServer_read_file_range);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<FileServer_read_file_range>(); };
	type_code->is_const = true;
	type_code->return_type = ::vnx::addons::FileServer_read_file_range_return::static_get_type_code();
	type_code->fields.resize(3);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "path";
		field.code = {32};
	}
	{
		auto& field = type_code->fields[1];
		field.data_size = 8;
		field.name = "offset";
		field.code = {8};
	}
	{
		auto& field = type_code->fields[2];
		field.data_size = 8;
		field.name = "length";
		field.code = {8};
	}
	type_code->permission = "vnx.addons.permission_e.FILE_DOWNLOAD";
	type_code->build();
	return type_code;
}


} // namespace vnx
} // namespace addons


namespace vnx {

void read(TypeInput& in, ::vnx::addons::FileServer_read_file_range& value, const TypeCode* type_code, const uint16_t* code) {
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
		if(const auto* const _field = type_code->field_map[1]) {
			vnx::read_value(_buf + _field->offset, value.offset, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[2]) {
			vnx::read_value(_buf + _field->offset, value.length, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.path, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::vnx::addons::FileServer_read_file_range& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = vnx::addons::vnx_native_type_code_FileServer_read_file_range;
		out.write_type_code(type_code);
		vnx::write_class_header<::vnx::addons::FileServer_read_file_range>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	auto* const _buf = out.write(16);
	vnx::write_value(_buf + 0, value.offset);
	vnx::write_value(_buf + 8, value.length);
	vnx::write(out, value.path, type_code, type_code->fields[0].code.data());
}

void read(std::istream& in, ::vnx::addons::FileServer_read_file_range& value) {
	value.read(in);
}

void write(std::ostream& out, const ::vnx::addons::FileServer_read_file_range& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::vnx::addons::FileServer_read_file_range& value) {
	value.accept(visitor);
}

} // vnx
