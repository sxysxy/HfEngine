#include "../Include/extension.h"
#include "../Include/stdafx.h"
#include "../Include/MathTool.h"
#include <DirectXMath.h>
using namespace DirectX;

#pragma warning(push)
#pragma warning(disable : 4244)

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
        static VALUE M_initialize(VALUE self) {
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
            CheckArgs({ _om }, { klass_matrix4x4 });
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
            CheckArgs({ _om }, {klass_matrix4x4});
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
#ifdef _DEBUG
            if(!RB_FIXNUM_P(m) || !RB_FIXNUM_P(n) || !RB_FLOAT_TYPE_P(v))
                rb_raise(rb_eArgError, "Matrix4x4#set(m : Fixnum, n : Fixnum, v : Float)");
#endif
            auto mt = GetNativeObject<XMFLOAT4X4>(self);
            mt->m[FIX2INT(m)][FIX2INT(n)] = (float)rb_float_value(v);
            return v;
        }

        static VALUE M_get(VALUE self, VALUE m, VALUE n) {
            auto mt = GetNativeObject<XMFLOAT4X4>(self);
            return rb_float_new(mt->m[FIX2INT(m)][FIX2INT(n)]);
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
        template<class T>
        static VALUE M_row_data_ptr(VALUE self) {
            auto m = GetNativeObject<T>(self);
#ifdef _WIN64
            return ULL2NUM((INT64)m);
#else
            return INT2NUM((int)m);
#endif
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


        //
        static VALUE identity(VALUE self) {
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto m = GetNativeObject<XMFLOAT4X4>(obj);
            XMStoreFloat4x4(m, XMMatrixIdentity());
            return self;
        }
        static VALUE perspective(VALUE self, VALUE fovangleY, VALUE aspect, VALUE znear, VALUE zfar) {
            // if you called MathTool.perspective, it returns a new matrix4x4
            // if you called Matrix4x4#perspective!, it change the object it self
            CheckAllFloat({fovangleY, aspect, znear, zfar});
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m = XMMatrixPerspectiveFovLH(RFLOAT_VALUE(fovangleY), RFLOAT_VALUE(aspect), 
                RFLOAT_VALUE(znear), RFLOAT_VALUE(zfar));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        XMVECTOR ary2vec4(VALUE a) {
            if(RARRAY_LEN(a) < 4)
                rb_raise(rb_eArgError, "MathTool : the vector array you provide should at least contains 4 float numbers");
            VALUE *p = RARRAY_PTR(a);
            return XMVectorSet(RFLOAT_VALUE(p[0]), 
                RFLOAT_VALUE(p[1]),
                RFLOAT_VALUE(p[2]),
                RFLOAT_VALUE(p[3]));
                               
        }
        static VALUE lookat(int argc, VALUE *argv, VALUE self) {
            if(argc < 2 || argc > 3)
                rb_raise(rb_eArgError,
                "Mathtool::lookat(eyepos, target, [up = [0.0, 1.0, 0.0, 0.0]] expecting (2..3) args but got %d", argc);
            CheckAllFloat({argv[0], argv[1]});
            VALUE eyepos = argv[0];
            VALUE target = argv[1];
            VALUE up = argc == 3? argv[2] : 0;
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            XMMATRIX m;
            if(up)
                m = XMMatrixLookAtLH(ary2vec4(eyepos), ary2vec4(target), ary2vec4(up));
            else
                m = XMMatrixLookAtLH(ary2vec4(eyepos), ary2vec4(target), XMVectorSet(0.0, 1.0, 0.0, 0.0));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        static VALUE rotate_round(VALUE self, VALUE vector, VALUE angle) {
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m =XMMatrixRotationAxis(ary2vec4(vector), RFLOAT_VALUE(angle));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        static VALUE rotateX(VALUE self, VALUE angle) {
            CheckAllFloat({angle});
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m = XMMatrixRotationX(RFLOAT_VALUE(angle));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        static VALUE rotateY(VALUE self, VALUE angle) {
            CheckAllFloat({ angle });
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m = XMMatrixRotationY(RFLOAT_VALUE(angle));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        static VALUE rotateZ(VALUE self, VALUE angle) {
            CheckAllFloat({ angle });
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            auto m = XMMatrixRotationZ(RFLOAT_VALUE(angle));
            XMStoreFloat4x4(nobj, m);
            return obj;
        }
        static VALUE move(VALUE self, VALUE mx, VALUE my, VALUE mz) {
            CheckAllFloat({ mx, my, mz });
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            identity(obj);
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            nobj->_41 = RFLOAT_VALUE(mx);
            nobj->_42 = RFLOAT_VALUE(my);
            nobj->_43 = RFLOAT_VALUE(mz);
            return obj;
        }
        static VALUE zoom(VALUE self, VALUE zoomx, VALUE zoomy, VALUE zoomz) {
            CheckAllFloat({zoomx, zoomy, zoomz});
            auto obj = self == module ? New<XMFLOAT4X4>(klass_matrix4x4) : self;
            identity(obj);
            auto nobj = GetNativeObject<XMFLOAT4X4>(obj);
            nobj->_11 = RFLOAT_VALUE(zoomx);
            nobj->_22 = RFLOAT_VALUE(zoomy);
            nobj->_33 = RFLOAT_VALUE(zoomz);
            return obj;
        }

        /*
        template<class T>
        inline void defmtfunc(const char *name, T f, int argc) {
            rb_define_module_function(module, name, (rubyfunc)f, argc);
            rb_define_method(module, name, (rubyfunc)f, argc);
        };
        */
#define defmtfunc(name, f, argc) \
        rb_define_module_function(module, name, (rubyfunc)f, argc); \
        rb_define_method(klass_matrix4x4, name##"!", (rubyfunc)f, argc);

        void Init() {
            module = rb_define_module("MathTool");
            klass_matrix4x4 = rb_define_class_under(module, "Matrix4x4", rb_cObject);
            rb_define_alloc_func(klass_matrix4x4, New<XMFLOAT4X4>);
            rb_define_method(klass_matrix4x4, "initialize", (rubyfunc)M_initialize, 0);
            rb_define_method(klass_matrix4x4, "tranpose", (rubyfunc)M_tranpose, 0);
            rb_define_method(klass_matrix4x4, "tranpose!", (rubyfunc)M_tranpose_to, 0);
            rb_define_method(klass_matrix4x4, "*=", (rubyfunc)M_matrix_mul_to, 1);
            rb_define_method(klass_matrix4x4, "*", (rubyfunc)M_matrix_mul, 1);
            rb_define_method(klass_matrix4x4, "=", (rubyfunc)M_copy<XMFLOAT4X4>, 1);
            rb_define_method(klass_matrix4x4, "[]=", (rubyfunc)M_set, 3);
            rb_define_method(klass_matrix4x4, "[]", (rubyfunc)M_get, 2);
            rb_define_method(klass_matrix4x4, "row_data", (rubyfunc)M_row_data<XMFLOAT4X4>, 0);
            rb_define_method(klass_matrix4x4, "row_data_ptr", (rubyfunc)M_row_data_ptr<XMFLOAT4X4>, 0);
            rb_define_method(klass_matrix4x4, "array_data", (rubyfunc)M_array_data, 0);

            //
            rb_define_const(module, "PI", rb_float_new(XM_PI));
            rb_define_const(module, "PIDIV2", rb_float_new(XM_PIDIV2));
            rb_define_const(module, "PIDIV4", rb_float_new(XM_PIDIV4));
            
            defmtfunc("identity", identity, 0);
            defmtfunc("perspective", perspective, 4);
            defmtfunc("lookat", lookat, -1);
            defmtfunc("rotate_round", rotate_round, 2);
            defmtfunc("rotateX", rotateX, 1);
            defmtfunc("rotateY", rotateY, 1);
            defmtfunc("rotateZ", rotateZ, 1);
            defmtfunc("move", move, 3);
            defmtfunc("zoom", zoom, 3);
#undef defmtfunc
            //klass_vector4 = rb_define_class_under(module, "Vector4", rb_cObject);
            //rb_define_alloc_func(klass_vector4, New<XMFLOAT4>);

            
        }
    }
}
#pragma warning(pop)