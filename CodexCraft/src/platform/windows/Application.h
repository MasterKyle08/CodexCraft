#pragma once

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include <DirectXMath.h>
#include <string>

#include "../../camera/FreeFlyCamera.h"

namespace CodexCraft::Platform::Windows {

class Renderer {
public:
    static Renderer& Instance();

    void Initialize(HWND hwnd, UINT width, UINT height, bool enableDebugLayer = false);
    void Shutdown();
    void Resize(UINT width, UINT height);
    void BeginFrame(const float clearColor[4]);
    void EndFrame(bool vsync);

    void UpdateCameraConstants(DirectX::FXMMATRIX view,
                               DirectX::CXMMATRIX projection,
                               const DirectX::XMFLOAT3& cameraPosition);
    [[nodiscard]] ID3D11Buffer* GetCameraConstantBuffer() const noexcept;

    ID3D11Device* GetDevice() const noexcept;
    ID3D11DeviceContext* GetContext() const noexcept;
    IDXGISwapChain* GetSwapChain() const noexcept;
    ID3D11RenderTargetView* GetRenderTargetView() const noexcept;
    ID3D11DepthStencilView* GetDepthStencilView() const noexcept;

private:
    Renderer() = default;

    void CreateBackBufferResources();
    void ReleaseBackBufferResources();
    void CreateCameraConstantBuffer();

    Microsoft::WRL::ComPtr<IDXGIFactory1> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraConstantBuffer;
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
    void RegisterRawInputDevices();
    void ProcessPendingMessages();
    void UpdateSimulation(double deltaSeconds);
    void UpdateCamera(double deltaSeconds);
    void ApplyHeightConstraints();
    float QueryTerrainHeight(float worldX, float worldZ) const;
    void ResetInputDeltas();
    void HandleRawInput(LPARAM lParam);
    void OnKeyDown(WPARAM key, LPARAM lParam);
    void OnKeyUp(WPARAM key);
    void OnLostFocus();
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    struct InputState {
        bool forward{ false };
        bool backward{ false };
        bool left{ false };
        bool right{ false };
        bool ascend{ false };
        bool descend{ false };
        bool rollLeft{ false };
        bool rollRight{ false };
        bool boost{ false };
        LONG mouseDeltaX{ 0 };
        LONG mouseDeltaY{ 0 };

        void ResetDeltas() noexcept {
            mouseDeltaX = 0;
            mouseDeltaY = 0;
        }

        void Clear() noexcept {
            forward = backward = left = right = ascend = descend = rollLeft = rollRight = boost = false;
            ResetDeltas();
        }
    };

    HINSTANCE m_instance;
    HWND m_hwnd{ nullptr };
    UINT m_width{ 1280 };
    UINT m_height{ 720 };
    bool m_running{ true };
    bool m_vsync{ true };
    std::wstring m_windowClassName;
    LARGE_INTEGER m_frequency{};
    LARGE_INTEGER m_lastCounter{};
    Camera::FreeFlyCamera m_camera;
    InputState m_input;
    float m_mouseSensitivity{ 0.0015f };
    float m_rollSpeed{ DirectX::XMConvertToRadians(45.0f) };
    float m_eyeHeight{ 1.8f };
};

} // namespace CodexCraft::Platform::Windows

