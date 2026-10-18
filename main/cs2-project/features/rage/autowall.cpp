#include "autowall.h"
#include "../../sdk/sdk.h"
#include <assert.h>
#include "../../sdk/game_class/cconvar.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include <immintrin.h>

float c_autowall::penetration_damage(
    const vec3_t& local_pos,
    const vec3_t& target_pos,
    c_base_entity* local,
    c_player_pawn* local_pawn,
    c_base_entity* target,
    c_player_pawn* target_pawn,
    bool has_heavy_armor,
    int target_team, c_cs_weapon_base_vdata* weapon_data)
{
    if (!weapon_data || !local_pawn || !target_pawn || (target_pos - local_pos).length() < 0.001f) return 0.f;
    const float MAX_TRACE_LENGTH = weapon_data->m_range();
    constexpr float DAMAGE_SCALE = 500.f;
    constexpr uint32_t TARGET_CONTENTS = 268828672;

    trace_data_t trace_data{};
    trace_data.arr_pointer = &trace_data.arr;

    __m128 start = _mm_set_ps(0, local_pos.z, local_pos.y, local_pos.x);
    __m128 end = _mm_set_ps(0, target_pos.z, target_pos.y, target_pos.x);
    __m128 dir = _mm_sub_ps(end, start);

    __m128 length_sq = _mm_dp_ps(dir, dir, 0x7F);
    __m128 inv_length = _mm_rsqrt_ps(length_sq);
    dir = _mm_mul_ps(dir, inv_length);
    if (!weapon_data)
        return 0;

    if ((uintptr_t)weapon_data <= 0x0000000000000001)
        return 0;

    __m128 scaled_dir = _mm_mul_ps(dir, _mm_set1_ps(weapon_data->m_range()));
    vec3_t end_pos;
    alignas(16) float temp[4];
    _mm_storeu_ps(temp, scaled_dir);
    end_pos = vec3_t(temp[0], temp[1], temp[2]);

    TraceFilter_t filter(0x1C300B, local_pawn, nullptr, 4);

    g_interface->m_game_trace->create_trace(&trace_data, local_pos, end_pos, filter, 4);

    BulletData bullet_data(
        static_cast<float>(weapon_data->m_damage()),
        weapon_data->m_penetration(),
        weapon_data->m_range_modifier(),
        weapon_data->m_range(),
        4
    );

    float current_damage = static_cast<float>(weapon_data->m_damage());
    float trace_length = 0.f;
    float max_range = weapon_data->m_range();

    __m128 damage_vec = _mm_set1_ps(current_damage);
    __m128 range_mod_vec = _mm_set1_ps(weapon_data->m_range_modifier());
    __m128 trace_scale = _mm_set1_ps(1.0f / DAMAGE_SCALE);

    if (trace_data.num_update > 0 && trace_data.num_update <= 8 && trace_data.pointer_update_value) {
        for (int i = 0; i < trace_data.num_update; ++i) {
            auto* value = reinterpret_cast<update_value_t*>(
                reinterpret_cast<std::uintptr_t>(trace_data.pointer_update_value) +
                i * sizeof(update_value_t)
                );

            game_trace_t game_trace{};
            g_interface->m_game_trace->init_trace_info(&game_trace);
            g_interface->m_game_trace->get_trace_info(
                &trace_data,
                &game_trace,
                0.0f,
                value
            );

            max_range -= trace_length;
            if (game_trace.Fraction == 1.0f) break;

            trace_length += game_trace.Fraction * max_range;

            __m128 length_factor = _mm_mul_ps(_mm_set1_ps(trace_length), trace_scale);
            __m128 power = _mm_pow_ps(range_mod_vec, length_factor);
            damage_vec = _mm_mul_ps(damage_vec, power);
            _mm_store_ss(&current_damage, damage_vec);

            if (trace_length > MAX_TRACE_LENGTH) break;

            if (game_trace.HitEntity &&
                game_trace.HitEntity == target_pawn && game_trace.HitboxData)
            {
                scale_damage(
                    static_cast<int>(game_trace.HitboxData->m_nHitGroup),
                    target_pawn,
                    weapon_data->m_armor_ratio(),
                    weapon_data->m_headshot_multiplier(),
                    &current_damage,
                    has_heavy_armor,
                    target_team
                );
                return current_damage;
            }

            if (g_interface->m_game_trace->handle_bullet_penetration(
                &trace_data,
                reinterpret_cast<void*>(&bullet_data),
                value,
                local_pawn->m_team_num()
            )) return 0.0f;

            current_damage = bullet_data.damage;
            damage_vec = _mm_set1_ps(current_damage);
        }
    }

    return 0.0f;
}

