#pragma once
#include <d3d11.h>
#include "../dependencies/imgui/imgui.h"

namespace LiquidGlass
{
    struct Params
    {
        float exponent     = 5.0f;  // 2 ellipse 10 near rect
        float lensBand     = 0.70f; // fraction of the shortest side where refraction happens
        float lensStrength = 1.0f;  // edge samples from min(halfSize) px inward
        float blur         = 1.2f;  // blur tap spacing in px
        float aberration   = 3.0f;  // chromatic aberration in px at the very edge
        float tint         = 0.90f; // brightness multiplier
        float rim          = 0.25f; // edge highlight strength. 0 = off
        float edgeSoftness = 1.0f;  // anti-aliasing width of the shape edge in px
        float shape        = 0.0f;  // 0 = hyperellipse, 1 = capsule
    };

    bool Init(ID3D11Device* device, ID3D11DeviceContext* ctx);
    void Shutdown();
    void ReleaseSnapshot();          // call before swapchain ResizeBuffers and on device reset
    void CaptureRenderTarget();      // call once per frame, after the scene, right before ImGui_ImplDX11_RenderDrawData
    void Draw(ImDrawList* dl, ImVec2 center, ImVec2 halfSize, const Params& p, ImU32 tint = IM_COL32_WHITE);
}
