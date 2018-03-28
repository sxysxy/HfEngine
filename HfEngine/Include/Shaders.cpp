#include "Shaders.h"
#include "stdafx.h"
#pragma comment(lib, "d3dx11.lib")

void VertexShader::CreateFromHLSLFile(D3DDevice * device, const std::wstring & filename){
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
    hr = device->native_device->CreateVertexShader(byte_code->GetBufferPointer(), 
        byte_code->GetBufferSize(), 0, &native_vshader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create VertexShader, Error code:", hr);
}

void VertexShader::CreateFromString(D3DDevice * device, const std::string & code){
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, "main", "vs_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
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
    device->native_device->CreateVertexShader(byte_code->GetBufferPointer(), 
        byte_code->GetBufferSize(), 0, &native_vshader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create VertexShader, Error code:", hr);
}

void VertexShader::CreateFromBinary(D3DDevice * device, void *, int size){
}

void PixelShader::CreateFromHLSLFile(D3DDevice * device, const std::wstring & filename){
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
    device->native_device->CreatePixelShader(byte_code->GetBufferPointer(), 
        byte_code->GetBufferSize(), 0, &native_pshader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create PixelShader, Error code:", hr);
}

void PixelShader::CreateFromString(D3DDevice * device, const std::string & code){
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, "main", "ps_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
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
    device->native_device->CreatePixelShader(byte_code->GetBufferPointer(),
        byte_code->GetBufferSize(), 0, &native_pshader);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Fail to Create PixelShader, Error code:", hr);
}

void PixelShader::CreateFromBinary(D3DDevice * device, void *, int size){

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
            static void DeleteShader(T *s) {
                s->SubRefer();
            }
            static VALUE initialize(VALUE self) {
                if (rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
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
                try {
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
                rb_raise(rb_eNotImpError, "Shader::create_from_binfile implement is still not supported");
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

            //sampler
            static void DeleteSampler(Sampler *s) {
                s->SubRefer();
            }

            static VALUE sampler_initialize(VALUE self) {
                if (rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
                return self;
            }

            static VALUE set_filter(VALUE self, VALUE filter, VALUE comp) {
                auto sampler = GetNativeObject<Sampler>(self);
                sampler->SetFilter((D3D11_FILTER)FIX2INT(filter), (D3D11_COMPARISON_FUNC)FIX2INT(comp));
                return self;
            }

            static VALUE set_uvwaddress(VALUE self, VALUE u, VALUE v, VALUE w, VALUE border_color) {
                if (!rb_obj_is_kind_of(border_color, Ext::DX::klass_HFColor)) {
                    rb_raise(rb_eArgError, "D3DSamper::set_uvwaddress: The 4th param should be a HFColorRGBA");
                }
                auto sampler = GetNativeObject<Sampler>(self);
                sampler->SetUVWAddress((D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(u), (D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(v),
                    (D3D11_TEXTURE_ADDRESS_MODE)FIX2INT(w),
                    *GetNativeObject<Utility::Color>(border_color));
                return self;
            }
            static VALUE set_mip(VALUE self, VALUE min_mip, VALUE max_mip, VALUE mip_bias) {
                auto sampler = GetNativeObject<Sampler>(self);
                sampler->SetMip((float)rb_float_value(min_mip), (float)rb_float_value(max_mip), (float)rb_float_value(mip_bias));
                return self;
            }

            static VALUE set_max_anisotropy(VALUE self, VALUE v) {
                auto sampler = GetNativeObject<Sampler>(self);
                sampler->SetMaxAnisotropy((unsigned)FIX2INT(v));
                return self;
            }
            static VALUE use_default(VALUE self) {
                auto sampler = GetNativeObject<Sampler>(self);
                sampler->UseDefault();
                return self;
            }

            static VALUE create_state(VALUE self, VALUE device) {
                auto sampler = GetNativeObject<Sampler>(self);
                if (!rb_obj_is_kind_of(device, Ext::DX::D3DDevice::klass))
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

                klass_vshader = rb_define_class_under(module, "VertexShader", rb_cObject);
                rb_define_alloc_func(klass_vshader, [](VALUE k)->VALUE {
                    auto s = new ::VertexShader;
                    s->AddRefer();
                    return Data_Wrap_Struct(k, nullptr, DeleteShader<::VertexShader>, s);
                });
                rb_define_method(klass_vshader, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_vshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, 2);
                rb_define_method(klass_vshader, "create_from_binfile", (rubyfunc)create_from_binfile, 2);
                rb_define_method(klass_vshader, "create_from_string", (rubyfunc)create_from_string, 2);
                                                                                                      //¡ü
                klass_pshader = rb_define_class_under(module, "PixelShader", klass);                   //
                rb_define_alloc_func(klass_pshader, [](VALUE k)->VALUE {                               //
                    auto s = new ::PixelShader;                                                        //
                    s->AddRefer();                                                                     //
                    return Data_Wrap_Struct(k, nullptr, DeleteShader<::PixelShader>, s);               //
                });                                                                                    //
                //Please use Ctrl-C and Ctrl-V                          --------------------------------|                                    
                rb_define_method(klass_pshader, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_pshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, 2);
                rb_define_method(klass_pshader, "create_from_binfile", (rubyfunc)create_from_binfile, 2);
                rb_define_method(klass_pshader, "create_from_string", (rubyfunc)create_from_string, 2);

                //sampler
                klass_sampler = rb_define_class_under(module, "Sampler", rb_cObject);
                rb_define_alloc_func(klass_sampler, [](VALUE k)->VALUE {
                    auto sampler = new Sampler;
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
    }
}