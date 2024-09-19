
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_vnx_addons_HttpClient_get_HXX_
#define INCLUDE_vnx_addons_HttpClient_get_HXX_

#include <vnx/addons/package.hxx>
#include <vnx/Value.h>
#include <vnx/addons/http_request_options_t.hxx>


namespace vnx {
namespace addons {

class VNX_ADDONS_EXPORT HttpClient_get : public ::vnx::Value {
public:
	
	std::string url;
	::vnx::addons::http_request_options_t options;
	
	typedef ::vnx::Value Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0xa1ae3645458beb92ull;
	
	HttpClient_get() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	static std::shared_ptr<HttpClient_get> create();
	std::shared_ptr<vnx::Value> clone() const override;
	
	void read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) override;
	void write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const override;
	
	void read(std::istream& _in) override;
	void write(std::ostream& _out) const override;
	
	template<typename T>
	void accept_generic(T& _visitor) const;
	void accept(vnx::Visitor& _visitor) const override;
	
	vnx::Object to_object() const override;
	void from_object(const vnx::Object& object) override;
	
	vnx::Variant get_field(const std::string& name) const override;
	void set_field(const std::string& name, const vnx::Variant& value) override;
	
	friend std::ostream& operator<<(std::ostream& _out, const HttpClient_get& _value);
	friend std::istream& operator>>(std::istream& _in, HttpClient_get& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};

template<typename T>
void HttpClient_get::accept_generic(T& _visitor) const {
	_visitor.template type_begin<HttpClient_get>(2);
	_visitor.type_field("url", 0); _visitor.accept(url);
	_visitor.type_field("options", 1); _visitor.accept(options);
	_visitor.template type_end<HttpClient_get>(2);
}


} // namespace vnx
} // namespace addons


namespace vnx {

} // vnx

#endif // INCLUDE_vnx_addons_HttpClient_get_HXX_
