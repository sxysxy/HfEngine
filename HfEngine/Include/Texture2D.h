#include "stdafx.h"
#include "DX.h"

class Texture2D {
public:
    ComPtr<ID3D11Texture2D> native_texture2d;
    ComPtr<ID3D11ShaderResourceView> native_shader_resource_view;
    ComPtr<ID3D11RenderTargetView> native_render_target_view;
};