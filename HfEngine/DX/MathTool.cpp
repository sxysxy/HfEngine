#include "../Include/extension.h"
#include "../Include/stdafx.h"
#include "../Include/MathTool.h"
#include <DirectXMath.h>
using namespace DirectX;

namespace Ext {
    namespace MathTool {
        VALUE module;
        VALUE klass_matrix4x4;
        VALUE klass_vector4;
        
        template<class T>
        void Delete(T *t) {
            delete t;
        }

        template<class T>
        VALUE New(VALUE k) {
            auto p = new T;
            return Data_Wrap_Struct(k, nullptr, Delete<T>, p);
        }

        static VALUE M_identity(VALUE self) {
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            XMStoreFloat4x4(m, XMMatrixIdentity());
            return self;
        }
        static VALUE M_tranpose_to(VALUE self) {
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            XMMATRIX xm = XMLoadFloat4x4(m);
            XMStoreFloat4x4(m, XMMatrixTranspose(xm));
            return self;
        }
        static VALUE M_tranpose(VALUE self) {
            auto obj = New<XMFLOAT4X4>(klass_matrix4x4);
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            memcpy(nobj, m, sizeof XMFLOAT4X4);
            M_tranpose_to(obj);
            return obj;
        }
        static VALUE M_matrix_mul_to(VALUE self, VALUE _om) {
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            XMMATRIX xm = XMLoadFloat4x4(m);
            auto om = GetNativeObject<XMFLOAT4X4>(_om);
            XMMATRIX xom = XMLoadFloat4x4(om);
            XMMATRIX res = xm * xom;
            XMStoreFloat4x4(m, res);
            return self;
        }

        //returns a new object
        static VALUE M_matrix_mul(VALUE self, VALUE _om) {
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            XMMATRIX xm = XMLoadFloat4x4(m);
            auto om = GetNativeObject<XMFLOAT4X4>(_om);
            XMMATRIX xom = XMLoadFloat4x4(om);
            XMMATRIX res = xm * xom;
            //...
            auto resm = new XMFLOAT4X4;
            XMStoreFloat4x4(resm, res);
            return Data_Wrap_Struct(klass_matrix4x4, nullptr, Delete<>, resm);
        }

        static VALUE M_set(VALUE self, VALUE m, VALUE n, VALUE v) {
            auto mt = GetNativeObject<XMFLOAT4X4>(self);
            mt->m[FIX2INT(m)][FIX2INT(n)] = (float)rb_float_value(v);
            return v;
        }

        //returns a String
        template<class T>
        static VALUE M_row_data(VALUE self) {
            auto m = GetNativeObject<T>(self);
            auto d = rb_str_buf_new(sizeof T);
            char *p = RSTRING_PTR(d);
            memcpy(p, m, sizeof T);
            return d;
        }

        static VALUE M_array_data(VALUE self) {
            auto m = GetNativeObject<XMFLOAT4X4>(self);
            VALUE as[4];
            for(int i = 0; i < 4; i++)as[i] = rb_ary_new();
            for (int i = 0; i < 4; i++) {
                rb_ary_resize(as[i], 4);
                VALUE *p = RARRAY_PTR(as[i]);
                for (int j = 0; j < 4; j++) {
                    p[j] = rb_float_new(m->m[i][j]);
                }
            }
            return rb_ary_new_from_args(4, as[0], as[1], as[2], as[3]);
        }

        template<class T>
        static VALUE M_copy(VALUE self, VALUE o) {
            auto m = GetNativeObject<T>(self);
            auto om = GetNativeObject<T>(o);
            memcpy(m, om, sizeof T);
            return self;
        }

        void Init() {
            module = rb_define_module("MathTool");
            klass_matrix4x4 = rb_define_class_under(module, "Matrix4x4", rb_cObject);
            rb_define_alloc_func(klass_matrix4x4, New<XMFLOAT4X4>);
            rb_define_method(klass_matrix4x4, "initialize", (rubyfunc)M_identity, 0);
            rb_define_method(klass_matrix4x4, "identity!", (rubyfunc)M_identity, 0);
            rb_define_method(klass_matrix4x4, "tranpose", (rubyfunc)M_tranpose, 0);
            rb_define_method(klass_matrix4x4, "tranpose!", (rubyfunc)M_tranpose_to, 0);
            rb_define_method(klass_matrix4x4, "*=", (rubyfunc)M_matrix_mul_to, 1);
            rb_define_method(klass_matrix4x4, "*", (rubyfunc)M_matrix_mul, 1);
            rb_define_method(klass_matrix4x4, "=", (rubyfunc)M_copy<XMFLOAT4X4>, 1);
            rb_define_method(klass_matrix4x4, "[]", (rubyfunc)M_set, 3);
            rb_define_method(klass_matrix4x4, "row_data", (rubyfunc)M_row_data<XMFLOAT4X4>, 0);
            rb_define_method(klass_matrix4x4, "array_data", (rubyfunc)M_array_data, 0);

            klass_vector4 = rb_define_class_under(module, "Vector4", rb_cObject);
            rb_define_alloc_func(klass_vector4, New<XMFLOAT4>);
        }
    }
}