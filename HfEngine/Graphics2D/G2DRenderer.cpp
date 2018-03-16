#include "G2DRenderer.h"
#include <shapes.h>

namespace G2D {

    static const char *draw_rect_ps = " \
    float4 main(float4 pos : SV_POSITION, float4 color : COLOR) : SV_TARGET {\n \
        return color;                                                        \n \
    }                                                                        \n \
";

    static const char *draw_rect_vs = " \
    struct vs_output {             \n     \
        float4 pos : SV_POSITION;  \n     \
        float4 color : COLOR;      \n     \
    };                             \n     \
    vs_output main(float4 pos : POSITION, float4 color : COLOR) { \n  \
            vs_output opt;                                        \n  \
            opt.color = color;                                    \n  \
            opt.pos = pos;                                        \n  \
            return opt;                                           \n  \
    }                                                             \n  \
";

 static const char *draw_texture_vs = " \
    struct vs_output {            \n    \
        float4 pos : SV_POSITION; \n    \
        float2 tex : TEXCOORD;    \n    \
    };\
    cbuffer params : register(b0) { \n    \
        float sina, cosa;                 \
    }; \n                                 \
    vs_output main(float4 pos : POSITION, float2 tex : TEXCOORD) { \n \
        vs_output opt = (vs_output)0;                          \n \
        opt.pos = pos;            \n \
                                  \n \
        float2 t = opt.pos.xy;    \n \
        opt.pos.x = (cosa*t.x - sina*t.y); \n \
        opt.pos.y = (sina*t.x + cosa*t.y); \n \
                                  \n \
        opt.tex = tex;          \n \
        return opt;             \n \
    }       \n";

static const char *draw_texture_ps = " \
    Texture2D color_map : register(t0);   \n \
    SamplerState color_sampler : register(s0); \n \
    float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET { return color_map.Sample(color_sampler, tex); }";
       
struct DT_PS_Param {
    float sina, cosa;
    float unused[2];  //fill to 16 bytes
};

void Renderer::InitPipelines() {
    //draw_rect:
    draw_rect = ReferPtr<D3DDeviceContext>::New(device.Get());
    dr_pipeline = ReferPtr<RenderPipeline>::New();
    dr_pipeline->vshader = VertexShader::LoadCodeString(device.Get(), draw_rect_vs);
    dr_pipeline->pshader = PixelShader::LoadCodeString(device.Get(), draw_rect_ps);
    dr_pipeline->SetInputLayout(device.Get(),
        std::initializer_list<std::string>({ "POSITION", "COLOR" }).begin(),
        std::initializer_list<DXGI_FORMAT>({ DXGI_FORMAT_R32G32B32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT }).begin(),
        2);
    draw_rect->BindPipeline(dr_pipeline.Get());
    draw_rect_vbuffer = ReferPtr<D3DVertexBuffer>::New(device.Get(), 7 * sizeof(float) * 4);
    contexts.push_back(draw_rect);
    //draw_texture

    draw_texture = ReferPtr<D3DDeviceContext>::New(device.Get());
    dt_pipeline = ReferPtr<RenderPipeline>::New();
    dt_pipeline->vshader = VertexShader::LoadCodeString(device.Get(), draw_texture_vs);
    dt_pipeline->pshader = PixelShader::LoadCodeString(device.Get(), draw_texture_ps);
    dt_pipeline->SetInputLayout(device.Get(), 
        std::initializer_list<std::string>({"POSITION", "TEXCOORD"}).begin(),
        std::initializer_list<DXGI_FORMAT>({DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32_FLOAT}).begin(),
        2);
    draw_texture->BindPipeline(dt_pipeline.Get());
 
    draw_texture_vbuffer = ReferPtr<D3DVertexBuffer>::New(device.Get(), 5 * sizeof(float) * 4);
    draw_texture_cbuffer = ReferPtr<D3DConstantBuffer>::New(device.Get(), sizeof DT_PS_Param);
    draw_texture_sampler = ReferPtr<D3DSampler>::New();
    draw_texture_sampler->UseDefault();
    draw_texture_sampler->CreateState(device.Get());
    
    contexts.push_back(draw_texture);
}

void Renderer::FillRectWith4Colors(const Rect & rect, const std::initializer_list<Color>& colors) {
    //左下， 左上， 右下， 右上
    int midx = viewport.width / 2;
    int midy = viewport.height / 2;
#pragma warning(push)
#pragma warning(disable:4244)
    float x1 = 1.0f * (rect.x - midx) / midx;
    float x2 = 1.0f * (rect.x + rect.w - midx) / midx;
    float y2 = -1.0f * (rect.y - midy) / midy;
    float y1 = -1.0f * (rect.y + rect.h - midy) / midy;
#pragma warning(pop)

    struct ColoredVertex {
        float pos[3];
        float color[4];
    };
    const Color *color = colors.begin();
    const Color *check = colors.end();
    if (check - color != 4) {
        throw std::invalid_argument("Fill Rect Width four colors need four colors");
    }
    ColoredVertex vecs[4] = {
    { { x1, y1, _z_depth},{ color[0].r, color[0].g, color[0].b, color[0].a } },
    { { x1, y2, _z_depth},{ color[1].r, color[1].g, color[1].b, color[1].a } },
    { { x2, y1, _z_depth},{ color[2].r, color[2].g, color[2].b, color[2].a } },
    { { x2, y2, _z_depth},{ color[3].r, color[3].g, color[3].b, color[3].a } }
    };
    draw_rect->UpdateSubResource(draw_rect_vbuffer.Get(), vecs);
    draw_rect->BindVertexBuffer(0, draw_rect_vbuffer.Get(), 7 * sizeof(float));
    draw_rect->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    draw_rect->Draw(0, 4);
}

void Renderer::FillRect(const Rect & rect, const Color &color) {
    auto colors = {color, color, color, color};
    FillRectWith4Colors(rect, colors);
}

void Renderer::FillVerticalGradientRect(const Rect & rect, const Color & color1, const Color & color2) {

}

void Renderer::FillHorizontalGradientRect(const Rect & rect, const Color & color1, const Color & color2) {

}

void Renderer::DrawTexture(const D3DTexture2D * texture, const Rect & rect) {
    int midx = viewport.w / 2;
    int midy = viewport.h / 2;
#pragma warning(push)
#pragma warning(disable:4244)
    float x1 = 1.0 * (rect.x - midx) / midx;
    float x2 = 1.0 * (rect.x + rect.w - midx) / midx;
    float y2 = -1.0 * (rect.y - midy) / midy;
    float y1 = -1.0 * (rect.y + rect.h - midy) / midy;
#pragma warning(pop)
    struct VertexXXX {
        float pos[3];
        float tex[2];
    }vecs[] = {
        {x1, y1, _z_depth}, {0.0, 1.0},
        {x1, y2, _z_depth}, {0.0, 0.0},
        {x2, y1, _z_depth}, {1.0, 1.0},
        {x2, y2, _z_depth}, {1.0, 0.0}
    };
    DT_PS_Param param = {0.0, 1.0};
    draw_texture->UpdateSubResource(draw_texture_vbuffer.Get(), vecs);
    draw_texture->UpdateSubResource(draw_texture_cbuffer.Get(), &param);
    draw_texture->BindVertexBuffer(0, draw_texture_vbuffer.Get(), 5 * sizeof(float));
    draw_texture->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    draw_texture->BindShaderConstantBuffer(0, draw_texture_cbuffer.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderResource(0, texture, SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderSampler(0, draw_texture_sampler.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->Draw(0, 4);
}

}