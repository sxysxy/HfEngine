#include "../Include/DX.h"
#include "../Include/Input.h"
#include "../Include/extension.h"
#include "../Include/D3DDevice.h"
#include "../Include/Shaders.h"
#include "../Include/D3DBuffer.h"
#include "../Include/RenderPipeline.h"
#include "../Include/Texture2D.h"
#include "../Include/SwapChain.h"

namespace Ext{
    namespace DX{
        VALUE module;

        VALUE klass_HFRect;
        VALUE klass_HFColor;
        void HFRDelete(Utility::Rect *r) {
            delete r;
        }
        VALUE HFRNew(VALUE k) {
            return Data_Wrap_Struct(k, nullptr, HFRDelete, new Utility::Rect);
        }
        static VALUE HFR_x(VALUE self) {
            return INT2FIX(GetNativeObject<Utility::Rect>(self)->x);
        }
        static VALUE HFR_y(VALUE self) {
            return INT2FIX(GetNativeObject<Utility::Rect>(self)->y);
        }
        static VALUE HFR_w(VALUE self) {
            return INT2FIX(GetNativeObject<Utility::Rect>(self)->w);
        }
        static VALUE HFR_h(VALUE self) {
            return INT2FIX(GetNativeObject<Utility::Rect>(self)->h);
        }
        static VALUE HRF_setx(VALUE self, VALUE sx) {
            GetNativeObject<Utility::Rect>(self)->x = FIX2INT(sx);
            return sx;
        }
        static VALUE HRF_sety(VALUE self, VALUE sx) {
            GetNativeObject<Utility::Rect>(self)->y = FIX2INT(sx);
            return sx;
        }
        static VALUE HRF_setw(VALUE self, VALUE sx) {
            GetNativeObject<Utility::Rect>(self)->w = FIX2INT(sx);
            return sx;
        }
        static VALUE HRF_seth(VALUE self, VALUE sx) {
            GetNativeObject<Utility::Rect>(self)->h = FIX2INT(sx);
            return sx;
        }
        static VALUE HFR_initialize(int argc, VALUE *argv, VALUE self) {
            if (argc == 1) {
                if(!rb_obj_is_kind_of(argv[0], klass_HFRect))
                    rb_raise(rb_eArgError, "HFRect::initalize: If you give one param please ensure it is a HFRect");
                (*GetNativeObject<Utility::Rect>(self)) = (*GetNativeObject<Utility::Rect>(argv[0]));
            }
            else if (argc == 4) {
                auto rect = GetNativeObject<Utility::Rect>(self);
                rect->x = FIX2INT(argv[0]);
                rect->y = FIX2INT(argv[1]);
                rect->w = FIX2INT(argv[2]);
                rect->h = FIX2INT(argv[3]);
            }
            else {
                rb_raise(rb_eArgError, "HFRect::initialize: Wrong number of arguments. expecting 1 or 4 but got %d", argc);
            }
            return self;
        }
        static VALUE HFR(int argc, VALUE *argv, VALUE self) {
            VALUE r = HFRNew(klass_HFRect);
            return rb_funcall2(r, rb_intern("initialize"), argc, argv);
            return r;
        }

#pragma warning(push)
#pragma warning(disable:4244)

