#pragma once
#include "DX.h"
#include "D3DDevice.h"

class D3DBuffer : public Utility::ReferredObject {
    int _size;
public:
    const int &size = _size;
    ComPtr<ID3D11Buffer> native_buffer;
    D3DBuffer(){}
    D3DBuffer(D3DDevice *device, size_t sz, UINT flags, void *init_data) {Initialize(device, sz, flags, init_data);}
    void Initialize(D3DDevice *device, size_t sz, UINT flags, void *init_data);
    void UnInitialize() {native_buffer.ReleaseAndGetAddressOf();}
    virtual void Release() {UnInitialize();}
};

class D3DConstantBuffer : public D3DBuffer {
public:
    D3DConstantBuffer() {}
    D3DConstantBuffer(D3DDevice *device, size_t sz, void *init_data = nullptr) { Initialize(device, sz, init_data); }
    void Initialize(D3DDevice *device, size_t sz, void *init_data = nullptr){
        if(sz % 16)throw std::invalid_argument("D3DConstantBuffer::Initialize: size should can be devied by 16");
        D3DBuffer::Initialize(device, sz, D3D11_BIND_CONSTANT_BUFFER, init_data);
    }
};

class D3DVertexBuffer : public D3DBuffer {
public:
    D3DVertexBuffer() {}
    D3DVertexBuffer(D3DDevice *device, size_t sz, void *init_data = nullptr) { Initialize(device, sz, init_data); }
    void Initialize(D3DDevice *device, size_t sz, void *init_data = nullptr) 
    {D3DBuffer::Initialize(device, sz, D3D11_BIND_VERTEX_BUFFER, init_data);}
};

namespace Ext {
    namespace DX {
        namespace D3DBuffer {
            extern VALUE klass;
            extern VALUE klass_vbuffer;
            extern VALUE klass_cbuffer;
            void Init();
        }
    }
}