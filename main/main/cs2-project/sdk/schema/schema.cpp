#include "schema.h"
#include "current_schema.h"
#include "../../core/interfaces/interface.h"
#include "../game_class/interfaces/ischemasystem.h"
#include "../../dependencies/fnv1a.h"
#include <mutex>
bool c_schema_system::init(schema_table_map_t& maps, const char* cls, uint64_t key) {
    auto& values=maps[key];
    for (const auto& field:frlsCurrentFields) if (std::strcmp(field.cls,cls)==0) values[fnv1a::hash(field.field)]=field.offset;
    auto* scope=g_interface->schema_system ? g_interface->schema_system->find_type_scope_for_module("client.dll") : nullptr;
    auto* info=scope ? scope->FindDeclaredClass(cls) : nullptr;
    auto collect=[&](auto&& self,c_schema_class_info_data* current,int32_t offset,int depth)->void {
        if (!current || depth>16) return;
        auto* raw=reinterpret_cast<std::byte*>(current);
        const int count=*reinterpret_cast<uint16_t*>(raw+0x24);
        auto* fields=*reinterpret_cast<SchemaClassFieldData_t**>(raw+0x30);
        if (count>8192 || (count && !fields)) return;
        struct Base { int32_t offset; c_schema_class_info_data* info; };
        const int baseCount=*reinterpret_cast<uint8_t*>(raw+0x2b);
        auto* bases=*reinterpret_cast<Base**>(raw+0x40);
        if (bases && baseCount<=16) for(int i=0;i<baseCount;++i) self(self,bases[i].info,offset+bases[i].offset,depth+1);
        for(int i=0;i<count;++i) if(fields[i].szName) values[fnv1a::hash(fields[i].szName)]=offset+fields[i].nSingleInheritanceOffset;
    };
    collect(collect,info,0,0);
    return !values.empty();
}
int32_t c_schema_system::schema_get_offset(const char* cls,const char* field) {
    static schema_table_map_t maps;
    static std::recursive_mutex mutex;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    const auto key=fnv1a::hash(cls), prop=fnv1a::hash(field);
    if (!maps.contains(key)) init(maps,cls,key);
    auto& values=maps[key];auto it=values.find(prop);
    if(it!=values.end()) return it->second;
    std::printf("Missing schema: %s->%s\n",cls,field); return 0;
}
int32_t c_schema_system::get_schema_offset(const uint32_t& cls,const uint32_t& field) {
    auto it=g_schema_data.find(cls);if(it==g_schema_data.end()) return 0;
    auto member=it->second.find(field);return member==it->second.end() ? 0 : member->second.m_offset;
}
