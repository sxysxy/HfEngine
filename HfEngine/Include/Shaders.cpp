#include "Shaders.h"
#include "stdafx.h"

void VertexShader::CreateFromHLSLFile(D3DDevice * device, std::wstring & filename){
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

void VertexShader::CreateFromString(D3DDevice * device, std::string & code){
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

void PixelShader::CreateFromHLSLFile(D3DDevice * device, std::wstring & filename){
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

void PixelShader::CreateFromString(D3DDevice * device, std::string & code){
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
            extern VALUE klass;
            extern VALUE klass_vshader;
            extern VALUE klass_pshader;
            extern VALUE klass_sampler;
            extern VALUE klass_eShaderCompileError;

            void Init() {
                klass = rb_define_class_under(module, "Shader", rb_cObject);

                klass_vshader = rb_define_class_under(module, "VertexShader", rb_cObject);
            }
        }
    }
}