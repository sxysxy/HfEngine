#include "G2DRenderer.h"
#include <shapes.h>
#include <DX\RenderPipeline.h>

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
        //float2 t = opt.pos.xy;    \n \
        //opt.pos.x = (cosa*t.x - sina*t.y); \n \
        //opt.pos.y = (sina*t.x + cosa*t.y); \n \
                                  \n \
        opt.tex = tex;          \n \
        return opt;             \n \
    }       \n";

static const char *draw_texture_ps = " \
    Texture2D color_map : register(t0);   \n \
    SamplerState color_sampler : register(s0); \n \
    float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET { return color_map.Sample(color_sampler, tex); }";
       

static const char *draw_texture_ps_ex = " \n \
    cbuffer params : register(b1) {       \n \
        float4 color_mod;                  \n \
        float4 tone_mod;                   \n \
    };                                    \n \
    Texture2D color_map : register(t0);   \n \
    SamplerState color_sampler : register(s0); \n \
    float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET { \
        float4 color = color_map.Sample(color_sampler, tex); \n \
        color *= color_mod;                                  \n \
        color += tone_mod;                                   \n \
        return color; \n \
    } \
";

struct DT_VS_Param {
    float sina, cosa;
    float unused[2];  //fill to 16 bytes
};

struct DT_PS_EX_Param {
    float color_mod[4], tone_mod[4];  //32 bytes in total
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
    draw_texture_cbuffer = ReferPtr<D3DConstantBuffer>::New(device.Get(), sizeof DT_VS_Param);
    draw_texture_sampler = ReferPtr<D3DSampler>::New();
    draw_texture_sampler->UseDefault();
    draw_texture_sampler->CreateState(device.Get());
    
