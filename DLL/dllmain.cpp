// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// DirectX Resizing.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "d3d11.h"
#include "d3d11_1.h"
#include "dxgi1_2.h"
#include <directxcolors.h>
#include <dcomp.h>

#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dcomp")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ID3D11Device* D3D11Device;
ID3D11Device1* D3D11Device1;
ID3D11DeviceContext* ImmediateContext;
ID3D11DeviceContext1* ImmediateContext1;
IDXGISwapChain* SwapChain = nullptr;
IDXGISwapChain1* SwapChain1 = nullptr;
HWND hWnd;
ID3D11RenderTargetView* RenderTargetView = nullptr;
IDCompositionDevice* m_pDCompDevice;
IDCompositionTarget* m_pDCompTarget;
IDCompositionVisual* pVisual = nullptr;



// Forward declarations of functions included in this code module:
void InitDevice();
void Render();
const DWORD& Loop();


DWORD WINAPI MyThreadProc(
    _In_ LPVOID lpParameter
)
{
    return Loop();
}


extern "C" void Start(HWND hwnd)
{
    hWnd = hwnd;
    CreateThread(NULL, 0, MyThreadProc, NULL, 0, NULL);

}

const DWORD& Loop()
{
    InitDevice();

    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }

    return (int)msg.wParam;
}



void InitDevice()
{
    HRESULT hr;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
        D3D11_SDK_VERSION, &D3D11Device, nullptr, &ImmediateContext);

    IDXGIFactory1* dxgiFactory = nullptr;
    IDXGIDevice* dxgiDevice = nullptr;
    {
        hr = D3D11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
        }
    }
    if (FAILED(hr))
        return;

    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = D3D11Device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&D3D11Device1));
        if (SUCCEEDED(hr))
        {
            (void)ImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&ImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = 200;
        sd.Height = 200;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.Scaling = DXGI_SCALING_STRETCH;// DXGI_SCALING_NONE;
        //sd.BufferCount = 1;
        //sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        //sd.Stereo = true;
        sd.BufferCount = 2;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        //DXGI_SWAP_CHAIN_DESC1 sd = {};
        //sd.Width = 500;
        //sd.Height = 500;
        //sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        //sd.SampleDesc.Count = 1;
        //sd.SampleDesc.Quality = 0;
        //sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        //sd.Scaling = DXGI_SCALING_STRETCH;// DXGI_SCALING_NONE;
        ////sd.BufferCount = 1;
        //sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        ////sd.Stereo = true;
        //sd.BufferCount = 2;
        //sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        hr = dxgiFactory2->CreateSwapChainForComposition(D3D11Device, &sd, nullptr, &SwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = SwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&SwapChain));
        }

        dxgiFactory2->Release();
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = SwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return;

    hr = D3D11Device->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return;






    if (SUCCEEDED(hr))
    {
        // Create the DirectComposition device object.
        hr = DCompositionCreateDevice(dxgiDevice,
            __uuidof(IDCompositionDevice),
            reinterpret_cast<void**>(&m_pDCompDevice));
    }




    if (SUCCEEDED(hr))
    {
        // Create the composition target object based on the 
        // specified application window.
        hr = m_pDCompDevice->CreateTargetForHwnd(hWnd, TRUE, &m_pDCompTarget);
    }



    // Create a visual object.          
    hr = m_pDCompDevice->CreateVisual(&pVisual);


    if (SUCCEEDED(hr))
    {
        // Set the bitmap content of the visual. 
        hr = pVisual->SetContent(SwapChain1);
    }

    if (SUCCEEDED(hr))
    {
        // Set the horizontal and vertical position of the visual relative
        // to the upper-left corner of the composition target window.
        //hr = pVisual->SetOffsetX(100);
        if (SUCCEEDED(hr))
        {
            hr = pVisual->SetOffsetY(20);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the visual to be the root of the visual tree.          
        hr = m_pDCompTarget->SetRoot(pVisual);
    }


    dxgiDevice->Release();
}
static int count = 0;

void Render()
{
    // Just clear the backbuffer
    if (count++ % 2 == 0)
        ImmediateContext1->ClearRenderTargetView(RenderTargetView, DirectX::Colors::MidnightBlue);
    else
        ImmediateContext1->ClearRenderTargetView(RenderTargetView, DirectX::Colors::Red);
    HRESULT hr = SwapChain1->Present(1, 0);
    if (SUCCEEDED(hr))
    {
        // Commit the visual to be composed and displayed.
        hr = m_pDCompDevice->Commit();
    }
}
