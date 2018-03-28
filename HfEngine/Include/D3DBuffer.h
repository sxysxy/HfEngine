#pragma once
#include "stdafx.h"
#include "DX.h"
#include "D3DDevice.h"

class D3DBuffer : public Utility::ReferredObject {
    int _size;
public:
    const int &size = _size;
    ComPtr<ID3D11Buffer> native_buffer;
    D3DBuffer() {}
    void Initialize(D3DDevice *device, UINT usage, UINT flag, size_t size, const void *init_data);
    void UnInitialize() {
        native_buffer.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class ConstantBuffer : public D3DBuffer {
public:
    ConstantBuffer() {}
    ConstantBuffer(D3DDevice *device, size_t sz, const void *init_data = nullptr) { Initialize(device, sz, init_data); }
    void Initialize(D3DDevice *device, size_t sz, const void *init_data = nullptr) {
        if (sz % 16)throw std::invalid_argument("ConstantBuffer::Initialize: size should can be devied by 16");
        D3DBuffer::Initialize(device, D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, sz, init_data);
    }
};

class VertexBuffer : public D3DBuffer {
    size_t _size_per_vertex;
public:
    const size_t &size_per_vertex = _size_per_vertex;
    VertexBuffer() {}
    VertexBuffer(D3DDevice *device, size_t per_vertex, size_t num_vertex, const void *init_data = nullptr) { 
        Initialize(device, per_vertex, num_vertex, init_data); 
    }
    void Initialize(D3DDevice *device, size_t per_vertex, size_t num_vertex, const void *init_data = nullptr) {
        _size_per_vertex = per_vertex;
        D3DBuffer::Initialize(device, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, per_vertex * num_vertex, init_data);
    }
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