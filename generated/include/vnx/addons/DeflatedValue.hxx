
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_vnx_addons_DeflatedValue_HXX_
#define INCLUDE_vnx_addons_DeflatedValue_HXX_

#include <vnx/addons/package.hxx>
#include <vnx/Buffer.hpp>
#include <vnx/Value.h>
#include <vnx/addons/CompressedValue.hxx>


namespace vnx {
namespace addons {

class DeflatedValue : public ::vnx::addons::CompressedValue {
public:
	
	::vnx::Buffer data;
	
	typedef ::vnx::addons::CompressedValue Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	vnx::Hash64 get_type_hash() const override;
	const char* get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	virtual std::shared_ptr<const ::vnx::Value> decompress() const;
	static std::shared_ptr<const ::vnx::addons::DeflatedValue> compress(std::shared_ptr<const ::vnx::Value> value);
	static std::shared_ptr<const ::vnx::addons::DeflatedValue> compress_ex(std::shared_ptr<const ::vnx::Value> value, const int32_t& level);
	
	static std::shared_ptr<DeflatedValue> create();
	std::shared_ptr<vnx::Value> clone() const;
	
	void read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code);
	void write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const;
	
	void read(std::istream& _in);
	void write(std::ostream& _out) const;
	
	void accept(vnx::Visitor& _visitor) const;
	
	vnx::Object to_object() const;
	void from_object(const vnx::Object& object);
	
	vnx::Variant get_field(const std::string& name) const;
	void set_field(const std::string& name, const vnx::Variant& value);
	
	friend std::ostream& operator<<(std::ostream& _out, const DeflatedValue& _value);
	friend std::istream& operator>>(std::istream& _in, DeflatedValue& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};


} // namespace vnx
} // namespace addons

#endif // INCLUDE_vnx_addons_DeflatedValue_HXX_
