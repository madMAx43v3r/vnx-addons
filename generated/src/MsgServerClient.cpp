
// AUTO GENERATED by vnxcppcodegen

#include <vnx/addons/package.hxx>
#include <vnx/addons/MsgServerClient.hxx>
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
#include <vnx/addons/TcpServer.h>

#include <vnx/Generic.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

MsgServerClient::MsgServerClient(const std::string& service_name)
	:	Client::Client(vnx::Hash64(service_name))
{
}

MsgServerClient::MsgServerClient(vnx::Hash64 service_addr)
	:	Client::Client(service_addr)
{
}

::vnx::Object MsgServerClient::vnx_get_config_object() {
	auto _method = ::vnx::ModuleInterface_vnx_get_config_object::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_config_object_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Object>();
	} else {
		throw std::logic_error("MsgServerClient: invalid return value");
	}
}

::vnx::Variant MsgServerClient::vnx_get_config(const std::string& name) {
	auto _method = ::vnx::ModuleInterface_vnx_get_config::create();
	_method->name = name;
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_config_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::Variant>();
	} else {
		throw std::logic_error("MsgServerClient: invalid return value");
	}
}

void MsgServerClient::vnx_set_config_object(const ::vnx::Object& config) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config_object::create();
	_method->config = config;
	vnx_request(_method, false);
}

void MsgServerClient::vnx_set_config_object_async(const ::vnx::Object& config) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config_object::create();
	_method->config = config;
	vnx_request(_method, true);
}

void MsgServerClient::vnx_set_config(const std::string& name, const ::vnx::Variant& value) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config::create();
	_method->name = name;
	_method->value = value;
	vnx_request(_method, false);
}

void MsgServerClient::vnx_set_config_async(const std::string& name, const ::vnx::Variant& value) {
	auto _method = ::vnx::ModuleInterface_vnx_set_config::create();
	_method->name = name;
	_method->value = value;
	vnx_request(_method, true);
}

::vnx::TypeCode MsgServerClient::vnx_get_type_code() {
	auto _method = ::vnx::ModuleInterface_vnx_get_type_code::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_type_code_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<::vnx::TypeCode>();
	} else {
		throw std::logic_error("MsgServerClient: invalid return value");
	}
}

std::shared_ptr<const ::vnx::ModuleInfo> MsgServerClient::vnx_get_module_info() {
	auto _method = ::vnx::ModuleInterface_vnx_get_module_info::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_get_module_info_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<std::shared_ptr<const ::vnx::ModuleInfo>>();
	} else {
		throw std::logic_error("MsgServerClient: invalid return value");
	}
}

void MsgServerClient::vnx_restart() {
	auto _method = ::vnx::ModuleInterface_vnx_restart::create();
	vnx_request(_method, false);
}

void MsgServerClient::vnx_restart_async() {
	auto _method = ::vnx::ModuleInterface_vnx_restart::create();
	vnx_request(_method, true);
}

void MsgServerClient::vnx_stop() {
	auto _method = ::vnx::ModuleInterface_vnx_stop::create();
	vnx_request(_method, false);
}

void MsgServerClient::vnx_stop_async() {
	auto _method = ::vnx::ModuleInterface_vnx_stop::create();
	vnx_request(_method, true);
}

vnx::bool_t MsgServerClient::vnx_self_test() {
	auto _method = ::vnx::ModuleInterface_vnx_self_test::create();
	auto _return_value = vnx_request(_method, false);
	if(auto _result = std::dynamic_pointer_cast<const ::vnx::ModuleInterface_vnx_self_test_return>(_return_value)) {
		return _result->_ret_0;
	} else if(_return_value && !_return_value->is_void()) {
		return _return_value->get_field_by_index(0).to<vnx::bool_t>();
	} else {
		throw std::logic_error("MsgServerClient: invalid return value");
	}
}


} // namespace vnx
} // namespace addons
