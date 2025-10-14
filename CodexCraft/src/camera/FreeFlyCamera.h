#pragma once

#include <DirectXMath.h>

namespace CodexCraft::Camera {

class FreeFlyCamera {
public:
    FreeFlyCamera();

    void SetPosition(const DirectX::XMFLOAT3& position) noexcept;
    [[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const noexcept { return m_position; }

    void SetYaw(float radians) noexcept;
    void SetPitch(float radians) noexcept;
    void SetRoll(float radians) noexcept;

    void AddYaw(float radians) noexcept;
    void AddPitch(float radians) noexcept;
    void AddRoll(float radians) noexcept;

    [[nodiscard]] float GetYaw() const noexcept { return m_yaw; }
    [[nodiscard]] float GetPitch() const noexcept { return m_pitch; }
    [[nodiscard]] float GetRoll() const noexcept { return m_roll; }

    void SetMovementSpeed(float unitsPerSecond) noexcept { m_movementSpeed = unitsPerSecond; }
    [[nodiscard]] float GetMovementSpeed() const noexcept { return m_movementSpeed; }

    void SetPerspective(float fovYRadians, float aspectRatio, float nearPlane, float farPlane) noexcept;

    [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const noexcept;
    [[nodiscard]] DirectX::XMMATRIX GetProjectionMatrix() const noexcept;
    [[nodiscard]] DirectX::XMMATRIX GetViewProjectionMatrix() const noexcept;

    [[nodiscard]] DirectX::XMFLOAT3 GetForwardVector() const noexcept;
    [[nodiscard]] DirectX::XMFLOAT3 GetRightVector() const noexcept;
    [[nodiscard]] DirectX::XMFLOAT3 GetUpVector() const noexcept;

    void Translate(const DirectX::XMFLOAT3& delta) noexcept;
    void Translate(DirectX::FXMVECTOR delta) noexcept;

    void SetPitchLimits(float minPitchRadians, float maxPitchRadians) noexcept;

private:
    void ClampPitch() noexcept;
    void InvalidateView() noexcept;
    void UpdateViewMatrix() const noexcept;

    DirectX::XMFLOAT3 m_position;
    float m_yaw;
    float m_pitch;
    float m_roll;
    float m_movementSpeed;
    float m_minPitch;
    float m_maxPitch;

    mutable DirectX::XMFLOAT4X4 m_view;
    mutable DirectX::XMFLOAT4X4 m_projection;
    mutable DirectX::XMFLOAT4X4 m_viewProjection;
    mutable bool m_viewDirty;
    mutable bool m_projectionDirty;

    float m_fovY;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
};

} // namespace CodexCraft::Camera

