#pragma once

// forward declarations
struct IDXGISwapChain;

class ISwapChainDx11
{
public:
    char pad[(0x170)];
    IDXGISwapChain* pDXGISwapChain;
};