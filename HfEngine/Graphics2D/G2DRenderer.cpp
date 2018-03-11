#include "G2DRenderer.h"
#include <shapes.h>

namespace G2D {

static const char *draw_rect_ps = " \
    float4 main(float4 pos : SV_POSITION, float4 color : COLOR) : SV_TARGET { \
        return color;                                                         \
    }                                                                         \
";

static const char *draw_rect_vs = " \
    struct vs_output {              \
        float4 pos : SV_POSITION;       \
        float4 color : COLOR;           \
    };                                  \
    vs_output main(float4 pos : POSITION, float4 color : COLOR) {   \
            vs_output opt;                                          \
            opt.pos.xy = pos.xy;                                    \
            opt.pos.z = opt.pos.w = 0;                              \
            opt.color = color;                                      \
            return opt;                                             \
    }                                                               \
";

void Renderer::InitPipelines() {
    //draw_rect:
    draw_rect = ReferPtr<D3DDeviceContext>::New(device.Get());
    draw_rect->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    auto dr_pipeline = ReferPtr<RenderPipeline>::New();
    dr_pipeline->vshader = VertexShader::LoadCodeString(device.Get(), draw_rect_vs);
    dr_pipeline->pshader = PixelShader::LoadCodeString(device.Get(), draw_rect_ps);
    dr_pipeline->SetInputLayout(device.Get(),
        std::initializer_list<std::string>({ "POSITION", "COLOR" }).begin(),
        std::initializer_list<DXGI_FORMAT>({ DXGI_FORMAT_R32G32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT }).begin(),
        2);
    draw_rect->BindPipeline(dr_pipeline.Get());
    draw_rect_vbuffer = ReferPtr<D3DVertexBuffer>::New(device.Get(), 6 * sizeof(float) * 4);
    draw_rect->BindVertexBuffer(0, draw_rect_vbuffer.Get(), 6 * sizeof(float));
   
    //draw_texture

    contexts.push_back(draw_rect);
    //contexts.push_back(draw_texture);
}

void Renderer::FillRectWith4Colors(const Rect & rect, const std::initializer_list<Color>& colors) {
    //左下， 左上， 右下， 右上
    int midx = viewport.width / 2;
    int midy = viewport.height / 2;
    float x1 = 1.0f * (rect.x - midx) / midx;
    float x2 = 1.0f * (rect.x + rect.w - midx) / midx;
    float y2 = -1.0f * (rect.y - midy) / midy;
    float y1 = -1.0f * (rect.y + rect.h - midy) / midy;

    struct ColoredVertex {
        float pos[2], color[4];
    };
    const Color *color = colors.begin();
    const Color *check = colors.end();
    if (check - color != 4) {
        throw std::invalid_argument("Fill Rect Width four colors need four colors");
    }
    ColoredVertex vecs[4] = {
    { { x1, y1},{ color[0].r, color[0].g, color[0].b, color[0].a } },
    { { x1, y2},{ color[1].r, color[1].g, color[1].b, color[1].a } },
    { { x2, y1},{ color[2].r, color[2].g, color[2].b, color[2].a } },
    { { x2, y2},{ color[3].r, color[3].g, color[3].b, color[3].a } }
    };
    draw_rect->UpdateSubResource(draw_rect_vbuffer.Get(), vecs);
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

}