        static void CRGBADelete(Utility::Color *c) {
            delete c;
        }
        static VALUE CRGBANew(VALUE k) {
            return Data_Wrap_Struct(k, nullptr, CRGBADelete, new Utility::Color);
        }
        static VALUE CRGBA_r(VALUE self) {
            return rb_float_new(GetNativeObject<Utility::Color>(self)->r);
        }
        static VALUE CRGBA_g(VALUE self) {
            return rb_float_new(GetNativeObject<Utility::Color>(self)->g);
        }
        static VALUE CRGBA_b(VALUE self) {
            return rb_float_new(GetNativeObject<Utility::Color>(self)->b);
        }
        static VALUE CRGBA_a(VALUE self) {
            return rb_float_new(GetNativeObject<Utility::Color>(self)->a);
        }
        static VALUE CRBGA_sr(VALUE self, VALUE r) {
            GetNativeObject<Utility::Color>(self)->r = rb_float_value(r);
            return r;
        }
        static VALUE CRBGA_sg(VALUE self, VALUE r) {
            GetNativeObject<Utility::Color>(self)->g = rb_float_value(r);
            return r;
        }
        static VALUE CRBGA_sb(VALUE self, VALUE r) {
            GetNativeObject<Utility::Color>(self)->b = rb_float_value(r);
            return r;
        }
        static VALUE CRBGA_sa(VALUE self, VALUE r) {
            GetNativeObject<Utility::Color>(self)->a = rb_float_value(r);
            return r;
        }
        static VALUE CRGBA_row_data_ptr(VALUE self) {
#ifdef _WIN64
            return ULL2NUM((unsigned long long)GetNativeObject<Utility::Color>(self));
#else
            return INT2NUM((int)GetNativeObject<Utility::Color>(self));
#endif
        }
        static VALUE CRGBA_initialize(int argc, VALUE *argv, VALUE self) {
            if (argc == 1) {
                if (!rb_obj_is_kind_of(argv[0], klass_HFColor))
                    rb_raise(rb_eArgError, "HFColor::initalize: If you give one param please ensure it is a HFColor");
                (*GetNativeObject<Utility::Color>(self)) = (*GetNativeObject<Utility::Color>(argv[0]));
            }
            else if (argc == 4) {
                auto color = GetNativeObject<Utility::Color>(self);
                color->r = rb_float_value(argv[0]);
                color->g = rb_float_value(argv[1]);
                color->b = rb_float_value(argv[2]);
                color->a = rb_float_value(argv[3]);
            }
            else {
                rb_raise(rb_eArgError, "HFColor::initialize: Wrong number of arguments. expecting 1 or 4 but got %d", argc);
            }
            return self;
        }
        static VALUE CRGBA(int argc, VALUE *argv, VALUE self) {
            VALUE c = CRGBANew(klass_HFColor);
            rb_funcall2(c, rb_intern("initialize"), argc, argv);
            return c;
        }

        void BasicInit() {
            klass_HFRect = rb_define_class("HFRect", rb_cObject);
            rb_define_alloc_func(klass_HFRect, HFRNew);
            rb_define_method(klass_HFRect, "x", (rubyfunc)HFR_x, 0);
            rb_define_method(klass_HFRect, "y", (rubyfunc)HFR_y, 0);
            rb_define_method(klass_HFRect, "w", (rubyfunc)HFR_w, 0);
            rb_define_method(klass_HFRect, "h", (rubyfunc)HFR_h, 0);
            rb_define_method(klass_HFRect, "x=", (rubyfunc)HRF_setx, 1);
            rb_define_method(klass_HFRect, "y=", (rubyfunc)HRF_sety, 1);
            rb_define_method(klass_HFRect, "w=", (rubyfunc)HRF_setw, 1);
            rb_define_method(klass_HFRect, "h=", (rubyfunc)HRF_seth, 1);
            rb_define_method(klass_HFRect, "initialize", (rubyfunc)HFR_initialize, -1);
            rb_define_module_function(rb_mKernel, "HFRect", (rubyfunc)HFR, -1);

            klass_HFColor = rb_define_class("HFColorRGBA", rb_cObject);
            rb_define_alloc_func(klass_HFColor, CRGBANew);
            rb_define_method(klass_HFColor, "r", (rubyfunc)CRGBA_r, 0);
            rb_define_method(klass_HFColor, "g", (rubyfunc)CRGBA_g, 0);
            rb_define_method(klass_HFColor, "b", (rubyfunc)CRGBA_b, 0);
            rb_define_method(klass_HFColor, "a", (rubyfunc)CRGBA_a, 0);
            rb_define_method(klass_HFColor, "r=", (rubyfunc)CRBGA_sr, 1);
            rb_define_method(klass_HFColor, "g=", (rubyfunc)CRBGA_sg, 1);
            rb_define_method(klass_HFColor, "b=", (rubyfunc)CRBGA_sb, 1);
            rb_define_method(klass_HFColor, "a=", (rubyfunc)CRBGA_sa, 1);
            rb_define_method(klass_HFColor, "row_data_ptr", (rubyfunc)CRGBA_row_data_ptr, 0);
            rb_define_method(klass_HFColor, "initialize", (rubyfunc)CRGBA_initialize, -1);
            rb_define_module_function(rb_mKernel, "HFColorRGBA", (rubyfunc)CRGBA, -1);
        }
#pragma warning(pop)

        void Ext::DX::Init() {
            module = rb_define_module("DX");
            BasicInit();
            Ext::DX::Input::Init();
            Ext::DX::D3DDevice::Init();
            Ext::DX::Shader::Init();
            Ext::DX::D3DBuffer::Init();
            Ext::DX::Texture::Init();
            Ext::DX::RenderPipeline::Init();
            Ext::DX::SwapChain::Init();
        }
       
    }
}
