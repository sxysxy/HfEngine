#include "RenderPipeline.h"
#include "RenderPipelineM.h"

void RenderPipeline::SetInputLayout(D3DDevice *device, const std::string *idents, 
                                           const DXGI_FORMAT *formats, int count) {
#ifdef _DEBUG
    if (!vshader) {
        throw std::runtime_error("No vertex shader provided, you can not set input layout now");
    }
#endif
    std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
    for (int i = 0; i < count; i++) {
        ied.push_back(D3D11_INPUT_ELEMENT_DESC{ idents[i].c_str(), 0, formats[i], 0,
            D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 });
    }
    ComPtr<ID3D11InputLayout> native_input_layout;
    HRESULT hr = S_FALSE;
    if (FAILED(hr = device->native_device->CreateInputLayout(ied.data(), count, vshader->byte_code->GetBufferPointer(),
        vshader->byte_code->GetBufferSize(), &native_input_layout))) {
        MAKE_ERRMSG<std::runtime_error>("Fail to Create InputLayout, Error code:", hr);
    }   
    
    native_context->IASetInputLayout(native_input_layout.Get());
}

void InputLayout::Initialize(D3DDevice *device, const std::string *idents,
    const DXGI_FORMAT *formats, int count, VertexShader *vs) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
    for (int i = 0; i < count; i++) {
        ied.push_back(D3D11_INPUT_ELEMENT_DESC{ idents[i].c_str(), 0, formats[i], 0,
            D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 });
    }
    HRESULT hr = S_FALSE;
    if (FAILED(hr = device->native_device->CreateInputLayout(ied.data(), count, vs->byte_code->GetBufferPointer(),
        vs->byte_code->GetBufferSize(), &native_input_layout))) {
        MAKE_ERRMSG<std::runtime_error>("Fail to Create InputLayout, Error code:", hr);
    }
}

