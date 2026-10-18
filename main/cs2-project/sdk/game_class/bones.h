#pragma once

enum bones
{
    bone_pelvis = 1,
    bone_spine_2 = 2,
    bone_spine_1 = 3,
    bone_neck = 6,
    bone_head = 7,
    bone_arm_top_l = 9,
    bone_arm_bot_l = 10,
    bone_hand_l = 11,
    bone_arm_top_r = 13,
    bone_arm_bot_r = 14,
    bone_hand_r = 15,
    bone_leg_top_l = 17,
    bone_leg_bot_l = 18,
    bone_ankle_l = 19,
    bone_leg_top_r = 20,
    bone_leg_bot_r = 21,
    bone_ankle_r = 22
};

inline constexpr int skeleton_links[][2] = {
    {bone_head, bone_neck},
    {bone_neck, bone_spine_1},
    {bone_spine_1, bone_spine_2},
    {bone_spine_2, bone_pelvis},
    {bone_neck, bone_arm_top_l},
    {bone_arm_top_l, bone_arm_bot_l},
    {bone_arm_bot_l, bone_hand_l},
    {bone_neck, bone_arm_top_r},
    {bone_arm_top_r, bone_arm_bot_r},
    {bone_arm_bot_r, bone_hand_r},
    {bone_pelvis, bone_leg_top_l},
    {bone_leg_top_l, bone_leg_bot_l},
    {bone_leg_bot_l, bone_ankle_l},
    {bone_pelvis, bone_leg_top_r},
    {bone_leg_top_r, bone_leg_bot_r},
    {bone_leg_bot_r, bone_ankle_r}
};