void c_autowall::scale_damage(const int hitGroup, c_player_pawn* player,
    const float armorRatio, const float headshotMultiplier,
    float* damage, bool hasHeavyArmor, int team)
{
    // Cache these values
    static const auto get_damage_scales = []() {
        static CConVar* mp_damage_scale_ct_head = g_interface->cvar->Find(fnv1a::hash("mp_damage_scale_ct_head"));
        static CConVar* mp_damage_scale_t_head = g_interface->cvar->Find(fnv1a::hash("mp_damage_scale_t_head"));
        static CConVar* mp_damage_scale_ct_body = g_interface->cvar->Find(fnv1a::hash("mp_damage_scale_ct_body"));
        static CConVar* mp_damage_scale_t_body = g_interface->cvar->Find(fnv1a::hash("mp_damage_scale_t_body"));
        return std::make_tuple(mp_damage_scale_ct_head, mp_damage_scale_t_head,
            mp_damage_scale_ct_body, mp_damage_scale_t_body);
        };

    static const auto [ct_head, t_head, ct_body, t_body] = get_damage_scales();

    const float headScale = (team == 3 ? (ct_head ? ct_head->value.fl : 1.f) : (t_head ? t_head->value.fl : 1.f)) * (hasHeavyArmor ? 0.5f : 1.0f);
    const float bodyScale = (team == 3 ? (ct_body ? ct_body->value.fl : 1.f) : (t_body ? t_body->value.fl : 1.f));

    // Use SIMD for multiplications
    __m128 damage_vec = _mm_set1_ps(*damage);
    __m128 scale_vec;

    switch (hitGroup) {
    case static_cast<int>(HitGroup::HEAD):
        scale_vec = _mm_set1_ps(headshotMultiplier * headScale);
        break;
    case static_cast<int>(HitGroup::CHEST):
    case static_cast<int>(HitGroup::LEFTARM):
    case static_cast<int>(HitGroup::RIGHTARM):
    case static_cast<int>(HitGroup::NECK):
        scale_vec = _mm_set1_ps(bodyScale);
        break;
    case static_cast<int>(HitGroup::STOMACH):
        scale_vec = _mm_set1_ps(1.25f * bodyScale);
        break;
    case static_cast<int>(HitGroup::LEFTLEG):
    case static_cast<int>(HitGroup::RIGHTLEG):
        scale_vec = _mm_set1_ps(0.75f * bodyScale);
        break;
    default:
        return;
    }

    damage_vec = _mm_mul_ps(damage_vec, scale_vec);
    _mm_store_ss(damage, damage_vec);

    if (player->hasArmour(hitGroup)) {
        const int armorValue = player->m_armor_value();
        const float heavyArmorBonus = hasHeavyArmor ? 0.25f : 1.0f;
        const float armorBonus = hasHeavyArmor ? 0.33f : 0.5f;
        const float adjustedArmorRatio = armorRatio * (hasHeavyArmor ? 0.2f : 0.5f);

        const float healthDamage = *damage * adjustedArmorRatio;
        const float armorDamage = (*damage - healthDamage) * heavyArmorBonus * armorBonus;

        *damage = armorDamage > static_cast<float>(armorValue)
            ? *damage - static_cast<float>(armorValue) / armorBonus
            : healthDamage;
    }
}
