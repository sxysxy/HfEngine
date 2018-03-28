#include "D3DBuffer.h"

void D3DBuffer::Initialize(D3DDevice *device, UINT usage, UINT flag, size_t size, const void *init_data) {
    D3D11_BUFFER_DESC bd;
    RtlZeroMemory(&bd, sizeof bd);
    bd.BindFlags = flag;
    bd.Usage = (D3D11_USAGE)usage;
    bd.ByteWidth = size;
    D3D11_SUBRESOURCE_DATA data;
    if (init_data) {
        RtlZeroMemory(&data, sizeof data);
        data.pSysMem = init_data;
    }
    HRESULT hr = device->native_device->CreateBuffer(&bd, init_data ? &data : nullptr, &native_buffer);
    if (FAILED(hr))
        MAKE_ERRMSG<std::runtime_error>("Failed to create D3DBuffer, Error code:", hr);
    _size = (int)size;
}

namespace Ext {
    namespace DX {
        namespace D3DBuffer {
            VALUE klass;
            VALUE klass_vbuffer;
            VALUE klass_cbuffer;

            static VALUE initialize_d3dbuffer(int argc, VALUE *argv, VALUE self) {
                rb_raise(rb_eNotImpError, "Note: D3DBuffer class is only an abstract class, you should not call D3DBuffer.new");
                return Qnil;
            }

            template<class T>
            void Delete(T *bf) {
                bf->SubRefer();
            }

            template<class T>
            VALUE New(VALUE k) {
                auto b = new T;
                b->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete<T>, b);
            }

            VALUE vbuffer_initialize(int argc, VALUE *argv, VALUE self) {
                if (argc < 3 || argc > 4)rb_raise(rb_eArgError,
                    "VertexBuffer::initialize:Wrong number of arguments. expecting (3..4) but got %d", argc);
                if (!rb_obj_is_kind_of(argv[0], Ext::DX::D3DDevice::klass)) {
                    rb_raise(rb_eArgError, "VertexBuffer::initialize:The first param 'device' should be a DX::D3DDevice");
                }
                void *init_data = nullptr;
                if (argc == 4) {
                    if (rb_obj_is_kind_of(argv[3], rb_cInteger)) {
                        init_data = (void *)FIX2PTR(argv[3]);

                    }
                    else if (rb_obj_is_kind_of(argv[3], rb_cString)) {
                        init_data = (void *)rb_string_value_ptr(&argv[3]);
                    }
                    else {
                        rb_raise(rb_eArgError,
                            "VertexBuffer::initialize: The forth param(initial data) could be a String(providing a buffer) \
                            or a Integer(providing an address. e.g from Fiddle::Pointer).");
                    }
                }
                auto buf = GetNativeObject<VertexBuffer>(self);
                auto device = GetNativeObject<::D3DDevice>(argv[0]);
                try {
                    buf->Initialize(device, FIX2INT(argv[1]), FIX2INT(argv[2]), init_data);
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            VALUE cbuffer_initialize(int argc, VALUE *argv, VALUE self) {
                if (argc < 2 || argc > 3)rb_raise(rb_eArgError,
                    "ConstantBuffer::initialize:Wrong number of arguments. expecting (2..3) but got %d", argc);
                if (!rb_obj_is_kind_of(argv[0], Ext::DX::D3DDevice::klass)) {
                    rb_raise(rb_eArgError, "D3D...Buffer::initialize:The first param 'device' should be a DX::D3DDevice");
                }
                void *init_data = nullptr;
                if (argc == 3) {
                    if (rb_obj_is_kind_of(argv[2], rb_cInteger)) {
                        init_data = (void *)FIX2PTR(argv[2]);

                    }
                    else if (rb_obj_is_kind_of(argv[2], rb_cString)) {
                        init_data = (void *)rb_string_value_ptr(&argv[2]);
                    }
                    else {
                        rb_raise(rb_eArgError,
                            "ConstantBuffer::initialize: The third param(initial data) could be a String(providing a buffer) \
                            or a Integer(providing an address. e.g from Fiddle::Pointer).");
                    }
                }
                auto buf = GetNativeObject<ConstantBuffer>(self);
                auto device = GetNativeObject<::D3DDevice>(argv[0]);
                try {
                    buf->Initialize(device, FIX2INT(argv[1]), init_data);
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            void Init() {
                klass = rb_define_class_under(module, "D3DBuffer", rb_cObject);
                rb_define_method(klass, "initialize", (rubyfunc)initialize_d3dbuffer, -1);

                klass_vbuffer = rb_define_class_under(module, "VertexBuffer", klass);
                rb_define_alloc_func(klass_vbuffer, New<::VertexBuffer>);
                rb_define_method(klass_vbuffer, "initialize", (rubyfunc)vbuffer_initialize, -1);

                klass_cbuffer = rb_define_class_under(module, "ConstantBuffer", klass);
                rb_define_alloc_func(klass_cbuffer, New<::ConstantBuffer>);
                rb_define_method(klass_cbuffer, "initialize", (rubyfunc)cbuffer_initialize, -1);
            }
        }
    }
}