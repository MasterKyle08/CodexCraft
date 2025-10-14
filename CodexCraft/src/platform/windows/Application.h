#pragma once

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include <string>

namespace CodexCraft::Platform::Windows {

class Renderer {
public:
    static Renderer& Instance();

    void Initialize(HWND hwnd, UINT width, UINT height, bool enableDebugLayer = false);
    void Shutdown();
    void Resize(UINT width, UINT height);
    void BeginFrame(const float clearColor[4]);
    void EndFrame(bool vsync);

    ID3D11Device* GetDevice() const noexcept;
    ID3D11DeviceContext* GetContext() const noexcept;
    IDXGISwapChain* GetSwapChain() const noexcept;
    ID3D11RenderTargetView* GetRenderTargetView() const noexcept;
    ID3D11DepthStencilView* GetDepthStencilView() const noexcept;

private:
    Renderer() = default;

    void CreateBackBufferResources();
    void ReleaseBackBufferResources();

    Microsoft::WRL::ComPtr<IDXGIFactory1> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    D3D_FEATURE_LEVEL m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };
    UINT m_width{ 0 };
    UINT m_height{ 0 };
};

class Application {
public:
    Application(HINSTANCE instance, int showCommand);

    int Run();

private:
    void RegisterWindowClass();
    void CreateWindowInstance();
    void ProcessPendingMessages();
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_instance;
    HWND m_hwnd{ nullptr };
    UINT m_width{ 1280 };
    UINT m_height{ 720 };
    bool m_running{ true };
    bool m_vsync{ true };
    std::wstring m_windowClassName;
    LARGE_INTEGER m_frequency{};
    LARGE_INTEGER m_lastCounter{};
};

} // namespace CodexCraft::Platform::Windows

