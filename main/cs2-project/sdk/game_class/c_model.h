#pragma once
#include "bones.h"
#include <vector>
#include <chrono>
#include "color.h"
#include "../math/vector.h"
#include "utl.h"
#include "strong_handle.h"

enum bone_flags : uint32_t
{
	FLAG_NO_BONE_FLAGS = 0x0,
	FLAG_BONEFLEXDRIVER = 0x4,
	FLAG_CLOTH = 0x8,
	FLAG_PHYSICS = 0x10,
	FLAG_ATTACHMENT = 0x20,
	FLAG_ANIMATION = 0x40,
	FLAG_MESH = 0x80,
	FLAG_HITBOX = 0x100,
	FLAG_BONE_USED_BY_VERTEX_LOD0 = 0x400,
	FLAG_BONE_USED_BY_VERTEX_LOD1 = 0x800,
	FLAG_BONE_USED_BY_VERTEX_LOD2 = 0x1000,
	FLAG_BONE_USED_BY_VERTEX_LOD3 = 0x2000,
	FLAG_BONE_USED_BY_VERTEX_LOD4 = 0x4000,
	FLAG_BONE_USED_BY_VERTEX_LOD5 = 0x8000,
	FLAG_BONE_USED_BY_VERTEX_LOD6 = 0x10000,
	FLAG_BONE_USED_BY_VERTEX_LOD7 = 0x20000,
	FLAG_BONE_MERGE_READ = 0x40000,
	FLAG_BONE_MERGE_WRITE = 0x80000,
	FLAG_ALL_BONE_FLAGS = 0xfffff,
	BLEND_PREALIGNED = 0x100000,
	FLAG_RIGIDLENGTH = 0x200000,
	FLAG_PROCEDURAL = 0x400000,
};

class c_transform
{
public:
	vec3_t position;
	vec4_t rotate;
	//public:
	//	matrix3x4_t to_matrix();
};

class c_model;

class c_perm_model_ext_part
{
public:
	c_transform m_Transform; // 0x0	
	const char* m_Name; // 0x20	
	int32_t m_nParent; // 0x28	
private:
	[[maybe_unused]] uint8_t __pad002c[0x4]; // 0x2c
public:
	c_strong_handle<c_model > m_refModel; // 0x30	
};

class c_drawcalls
{
public:
	__int32 m_nPrimitiveType; // 0x0000
	__int32 m_nBaseVertex;	  // 0x0004
	__int32 m_nVertexCount;	  // 0x0008
	__int32 m_nStartIndex;	  // 0x000C
	__int32 m_nIndexCount;	  // 0x0010
	float m_flUvDensity;	  // 0x0014
	float m_vTintColor[3];  // 0x0018
	float m_flAlpha;		  // 0x0024
	std::byte pad_0x0028[0xC0]; // 0x0028
};

class c_bones
{
public:
	const char* m_boneName;		   // 0x0000
	const char* m_parentName;	   // 0x0008
	float m_invBindPose[12]; // 0x0010
	vec3_t m_vecCenter;	   // 0x0040
	vec3_t m_vecSize;		   // 0x004C
	float m_flSphereradius;  // 0x0058
	std::byte pad_0x005C[0x4];   // 0x005C
};

class c_attachments
{
public:
	std::byte pad_0x0000[0x10];		  // 0x0000
	const char* m_key;					  // 0x0010
	std::byte pad_0x0018[0x8];		  // 0x0018
	const char* m_name;					  // 0x0020
	const char* m_influenceNames[3];	  // 0x0028
	std::byte pad_0x0030[0x10];		  // 0x0030
	vec4_t m_vInfluenceRotations[3]; // 0x0040
	vec3_t m_vInfluenceOffsets[3];	  // 0x0070
	float m_influenceWeights[3];	  // 0x0094
	std::byte pad_0x00A0[0x10];		  // 0x00A0
};

// big thx to @mmirraacclee
class c_hitbox
{
public:
	char* m_name; //0x0000
	char* m_surface_property; //0x0008
	char* m_bone_name; //0x0010
	vec3_t m_vec_min; //0x0018
	vec3_t m_vec_maxs; //0x0024
	float m_shape_radius; //0x0030
	uint32_t m_bone_hash; //0x0034
	int32_t m_hitgroup; //0x0038
	uint8_t m_shape_type; //0x003C
	bool m_translation_only; //0x003D
	char pad_003E[50]; //0x003E
};

class c_hitboxsets
{
public:
	std::byte pad_0x0000[0x20]; // 0x0000
	uint32_t m_nNameHash;		// 0x0020
	std::byte pad_0x0024[0x4];	// 0x0024
	__int32	m_nHitboxCount;	// 0x0028
	std::byte pad_0x002C[0x4];	// 0x002C
	c_hitbox* m_hitbox;			// 0x0030
	std::byte pad_0x0038[0x18]; // 0x0038
};

