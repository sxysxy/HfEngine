#include <stdafx.h>
#include <extension.h>
#include "D3DDevice.h"
#include "DX.h"
#include "D3DDeviceContext.h"
#include "D3DBuffer.h"
#include "SwapChain.h"

D3DDeviceContext::D3DDeviceContext(D3DDevice *device) {
    Initialize(device);
}
void D3DDeviceContext::Initialize(D3DDevice *device) {
    HRESULT hr = device->native_device->CreateDeferredContext(0, &native_context);
    if(FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to create D3DDeviceContext(deferred context), Error code:", hr);
    native_device = device->native_device;
}
void D3DDeviceContext::FinishiCommandList() {
    native_context->FinishCommandList(true, &native_command_list);
}
void D3DDeviceContext::BindPipeline(RenderPipeline *pipeline) {
    if(pipeline->vshader)
        BindVertexShader(pipeline->vshader.Get());
    if(pipeline->pshader)
        BindPixelShader(pipeline->pshader.Get());
    if(pipeline->native_input_layout)
        native_context->IASetInputLayout(pipeline->native_input_layout.Get());
}
void D3DDeviceContext::BindVertexShader(VertexShader *vshader) {
    native_context->VSSetShader(vshader->native_shader.Get(), 0, 0);
}
void D3DDeviceContext::BindPixelShader(PixelShader *pshader) {
    native_context->PSSetShader(pshader->native_shader.Get(), 0, 0);
}

//---
void D3DDeviceContext::BindShaderConstantBuffer(int start_slot, int count, D3DConstantBuffer * const * cs, SHADERS_WHICH_TO_APPLAY which) {
    ID3D11Buffer *bs[16];
    for (int i = 0; i < count; i++)bs[i] = cs[i]->native_buffer.Get();
    if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
        native_context->VSSetConstantBuffers(start_slot, count, bs);
    if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
        native_context->PSSetConstantBuffers(start_slot, count, bs);
}
void D3DDeviceContext::BindShaderConstantBuffer(int slot_pos, D3DConstantBuffer *bf, SHADERS_WHICH_TO_APPLAY which) {
    if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
        native_context->VSSetConstantBuffers(slot_pos, 1, bf->native_buffer.GetAddressOf());
    if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
        native_context->PSSetConstantBuffers(slot_pos, 1, bf->native_buffer.GetAddressOf());
}
void D3DDeviceContext::BindVertexBuffer(int start_slot, int count, D3DVertexBuffer * const * vbs, UINT stride){
    ID3D11Buffer *sss[16];
    for(int i = 0; i < count; i++)sss[i] = vbs[i]->native_buffer.Get();
    UINT offset = 0;
    native_context->IASetVertexBuffers(start_slot, count, sss, &stride, &offset);
}
void D3DDeviceContext::BindVertexBuffer(int slot_pos, const D3DVertexBuffer * vb, UINT stride) {
    UINT offset = 0;
    native_context->IASetVertexBuffers(slot_pos, 1, vb->native_buffer.GetAddressOf(), &stride, &offset);
}

//---
void D3DDeviceContext::ClearState() {
    native_context->ClearState();
}
void D3DDeviceContext::UpdateSubResource(D3DBuffer *b, void *data) {
    native_context->UpdateSubresource(b->native_buffer.Get(), 0, 0, data, 0, 0);
}
D3DDeviceImmdiateContext::D3DDeviceImmdiateContext(D3DDevice *device) {
    native_device = device->native_device;
    native_device->GetImmediateContext(&native_context);
}
void D3DDeviceImmdiateContext::ExecuteCommandList(D3DDeviceContext *ocontext) {
    native_context->ExecuteCommandList(ocontext->native_command_list.Get(), false);
}

void D3DDeviceImmdiateContext::ExecuteCommandList(ID3D11CommandList *command_list){
    native_context->ExecuteCommandList(command_list, false);
}

//Rendering Thread
const RenderingThreadParam & RenderingThreadParam::operator=(const RenderingThreadParam & op) {
    swap_chain = op.swap_chain;
    device = op.device;
    frame_rate = op.frame_rate;
    return op;
}
Utility::ReferPtr<Utility::NativeThread<ContextInteractData>> CreateRenderingThread(RenderingThreadParam *param) {
    RenderingThreadParam p;
    p.operator=(*param);
    return Utility::ReferPtr<Utility::NativeThread<ContextInteractData>>::New(0, nullptr,
        [=](Utility::NativeThread<ContextInteractData> *th, int argc, void *argv) {
        Utility::SleepFPSTimer timer;
        timer.Restart(p.frame_rate);
        auto t = th->AccessBuffer(true);
        t->frame_rate = p.frame_rate;
        th->AccessBuffer(false);
        while (true) {
            auto t = th->AccessBuffer(true);
            if(t->frame_rate != timer.rate)
                timer.Restart(t->frame_rate);
            if(t->exit_flag)break;
            while (!t->command_lists.empty()) {
                auto d = t->command_lists.front();
                if(d)
                    p.device->immcontext->ExecuteCommandList(d.Get());
                t->command_lists.pop();
            }
            p.swap_chain->Present();
            th->AccessBuffer(false);
            if(!p.swap_chain->vsync)
                timer.Await();
        }
    });
}
void RenderingThread::Initialize(D3DDevice *d, SwapChain * s, int frame_rate) {
    RenderingThreadParam p;
    p.device = d;
    p.swap_chain = s;
    p.frame_rate = frame_rate;
    rendering_thread = CreateRenderingThread(&p);
}

namespace Ext {
    namespace DX {
        namespace D3DDeviceContext {
            VALUE klass;
            VALUE klass_immcontext;

            void Delete(::D3DDeviceContext *d) {
                d->SubRefer();
            }

            VALUE New(VALUE k){
                auto d = new ::D3DDeviceContext;
                d->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete, d);
            }

            static VALUE initialize(VALUE self, VALUE _device) {
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass)) {
                    rb_raise(rb_eArgError, "D3DDeviceContext::initialize:Param device should be a D3DDevice.");
                }
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto device = GetNativeObject<::D3DDevice>(_device);
                context->Initialize(device);
                
                return self;
            }
            static VALUE bind_pipeline(VALUE self, VALUE rp) {
                if (!rb_obj_is_kind_of(rp, Ext::DX::RenderPipeline::klass)) {
                    rb_raise(rb_eArgError, "D3DDeviceContext::bind_pipeline: The param should be a DX::RenderPipeline");
                }
                rb_iv_set(self, "@pipeline", rp);
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto rpp = GetNativeObject<::RenderPipeline>(rp);
                context->BindPipeline(rpp);
                return self;
            }
            static VALUE clear_state(VALUE self) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                context->ClearState();
                return self;
            }
            static VALUE finishi_command_list(VALUE self) { //finish
                //rb_raise(rb_eNotImpError, "D3DDeviceContext::finish_command_list implement is not supported.");
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                context->FinishiCommandList();
                return self;
            }
            VALUE exec_command_list(VALUE self, VALUE _deferred_context) {
                if (!rb_obj_is_kind_of(_deferred_context, klass) || rb_obj_is_kind_of(_deferred_context, klass_immcontext)) {
                    rb_raise(rb_eArgError, "D3DDeviceImmdiateContext::exec_command_list: Given argument should be a DX::DeviceContext");
                }

                auto context = GetNativeObject<::D3DDeviceImmdiateContext>(self);
                auto ocontext = GetNativeObject<::D3DDeviceContext>(_deferred_context);
                context->ExecuteCommandList(ocontext);
                return self;
            }

            static VALUE set_topology(VALUE self, VALUE topo) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                D3D11_PRIMITIVE_TOPOLOGY t = (D3D11_PRIMITIVE_TOPOLOGY)FIX2INT(topo);
                if(t > 64)rb_raise(rb_eArgError, "Invalid topology option.");
                context->SetTopology(t);
                return self;
            }
            static VALUE bind_vbuffer(VALUE self, VALUE slot, VALUE buf, VALUE stride) {
                if (!rb_obj_is_kind_of(buf, Ext::DX::D3DBuffer::klass_vbuffer)) {
                    rb_raise(rb_eArgError, "D3DDeviceContext::bind_vbuffer: param should be a DX::D3DVertexBuffer");
                }
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto vbuffer = GetNativeObject<::D3DVertexBuffer>(buf);
                context->BindVertexBuffer(FIX2INT(slot), vbuffer, FIX2INT(stride));
                return self;
            }
            static VALUE set_render_target(VALUE self, VALUE tar) {
                if (!rb_obj_is_kind_of(tar, Ext::DX::D3DTexture2D::klass_D3DTexture)) {
                    rb_raise(rb_eArgError, "D3DDeviceContext::set_render_target: The first param should be a DX::D3DTexture");
                }
                auto context = GetNativeObject<::D3DDeviceContext>(self);

#pragma warning("This implement here need noticing")
                auto tex = GetNativeObject<::D3DTexture2D>(tar); //!
                context->SetRenderTarget(tex);                   //!
                return self;
            }
            static VALUE clear_render_target(VALUE self, VALUE tar, VALUE _color) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto tex = GetNativeObject<::D3DTexture2D>(tar);    //!
                auto color = GetNativeObject<Utility::Color>(_color);
                context->ClearRenderTarget(tex, reinterpret_cast<const FLOAT*>(&color));
                return self;
            }
            static VALUE bind_resource(VALUE self, VALUE slot_pos, VALUE res, VALUE which) {
                if (!rb_obj_is_kind_of(res, Ext::DX::D3DTexture2D::klass_D3DTexture))
                    rb_raise(rb_eArgError, "D3DDeviceContext::bind_resource: The second params should be a D3DTexture");
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto resource = GetNativeObject<::D3DTexture2D>(res);  //!
                context->BindShaderResource(FIX2INT(slot_pos), resource, (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                return self;
            }
            static VALUE bind_resources(VALUE self, VALUE start_slot, VALUE reses, VALUE which) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                if (!rb_obj_is_kind_of(reses, rb_cArray))rb_raise(rb_eArgError,
                    "D3DDeviceContext::bind_resources: The second param should be an Array filled with D3DTexture");
                std::vector<::D3DTexture2D *> buffers;  //!
                int len = RARRAY_LEN(reses);
                VALUE *p = RARRAY_PTR(reses);
                for (int i = 0; i < len; i++) {
                    if (!rb_obj_is_kind_of(p[i], Ext::DX::D3DTexture2D::klass_D3DTexture))
                        rb_raise(rb_eArgError,
                            "D3DDeviceContext::bind_resources: The second param should be an Array filled with D3DTexture");
                    buffers.push_back(GetNativeObject<::D3DTexture2D>(p[i]));  //!
                }
                context->BindShaderResource(FIX2INT(start_slot), len, buffers.data(),
                    (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                return self;
            }
#pragma warning("The implement ¡ü need noticing")
            static VALUE set_viewport(VALUE self, VALUE vp, VALUE min_deep, VALUE max_deep) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                if(!rb_obj_is_kind_of(vp, DX::klass_HFRect))rb_raise(rb_eArgError, 
                    "D3DDeviceContext::set_viewport:The first param should be a HFRect");
                min_deep = rb_to_float(min_deep);
                max_deep = rb_to_float(max_deep);
                context->SetViewport((*GetNativeObject<Utility::Rect>(vp)), 
                    (float)rb_float_value(min_deep), (float)rb_float_value(max_deep));
                return self;
            }
            static VALUE draw(VALUE self, VALUE start_pos, VALUE count) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                context->Draw(FIX2INT(start_pos), FIX2INT(count));
                return self;
            }

            static VALUE bind_vbuffers(VALUE self, VALUE start_pos, VALUE bufs, VALUE stride) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                if(!rb_obj_is_kind_of(bufs, rb_cArray))rb_raise(rb_eArgError, 
                    "D3DDeviceContext::bind_...buffers: The second param should be an Array filled with DX::D3D...Buffer");
                std::vector<D3DVertexBuffer *> buffers;
                int len = RARRAY_LEN(bufs);
                VALUE *p = RARRAY_PTR(bufs);
                for (int i = 0; i < len; i++) {
                    if(!rb_obj_is_kind_of(p[i], Ext::DX::D3DBuffer::klass_vbuffer))
                        rb_raise(rb_eArgError, 
                            "D3DDeviceContext::bind_...buffers: The second param should be an Array filled with DX::D3D...Buffer");
                    buffers.push_back(GetNativeObject<D3DVertexBuffer>(p[i]));
                }
                context->BindVertexBuffer(FIX2INT(start_pos), len, buffers.data(), FIX2INT(stride));
                return self;
            }

            static VALUE bind_cbuffers(VALUE self, VALUE start_pos, VALUE bufs, VALUE which) {
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                if (!rb_obj_is_kind_of(bufs, rb_cArray))rb_raise(rb_eArgError,
                    "D3DDeviceContext::bind_...buffers: The second param should be an Array filled with DX::D3D...Buffer");
                std::vector<D3DConstantBuffer *> buffers;
                int len = RARRAY_LEN(bufs);
                VALUE *p = RARRAY_PTR(bufs);
                for (int i = 0; i < len; i++) {
                    if (!rb_obj_is_kind_of(p[i], Ext::DX::D3DBuffer::klass_cbuffer))
                        rb_raise(rb_eArgError,
                            "D3DDeviceContext::bind_...buffers: The second param should be an Array filled with DX::D3D...Buffer");
                    buffers.push_back(GetNativeObject<D3DConstantBuffer>(p[i]));
                }
                context->BindShaderConstantBuffer(FIX2INT(start_pos), len, buffers.data(), (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                return self;
            }
            static VALUE bind_cbuffer(VALUE self, VALUE slot_pos, VALUE buf, VALUE which) {            
                if (!rb_obj_is_kind_of(buf, Ext::DX::D3DBuffer::klass_cbuffer)) {
                    rb_raise(rb_eArgError, "DeviceContext::bind_cbuffer: the second param should be a DX::D3DConstantBuffer");
                }
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto buffer = GetNativeObject<::D3DConstantBuffer>(buf);
                context->BindShaderConstantBuffer(FIX2INT(slot_pos), buffer, (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                return self;
            }
            static VALUE bind_sampler(VALUE self, VALUE slot_pos, VALUE sam, VALUE which) {
                if(!rb_obj_is_kind_of(sam, Ext::DX::Shader::klass_sampler))
                    rb_raise(rb_eArgError, "D3DDeviceContext::bind_sampler: The second param should be a D3DSampler");
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto sampler = GetNativeObject<::D3DSampler>(sam);
                try {
                    context->BindShaderSampler(FIX2INT(slot_pos), sampler, (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                }
                catch(std::invalid_argument &e){
                    rb_raise(rb_eArgError, e.what());
                }
                return self;
            }
            static VALUE bind_samplers(VALUE self, VALUE start_slot, VALUE sams, VALUE which) {
                if (!rb_obj_is_kind_of(sams, rb_cArray))rb_raise(rb_eArgError,
                    "D3DDeviceContext::bind_samplers: The second param should be an Array filled with DX::D3DSampler");
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                std::vector<D3DSampler *> buffers;
                int len = RARRAY_LEN(sams);
                VALUE *p = RARRAY_PTR(sams);
                for (int i = 0; i < len; i++) {
                    if (!rb_obj_is_kind_of(p[i], Ext::DX::Shader::klass_sampler))
                        rb_raise(rb_eArgError,
                            "D3DDeviceContext::bind_samplers: The second param should be an Array filled with DX::D3DSampler");
                    buffers.push_back(GetNativeObject<D3DSampler>(p[i]));
                }
                try {
                    context->BindShaderSampler(FIX2INT(start_slot), len, buffers.data(), (SHADERS_WHICH_TO_APPLAY)FIX2INT(which));
                }
                catch (std::invalid_argument &e) {
                    rb_raise(rb_eArgError, e.what());
                }
                return self;
            }
            static VALUE update_subresource(VALUE self, VALUE buf, VALUE data) {
                if(!rb_obj_is_kind_of(buf, Ext::DX::D3DBuffer::klass))
                    rb_raise(rb_eArgError, "D3DDeviceContext::update_subresource: The first param should be a DX::D3DBuffer");
                auto context = GetNativeObject<::D3DDeviceContext>(self);
                auto buffer = GetNativeObject<::D3DBuffer>(buf);
                if (rb_obj_is_kind_of(data, rb_cString)) {
                    void *ptr = rb_string_value_ptr(&data);
                    context->UpdateSubResource(buffer, ptr);
                }
                else if (rb_obj_is_kind_of(data, rb_cInteger)) {
                    context->UpdateSubResource(buffer, (void*)FIX2PTR(data));
                }
                return self;
            }
            void Init() {
                klass = rb_define_class_under(module, "D3DDeviceContext", rb_cObject);
                rb_define_alloc_func(klass, New);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 1);
                rb_define_method(klass, "finish_command_list", (rubyfunc)finishi_command_list, 0);
                rb_define_method(klass, "bind_pipeline", (rubyfunc)bind_pipeline, 1);
                rb_define_method(klass, "clear_state", (rubyfunc)clear_state, 0);
                rb_attr_get(klass, rb_intern("pipeline"));
                //Topology:
                rb_define_const(module, "TOPOLOGY_TRIANGLELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
                rb_define_const(module, "TOPOLOGY_TRIANGLELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_TRIANGLESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ));
                rb_define_const(module, "TOPOLOGY_LINELIST", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
                rb_define_const(module, "TOPOLOGY_LINESTRIP", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP));
                rb_define_const(module, "TOPOLOGY_LINELIST_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ));
                rb_define_const(module, "TOPOLOGY_LINESTRIP_ADJ", INT2FIX(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ));
                rb_define_method(klass, "set_topology", (rubyfunc)set_topology, 1);
                rb_define_method(klass, "bind_vbuffer", (rubyfunc)bind_vbuffer, 3);
                rb_define_method(klass, "bind_vbuffers", (rubyfunc)bind_vbuffers, 3);
                rb_define_const(module, "SHADERS_APPLYTO_VSHADER", INT2FIX(SHADERS_APPLYTO_VSHADER));
                rb_define_const(module, "SHADERS_APPLYTO_PSHADER", INT2FIX(SHADERS_APPLYTO_PSHADER));
                rb_define_method(klass, "bind_cbuffers", (rubyfunc)bind_cbuffers, 3);
                rb_define_method(klass, "bind_cbuffer", (rubyfunc)bind_cbuffer, 3);
                rb_define_method(klass, "bind_resources", (rubyfunc)bind_resources, 3);
                rb_define_method(klass, "bind_resource", (rubyfunc)bind_resource, 3);
                rb_define_method(klass, "bind_sampler", (rubyfunc)bind_sampler, 3);
                rb_define_method(klass, "bind_samplers", (rubyfunc)bind_samplers, 3);
                rb_define_method(klass, "update_subresource", (rubyfunc)update_subresource, 2);
                rb_define_method(klass, "set_viewport", (rubyfunc)set_viewport, 3);
                rb_define_method(klass, "set_render_target", (rubyfunc)set_render_target, 1);
                rb_define_method(klass, "clear_render_target", (rubyfunc)clear_render_target, 2);
                rb_define_method(klass, "draw", (rubyfunc)draw, 2);

                klass_immcontext = rb_define_class_under(module, "D3DDeviceImmdiateContext", klass);
                rb_undef_alloc_func(klass_immcontext);
                rb_undef_method(CLASS_OF(klass_immcontext), "new");
                rb_define_method(klass_immcontext, "exec_command_list", (rubyfunc)exec_command_list, 1);
                rb_undef_method(klass_immcontext, "finish_command_list");

            }
        }
} }

namespace Ext {
    namespace DX {
        namespace RenderingThread {
            VALUE klass;


            static VOID Delete(::RenderingThread *rth) {
                rth->SubRefer();
            }
            static VALUE New(VALUE k){
                auto rth = new ::RenderingThread;
                rth->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete, rth);
            }
                                                //device, swap_chain, frame_rate
            static VALUE initialize(VALUE self, VALUE d, VALUE s, VALUE f) {
                if(!rb_obj_is_kind_of(d, Ext::DX::D3DDevice::klass))
                    rb_raise(rb_eArgError, "RenderingThread::initialize: The first param should be a DX::D3DDevice");
                if(!rb_obj_is_kind_of(s, Ext::DX::SwapChain::klass))
                    rb_raise(rb_eArgError, "RenderingThread::initialize: The second param should be a DX::SwapChain");
                auto rth = GetNativeObject<::RenderingThread>(self);
                rth->Initialize(GetNativeObject<::D3DDevice>(d), GetNativeObject<::SwapChain>(s), FIX2INT(f));
                return self;
            }
            static VALUE set_frame_rate(VALUE self, VALUE f) {
                auto rth = GetNativeObject<::RenderingThread>(self);
                rth->SetFrameRate(FIX2INT(f));
                return self;
            }
            static VALUE rth_terminate(VALUE self) {
                auto rth = GetNativeObject<::RenderingThread>(self);
                rth->Terminate();
                return Qnil;
            }
            static VALUE push_command_list(VALUE self, VALUE c) {
                if(!rb_obj_is_kind_of(c, Ext::DX::D3DDeviceContext::klass))
                    rb_raise(rb_eArgError, 
                        "RenderingThread::push_command_list: The first param should be a DX::D3DDeviceContext");
                auto rth = GetNativeObject<::RenderingThread>(self);
                rth->PushCommandList(GetNativeObject<::D3DDeviceContext>(c));
                return self;
            }
            void Init() {
                klass = rb_define_class_under(module, "RenderingThread", rb_cObject);
                rb_define_alloc_func(klass, New);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 3);
                rb_define_method(klass, "set_frame_rate", (rubyfunc)set_frame_rate, 1);
                rb_define_method(klass, "terminate", (rubyfunc)rth_terminate, 0); //void __cdecl terminate !?
                rb_define_method(klass, "push_command_list", (rubyfunc)push_command_list, 1);
            }
        }
    }
}


