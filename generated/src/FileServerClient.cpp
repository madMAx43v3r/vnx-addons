
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/FileServerClient.hxx>
#include <vnx/Buffer.hpp>
#include <vnx/Module.h>
#include <vnx/ModuleInterface_vnx_get_config.hxx>
#include <vnx/ModuleInterface_vnx_get_config_return.hxx>
#include <vnx/ModuleInterface_vnx_get_config_object.hxx>
#include <vnx/ModuleInterface_vnx_get_config_object_return.hxx>
#include <vnx/ModuleInterface_vnx_get_module_info.hxx>
#include <vnx/ModuleInterface_vnx_get_module_info_return.hxx>
#include <vnx/ModuleInterface_vnx_get_type_code.hxx>
#include <vnx/ModuleInterface_vnx_get_type_code_return.hxx>
#include <vnx/ModuleInterface_vnx_restart.hxx>
#include <vnx/ModuleInterface_vnx_restart_return.hxx>
#include <vnx/ModuleInterface_vnx_self_test.hxx>
#include <vnx/ModuleInterface_vnx_self_test_return.hxx>
#include <vnx/ModuleInterface_vnx_set_config.hxx>
#include <vnx/ModuleInterface_vnx_set_config_return.hxx>
#include <vnx/ModuleInterface_vnx_set_config_object.hxx>
#include <vnx/ModuleInterface_vnx_set_config_object_return.hxx>
#include <vnx/ModuleInterface_vnx_stop.hxx>
#include <vnx/ModuleInterface_vnx_stop_return.hxx>
#include <vnx/addons/FileServer_delete_file.hxx>
#include <vnx/addons/FileServer_delete_file_return.hxx>
#include <vnx/addons/FileServer_get_file_info.hxx>
#include <vnx/addons/FileServer_get_file_info_return.hxx>
#include <vnx/addons/FileServer_read_directory.hxx>
#include <vnx/addons/FileServer_read_directory_return.hxx>
#include <vnx/addons/FileServer_read_file.hxx>
#include <vnx/addons/FileServer_read_file_return.hxx>
#include <vnx/addons/FileServer_read_file_range.hxx>
#include <vnx/addons/FileServer_read_file_range_return.hxx>
#include <vnx/addons/FileServer_write_file.hxx>
#include <vnx/addons/FileServer_write_file_return.hxx>
#include <vnx/addons/HttpComponent_http_request.hxx>
#include <vnx/addons/HttpComponent_http_request_return.hxx>
#include <vnx/addons/HttpComponent_http_request_chunk.hxx>
#include <vnx/addons/HttpComponent_http_request_chunk_return.hxx>
#include <vnx/addons/HttpData.hxx>
#include <vnx/addons/HttpRequest.hxx>
#include <vnx/addons/HttpResponse.hxx>
#include <vnx/addons/file_info_t.hxx>

#include <vnx/Generic.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

FileServerClient::FileServerClient(const std::string& service_name)
	:	Client::Client(vnx::Hash64(service_name))
{
}

FileServerClient::FileServerClient(vnx::Hash64 service_addr)
	:	Client::Client(service_addr)
{
}

