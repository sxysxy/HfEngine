#pragma once
#include <ThirdParties.h>
#include <DirectXMath.h>

HFENGINE_NAMESPACE_BEGIN
class Transform {
public:
    DirectX::XMFLOAT4X4 matrix;
    Transform() {
        memset(&matrix, 0, sizeof(matrix));
        matrix.m[0][0] = matrix.m[1][1] = matrix.m[2][2] = matrix.m[3][3] = 1.0f;
    }
    
    float* data() {
        return reinterpret_cast<float*>(matrix.m);
    }
    const float* data() const{
        return reinterpret_cast<const float*>(matrix.m);
    }

    Transform& translate(float tx, float ty, float tz) {
        DirectX::XMFLOAT4X4 t;
        memset(&t, 0, sizeof(t));
        t.m[0][0] = t.m[1][1] = t.m[2][2] = t.m[3][3] = 1.0f;
        t.m[3][0] = tx;
        t.m[3][1] = ty;
        t.m[3][2] = tz;
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        auto trans = DirectX::XMLoadFloat4x4(&t);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
    Transform& scale(float sx, float sy, float sz) {
        DirectX::XMFLOAT4X4 t;
        memset(&t, 0, sizeof(t));
        t.m[0][0] = sx;
        t.m[1][1] = sy;
        t.m[2][2] = sz;
        t.m[3][3] = 1.0f;
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        auto trans = DirectX::XMLoadFloat4x4(&t);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
    Transform& rotate(const float* axis, float angle) {
        auto trans = DirectX::XMMatrixRotationAxis({axis[0], axis[1], axis[2]}, angle);
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
    Transform& view(const float* eye_pos, const float* target_pos, const float* up_dir) {
        auto trans = DirectX::XMMatrixLookAtLH({ eye_pos[0], eye_pos[1], eye_pos[2] },
            { target_pos[0], target_pos[1], target_pos[2] },
            { up_dir[0], up_dir[1], up_dir[1] });
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
    Transform& perspective(float fov_angle, float aspect_ratio, float z_near, float z_far) {
        auto trans = DirectX::XMMatrixPerspectiveFovLH(fov_angle, aspect_ratio, z_near, z_far);
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
    Transform& ortho(const float* mins, const float* maxs) {
        auto trans = DirectX::XMMatrixOrthographicOffCenterLH(mins[0], maxs[0], 
            mins[1], maxs[1], 
            mins[2], maxs[2]);
        auto cur = DirectX::XMLoadFloat4x4(&matrix);
        cur *= trans;
        DirectX::XMStoreFloat4x4(&matrix, cur);
        return *this;
    }
};

extern thread_local RClass* ClassTransform;
bool InjectTransformExtension();

HFENGINE_NAMESPACE_END