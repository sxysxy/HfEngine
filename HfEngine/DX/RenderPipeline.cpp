#include "RenderPipeline.h"
#include "D3DTexture2D.h"
#include "D3DDeviceContext.h"
#include "D3DDevice.h"
#include "DX.h"
#include <extension.h>

void VertexShader::CreateFromHLSLFile(D3DDevice *device, const cstring & filename) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, "main", 
        "vs_4_0", 0, 0, 0, 
        &sbuffer, &errmsg, nullptr); //cause a _com_error,,but why?, it returns S_OK...         

    if (FAILED(hr)) {
        std::string msg;
        Ext::U16ToU8(filename.c_str(), msg);
        if (errmsg) {
            msg.append("VertexShader :Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw ShaderCompileError(msg);
        }
        else {
            MAKE_ERRMSG<std::runtime_error>("Fail to Create Shader from hlsl file, Error code:", hr);
        }
    }
    byte_code = sbuffer;
    hr = device->native_device->CreateVertexShader(byte_code->GetBufferPointer(), byte_code->GetBufferSize(), 0, &native_shader);
    if(FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create VertexShader, Error code:", hr);
}

void VertexShader::CreateFromBinaryFile(D3DDevice *device, const cstring & filename) {
    throw ImplementStillNotSupported("VertexShader::CreateFromBinaryFile:The implement is still not supported.");
}

void VertexShader::CreateFromString(D3DDevice *device, const std::string & str) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(str.c_str(), str.length(), 0, 0, 0, "main", "vs_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
    if (FAILED(hr)) {
        if (errmsg) {
            std::string msg;
            msg.append("VertexShader :Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw ShaderCompileError(msg);
        }
        else {
            MAKE_ERRMSG<std::runtime_error>("Fail to Create Shader from string, Error code:", hr);
        }
    }
    byte_code = sbuffer;
    device->native_device->CreateVertexShader(byte_code->GetBufferPointer(), byte_code->GetBufferSize(), 0, &native_shader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create VertexShader, Error code:", hr);
}

void PixelShader::CreateFromHLSLFile(D3DDevice *device, const cstring & filename) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, "main",
        "ps_4_0", 0, 0, 0,
        &sbuffer, &errmsg, nullptr);
    if (FAILED(hr)) {
        std::string msg;
        Ext::U16ToU8(filename.c_str(), msg);
        if (errmsg) {
            msg.append(":Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw ShaderCompileError(msg);
        }
        else {
            MAKE_ERRMSG<std::runtime_error>("Fail to Create Shader from hlsl file, Error code:", hr);
        }
    }
    byte_code = sbuffer;
    device->native_device->CreatePixelShader(byte_code->GetBufferPointer(), byte_code->GetBufferSize(), 0, &native_shader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create PixelShader, Error code:", hr);
}

void PixelShader::CreateFromBinaryFile(D3DDevice *device, const cstring & filename) {
    throw ImplementStillNotSupported("PixelShader::CreateFromBinaryFile:The implement is still not supported.");
}

void PixelShader::CreateFromString(D3DDevice *device, const std::string & str) {
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(str.c_str(), str.length(), 0, 0, 0, "main", "ps_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
    if (FAILED(hr)) {
        if (errmsg) {
            std::string msg;
            msg.append(":Compiler Message:\n");
            msg.append((LPCSTR)errmsg->GetBufferPointer());
            throw ShaderCompileError(msg);
        }
        else {
            MAKE_ERRMSG<std::runtime_error>("Fail to Create Shader from string, Error code:", hr);
        }
    }
    byte_code = sbuffer;
    device->native_device->CreatePixelShader(byte_code->GetBufferPointer(), byte_code->GetBufferSize(), 0, &native_shader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create PixelShader, Error code:", hr);
}


void RenderPipeline::SetInputLayout(D3DDevice *device, const std::string *idents, const DXGI_FORMAT *fmts, int count) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> ied;

    for (int i = 0; i < count; i++) {
        ied.push_back(D3D11_INPUT_ELEMENT_DESC{ idents[i].c_str(), 0, fmts[i], 0,
            D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 });
    }
    HRESULT hr = S_FALSE;
    if (FAILED(hr = device->native_device->CreateInputLayout(ied.data(), count, vshader->byte_code->GetBufferPointer(),
        vshader->byte_code->GetBufferSize(), &native_input_layout))) {    
        MAKE_ERRMSG<std::runtime_error>("Fail to Create InputLayout, Error code:", hr);
    }
}

void D3DSampler::CreateState(D3DDevice *device) {
    HRESULT hr = device->native_device->CreateSamplerState(&desc, &native_sampler);
    if(FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create Sampler State, Error code:", hr);
}

namespace Ext {
    namespace DX {
        namespace Shader {
            VALUE klass;
            VALUE klass_vshader;
            VALUE klass_pshader;
            VALUE klass_sampler;
            VALUE klass_eShaderCompileError;

            template<class T>
            static void DeleteShader(T *vs) {
                vs->SubRefer();
            }

            static VALUE initialize(VALUE self) {
                if(rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
                return self;
            }

            static VALUE create_from_hlsl(VALUE self, VALUE _device, VALUE _filename) {
                auto shader = GetNativeObject<::Shader>(self);
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass) ||
                    !rb_obj_is_kind_of(_filename, rb_cString)) {
                    rb_raise(rb_eArgError, "Shader::create_from_hlsl: This first param should be a DX::D3DDevice and the \
                        second one should be a String. (No Automatically Converting).");
                }
                auto device = GetNativeObject<::D3DDevice>(_device);
                cstring filename;
                U8ToU16(rb_string_value_cstr(&_filename), filename);
                try{
                    shader->CreateFromHLSLFile(device, filename);
                }
                catch (ShaderCompileError ce) {
                    rb_raise(klass_eShaderCompileError, ce.what());
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            static VALUE create_from_binfile(VALUE self, VALUE _device, VALUE _filename) {
                rb_raise(klass_eImplementStillNotSupported, "Shader::create_from_binfile implement is still not supported");
                return self;
            }

            static VALUE create_from_string(VALUE self, VALUE _device, VALUE str) {
                auto shader = GetNativeObject<::Shader>(self);
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass) ||
                    !rb_obj_is_kind_of(str, rb_cString)) {
                    rb_raise(rb_eArgError, "Shader::create_from_string: This first param should be a DX::D3DDevice and the \
                        second one should be a String. (No Automatically Converting).");
                }
                auto device = GetNativeObject<::D3DDevice>(_device);
                std::string code = rb_string_value_cstr(&str);
                try {
                    shader->CreateFromString(device, code);
                }
                catch (ShaderCompileError ce) {
                    rb_raise(klass_eShaderCompileError, ce.what());
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            static VALUE shader_initialize(int argc, VALUE *argv, VALUE self) {
                rb_raise(rb_eNotImpError, "Note: Shader class is only an abstract class, you should not call Shader.new");
                return Qnil;
            }

            static void DeleteSampler(D3DSampler *s) {
                s->SubRefer();
            }

            static VALUE sampler_initialize(VALUE self) {
                if(rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
                return self;
            }
            
            static VALUE set_filter(VALUE self, VALUE filter, VALUE comp) {
                auto sampler = GetNativeObject<D3DSampler>(self);
                sampler->SetFilter((D3D11_FILTER)FIX2INT(filter), (D3D11_COMPARISON_FUNC)FIX2INT(comp));
                return self;
            }

            static VALUE set_uvwaddress(VALUE self, VALUE u, VALUE v, VALUE w, VALUE border_color) {
                if (!rb_obj_is_kind_of(border_color, Ext::DX::klass_HFColor)) {
                    rb_raise(rb_eArgError, "D3DSamper::set_uvwaddress: The 4th param should be a HFColorRGBA");
                }
                auto sampler = GetNativeObject<D3DSampler>(self);
                sampler->SetUVWAddress((D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(u), (D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(v), 
                    (D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(w),
                    *GetNativeObject<Utility::Color>(border_color));
                return self;
            }
            
            static VALUE set_mip(VALUE self, VALUE min_mip, VALUE max_mip, VALUE mip_bias) {
                auto sampler = GetNativeObject<D3DSampler>(self);
                sampler->SetMip((float)rb_float_value(min_mip), (float)rb_float_value(max_mip), (float)rb_float_value(mip_bias));
                return self;
            }

            static VALUE set_max_anisotropy(VALUE self, VALUE v) {
                auto sampler = GetNativeObject<D3DSampler>(self);
                sampler->SetMaxAnisotropy((unsigned)FIX2INT(v));
                return self;
            }

            static VALUE use_default(VALUE self) {
                auto sampler = GetNativeObject<D3DSampler>(self);
                sampler->UseDefault();
                return self;
            }

            static VALUE create_state(VALUE self, VALUE device) {
                auto sampler = GetNativeObject<D3DSampler>(self);
                if(!rb_obj_is_kind_of(device, Ext::DX::D3DDevice::klass))
                    rb_raise(rb_eArgError, "D3DSampler::create_state: Param device should be a DX::D3DDevice");
                auto d = GetNativeObject<::D3DDevice>(device);
                try {
                    sampler->CreateState(d);
                }
                catch (const std::runtime_error &err) {
                    rb_raise(rb_eRuntimeError, err.what());
                }
                return self;
            }
            void Init() {
                klass_eShaderCompileError = rb_define_class_under(module, "ShaderCompileError", rb_eException);
                klass = rb_define_class_under(module, "Shader", rb_cObject);
                rb_define_method(klass, "initialize", (rubyfunc)shader_initialize, -1);

                klass_vshader = rb_define_class_under(module, "VertexShader", klass);
                rb_define_alloc_func(klass_vshader, [](VALUE k)->VALUE{
                    auto s = new ::VertexShader;
                    s->AddRefer();
                    return Data_Wrap_Struct(k, nullptr, DeleteShader<::VertexShader>, s);
                });
                rb_define_method(klass_vshader, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_vshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, 2);
                rb_define_method(klass_vshader, "create_from_binfile", (rubyfunc)create_from_binfile, 2);
                rb_define_method(klass_vshader, "create_from_string", (rubyfunc)create_from_string, 2);

                klass_pshader = rb_define_class_under(module, "PixelShader", klass);
                rb_define_alloc_func(klass_pshader, [](VALUE k)->VALUE {
                    auto s = new ::PixelShader;
                    s->AddRefer();
                    return Data_Wrap_Struct(k, nullptr, DeleteShader<::PixelShader>, s);
                });
                //copy-past-modify (*£þ¦á£þ)
                rb_define_method(klass_pshader, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_pshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, 2);
                rb_define_method(klass_pshader, "create_from_binfile", (rubyfunc)create_from_binfile, 2);
                rb_define_method(klass_pshader, "create_from_string", (rubyfunc)create_from_string, 2);

                //sampler
                klass_sampler = rb_define_class_under(module, "D3DSampler", rb_cObject);
                rb_define_alloc_func(klass_sampler, [](VALUE k)->VALUE {
                    auto sampler = new D3DSampler;
                    sampler->AddRefer();
                    return Data_Wrap_Struct(klass_sampler, nullptr, DeleteSampler, sampler);
                });
                
                rb_define_method(klass_sampler, "initialize", (rubyfunc)sampler_initialize, 0);
                rb_define_method(klass_sampler, "set_filter", (rubyfunc)set_filter, 2);
                rb_define_method(klass_sampler, "set_uvwaddress", (rubyfunc)set_uvwaddress, 4);
                rb_define_method(klass_sampler, "set_mip", (rubyfunc)set_mip, 3);
                rb_define_method(klass_sampler, "set_max_anisotropy", (rubyfunc)set_max_anisotropy, 1);
                rb_define_method(klass_sampler, "use_default", (rubyfunc)use_default, 0);
                rb_define_method(klass_sampler, "create_state", (rubyfunc)create_state, 1);
                /*
                D3D11_FILTER_MIN_MAG_MIP_POINT
                D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR
                D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT
                D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR
                D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT
                D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR
                D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT
                D3D11_FILTER_MIN_MAG_MIP_LINEAR
                D3D11_FILTER_ANISOTROPIC
                D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT
                D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR
                D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT
                D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR
                D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT
                D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR
                D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT
                D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
                D3D11_FILTER_COMPARISON_ANISOTROPIC
                D3D11_FILTER_TEXT_1BIT
                */
                rb_define_const(module, "FILTER_MIN_MAG_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_MAG_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_LINEAR_MAG_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_MAG_LINEAR_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT));
                rb_define_const(module, "FILTER_ANISOTROPIC", INT2FIX(D3D11_FILTER_ANISOTROPIC));

                //Address mode
                rb_define_const(module, "ADDRESS_WRAP", INT2FIX(D3D11_TEXTURE_ADDRESS_WRAP));
                rb_define_const(module, "ADDRESS_MIRROR", INT2FIX(D3D11_TEXTURE_ADDRESS_MIRROR));
                rb_define_const(module, "ADDRESS_MIRROR_ONCE", INT2FIX(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE));
                rb_define_const(module, "ADDRESS_CLAMP", INT2FIX(D3D11_TEXTURE_ADDRESS_CLAMP));
                rb_define_const(module, "ADDRESS_BORDER", INT2FIX(D3D11_TEXTURE_ADDRESS_BORDER));

            }
        }

        namespace RenderPipeline {
            VALUE klass;

            void Delete(::RenderPipeline *rp) {
                rp->SubRefer();
            }

            static VALUE New(VALUE k) {
                auto rp = new ::RenderPipeline;
                rp->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete, rp);
            }

            static VALUE initialize(VALUE self) {
                if(rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
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
            
            static VALUE set_input_layout(VALUE self, VALUE _device, VALUE names, VALUE fmts) {
                if(!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass))
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: First param should be a DX::D3DDevice");
                if(!rb_obj_is_kind_of(names, rb_cArray) || !rb_obj_is_kind_of(fmts, rb_cArray))
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: 2nd and 3rd params should be Array");
                VALUE *pnames = (VALUE *)RARRAY_PTR(names);
                VALUE *pfmts = (VALUE *)RARRAY_PTR(fmts);
                int len1 = RARRAY_LEN(names);
                int len2 = RARRAY_LEN(fmts);
                if(len1 != len2)
                    rb_raise(rb_eArgError, "RenderPipeline::set_input_layout: Array lengths do not match");
                std::vector<std::string> ns;
                std::vector<DXGI_FORMAT> fs;
                for (int i = 0; i < len1; i++) {
                    ns.push_back(rb_string_value_cstr(pnames+i));
                    fs.push_back((DXGI_FORMAT)FIX2INT(pfmts[i]));
                }
                auto rp = GetNativeObject<::RenderPipeline>(self);
                auto device = GetNativeObject<::D3DDevice>(_device);
                try{
                    rp->SetInputLayout(device, ns.data(), fs.data(), len1);
                }
                catch (std::runtime_error re){
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            void Init() {
                Ext::DX::Shader::Init();
                klass = rb_define_class_under(module, "RenderPipeline", rb_cObject);
                rb_define_alloc_func(klass, New);
                rb_define_method(klass, "initialize", (rubyfunc)initialize, 0);
                rb_attr_get(klass, rb_intern("vshader"));
                rb_attr_get(klass, rb_intern("pshader"));
                rb_define_method(klass, "set_vshader", (rubyfunc)set_vshader, 1);
                rb_define_method(klass, "set_pshader", (rubyfunc)set_pshader, 1);
                rb_alias(klass, rb_intern("vshader="), rb_intern("set_vshader"));
                rb_alias(klass, rb_intern("pshader="), rb_intern("set_pshader"));
                //inputlayout:
                rb_define_const(module, "R32G32B32A32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32A32_FLOAT));
                rb_define_const(module, "R32G32B32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32B32_FLOAT));
                rb_define_const(module, "R32G32_FLOAT", INT2FIX(DXGI_FORMAT_R32G32_FLOAT));
                rb_define_const(module, "R32_FLOAT", INT2FIX(DXGI_FORMAT_R32_FLOAT));
                rb_define_const(module, "D32_FLOAT", INT2FIX(DXGI_FORMAT_D32_FLOAT));
                rb_define_const(module, "R8G8B8A8_UINT", INT2FIX(DXGI_FORMAT_R8G8B8A8_UINT));
                rb_define_method(klass, "set_input_layout", (rubyfunc)set_input_layout, 3);
                
                
            }
        }
    }
}

