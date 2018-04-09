#include "RenderPipeline.h"

void RenderPipeline::SetInputLayout(D3DDevice *device, const std::string *idents, 
                                           const DXGI_FORMAT *formats, int count) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
    for (int i = 0; i < count; i++) {
        ied.push_back(D3D11_INPUT_ELEMENT_DESC{ idents[i].c_str(), 0, formats[i], 0,
            D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 });
    }
    HRESULT hr = S_FALSE;
    if (FAILED(hr = device->native_device->CreateInputLayout(ied.data(), count, vshader->byte_code->GetBufferPointer(),
        vshader->byte_code->GetBufferSize(), &native_input_layout))) {
        MAKE_ERRMSG<std::runtime_error>("Fail to Create InputLayout, Error code:", hr);
    }   
    
    native_context->IASetInputLayout(native_input_layout.Get());
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

//VS
void RenderPipeline::SetVertexShader(VertexShader *vs) {
    vshader = vs;
    native_context->VSSetShader(vs->native_vshader.Get(), 0, 0);
}
void RenderPipeline::SetVSSampler(int slot, Sampler * sampler) {
    native_context->VSSetSamplers(slot, 1, sampler->native_sampler.GetAddressOf());
}
void RenderPipeline::SetVSCBuffer(int slot, ConstantBuffer * cbuffer) {
    native_context->VSSetConstantBuffers(slot, 1, cbuffer->native_buffer.GetAddressOf());
}
void RenderPipeline::SetVSResource(int slot, Texture2D *tex) {
    native_context->VSSetShaderResources(slot, 1, tex->native_shader_resource_view.GetAddressOf());
}
//PS
void RenderPipeline::SetPixelShader(PixelShader *ps) {
    pshader = ps;
    native_context->PSSetShader(ps->native_pshader.Get(), 0, 0);
}
void RenderPipeline::SetPSSampler(int slot, Sampler * sampler) {
    native_context->PSSetSamplers(slot, 1, sampler->native_sampler.GetAddressOf());
}
void RenderPipeline::SetPSCBuffer(int slot, ConstantBuffer * cbuffer) {
    native_context->PSSetConstantBuffers(slot, 1, cbuffer->native_buffer.GetAddressOf());
}
void RenderPipeline::SetPSResource(int slot, Texture2D *tex) {
    native_context->PSSetShaderResources(slot, 1, tex->native_shader_resource_view.GetAddressOf());
}

namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            VALUE klass;

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
                return self;
            }

            static VALUE set_input_layout(VALUE self, VALUE _device, VALUE names, VALUE fmts) {
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass))
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: First param should be a DX::D3DDevice");
                if (!rb_obj_is_kind_of(names, rb_cArray) || !rb_obj_is_kind_of(fmts, rb_cArray))
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: 2nd and 3rd params should be Array");
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
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto sd = GetNativeObject<::VertexShader>(s);
                rp->SetVertexShader(sd);
                rb_iv_set(self, "@vshader", s);
                return self;
            }
            static VALUE set_pshader(VALUE self, VALUE s) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto sd = GetNativeObject<::PixelShader>(s);
                rp->SetPixelShader(sd);
                rb_iv_set(self, "@pshader", s);
                return self;
            }
            static VALUE vshader(VALUE self) {
                return rb_iv_get(self, "@vshader");
            }
            static VALUE pshader(VALUE self) {
                return rb_iv_get(self, "@pshader");
            }

            static VALUE set_topology(VALUE self, VALUE t) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetTopology((D3D11_PRIMITIVE_TOPOLOGY)(FIX2INT(t)));
                return self;
            }
            static VALUE set_vbuffer(VALUE self, VALUE vb) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetVertexBuffer(GetNativeObject<VertexBuffer>(vb));
                return self;
            }
            static VALUE set_ibuffer(VALUE self, VALUE ib) {
                auto rp = GetNativeObject<::RenderPipeline>(self);
                rp->SetIndexBuffer(GetNativeObject<IndexBuffer>(ib));
                return self;
            }

            void Init() {
                klass = rb_define_class_under(module, "RenderPipeline", rb_cObject);

                rb_define_alloc_func(klass, new_rp);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 1);
                rb_define_method(klass, "set_input_layout", (rubyfunc)set_input_layout, 3);
                rb_define_method(klass, "set_vbuffer", (rubyfunc)set_vbuffer, 1);
                rb_define_method(klass, "set_ibuffer", (rubyfunc)set_ibuffer, 1);

                rb_define_method(klass, "set_vshader", (rubyfunc)set_vshader, 1);
                rb_define_method(klass, "vshader", (rubyfunc)vshader, 0);

                rb_define_method(klass, "set_pshader", (rubyfunc)set_pshader, 1);
                rb_define_method(klass, "pshader", (rubyfunc)pshader, 0);


                rb_define_const(module, "R32G32B32A32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32A32_FLOAT));
                rb_define_const(module, "R32G32B32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32_FLOAT));
                rb_define_const(module, "R32G32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32_FLOAT));
                rb_define_const(module, "R32_FLOAT", INT2FIX(DXGI_FORMAT_R32_FLOAT));
                rb_define_const(module, "D32_FLOAT", INT2FIX(DXGI_FORMAT_D32_FLOAT));
                rb_define_const(module, "R8G8B8A8_UINT", INT2FIX(DXGI_FORMAT_R8G8B8A8_UINT));

                rb_define_const(module, "TOPOLOGY_TRIANGLELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
                rb_define_const(module, "TOPOLOGY_TRIANGLELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ));
                rb_define_const(module, "TOPOLOGY_LINELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
                rb_define_const(module, "TOPOLOGY_LINESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP));
                rb_define_const(module, "TOPOLOGY_LINELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_LINESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ));
                rb_define_method(klass, "set_topology", (rubyfunc)set_topology, 1);
            }
        }
    }
}