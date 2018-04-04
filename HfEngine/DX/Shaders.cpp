#include "Shaders.h"
#include "stdafx.h"
#pragma comment(lib, "d3dx11.lib")

void VertexShader::CreateFromHLSLFile(D3DDevice * device, const std::wstring & filename, const std::string &entry){
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, entry.c_str(),
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

void VertexShader::CreateFromString(D3DDevice * device, const std::string & code, const std::string &entry){
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, entry.c_str(), 
        "vs_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
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

void PixelShader::CreateFromHLSLFile(D3DDevice * device, const std::wstring & filename, const std::string &entry){
    ComPtr<ID3D10Blob> sbuffer, errmsg;

    HRESULT hr = D3DX11CompileFromFileW(filename.c_str(), nullptr, nullptr, entry.c_str(),
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

void PixelShader::CreateFromString(D3DDevice * device, const std::string & code, const std::string &entry){
    ComPtr<ID3D10Blob> sbuffer, errmsg;
    HRESULT hr = D3DX11CompileFromMemory(code.c_str(), code.length(), 0, 0, 0, entry.c_str(), 
        "ps_4_0", 0, 0, 0, &sbuffer, &errmsg, 0);
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
            VALUE klass_blender;
            VALUE klass_eShaderCompileError;

            template<class T>
            static void DeleteShader(T *s) {
                s->SubRefer();
            }
            static VALUE initialize(VALUE self) {
                if (rb_block_given_p())rb_obj_instance_eval(0, nullptr, self);
                return self;
            }
            static VALUE create_from_hlsl(int argc, VALUE *argv, VALUE self) {
                if (argc < 2 || argc > 3) {
                    rb_raise(rb_eArgError, "Shader::create_from_hlsl: Expecting to get (2..3) arguments but got %d", argc);
                }
                VALUE _device = argv[0];
                VALUE _filename = argv[1];
                VALUE entry = 0;
                if(argc == 3)
                    entry = argv[2];
                auto shader = GetNativeObject<::Shader>(self);
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass) ||
                    !rb_obj_is_kind_of(_filename, rb_cString) || 
                    !rb_obj_is_kind_of(entry, rb_cString)) {
                    rb_raise(rb_eArgError, "Shader::create_from_hlsl: Usage:(device, filename, [entry = \"main\"])");
                }
                auto device = GetNativeObject<::D3DDevice>(_device);
                cstring filename;
                U8ToU16(rb_string_value_cstr(&_filename), filename);
                try {
                    if(entry)
                        shader->CreateFromHLSLFile(device, filename, rb_string_value_cstr(&entry));
                    else
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

            static VALUE create_from_string(int argc, VALUE *argv, VALUE self) {
                if (argc < 2 || argc > 3) {
                    rb_raise(rb_eArgError, "Shader::create_from_string: Expecting to get (2..3) arguments but got %d", argc);
                }
                auto shader = GetNativeObject<::Shader>(self);
                VALUE _device = argv[0];
                VALUE str = argv[1];
                VALUE entry = 0;
                if (argc == 3)
                    entry = argv[2];
                if (!rb_obj_is_kind_of(_device, Ext::DX::D3DDevice::klass) ||
                    !rb_obj_is_kind_of(str, rb_cString) ||
                    !rb_obj_is_kind_of(entry, rb_cString)) {
                    rb_raise(rb_eArgError, "Shader::create_from_string: Usage:(device, filename, [entry = \"main\"])");
                }
                auto device = GetNativeObject<::D3DDevice>(_device);
                std::string code = rb_string_value_cstr(&str);
                try {
                    if(entry)
                        shader->CreateFromString(device, code, rb_string_value_cstr(&entry));
                    else
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

            template<class Native>
            static VALUE use_default(VALUE self) {
                auto obj = GetNativeObject<Native>(self);
                obj->UseDefault();
                return self;
            }
            template<class Native>
            static VALUE create_state(VALUE self, VALUE device) {
                auto obj = GetNativeObject<Native>(self);
                if (!rb_obj_is_kind_of(device, Ext::DX::D3DDevice::klass))
                    rb_raise(rb_eArgError, "create_state: Param device should be a DX::D3DDevice");
                auto d = GetNativeObject<::D3DDevice>(device);
                try {
                    obj->CreateState(d);
                }
                catch (const std::runtime_error &err) {
                    rb_raise(rb_eRuntimeError, err.what());
                }
                return self;
            }
            
            template<class NativeClass, class NativeDesc>
            static VALUE dump_description(VALUE self) {
                auto native = GetNativeObject<NativeClass>(self);
                NativeDesc d;
                native->DumpDescription(&d); 
                char *pd = reinterpret_cast<char*>(&d);
                VALUE data = rb_ary_new_capa(sizeof(d));
                rb_ary_resize(data, sizeof d);
                VALUE *p = RARRAY_PTR(data);
                for (int i = 0; i < sizeof d; i++) {
                    p[i] = INT2FIX(pd[i]);
                }
                return data;
            }
            template<class NativeClass, class NativeDesc>
            static VALUE load_description(VALUE self, VALUE s) {
                if (!rb_obj_is_kind_of(s, rb_cArray)) {
                    rb_raise(rb_eArgError, "Load Description : Param should be a String");
                }
                auto native = GetNativeObject<NativeClass>(self);
                NativeDesc x;
                char *pd = reinterpret_cast<char*>(&x);
                const VALUE *p = rb_array_const_ptr(s);
                for (int i = 0; i < sizeof x; i++) {
                    pd[i] = FIX2INT(p[i]);
                }
                native->LoadDescription(&x);
                return self;
            }

            static VALUE blender_enable(VALUE self, VALUE f) {
                auto b = GetNativeObject<Blender>(self);
                b->Enable(f == Qtrue);
                return self;
            }
            static VALUE set_color_blend(VALUE self, VALUE src_blend, VALUE dest_blend, VALUE op) {
                auto b = GetNativeObject<Blender>(self);
                b->SetColorBlend((D3D11_BLEND)FIX2INT(src_blend), (D3D11_BLEND)FIX2INT(dest_blend), 
                    (D3D11_BLEND_OP)FIX2INT(op));
                return self;
            }
            static VALUE set_alpha_blend(VALUE self, VALUE src_blend, VALUE dest_blend, VALUE op) {
                auto b = GetNativeObject<Blender>(self);
                b->SetAlphaBlend((D3D11_BLEND)FIX2INT(src_blend), (D3D11_BLEND)FIX2INT(dest_blend),
                    (D3D11_BLEND_OP)FIX2INT(op));
                return self;
            }
            static VALUE set_mask(VALUE self, VALUE m) {
                auto b = GetNativeObject<Blender>(self);
                b->SetMask((D3D11_COLOR_WRITE_ENABLE)FIX2INT(m));
                return self;
            }
            static VALUE set_blend_factor(VALUE self, VALUE f) {
                auto b = GetNativeObject<Blender>(self);
                if (!rb_obj_is_kind_of(f, Ext::DX::klass_HFColor)) {
                    rb_raise(rb_eArgError, "Blender::set_blend_factor : the param should be an HFColorRGBA");
                }
                b->SetBlendFactor(*GetNativeObject<Utility::Color>(f));
                return self;
            }
            static void DeleteBlender(Blender *b) {
                b->SubRefer();
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
                rb_define_method(klass_vshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, -1);
                rb_define_method(klass_vshader, "create_from_binfile", (rubyfunc)create_from_binfile, -1);
                rb_define_method(klass_vshader, "create_from_string", (rubyfunc)create_from_string, -1);
                                                                                                      //¡ü
                klass_pshader = rb_define_class_under(module, "PixelShader", klass);                   //
                rb_define_alloc_func(klass_pshader, [](VALUE k)->VALUE {                               //
                    auto s = new ::PixelShader;                                                        //
                    s->AddRefer();                                                                     //
                    return Data_Wrap_Struct(k, nullptr, DeleteShader<::PixelShader>, s);               //
                });                                                                                    //
                //Please use Ctrl-C and Ctrl-V                          --------------------------------|                                    
                rb_define_method(klass_pshader, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_pshader, "create_from_hlsl", (rubyfunc)create_from_hlsl, -1);
                rb_define_method(klass_pshader, "create_from_binfile", (rubyfunc)create_from_binfile, -1);
                rb_define_method(klass_pshader, "create_from_string", (rubyfunc)create_from_string, -1);

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
                rb_define_method(klass_sampler, "use_default", (rubyfunc)use_default<Sampler>, 0);
                rb_define_method(klass_sampler, "create_state", (rubyfunc)create_state<Sampler>, 1);
                rb_define_method(klass_sampler, "dump_description", 
                        (rubyfunc)dump_description<Sampler, D3D11_SAMPLER_DESC>, 0);
                rb_define_method(klass_sampler, "load_description", 
                        (rubyfunc)load_description<Sampler, D3D11_SAMPLER_DESC>, 1);

                /*
                D3D11_FILTER_MIN_MAG_MIP_POINT     *
                D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR *
                D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT  *
                D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR
                D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT  *
                D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR *
                D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT *
                D3D11_FILTER_MIN_MAG_MIP_LINEAR *
                D3D11_FILTER_ANISOTROPIC        *
                D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT  *
                D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR  *
                D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT *
                D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR  *
                D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT  *
                D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR *
                D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT *
                D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR *
                D3D11_FILTER_COMPARISON_ANISOTROPIC *
                D3D11_FILTER_TEXT_1BIT
                */
                rb_define_const(module, "FILTER_MIN_MAG_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_MAG_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_LINEAR_MAG_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_MIN_MAG_LINEAR_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT", INT2FIX(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT));
                rb_define_const(module, "FILTER_MIN_POINT_MAG_MIP_LINEAR", INT2FIX(D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR));
                rb_define_const(module, "FILTER_ANISOTROPIC", INT2FIX(D3D11_FILTER_ANISOTROPIC));
                rb_define_const(module, "FILTER_COMPARISON_MIN_MAG_MIP_POINT", INT2FIX(D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT", INT2FIX(D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT));
                rb_define_const(module, "FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR",INT2FIX(D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR));
                rb_define_const(module, "FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT",INT2FIX(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT));
                rb_define_const(module, "FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", INT2FIX(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR));
                rb_define_const(module, "FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT", INT2FIX(D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT));
                rb_define_const(module, "FILTER_COMPARISON_MIN_MAG_MIP_LINEAR", INT2FIX(D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR));
                rb_define_const(module, "FILTER_COMPARISON_ANISOTROPIC", INT2FIX(D3D11_FILTER_COMPARISON_ANISOTROPIC));

                //Address mode
                rb_define_const(module, "ADDRESS_WRAP", INT2FIX(D3D11_TEXTURE_ADDRESS_WRAP));
                rb_define_const(module, "ADDRESS_MIRROR", INT2FIX(D3D11_TEXTURE_ADDRESS_MIRROR));
                rb_define_const(module, "ADDRESS_MIRROR_ONCE", INT2FIX(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE));
                rb_define_const(module, "ADDRESS_CLAMP", INT2FIX(D3D11_TEXTURE_ADDRESS_CLAMP));
                rb_define_const(module, "ADDRESS_BORDER", INT2FIX(D3D11_TEXTURE_ADDRESS_BORDER));

                //blender
                klass_blender = rb_define_class_under(module, "Blender", rb_cObject);
                rb_define_alloc_func(klass_blender, [](VALUE k) -> VALUE{
                    Blender *b = new Blender;
                    b->AddRefer();
                    return Data_Wrap_Struct(k, nullptr, DeleteBlender, b);
                });
                rb_define_method(klass_blender, "initialize", (rubyfunc)initialize, 0);
                rb_define_method(klass_blender, "create_state", (rubyfunc)create_state<Blender>, 1);
                rb_define_method(klass_blender, "use_default", (rubyfunc)use_default<Blender>, 0);
                rb_define_method(klass_blender, "enable", (rubyfunc)blender_enable, 1);
                rb_define_method(klass_blender, "set_blend_factor", (rubyfunc)set_blend_factor, 1);
                rb_define_method(klass_blender, "dump_description",
                    (rubyfunc)dump_description<Blender, D3D11_BLEND_DESC>, 0);
                rb_define_method(klass_blender, "load_description",
                    (rubyfunc)load_description<Blender, D3D11_BLEND_DESC>, 1);
                rb_define_method(klass_blender, "set_color_blend", (rubyfunc)set_color_blend, 3);
                rb_define_method(klass_blender, "set_alpha_blend", (rubyfunc)set_alpha_blend, 3);
                rb_define_method(klass_blender, "set_mask", (rubyfunc)set_mask, 1);

                /*
                D3D11_BLEND_ZERO = 1
                D3D11_BLEND_ONE = 2
                D3D11_BLEND_SRC_COLOR = 3
                D3D11_BLEND_INV_SRC_COLOR = 4
                D3D11_BLEND_SRC_ALPHA = 5
                D3D11_BLEND_INV_SRC_ALPHA = 6
                D3D11_BLEND_DEST_ALPHA = 7
                D3D11_BLEND_INV_DEST_ALPHA = 8
                D3D11_BLEND_DEST_COLOR = 9
                D3D11_BLEND_INV_DEST_COLOR = 10
                D3D11_BLEND_SRC_ALPHA_SAT = 11
                D3D11_BLEND_BLEND_FACTOR = 14
                D3D11_BLEND_INV_BLEND_FACTOR = 15
                D3D11_BLEND_SRC1_COLOR = 16
                D3D11_BLEND_INV_SRC1_COLOR = 17
                D3D11_BLEND_SRC1_ALPHA = 18
                D3D11_BLEND_INV_SRC1_ALPHA = 19
                */
                rb_define_const(module, "BLEND_ZERO", INT2FIX(1));
                rb_define_const(module, "BLEND_ONE", INT2FIX(2));
                rb_define_const(module, "BLEND_SRC_COLOR", INT2FIX(3));
                rb_define_const(module, "BLEND_INV_SRC_COLOR", INT2FIX(4));
                rb_define_const(module, "BLEND_SRC_ALPHA", INT2FIX(5));
                rb_define_const(module, "BLEND_INV_SRC_ALPHA", INT2FIX(6));
                rb_define_const(module, "BLEND_DEST_ALPHA", INT2FIX(7));
                rb_define_const(module, "BLEND_INV_DEST_ALPHA", INT2FIX(8));
                rb_define_const(module, "BLEND_DEST_COLOR", INT2FIX(9));
                rb_define_const(module, "BLEND_INV_DEST_COLOR", INT2FIX(10));
                rb_define_const(module, "BLEND_SRC_ALPHA_SAT", INT2FIX(11));
                rb_define_const(module, "BLEND_BLEND_FACTOR", INT2FIX(14));
                rb_define_const(module, "BLEND_FACTOR", INT2FIX(14)); //alias
                rb_define_const(module, "BLEND_INV_BLEND_FACTOR", INT2FIX(15));
                rb_define_const(module, "BLEND_INV_FACTOR", INT2FIX(15)); //alias
                rb_define_const(module, "BLEND_SRC1_COLOR", INT2FIX(16));
                rb_define_const(module, "BLEND_INV_SRC1_COLOR", INT2FIX(17));
                rb_define_const(module, "BLEND_SRC1_ALPHA", INT2FIX(18));
                rb_define_const(module, "BLEND_INV_SRC1_ALPHA", INT2FIX(19));

                /*
                 D3D11_BLEND_OP_ADD = 1,
                 D3D11_BLEND_OP_SUBTRACT = 2,
                 D3D11_BLEND_OP_REV_SUBTRACT = 3,
                 D3D11_BLEND_OP_MIN = 4,
                 D3D11_BLEND_OP_MAX = 5,
                */
                rb_define_const(module, "BLEND_OP_ADD", INT2FIX(1));
                rb_define_const(module, "BLEND_OP_SUBTRACT", INT2FIX(2));
                rb_define_const(module, "BLEND_OP_REV_SUBTRACT", INT2FIX(3));
                rb_define_const(module, "BLEND_OP_MIN", INT2FIX(4));
                rb_define_const(module, "BLEND_OP_MAX", INT2FIX(5));

                /*
                    D3D11_COLOR_WRITE_ENABLE_RED     = 1,  
                    D3D11_COLOR_WRITE_ENABLE_GREEN   = 2,  
                    D3D11_COLOR_WRITE_ENABLE_BLUE    = 4,  
                    D3D11_COLOR_WRITE_ENABLE_ALPHA   = 8,  
                    D3D11_COLOR_WRITE_ENABLE_ALL     =   
      ( D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN |    
        D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA )   
                */
                rb_define_const(module, "COLOR_WRITE_ENABLE_RED", INT2FIX(1));
                rb_define_const(module, "COLOR_WRITE_ENABLE_GREEN", INT2FIX(2));
                rb_define_const(module, "COLOR_WRITE_ENABLE_BLUE", INT2FIX(4));
                rb_define_const(module, "COLOR_WRITE_ENABLE_ALPHA", INT2FIX(8));
                rb_define_const(module, "COLOR_WRITE_ENABLE_ALL", INT2FIX(D3D11_COLOR_WRITE_ENABLE_ALL));
            }
        }
    }
}