void RenderPipeline::SetVertexBuffer(VertexBuffer *vb) {
    vbuffer = vb;
    UINT stride = vb->size_per_vertex;
    UINT offset = 0;
    native_context->IASetVertexBuffers(0, 1, vbuffer->native_buffer.GetAddressOf(), &stride, &offset);
}
void RenderPipeline::SetIndexBuffer(IndexBuffer *ib) {
    native_context->IASetIndexBuffer(ib->native_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

static ID3D11SamplerState * empty_sampler[1] = { nullptr };
static ID3D11Buffer * const empty_buffer[1] = { nullptr };
static ID3D11ShaderResourceView * const empty_shader_resource[1] = { nullptr };

//VS
void RenderPipeline::SetVertexShader(VertexShader *vs) {
    vshader = vs;
    native_context->VSSetShader(vs ? vs->native_vshader.Get() : nullptr, 0, 0);
}
void RenderPipeline::SetVSSampler(int slot, Sampler * sampler) {
    native_context->VSSetSamplers(slot, 1, sampler ? sampler->native_sampler.GetAddressOf() : empty_sampler);
}
void RenderPipeline::SetVSCBuffer(int slot, ConstantBuffer * cbuffer) {
    native_context->VSSetConstantBuffers(slot, 1, cbuffer ? cbuffer->native_buffer.GetAddressOf() : empty_buffer);
}
void RenderPipeline::SetVSResource(int slot, Texture2D *tex) {
    native_context->VSSetShaderResources(slot, 1, tex ? tex->native_shader_resource_view.GetAddressOf() : empty_shader_resource);
}

//PS
void RenderPipeline::SetPixelShader(PixelShader *ps) {
    pshader = ps;
    native_context->PSSetShader(ps ? ps->native_pshader.Get() : nullptr, 0, 0);
}
void RenderPipeline::SetPSSampler(int slot, Sampler * sampler) {
    native_context->PSSetSamplers(slot, 1, sampler ? sampler->native_sampler.GetAddressOf() : empty_sampler);
}
void RenderPipeline::SetPSCBuffer(int slot, ConstantBuffer * cbuffer) {
    native_context->PSSetConstantBuffers(slot, 1, cbuffer ? cbuffer->native_buffer.GetAddressOf() : empty_buffer);
}
void RenderPipeline::SetPSResource(int slot, Texture2D *tex) {
    native_context->PSSetShaderResources(slot, 1, tex ? tex->native_shader_resource_view.GetAddressOf() : empty_shader_resource);
}

//GS
void RenderPipeline::SetGeometryShader(GeometryShader *gs) {
    gshader = gs;
    native_context->GSSetShader(gs ? gs->native_gshader.Get() : nullptr, 0, 0);
}
void RenderPipeline::SetGSCBuffer(int slot, ConstantBuffer *cb) {
    native_context->GSSetConstantBuffers(slot, 1, cb ? cb->native_buffer.GetAddressOf() : empty_buffer);
}
void RenderPipeline::SetGSResource(int slot, Texture2D * tex) {
    native_context->GSSetShaderResources(slot, 1, tex ? tex->native_shader_resource_view.GetAddressOf() : empty_shader_resource);
}
void RenderPipeline::SetGSSampler(int slot, Sampler * sampler) {
    native_context->GSSetSamplers(slot, 1, sampler ? sampler->native_sampler.GetAddressOf() : empty_sampler);
}

//OM
void RenderPipeline::SetBlender(Blender * b) {
    blender = b;
    if(b)
    native_context->OMSetBlendState(b->native_blender.Get(), 
        reinterpret_cast<const FLOAT *>(&b->blend_factor), 0xffffffff);
    else native_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

//RS
void RenderPipeline::SetViewport(const Utility::Rect &rect, float min_deep, float max_deep) {
    D3D11_VIEWPORT vp{ (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, min_deep, max_deep };
    native_context->RSSetViewports(1, &vp);
}
void RenderPipeline::SetRasterizer(Rasterizer *rs) {
    rasterizer = rs;
    native_context->RSSetState(rs ? rs->native_rasterizer.Get() : nullptr);
}
void RenderPipeline::SetScissorRect(const Utility::Rect &rect) {
    D3D11_RECT r{rect.x, rect.y, rect.x+rect.w, rect.y+rect.h};
    native_context->RSSetScissorRects(1, &r);
}


//OM
void RenderPipeline::SetTarget(RTT *rtt) {
    rtt_target = rtt;
    native_context->OMSetRenderTargets(1, rtt->native_rtt_view.GetAddressOf(),
         rtt->native_stencil_view.Get());
}

void RenderPipeline::ImmdiateRender() {
    ID3D11CommandList *list;
    native_context->FinishCommandList(true, &list);
    device->native_immcontext->ExecuteCommandList(list, false);
    list->Release();
}

void RenderPipeline::ImmdiateCopy2D(Texture2D *dest, Texture2D *src, 
    const Utility::Rect &dest_rect, const Utility::Rect &src_rect) {
    D3D11_BOX box;
    box.back = box.back = 0.0;
    box.top = src_rect.y;
    box.left = src_rect.x;
    box.right = box.left + src_rect.width;
    box.bottom = box.top + src_rect.height;
    device->native_immcontext->CopySubresourceRegion(dest->native_texture2d.Get(), 0, dest_rect.x, dest_rect.y, 0.0, 
            src->native_texture2d.Get(), 0, &box);
}


namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            VALUE klass;
            VALUE klass_input_layout;

            static void delete_rp(::RenderPipeline *rp) {
                rp->SubRefer();
            }

            static VALUE new_rp(VALUE klass) {
                auto p = new ::RenderPipeline;
                p->AddRefer();
                return Data_Wrap_Struct(klass, nullptr, delete_rp, p);
            }

            static VALUE initialize(VALUE self, VALUE device) {
                if (!rb_obj_is_kind_of(device, Ext::DX::D3DDevice::klass)) {
                    rb_raise(rb_eArgError, "RenderPipeline::initialize : param should be a D3DDevice");
                    return Qnil;
                }
                auto d = GetNativeObject<::D3DDevice>(device);
                auto r = GetNativeObject<::RenderPipeline>(self);
                r->Initialize(d);
                rb_iv_set(self, "@device", device);
                return self;
            }

            static VALUE get_device(VALUE self) {
                return rb_iv_get(self, "@device");
            }

            static VALUE set_input_layout(VALUE self, VALUE names, VALUE fmts) {
                CheckArgs({ names, fmts }, {rb_cArray, rb_cArray});
                VALUE _device = rb_iv_get(self, "@device");
                VALUE *pnames = (VALUE *)RARRAY_PTR(names);
                VALUE *pfmts = (VALUE *)RARRAY_PTR(fmts);
                int len1 = RARRAY_LEN(names);
                int len2 = RARRAY_LEN(fmts);
                if (len1 != len2)
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: Array lengths do not match");
                std::vector<std::string> ns;
                std::vector<DXGI_FORMAT> fs;
                for (int i = 0; i < len1; i++) {
                    ns.push_back(rb_string_value_cstr(pnames + i));
                    fs.push_back((DXGI_FORMAT)FIX2INT(pfmts[i]));
                }
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto device = GetNativeObject<::D3DDevice>(_device);
                try {
                    rp->SetInputLayout(device, ns.data(), fs.data(), len1);
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }
            
            static VALUE set_vshader(VALUE self, VALUE s) {
                CheckArgs({ s }, {Shader::klass_vshader});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto sd = GetNULLPTRableNativeObject<::VertexShader>(s);
                rp->SetVertexShader(sd);
                rb_iv_set(self, "@vshader", s);
                return self;
            }
            static VALUE set_pshader(VALUE self, VALUE s) {
                CheckArgs({ s }, { Shader::klass_pshader });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto sd = GetNULLPTRableNativeObject<::PixelShader>(s);
                rp->SetPixelShader(sd);
                rb_iv_set(self, "@pshader", s);
                return self;
            }
            static VALUE set_gshader(VALUE self, VALUE s) {
                CheckArgs({ s }, { Shader::klass_gshader });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto sd = GetNULLPTRableNativeObject<::GeometryShader>(s);
                rp->SetGeometryShader(sd);
                rb_iv_set(self, "@gshader", s);
                return self;
            }
            static VALUE vshader(VALUE self) {
                return rb_iv_get(self, "@vshader");
            }
            static VALUE pshader(VALUE self) {
                return rb_iv_get(self, "@pshader");
            }
            static VALUE gshader(VALUE self) {
                return rb_iv_get(self, "@gshader");
            }

            static VALUE set_topology(VALUE self, VALUE t) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetTopology((D3D11_PRIMITIVE_TOPOLOGY)(FIX2INT(t)));
                return self;
            }
            static VALUE set_vbuffer(VALUE self, VALUE vb) {
                CheckArgs({ vb }, { D3DBuffer::klass_vbuffer });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetVertexBuffer(GetNativeObject<VertexBuffer>(vb));
                return self;
            }
            static VALUE set_ibuffer(VALUE self, VALUE ib) {
                CheckArgs({ ib }, { D3DBuffer::klass_ibuffer });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetIndexBuffer(GetNativeObject<IndexBuffer>(ib));
                return self;
            }


            //-----
            static VALUE set_vs_sampler(VALUE self, VALUE slot, VALUE s) {
                CheckArgs({ slot, s }, { rb_cInteger, ArgType(Shader::klass_sampler, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetVSSampler(FIX2INT(slot), GetNULLPTRableNativeObject<::Sampler>(s));
                return self;
            }
            static VALUE set_vs_cbuffer(VALUE self, VALUE slot, VALUE cb) {
                CheckArgs({ slot, cb }, {rb_cInteger, ArgType(D3DBuffer::klass_cbuffer, true)});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetVSCBuffer(FIX2INT(slot), GetNULLPTRableNativeObject<::ConstantBuffer>(cb));
                return self;
            }
            
            static VALUE set_vs_resource(VALUE self, VALUE slot, VALUE res) {
                CheckArgs({ slot, res }, {rb_cInteger, ArgType(Texture::klass_texture2d, true)});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetVSResource(FIX2INT(slot), GetNULLPTRableNativeObject<::Texture2D>(res));
                return self;
            }
            
            
            //-----
            static VALUE set_ps_sampler(VALUE self, VALUE slot, VALUE s) {
                CheckArgs({ slot, s }, { rb_cInteger, ArgType(Shader::klass_sampler, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetPSSampler(FIX2INT(slot), GetNULLPTRableNativeObject<::Sampler>(s));
                return self;
            }
            static VALUE set_ps_cbuffer(VALUE self, VALUE slot, VALUE cb) {
                CheckArgs({ slot, cb }, { rb_cInteger, ArgType(D3DBuffer::klass_cbuffer, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetPSCBuffer(FIX2INT(slot), GetNULLPTRableNativeObject<::ConstantBuffer>(cb));
                return self;
            }

            static VALUE set_ps_resource(VALUE self, VALUE slot, VALUE res) {
                CheckArgs({ slot, res }, { rb_cInteger, ArgType(Texture::klass_texture2d, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetPSResource(FIX2INT(slot), GetNULLPTRableNativeObject<::Texture2D>(res));
                return self;
            }
            
            // --------
            static VALUE set_gs_sampler(VALUE self, VALUE slot, VALUE s) {
                CheckArgs({ slot, s }, { rb_cInteger, ArgType(Shader::klass_sampler, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetGSSampler(FIX2INT(slot), GetNULLPTRableNativeObject<::Sampler>(s));
                return self;
            }
            static VALUE set_gs_cbuffer(VALUE self, VALUE slot, VALUE cb) {
                CheckArgs({ slot, cb }, { rb_cInteger, ArgType(D3DBuffer::klass_cbuffer, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetGSCBuffer(FIX2INT(slot), GetNULLPTRableNativeObject<::ConstantBuffer>(cb));
                return self;
            }

            static VALUE set_gs_resource(VALUE self, VALUE slot, VALUE res) {
                CheckArgs({ slot, res }, { rb_cInteger, ArgType(Texture::klass_texture2d, true) });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetGSResource(FIX2INT(slot), GetNULLPTRableNativeObject<::Texture2D>(res));
                return self;
            }
            //----

            static VALUE set_viewport(int argc, VALUE *argv, VALUE self) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                if (argc == 1) {
                    CheckArgs({argv[0]}, {DX::klass_HFRect});
                    auto rect = GetNativeObject<::Utility::Rect>(argv[0]);
                    rp->SetViewport(*rect);
                }
                else if (argc == 3) {
                    CheckArgs(3, argv, {DX::klass_HFRect, rb_cFloat, rb_cFloat});
                    auto rect = GetNativeObject<::Utility::Rect>(argv[0]);
                    rp->SetViewport(*rect, (float)rb_float_value(argv[1]), (float)rb_float_value(argv[2]));
                }
                else {
                    rb_raise(rb_eArgError, "RenderPipeline::set_viewport : expecting 1 or 3 arg but got %d", argc);
                }
                rb_iv_set(self, "@viewport", argv[0]);
                return self;
            }
            static VALUE get_viewport(VALUE self) {
                return rb_iv_get(self, "@viewport");
            }
            static VALUE set_target(VALUE self, VALUE tar) {
                CheckArgs({ tar }, {DX::Texture::klass_rtt});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetTarget(GetNativeObject<RTT>(tar));
                rb_iv_set(self, "@target", tar);
                return self;
            }
            static VALUE set_scissor_rect(VALUE self, VALUE r) {
                CheckArgs({ r }, {DX::klass_HFRect});
                GetNativeObject<::RenderPipeline>(self)->SetScissorRect(*GetNativeObject<Utility::Rect>(r));
                return self;
            }
            static VALUE get_target(VALUE self) {
                return rb_iv_get(self, "@target");
            }
            static VALUE set_blender(VALUE self, VALUE b) {
                CheckArgs({ b }, {ArgType(DX::Shader::klass_blender, true)});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetBlender(GetNULLPTRableNativeObject<Blender>(b));
                rb_iv_set(self, "@blender", b);
                return self;
            }
            static VALUE get_blender(VALUE self) {
                return rb_iv_get(self, "@blender");
            }
            static VALUE set_rasterizer(VALUE self, VALUE r) {
                CheckArgs({ r }, { ArgType{Shader::klass_rasterizer, true} });
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetRasterizer(GetNULLPTRableNativeObject<Rasterizer>(r));
                rb_iv_set(self, "@rasterizer", r);
                return self;
            }
            static VALUE get_rasterizer(VALUE self) {
                return rb_iv_get(self, "@rasterizer");
            }

            static VALUE clear_target(int argc, VALUE *argv, VALUE self) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                if(argc < 1 || argc > 2)rb_raise(rb_eArgError, "RenderPipeline::clear_target(color, [depth]): expecting(1..2) args but got %d", argc);
#ifdef _DEBUG
               if (!rb_obj_is_kind_of(argv[0], DX::klass_HFColor))
                   rb_raise(rb_eArgError, "RenderPipeline::clear_target(color, [depth]), First param should be a HFColorRGBA");
#endif
                auto color = GetNativeObject<Utility::Color>(argv[0]);
                if(argc == 2)
                    rp->Clear(*color, (float)rb_float_value(argv[1]));
                else rp->Clear(*color);
                return self;
            }
            static VALUE draw(VALUE self, VALUE s, VALUE c) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->Draw(FIX2INT(s), FIX2INT(c));
                return self;
            }
            static VALUE draw_index(VALUE self, VALUE s, VALUE c) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->DrawIndex(FIX2INT(s), FIX2INT(c));
                return self;
            }

            //---
            static VALUE immdiate_render(VALUE self) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->ImmdiateRender();
                return self;
            }
            static VALUE immdiate_copy2d(VALUE self, VALUE dest, VALUE src, VALUE dest_rect, VALUE src_rect) {
                CheckArgs({ dest, src, dest_rect, src_rect }, 
                    {DX::Texture::klass_texture2d, DX::Texture::klass_texture2d , DX::klass_HFRect, DX::klass_HFRect});
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->ImmdiateCopy2D(GetNativeObject<::Texture2D>(dest), GetNativeObject<::Texture2D>(dest),
                    *GetNativeObject<Utility::Rect>(dest_rect), *GetNativeObject<Utility::Rect>(src_rect));
                return self;
            }

            //--
            static VALUE update_subresource(VALUE self, VALUE buf, VALUE d) {
                if (!rb_obj_is_kind_of(buf, Ext::DX::D3DBuffer::klass))
                    rb_raise(rb_eArgError, "D3DDeviceContext::update_subresource: The first param should be a DX::D3DBuffer");
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto buffer = GetNativeObject<::D3DBuffer>(buf);
                if (rb_obj_is_kind_of(d, rb_cString)) {
                    void *ptr = rb_string_value_ptr(&d);
                    rp->UpdateSubResource(buffer, ptr);
                }
                else if (rb_obj_is_kind_of(d, rb_cInteger)) {
                    rp->UpdateSubResource(buffer, (void*)FIX2PTR(d));
                }
                return self;
            }

            //-------
            static VALUE LY_initialize(VALUE self, VALUE names, VALUE fmts, VALUE vs) {
                return self;
            }

            void Init() {
                klass = rb_define_class_under(module, "RenderPipeline", rb_cObject);
                rb_include_module(klass, module_release);

                rb_define_alloc_func(klass, new_rp);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 1);
                rb_define_method(klass, "device", (rubyfunc)get_device, 0);
                rb_define_method(klass, "set_input_layout", (rubyfunc)set_input_layout, 2);
                rb_define_method(klass, "set_vbuffer", (rubyfunc)set_vbuffer, 1);
                rb_define_method(klass, "set_ibuffer", (rubyfunc)set_ibuffer, 1);

                //vs
                rb_define_method(klass, "set_vshader", (rubyfunc)set_vshader, 1);
                rb_define_method(klass, "vshader", (rubyfunc)vshader, 0);
                rb_define_method(klass, "set_vs_sampler", (rubyfunc)set_vs_sampler, 2);
                rb_define_method(klass, "set_vs_cbuffer", (rubyfunc)set_vs_cbuffer, 2);
                rb_define_method(klass, "set_vs_resource", (rubyfunc)set_vs_resource, 2);

                //ps
                rb_define_method(klass, "set_pshader", (rubyfunc)set_pshader, 1);
                rb_define_method(klass, "pshader", (rubyfunc)pshader, 0);
                rb_define_method(klass, "set_ps_sampler", (rubyfunc)set_ps_sampler, 2);
                rb_define_method(klass, "set_ps_cbuffer", (rubyfunc)set_ps_cbuffer, 2);
                rb_define_method(klass, "set_ps_resource", (rubyfunc)set_ps_resource, 2);

                //gs
                rb_define_method(klass, "set_gshader", (rubyfunc)set_gshader, 1);
                rb_define_method(klass, "gshader", (rubyfunc)gshader, 0);
                rb_define_method(klass, "set_gs_sampler", (rubyfunc)set_gs_sampler, 2);
                rb_define_method(klass, "set_gs_cbuffer", (rubyfunc)set_gs_cbuffer, 2);
                rb_define_method(klass, "set_gs_resource", (rubyfunc)set_gs_resource, 2);

                //rs
                rb_define_method(klass, "set_viewport", (rubyfunc)set_viewport, -1);
                rb_define_method(klass, "viewport", (rubyfunc)get_viewport, 0);
                rb_define_method(klass, "set_rasterizer", (rubyfunc)set_rasterizer, 1);
                rb_define_method(klass, "rasterizer", (rubyfunc)get_rasterizer, 0);
                rb_define_method(klass, "set_scissor_rect", (rubyfunc)set_scissor_rect, 1);

                //om
                rb_define_method(klass, "set_target", (rubyfunc)set_target, 1);
                rb_define_method(klass, "target", (rubyfunc)get_target, 0);
                rb_define_method(klass, "set_blender", (rubyfunc)set_blender, 1);
                rb_define_method(klass, "blender", (rubyfunc)get_blender, 0);

                //Draw
                rb_define_method(klass, "clear", (rubyfunc)clear_target, -1);
                rb_define_method(klass, "draw", (rubyfunc)draw, 2);
                rb_define_method(klass, "draw_index", (rubyfunc)draw_index, 2);

                //
                rb_define_method(klass, "immdiate_render", (rubyfunc)immdiate_render, 0);
                rb_define_method(klass, "immdiate_copy2d", (rubyfunc)immdiate_copy2d, 4);
                
                //
                rb_define_method(klass, "update_subresource", (rubyfunc)update_subresource, 2);

                //dxgi formats 
                rb_define_const(module, "R32G32B32A32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32A32_FLOAT));
                rb_define_const(module, "R32G32B32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32_FLOAT));
                rb_define_const(module, "R32G32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32_FLOAT));
                rb_define_const(module, "R32_FLOAT", INT2FIX(DXGI_FORMAT_R32_FLOAT));
                rb_define_const(module, "D32_FLOAT", INT2FIX(DXGI_FORMAT_D32_FLOAT));
                rb_define_const(module, "R8G8B8A8_UINT", INT2FIX(DXGI_FORMAT_R8G8B8A8_UINT));
                rb_define_const(module, "R8G8_UINT", INT2FIX(DXGI_FORMAT_R8G8_UINT));
                rb_define_const(module ,"R16G16_UINT", INT2FIX(DXGI_FORMAT_R16G16_UINT));
                rb_define_const(module, "R16G16B16A16_UINT", INT2FIX(DXGI_FORMAT_R16G16B16A16_UINT));

                //primitive topology
                rb_define_const(module, "TOPOLOGY_TRIANGLELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
                rb_define_const(module, "TOPOLOGY_TRIANGLELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ));
                rb_define_const(module, "TOPOLOGY_LINELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
                rb_define_const(module, "TOPOLOGY_LINESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP));
                rb_define_const(module, "TOPOLOGY_LINELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_LINESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ));
                rb_define_const(module, "TOPOLOGY_POINTLIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST));
                rb_define_method(klass, "set_topology", (rubyfunc)set_topology, 1);


                klass_input_layout = rb_define_class_under(module, "InputLayout", rb_cObject);
                rb_include_module(klass_input_layout, module_release);
                rb_define_alloc_func(klass_input_layout, RefObjNew<::InputLayout>);
                rb_define_method(klass_input_layout, "initialize", (rubyfunc)LY_initialize, 3);

                InitRPM();
            }
        }
    }
}