    //Ex
    dt_pshader_ex = PixelShader::LoadCodeString(device.Get(), draw_texture_ps_ex);
    draw_texture_cbuffer1 = ReferPtr<D3DConstantBuffer>::New(device.Get(), sizeof DT_PS_EX_Param);

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
        {{ x1, y1, _z_depth },{ 0.0, 1.0 }},
    {{ x1, y2, _z_depth },{ 0.0, 0.0 }},
    {{ x2, y1, _z_depth },{ 1.0, 1.0 }},
    {{ x2, y2, _z_depth },{ 1.0, 0.0 }}
    };
    draw_texture->UpdateSubResource(draw_texture_vbuffer.Get(), vecs);
    draw_texture->BindVertexBuffer(0, draw_texture_vbuffer.Get(), 5 * sizeof(float));
    draw_texture->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    draw_texture->BindShaderResource(0, texture, SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderSampler(0, draw_texture_sampler.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->BindPixelShader(dt_pipeline->pshader.Get());      //Use normal
    draw_texture->Draw(0, 4);
   // draw_texture->ClearShaderResource(0, SHADERS_APPLYTO_PSHADER);
}

void Renderer::DrawTextureEx(const D3DTexture2D * texture, const Rect & rect, const Color & color_mod,
    const Color &tone_mod, float angle)
{
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
        {{ x1, y1, _z_depth },{ 0.0, 1.0 }},
    {{ x1, y2, _z_depth },{ 0.0, 0.0 } },
    {{ x2, y1, _z_depth }, { 1.0, 1.0 }},
    {{ x2, y2, _z_depth },{ 1.0, 0.0 }}
    };
    DT_VS_Param param = { sin(angle), cos(angle), {0.0, 0.0} };
    DT_PS_EX_Param param1 = {
        {color_mod.r, color_mod.g, color_mod.b, color_mod.a},
        {tone_mod.r, tone_mod.g, tone_mod.b, tone_mod.a},
    };
    draw_texture->UpdateSubResource(draw_texture_vbuffer.Get(), vecs);
    draw_texture->UpdateSubResource(draw_texture_cbuffer.Get(), &param);
    draw_texture->UpdateSubResource(draw_texture_cbuffer1.Get(), &param1);
    draw_texture->BindPixelShader(dt_pshader_ex.Get());  //Use ex
    draw_texture->BindVertexBuffer(0, draw_texture_vbuffer.Get(), 5 * sizeof(float));
    draw_texture->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    draw_texture->BindShaderConstantBuffer(0, draw_texture_cbuffer.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderConstantBuffer(1, draw_texture_cbuffer1.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderResource(0, texture, SHADERS_APPLYTO_PSHADER);
    draw_texture->BindShaderSampler(0, draw_texture_sampler.Get(), SHADERS_APPLYTO_PSHADER);
    draw_texture->Draw(0, 4);
   // draw_texture->ClearShaderResource(0, SHADERS_APPLYTO_PSHADER);
}

}

namespace Ext {
    namespace G2D {
        VALUE module_G2D;
        namespace Renderer {
            VALUE klass;
            
            void Delete(::G2D::Renderer *r) {
                r->SubRefer();
            }
            VALUE New(VALUE klass) {
                auto r = new ::G2D::Renderer;
                r->AddRefer();
                return Data_Wrap_Struct(klass, nullptr, Delete, r);
            }
            static VALUE initialize(VALUE self, VALUE device, VALUE wnd) {
                if (!rb_obj_is_kind_of(device, Ext::DX::D3DDevice::klass)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#initialize: The first param should be a D3DDevice");
                }
                if (!rb_obj_is_kind_of(wnd, Ext::HFWindow::klass)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#initialize: The second param should be a HFWindow");
                }
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                auto ddddd = GetNativeObject<D3DDevice>(device);
                auto window = GetNativeObject<Ext::HFWindow::RHFWindow>(wnd);
                renderer->Initialize(ddddd, window);
                return self;
            }
            static VALUE set_render_target(VALUE self, VALUE target) {
                if (!rb_obj_is_kind_of(target, Ext::DX::D3DTexture2D::klass)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#set_render_target : The param should be a D3DTexture2D");
                }
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                auto texture = GetNativeObject<D3DTexture2D>(target);
                renderer->SetRenderTarget(texture);
                return self;
            }

            //color, 要求是一个HFColor
            static VALUE clear_target(VALUE self, VALUE color) {
                //float c[4];
                float *p;
                if (rb_obj_is_kind_of(color, Ext::DX::klass_HFColor)) {
                    auto hc = GetNativeObject<Utility::Color>(color);
                    p = reinterpret_cast<float *>(hc);
                }
                else {
                    rb_raise(rb_eArgError, "G2D::Renderer#clear_target : The param color should be a HFColor");
                }
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                renderer->ClearTarget({p[0], p[1], p[2], p[3]});
                return self;
            }
                                                                    //rect
            static VALUE draw_texture(VALUE self, VALUE texture, VALUE r) {
                //
                if (!rb_obj_is_kind_of(texture, Ext::DX::D3DTexture2D::klass) || !rb_obj_is_kind_of(r, Ext::DX::klass_HFRect)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#draw_texture : please call draw_texture(texture, rect)");
                }
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                auto tex = GetNativeObject<::D3DTexture2D>(texture);
                auto rect = GetNativeObject<Utility::Rect>(r);
                renderer->DrawTexture(tex, *rect);
                return self;
            }
                                                //rect, color
            static VALUE fill_rect(VALUE self, VALUE r, VALUE c) {
                if (!rb_obj_is_kind_of(r, Ext::DX::klass_HFRect) || !rb_obj_is_kind_of(c, Ext::DX::klass_HFColor)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#fill_rect : please call draw_rect(rect, color)");
                }
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                auto color = GetNativeObject<Utility::Color>(c);
                auto rect = GetNativeObject<Utility::Rect>(r);
                renderer->FillRect(*rect, *color);
                return self;
            }

            static VALUE set_z_depth(VALUE self, VALUE z) {
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                renderer->SetZDepth((float)rb_float_value(z));
                return self;
            }

            static VALUE get_z_depth(VALUE self) {
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                return rb_float_new(renderer->z_depth);
            }

            static VALUE execute_render(VALUE self) {
                GetNativeObject<::G2D::Renderer>(self)->ExecuteRender();
                return self;
            }

            static VALUE set_viewport(VALUE self, VALUE r) {
                if (!rb_obj_is_kind_of(r, Ext::DX::klass_HFRect)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#set_viewport : the first param should be a HFRect to specific the viewport area");
                }
                auto rect = GetNativeObject<Utility::Rect>(r);
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                renderer->SetViewport(*rect);
                return self;
            }
            static VALUE use_default_viewport(VALUE self) {
                GetNativeObject<::G2D::Renderer>(self)->UseDefaultViewport();
                return self;
            }

            //DT, DR PS
            static VALUE set_DT_PS(VALUE self, VALUE ps) {
                if (!rb_obj_is_kind_of(ps, Ext::DX::Shader::klass_pshader)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#set_DT_PS : The param should be a PixelShader");
                }
                auto s = GetNativeObject<PixelShader>(ps);
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                renderer->SetDTPixelShader(s);
                return self;
            }
            static VALUE set_DR_PS(VALUE self, VALUE ps) {
                if (!rb_obj_is_kind_of(ps, Ext::DX::Shader::klass_pshader)) {
                    rb_raise(rb_eArgError, "G2D::Renderer#set_DT_PS : The param should be a PixelShader");
                }
                auto s = GetNativeObject<PixelShader>(ps);
                auto renderer = GetNativeObject<::G2D::Renderer>(self);
                renderer->SetDRPixelShader(s);
                return self;
            }
            static VALUE use_default_DR_PS(VALUE self) {
                GetNativeObject<::G2D::Renderer>(self)->UseDefaultDRPixelShader();
                return self;
            }
            static VALUE use_default_DT_PS(VALUE self) {
                GetNativeObject<::G2D::Renderer>(self)->UseDefaultDTPixelShader();
                return self;
            }

            void Init() {
                module_G2D = rb_define_module("G2D");
                klass = rb_define_class_under(module_G2D, "Renderer", rb_cObject);
                
                rb_define_alloc_func(klass, New);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 2);
                rb_define_method(klass, "set_render_target", (rubyfunc)set_render_target, 1);
                rb_define_method(klass, "clear_target", (rubyfunc)clear_target, 1);
                rb_define_method(klass, "draw_texture", (rubyfunc)draw_texture, 2);
                rb_define_method(klass, "fill_rect", (rubyfunc)fill_rect, 2);
                rb_define_method(klass, "set_z_depth", (rubyfunc)set_z_depth, 1);
                rb_alias(klass, rb_intern("z_depth="), rb_intern("set_z_depth"));
                rb_define_method(klass, "get_z_depth", (rubyfunc)get_z_depth, 0);
                rb_alias(klass, rb_intern("z_depth"), rb_intern("get_z_depth"));
                rb_define_method(klass, "execute_render", (rubyfunc)execute_render, 0);
                rb_define_method(klass, "set_viewport", (rubyfunc)set_viewport, 1);
                rb_define_method(klass, "use_default_viewport", (rubyfunc)use_default_viewport, 0);
                rb_define_method(klass, "set_DT_PS", (rubyfunc)set_DT_PS, 1);
                rb_define_method(klass, "set_DR_PS", (rubyfunc)set_DR_PS, 1);
                rb_define_method(klass, "use_default_DT_PS", (rubyfunc)use_default_DT_PS, 0);
                rb_define_method(klass, "use_default_DR_PS", (rubyfunc)use_default_DR_PS, 0);
                
            }
        }
    }
}