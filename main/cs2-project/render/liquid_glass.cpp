#include "liquid_glass.h"
#include <d3dcompiler.h>
#include <cstring>
#include <cstdio>

#pragma comment(lib, "d3dcompiler.lib")

namespace LiquidGlass
{

    struct GPUParams
    {
        float resolution[2];   // row 0
        float center[2];
        float halfSize[2];     // row 1
        float exponent;
        float blur;
        float lensBand;        // row 2
        float lensStrength;
        float aberration;
        float tint;
        float rim;             // row 3
        float edgeSoftness;
        float shape;           // 0 = hyperellipse, 1 = capsule
        float pad1;
    };
    static_assert(sizeof(GPUParams) == 64, "GPUParams must be 64 bytes (4 HLSL rows)");

    static const char* kHLSL = R"HLSL(
#ifndef STAGE
#define STAGE 7
#endif

cbuffer GlassCB : register(b0)
{
    float2 uResolution;     // render target size in pixels
    float2 uCenter;         // glass center in pixels
    float2 uHalfSize;       // glass half width / half height in pixels
    float  uExponent;       // hyperellipse n
    float  uBlur;           // blur tap spacing in px
    float  uLensBand;       // 0.3
    float  uLensStrength;   // 1.0
    float  uAberration;     // px
    float  uTint;           // 0.9
    float  uRim;            // highlight strength
    float  uEdgeSoftness;   // AA width in px
    float  uShape;          // 0 = hyperellipse, 1 = capsule
    float  uPad1;
};

Texture2D     uBackdrop : register(t1);
SamplerState  uSamp     : register(s1);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

float3 Bg(float2 px)
{
    return uBackdrop.SampleLevel(uSamp, px / uResolution, 0).rgb;
}

float sdHyperellipse(float2 p, float2 ab, float n, out float2 nrm)
{
    float2 q  = max(abs(p) / ab, 1e-4);              // avoid 0 (pow(0, n-1) problems)
    float  G  = pow(pow(q.x, n) + pow(q.y, n), 1.0 / n);   // == 1 on the edge
    float2 g  = pow(q / G, n - 1.0) / ab;            // gradient magnitude per axis (always >= 0)
    float2 s  = float2(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0);
    float  gl = max(length(g), 1e-4);
    nrm = s * g / gl;
    return (G - 1.0) / gl;
}

float sdCapsule(float2 p, float2 ab, out float2 nrm)
{
    float r = ab.y;
    float sx = max(ab.x - r, 0.0);
    float2 a = float2(-sx, 0.0);
    float2 b = float2(sx, 0.0);
    float2 pa = p - a;
    float2 ba = b - a;
    float h = saturate(dot(pa, ba) / max(dot(ba, ba), 1e-8));
    float2 q = pa - ba * h;
    nrm = q / max(length(q), 1e-4);
    return length(q) - r;
}

float4 main(PS_INPUT i) : SV_Target
{
    float2 frag    = i.pos.xy;                       // DX pixel coords (y down) - do NOT flip
    float2 p       = frag - uCenter;
    float  minHalf = min(uHalfSize.x, uHalfSize.y);
    float  size    = 2.0 * minHalf;

    float2 nrm;
    float d;
    if (uShape < 0.5)
        d = sdHyperellipse(p, uHalfSize, clamp(uExponent, 2.0, 16.0), nrm);
    else
        d = sdCapsule(p, uHalfSize, nrm);

    // anti-aliased shape mask, 1 inside, 0 outside
    float mask = saturate(0.5 - d / max(uEdgeSoftness, 0.001));
    if (mask <= 0.0) discard;

    if (STAGE == 1) return float4(1.0, 0.0, 1.0, mask);
    if (STAGE == 2) return float4(Bg(frag), mask);
    if (STAGE == 3) return float4(Bg(frag) * 0.5, mask);

    float inv            = max(-d, 0.0) / size;                      // 0 at edge .. ~0.5 deep inside
    float distFromCenter = 1.0 - saturate(inv / uLensBand);          // 0 deep inside .. 1 at edge
    float distortion     = 1.0 - sqrt(saturate(1.0 - distFromCenter * distFromCenter));
    float2 offset        = distortion * nrm * minHalf * uLensStrength;
    float2 coord         = frag - offset;                            // minus = magnify (same as article)

    if (STAGE == 4) return float4(Bg(coord) * uTint, mask);

    float2 shift = nrm * uAberration * distFromCenter * distFromCenter;

    if (STAGE == 5)
    {
        float3 c = float3(Bg(coord - shift).r, Bg(coord).g, Bg(coord + shift).b);
        return float4(c * uTint, mask);
    }

    float blurRadius = uBlur * (1.0 - distFromCenter * 0.5);
    float3 acc = 0.0;
    float  wsum = 0.0;
    [unroll] for (int x = -2; x <= 2; x++)
    {
        [unroll] for (int y = -2; y <= 2; y++)
        {
            float2 o = float2((float)x, (float)y) * blurRadius;
            float  w = exp(-0.25 * (float)(x * x + y * y));          // == article's exp(-0.5*r2/2)
            acc.r += Bg(coord - shift + o).r * w;
            acc.g += Bg(coord         + o).g * w;
            acc.b += Bg(coord + shift + o).b * w;
            wsum  += w;
        }
    }
    float3 col = (acc / wsum) * uTint;

    if (STAGE == 6) return float4(col, mask);

    float  rimW = pow(distFromCenter, 3.0);
    float2 L    = normalize(float2(-0.6, -0.8));                     // light from top-left (y down)
    float  hi   = saturate(dot(nrm, L));
    float  lo   = saturate(dot(nrm, -L)) * 0.35;
    col += uRim * rimW * (hi + lo);

    col *= i.col.rgb;                                                // ImGui vertex color = extra tint (white = none)
    return float4(col, mask * i.col.a);
}
)HLSL";

    // state
    static ID3D11Device* g_dev = nullptr;
    static ID3D11DeviceContext* g_ctx = nullptr;
    static ID3D11PixelShader* g_ps = nullptr;
    static ID3D11Buffer* g_cb = nullptr;
    static ID3D11SamplerState* g_samp = nullptr;
    static ID3D11Texture2D* g_snap = nullptr;
    static ID3D11ShaderResourceView* g_snapSRV = nullptr;
    static UINT                       g_w = 0, g_h = 0;
    static DXGI_FORMAT                g_fmt = DXGI_FORMAT_UNKNOWN;

    template <class T> static void SafeRelease(T*& p) { if (p) { p->Release(); p = nullptr; } }

    void ReleaseSnapshot()
    {
        SafeRelease(g_snapSRV);
        SafeRelease(g_snap);
        g_w = g_h = 0;
        g_fmt = DXGI_FORMAT_UNKNOWN;
    }

    bool Init(ID3D11Device* device, ID3D11DeviceContext* ctx)
    {
        g_dev = device;
        g_ctx = ctx;

        ID3DBlob* blob = nullptr;
        ID3DBlob* err = nullptr;
        HRESULT hr = D3DCompile(kHLSL, strlen(kHLSL), "liquid_glass.hlsl", nullptr, nullptr,
            "main", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &blob, &err);
        if (FAILED(hr))
        {
            if (err) { OutputDebugStringA((const char*)err->GetBufferPointer()); err->Release(); }
            return false;
        }
        if (err) err->Release();
        hr = g_dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &g_ps);
        blob->Release();
        if (FAILED(hr)) return false;

        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = sizeof(GPUParams);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(g_dev->CreateBuffer(&bd, nullptr, &g_cb))) return false;

        D3D11_SAMPLER_DESC sd = {};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        if (FAILED(g_dev->CreateSamplerState(&sd, &g_samp))) return false;

        return true;
    }

    void Shutdown()
    {
        ReleaseSnapshot();
        SafeRelease(g_samp);
        SafeRelease(g_cb);
        SafeRelease(g_ps);
        g_ctx = nullptr;
        g_dev = nullptr;
    }

    void CaptureRenderTarget()
    {
        if (!g_ctx) return;

        ID3D11RenderTargetView* rtv = nullptr;
        g_ctx->OMGetRenderTargets(1, &rtv, nullptr);
        if (!rtv) return;

        ID3D11Resource* res = nullptr;
        rtv->GetResource(&res);
        rtv->Release();
        if (!res) return;

        ID3D11Texture2D* src = nullptr;
        res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&src);
        res->Release();
        if (!src) return;

        D3D11_TEXTURE2D_DESC sd = {};
        src->GetDesc(&sd);

        if (!g_snap || sd.Width != g_w || sd.Height != g_h || sd.Format != g_fmt)
        {
            ReleaseSnapshot();

            D3D11_TEXTURE2D_DESC td = sd;
            td.MipLevels = 1;
            td.ArraySize = 1;
            td.SampleDesc = { 1, 0 };            // snapshot is never multisampled
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            td.CPUAccessFlags = 0;
            td.MiscFlags = 0;

            if (FAILED(g_dev->CreateTexture2D(&td, nullptr, &g_snap))) { src->Release(); return; }
            if (FAILED(g_dev->CreateShaderResourceView(g_snap, nullptr, &g_snapSRV))) { ReleaseSnapshot(); src->Release(); return; }

            g_w = sd.Width; g_h = sd.Height; g_fmt = sd.Format;
        }

        if (sd.SampleDesc.Count > 1)
            g_ctx->ResolveSubresource(g_snap, 0, src, 0, sd.Format);   // msaa target
        else
            g_ctx->CopyResource(g_snap, src);

        src->Release();
    }

    static void GlassCallback(const ImDrawList*, const ImDrawCmd* cmd)
    {
        if (!g_snapSRV) return;                                   // no snapshot yet quad is drawn with imguis shader (white); avoid by always calling capture first

        GPUParams gp = *(const GPUParams*)cmd->UserCallbackData;  // local copy so we can patch resolution
        delete static_cast<const GPUParams*>(cmd->UserCallbackData);
        gp.resolution[0] = (float)g_w;
        gp.resolution[1] = (float)g_h;

        D3D11_MAPPED_SUBRESOURCE m;
        if (SUCCEEDED(g_ctx->Map(g_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &m)))
        {
            memcpy(m.pData, &gp, sizeof(gp));
            g_ctx->Unmap(g_cb, 0);
        }

        g_ctx->PSSetShader(g_ps, nullptr, 0);
        g_ctx->PSSetConstantBuffers(0, 1, &g_cb);
        g_ctx->PSSetShaderResources(1, 1, &g_snapSRV);            // slot is 1, not 0 (imgui overwrites slot 0)
        g_ctx->PSSetSamplers(1, 1, &g_samp);
    }

    void Draw(ImDrawList* dl, ImVec2 center, ImVec2 halfSize, const Params& p, ImU32 tint)
    {
        auto* gp = new GPUParams{};
        gp->center[0] = center.x; gp->center[1] = center.y;
        gp->halfSize[0] = halfSize.x; gp->halfSize[1] = halfSize.y;
        gp->exponent = p.exponent; gp->blur = p.blur;
        gp->lensBand = p.lensBand; gp->lensStrength = p.lensStrength;
        gp->aberration = p.aberration; gp->tint = p.tint;
        gp->rim = p.rim; gp->edgeSoftness = p.edgeSoftness; gp->shape = p.shape;

        dl->AddCallback(GlassCallback, gp);

        const float margin = 2.0f;
        ImVec2 a(center.x - halfSize.x - margin, center.y - halfSize.y - margin);
        ImVec2 b(center.x + halfSize.x + margin, center.y + halfSize.y + margin);
        dl->PrimReserve(6, 4);
        dl->PrimRect(a, b, tint);                                 // uses the white pixel of the font atlas; our shader ignores the texture

        dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr); // give imgui back its own state
    }

}
/*
+========++======++++====+=+++==+++++++===+++=======+=====+==================+==++==========+++++++
+++=+==========+++++++++=+==+++===++++++===+=+++++++=++==++++=++++==+==+++++==+==++=++====+=+++++=
+++=+++++++====++=++++++++=+=+++++++++++==+=++====++=+=+++++++++====+++++=+=+=+===+===+====++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++=+++=+++++++++++++=============++====+=++++++++++
+++++++++++++===+=++++++++++++++++++++++++++++++++++++++++++++++++++++++=+=++==++++=+++++++==+=+++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=++++++++
++++++++++++==++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++###############################+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@%@%%%@@@@@@@@@%@@@@@@%@@%%@@@%@@@%#++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++@%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++#%%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++%%%%%#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%+++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%%++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
github dibilovasanek-rgb aka strawman
*/