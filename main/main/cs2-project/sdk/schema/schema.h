#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <optional>
#include <algorithm>

#define OFFSET( type, name, offset )                                                              \
    [[nodiscard]] __forceinline std::add_lvalue_reference_t< type > name( ) const {                                             \
        return *reinterpret_cast< type* >( reinterpret_cast< std::uintptr_t >( this ) + offset ); \
    }

#define SCHEMA(function, type, class, prop) \
__inline type& function() \
{ \
    static const auto offset = g_schema->schema_get_offset(class, prop);  \
    return *reinterpret_cast< type* >( reinterpret_cast< uintptr_t >( this ) + offset); \
} \

#define SCHEMA_OFFSET(type, name, class_name, field, eoffset)                      \
    type& get_##name() {                                                                \
        /* Find the field offset and add the extra offset */                      \
        static auto offset = g_schema->schema_get_offset(class_name, field);           \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset + eoffset); \
    }

#define SCHEMA_ARRAY(varName, type, className, keyName)																	\
	type* varName() {																									\
		static const int32_t offset = g_schema->schema_get_offset(className, keyName);												\
																														\
		return reinterpret_cast<type*>(reinterpret_cast<unsigned __int64>(this) + offset);								\
	}

class c_schema_system final
{
public:

    struct schema_offset_t
    {
        uint32_t m_class;
        uint32_t m_prop;
        int32_t m_offset;
    };

    using schema_key_value_map_t = std::unordered_map<unsigned long long, int32_t>;
    using schema_table_map_t = std::unordered_map<unsigned long long, schema_key_value_map_t>;
    std::unordered_map< uint32_t, std::unordered_map< uint32_t, schema_offset_t > > g_schema_data;

    bool init(schema_table_map_t& table_map, const char* class_name, uint64_t class_key);
    int32_t schema_get_offset(const char* class_name, const char* key_name);
    int32_t get_schema_offset(const uint32_t& class_hash, const uint32_t& prop_hash);
    static c_schema_system& get_instance() {
        static c_schema_system instance;
        return instance;
    }
};

inline c_schema_system* g_schema = &c_schema_system::get_instance();