class CBoneData;
class c_rendermesh
{
public:
	char pad_0000[24]; //0x0000
	vec3_t m_vMinBounds; //0x0018
	vec3_t m_vMaxBounds; //0x0024
	int32_t m_drawCallsCount; //0x0030
	char pad_0034[4]; //0x0034
	c_drawcalls* m_drawCalls; //0x0038
	char pad_0040[72]; //0x0040
	int32_t m_skeletoncount; //0x0088
	char pad_008C[4]; //0x008C
	c_bones* m_skeleton; //0x0090
	char pad_0098[8]; //0x0098
	int32_t m_hashescount; //0x00A0
	char pad_00A4[4]; //0x00A4
	void* hashes; //0x00A8
	char pad_00B0[48]; //0x00B0
	c_attachments* m_attachments; //0x00E0
	int32_t m_attachments_count; //0x00E8
	char pad_00EC[28]; //0x00EC
	c_hitboxsets* m_hitboxsets; //0x0108
	int32_t m_nHitboxSets; //0x0110
};

class c_quaternion_storage
{
public:	
	float x, y, z, w;
};
struct ModelSkeletonData_t
{
public:
	c_utl_vector< const char* > m_boneName; // 0x0	
	char pad[0x8];
	c_utl_vector< int16_t > m_nParent; // 0x18	
	char pad2[0x8];
	c_utl_vector< float > m_boneSphere; // 0x30	
	char pad3[0x8];
	c_utl_vector< std::uint32_t > m_nFlag; // 0x48	
	char pad4[0x8];
	c_utl_vector< vec3_t > m_bonePosParent; // 0x60	
	char pad5[0x8];
	c_utl_vector< c_quaternion_storage > m_boneRotParent; // 0x78	
	char pad6[0x8];
	c_utl_vector< float > m_boneScaleParent; // 0x90	
	char pad7[0x8];
};
class c_perm_model_data
{
public:
	const char* name; //0x0000
	uint32_t flags; //0x0008
	vec3_t hull_min; //0x000C
	vec3_t hull_max; //0x0018
	vec3_t view_min; //0x0024
	vec3_t view_max; //0x0030
	float mass; //0x003C
	vec3_t eye_position; //0x0040
	float max_eye_deflection; //0x004C
	char* surface_property; //0x0050
	c_utl_vector<c_perm_model_ext_part> ext_parts;
	c_utl_vector<c_rendermesh*>  ref_meshes;
	char pad2[0x8];
	c_utl_vector<uint64_t> mesh_group_mask;
	char pad3[0x8];
	c_utl_vector<uint64_t> phys_group_mask;
	char pad5[0x30];
	c_utl_vector<void*> anim_groups; // 0x120  
	char pad9[0x70];
	ModelSkeletonData_t model_skeleton; // 0x188  
};


class c_model
{
public:
	void* vtable;
	c_perm_model_data perm_model_data;

	c_hitbox* get_hitbox(int index)
    {
        __try {
            if (index < 0) return nullptr;
            auto* raw = reinterpret_cast<std::byte*>(this);
            const int count = *reinterpret_cast<int*>(raw + 0x70);
            auto** meshes = *reinterpret_cast<std::byte***>(raw + 0x78);
            if (count <= 0 || count > 1024 || !meshes || !meshes[0]) return nullptr;
            auto* mesh = meshes[0];
            auto* set = *reinterpret_cast<c_hitboxsets**>(mesh + 0x150);
            const auto flags = *reinterpret_cast<uint32_t*>(mesh + 0x164);
            const auto set_count = *reinterpret_cast<int*>(mesh + 0x174);
            if (set_count > 0 && set_count <= 32)
                set = flags & 0x7fffffff ? *reinterpret_cast<c_hitboxsets**>(mesh + 0x168) : reinterpret_cast<c_hitboxsets*>(mesh + 0x168);
            if (!set || set->m_nHitboxCount <= 0 || set->m_nHitboxCount > 20 || index >= set->m_nHitboxCount || !set->m_hitbox) return nullptr;
            return &set->m_hitbox[index];
        } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    }

};

struct alignas(16) CBoneData
{
	vec3_t position;
	float scale;
	vec4_t rotation;
};

class c_model_state
{
public:
	std::uint8_t padding_0[0x80];
	CBoneData* bones;
	std::uint8_t padding_1[0x18];
	c_strong_handle<c_model> modelHandle;
};

struct scene_hitbox
{
	c_hitbox* hitbox;
	char pad[0x8];
};

struct scene_hitbox_data
{
	std::uint64_t hitbox_count{};
	void* hitbox_data{};
	char pad[0x8];
	scene_hitbox hitbox_array[19]{};
};

struct hitbox_position
{
	vec3_t position{};
	float scale{};
	vec4_t rotate{};
};

struct scene_hitbox_position
{
	std::uint64_t hitbox_count{};
	char pad[0x8]{};
	void* hitbox_data{};
	char pad2[0x8];
	hitbox_position hitbox_array[19]{};
};

struct hit_t
{
	float time;
	scene_hitbox_data hitbox_data;
	scene_hitbox_position hitbox_position;
	Color_t clr;
};
inline std::vector<hit_t> hit_capibara;