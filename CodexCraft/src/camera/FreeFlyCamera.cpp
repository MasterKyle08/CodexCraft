#include "FreeFlyCamera.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace CodexCraft::Camera {

namespace {
constexpr float kDefaultFov = XMConvertToRadians(70.0f);
constexpr float kDefaultAspect = 16.0f / 9.0f;
constexpr float kDefaultNear = 0.1f;
constexpr float kDefaultFar = 4096.0f;
constexpr float kDefaultSpeed = 16.0f;
constexpr float kMinPitchLimit = -XM_PIDIV2 + XMConvertToRadians(1.0f);
constexpr float kMaxPitchLimit = XM_PIDIV2 - XMConvertToRadians(1.0f);

[[nodiscard]] XMMATRIX RotationMatrix(float pitch, float yaw, float roll) noexcept {
    return XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
}

[[nodiscard]] XMFLOAT3 TransformDirection(const XMMATRIX& rotation, const XMFLOAT3& direction) noexcept {
    const XMVECTOR dir = XMVector3TransformNormal(XMLoadFloat3(&direction), rotation);
    XMFLOAT3 result{};
    XMStoreFloat3(&result, dir);
    return result;
}

} // namespace

FreeFlyCamera::FreeFlyCamera()
    : m_position(0.0f, 300.0f, 0.0f)
    , m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_roll(0.0f)
    , m_movementSpeed(kDefaultSpeed)
    , m_minPitch(kMinPitchLimit)
    , m_maxPitch(kMaxPitchLimit)
    , m_view{}
    , m_projection{}
    , m_viewProjection{}
    , m_viewDirty(true)
    , m_projectionDirty(true)
    , m_fovY(kDefaultFov)
    , m_aspectRatio(kDefaultAspect)
    , m_nearPlane(kDefaultNear)
    , m_farPlane(kDefaultFar) {}

void FreeFlyCamera::SetPosition(const XMFLOAT3& position) noexcept {
    m_position = position;
    InvalidateView();
}

void FreeFlyCamera::SetYaw(float radians) noexcept {
    m_yaw = radians;
    InvalidateView();
}

void FreeFlyCamera::SetPitch(float radians) noexcept {
    m_pitch = radians;
    ClampPitch();
    InvalidateView();
}

void FreeFlyCamera::SetRoll(float radians) noexcept {
    m_roll = radians;
    InvalidateView();
}

void FreeFlyCamera::AddYaw(float radians) noexcept {
    m_yaw += radians;
    InvalidateView();
}

void FreeFlyCamera::AddPitch(float radians) noexcept {
    m_pitch += radians;
    ClampPitch();
    InvalidateView();
}

void FreeFlyCamera::AddRoll(float radians) noexcept {
    m_roll += radians;
    InvalidateView();
}

void FreeFlyCamera::SetPerspective(float fovYRadians, float aspectRatio, float nearPlane, float farPlane) noexcept {
    m_fovY = fovYRadians;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionDirty = true;
}

XMMATRIX FreeFlyCamera::GetViewMatrix() const noexcept {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }

    return XMLoadFloat4x4(&m_view);
}

XMMATRIX FreeFlyCamera::GetProjectionMatrix() const noexcept {
    if (m_projectionDirty) {
        const XMMATRIX projection = XMMatrixPerspectiveFovLH(m_fovY, m_aspectRatio, m_nearPlane, m_farPlane);
        XMStoreFloat4x4(&m_projection, projection);
        m_projectionDirty = false;
    }

    return XMLoadFloat4x4(&m_projection);
}

XMMATRIX FreeFlyCamera::GetViewProjectionMatrix() const noexcept {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }
    if (m_projectionDirty) {
        const XMMATRIX projection = XMMatrixPerspectiveFovLH(m_fovY, m_aspectRatio, m_nearPlane, m_farPlane);
        XMStoreFloat4x4(&m_projection, projection);
        m_projectionDirty = false;
    }

    const XMMATRIX view = XMLoadFloat4x4(&m_view);
    const XMMATRIX projection = XMLoadFloat4x4(&m_projection);
    const XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
    XMStoreFloat4x4(&m_viewProjection, viewProjection);
    return XMLoadFloat4x4(&m_viewProjection);
}

XMFLOAT3 FreeFlyCamera::GetForwardVector() const noexcept {
    const XMMATRIX rotation = RotationMatrix(m_pitch, m_yaw, m_roll);
    static constexpr XMFLOAT3 kForward{ 0.0f, 0.0f, 1.0f };
    return TransformDirection(rotation, kForward);
}

XMFLOAT3 FreeFlyCamera::GetRightVector() const noexcept {
    const XMMATRIX rotation = RotationMatrix(m_pitch, m_yaw, m_roll);
    static constexpr XMFLOAT3 kRight{ 1.0f, 0.0f, 0.0f };
    return TransformDirection(rotation, kRight);
}

XMFLOAT3 FreeFlyCamera::GetUpVector() const noexcept {
    const XMMATRIX rotation = RotationMatrix(m_pitch, m_yaw, m_roll);
    static constexpr XMFLOAT3 kUp{ 0.0f, 1.0f, 0.0f };
    return TransformDirection(rotation, kUp);
}

void FreeFlyCamera::Translate(const XMFLOAT3& delta) noexcept {
    m_position.x += delta.x;
    m_position.y += delta.y;
    m_position.z += delta.z;
    InvalidateView();
}

void FreeFlyCamera::Translate(FXMVECTOR delta) noexcept {
    XMVECTOR position = XMLoadFloat3(&m_position);
    position = XMVectorAdd(position, delta);
    XMStoreFloat3(&m_position, position);
    InvalidateView();
}

void FreeFlyCamera::SetPitchLimits(float minPitchRadians, float maxPitchRadians) noexcept {
    m_minPitch = minPitchRadians;
    m_maxPitch = maxPitchRadians;
    ClampPitch();
}

void FreeFlyCamera::ClampPitch() noexcept {
    m_pitch = std::clamp(m_pitch, m_minPitch, m_maxPitch);
}

void FreeFlyCamera::InvalidateView() noexcept {
    m_viewDirty = true;
}

void FreeFlyCamera::UpdateViewMatrix() const noexcept {
    const XMMATRIX rotation = RotationMatrix(m_pitch, m_yaw, m_roll);
    const XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation);
    const XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation);
    const XMVECTOR position = XMLoadFloat3(&m_position);
    const XMMATRIX view = XMMatrixLookToLH(position, forward, up);
    XMStoreFloat4x4(&m_view, view);
    m_viewDirty = false;
}

} // namespace CodexCraft::Camera