::vnx::Buffer FileServerClient::read_file(const std::string& path) {
	auto _method = ::vnx::addons::FileServer_read_file::create();
	_method->path = path;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::FileServer_read_file_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Buffer>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

::vnx::Buffer FileServerClient::read_file_range(const std::string& path, const int64_t& offset, const int64_t& length) {
	auto _method = ::vnx::addons::FileServer_read_file_range::create();
	_method->path = path;
	_method->offset = offset;
	_method->length = length;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::FileServer_read_file_range_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Buffer>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

::vnx::addons::file_info_t FileServerClient::get_file_info(const std::string& path) {
	auto _method = ::vnx::addons::FileServer_get_file_info::create();
	_method->path = path;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::FileServer_get_file_info_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::addons::file_info_t>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

std::vector<::vnx::addons::file_info_t> FileServerClient::read_directory(const std::string& path) {
	auto _method = ::vnx::addons::FileServer_read_directory::create();
	_method->path = path;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::FileServer_read_directory_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<std::vector<::vnx::addons::file_info_t>>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

void FileServerClient::write_file(const std::string& path, const ::vnx::Buffer& data) {
	auto _method = ::vnx::addons::FileServer_write_file::create();
	_method->path = path;
	_method->data = data;
	vnx_request(_method, false);
}

void FileServerClient::write_file_async(const std::string& path, const ::vnx::Buffer& data) {
	auto _method = ::vnx::addons::FileServer_write_file::create();
	_method->path = path;
	_method->data = data;
	vnx_request(_method, true);
}

void FileServerClient::delete_file(const std::string& path) {
	auto _method = ::vnx::addons::FileServer_delete_file::create();
	_method->path = path;
	vnx_request(_method, false);
}

void FileServerClient::delete_file_async(const std::string& path) {
	auto _method = ::vnx::addons::FileServer_delete_file::create();
	_method->path = path;
	vnx_request(_method, true);
}

std::shared_ptr<const ::vnx::addons::HttpResponse> FileServerClient::http_request(std::shared_ptr<const ::vnx::addons::HttpRequest> request, const std::string& sub_path) {
	auto _method = ::vnx::addons::HttpComponent_http_request::create();
	_method->request = request;
	_method->sub_path = sub_path;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::HttpComponent_http_request_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<std::shared_ptr<const ::vnx::addons::HttpResponse>>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

std::shared_ptr<const ::vnx::addons::HttpData> FileServerClient::http_request_chunk(std::shared_ptr<const ::vnx::addons::HttpRequest> request, const std::string& sub_path, const int64_t& offset, const int64_t& max_bytes) {
	auto _method = ::vnx::addons::HttpComponent_http_request_chunk::create();
	_method->request = request;
	_method->sub_path = sub_path;
	_method->offset = offset;
	_method->max_bytes = max_bytes;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::addons::HttpComponent_http_request_chunk_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<std::shared_ptr<const ::vnx::addons::HttpData>>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

::vnx::Object FileServerClient::vnx_get_config_object() {
	auto _method = ::vnx::ModuleInterface_vnx_get_config_object::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_config_object_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Object>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

::vnx::Variant FileServerClient::vnx_get_config(const std::string& name) {
	auto _method = ::vnx::ModuleInterface_vnx_get_config::create();
	_method->name = name;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_config_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Variant>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

void FileServerClient::vnx_set_config_object(const ::vnx::Object& config) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config_object::create();
	_method->config = config;
	vnx_request(_method, false);
}

void FileServerClient::vnx_set_config_object_async(const ::vnx::Object& config) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config_object::create();
	_method->config = config;
	vnx_request(_method, true);
}

void FileServerClient::vnx_set_config(const std::string& name, const ::vnx::Variant& value) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config::create();
	_method->name = name;
	_method->value = value;
	vnx_request(_method, false);
}

void FileServerClient::vnx_set_config_async(const std::string& name, const ::vnx::Variant& value) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config::create();
	_method->name = name;
	_method->value = value;
	vnx_request(_method, true);
}

::vnx::TypeCode FileServerClient::vnx_get_type_code() {
	auto _method = ::vnx::ModuleInterface_vnx_get_type_code::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_type_code_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::TypeCode>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

std::shared_ptr<const ::vnx::ModuleInfo> FileServerClient::vnx_get_module_info() {
	auto _method = ::vnx::ModuleInterface_vnx_get_module_info::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_module_info_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<std::shared_ptr<const ::vnx::ModuleInfo>>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}

void FileServerClient::vnx_restart() {
	auto _method = ::vnx::ModuleInterface_vnx_restart::create();
	vnx_request(_method, false);
}

void FileServerClient::vnx_restart_async() {
	auto _method = ::vnx::ModuleInterface_vnx_restart::create();
	vnx_request(_method, true);
}

void FileServerClient::vnx_stop() {
	auto _method = ::vnx::ModuleInterface_vnx_stop::create();
	vnx_request(_method, false);
}

void FileServerClient::vnx_stop_async() {
	auto _method = ::vnx::ModuleInterface_vnx_stop::create();
	vnx_request(_method, true);
}

vnx::bool_t FileServerClient::vnx_self_test() {
	auto _method = ::vnx::ModuleInterface_vnx_self_test::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_self_test_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<vnx::bool_t>();
	} else {
		throw std::logic_error("FileServerClient: invalid return value");
	}
}


} // namespace vnx
} // namespace addons
