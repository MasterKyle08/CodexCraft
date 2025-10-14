#include "Application.h"

#include <chrono>
#include <stdexcept>
#include <string>
#include <system_error>

#include <dxgi1_2.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace CodexCraft::Platform::Windows {

namespace {

constexpr wchar_t kWindowClassName[] = L"CodexCraft::Application";

void ThrowIfFailed(HRESULT hr, const char* message) {
    if (FAILED(hr)) {
        throw std::system_error(hr, std::system_category(), message);
    }
}

} // namespace

Renderer& Renderer::Instance() {
    static Renderer instance;
    return instance;
}

void Renderer::Initialize(HWND hwnd, UINT width, UINT height, bool enableDebugLayer) {
    m_width = width;
    m_height = height;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    if (enableDebugLayer) {
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#else
    (void)enableDebugLayer;
#endif

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)), "Failed to create DXGI factory");

    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    DXGI_SWAP_CHAIN_DESC swapDesc{};
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = 2;
    swapDesc.OutputWindow = hwnd;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        &swapDesc,
        &m_swapChain,
        &m_device,
        &m_featureLevel,
        &m_context);

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            creationFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapDesc,
            &m_swapChain,
            &m_device,
            &m_featureLevel,
            &m_context);
    }

    ThrowIfFailed(hr, "Failed to create D3D11 device and swap chain");

    CreateBackBufferResources();
}

void Renderer::Shutdown() {
    if (!m_device) {
        return;
    }

    ReleaseBackBufferResources();

    if (m_context) {
        m_context->ClearState();
    }

    m_context.Reset();
    m_swapChain.Reset();
    m_device.Reset();
    m_factory.Reset();
}

void Renderer::Resize(UINT width, UINT height) {
    if (!m_swapChain) {
        return;
    }

    if (width == 0 || height == 0) {
        return;
    }

    if (width == m_width && height == m_height) {
        return;
    }

    m_width = width;
    m_height = height;

    ReleaseBackBufferResources();

    ThrowIfFailed(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0),
                  "Failed to resize swap chain buffers");

    CreateBackBufferResources();
}

void Renderer::BeginFrame(const float clearColor[4]) {
    if (!m_renderTargetView || !m_depthStencilView) {
        return;
    }

    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::EndFrame(bool vsync) {
    if (!m_swapChain) {
        return;
    }

    m_swapChain->Present(vsync ? 1 : 0, 0);
}

ID3D11Device* Renderer::GetDevice() const noexcept {
    return m_device.Get();
}

ID3D11DeviceContext* Renderer::GetContext() const noexcept {
    return m_context.Get();
}

IDXGISwapChain* Renderer::GetSwapChain() const noexcept {
    return m_swapChain.Get();
}

ID3D11RenderTargetView* Renderer::GetRenderTargetView() const noexcept {
    return m_renderTargetView.Get();
}

ID3D11DepthStencilView* Renderer::GetDepthStencilView() const noexcept {
    return m_depthStencilView.Get();
}

void Renderer::CreateBackBufferResources() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), "Failed to access back buffer");
    ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView),
                  "Failed to create render target view");

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = m_width;
    depthDesc.Height = m_height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ThrowIfFailed(m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencil),
                  "Failed to create depth stencil buffer");
    ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, &m_depthStencilView),
                  "Failed to create depth stencil view");

    ID3D11RenderTargetView* rtvs[] = { m_renderTargetView.Get() };
    m_context->OMSetRenderTargets(1, rtvs, m_depthStencilView.Get());

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &viewport);
}

void Renderer::ReleaseBackBufferResources() {
    if (m_context) {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_depthStencil.Reset();
}

Application::Application(HINSTANCE instance, int showCommand)
    : m_instance(instance) {
    m_windowClassName = kWindowClassName;

    RegisterWindowClass();
    CreateWindowInstance();

    ShowWindow(m_hwnd, showCommand);
    UpdateWindow(m_hwnd);

    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastCounter);
}

int Application::Run() {
    auto& renderer = Renderer::Instance();
    renderer.Initialize(m_hwnd, m_width, m_height);

    MSG msg{};
    while (m_running) {
        ProcessPendingMessages();

        if (!m_running) {
            break;
        }

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        const double deltaSeconds = static_cast<double>(now.QuadPart - m_lastCounter.QuadPart) /
                                    static_cast<double>(m_frequency.QuadPart);
        m_lastCounter = now;
        (void)deltaSeconds; // Available for simulation & rendering systems.

        const float clearColor[4] = { 0.07f, 0.07f, 0.12f, 1.0f };
        renderer.BeginFrame(clearColor);

        // Hook for debug overlays / ImGui integration can be inserted here.

        renderer.EndFrame(m_vsync);
    }

    renderer.Shutdown();

    return static_cast<int>(msg.wParam);
}

void Application::RegisterWindowClass() {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = &Application::WndProc;
    windowClass.hInstance = m_instance;
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = m_windowClassName.c_str();
    windowClass.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&windowClass)) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed to register window class");
    }
}

void Application::CreateWindowInstance() {
    RECT desiredRect{ 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRectEx(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE, 0);

    const LONG width = desiredRect.right - desiredRect.left;
    const LONG height = desiredRect.bottom - desiredRect.top;

    m_hwnd = CreateWindowExW(
        0,
        m_windowClassName.c_str(),
        L"CodexCraft",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        m_instance,
        this);

    if (!m_hwnd) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed to create application window");
    }
}

void Application::ProcessPendingMessages() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            m_running = false;
            return;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT Application::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        m_running = false;
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            Renderer::Instance().Resize(m_width, m_height);
        }
        return 0;
    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 320;
        mmi->ptMinTrackSize.y = 240;
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;
    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Application* application = nullptr;

    if (msg == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        application = static_cast<Application*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
        application->m_hwnd = hwnd;
    } else {
        application = reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (application) {
        return application->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace CodexCraft::Platform::Windows

