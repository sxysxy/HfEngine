#include "Math.h"
#include "DX.h"

namespace Ext {
    namespace DX {
        namespace Math {
            VALUE module_DXMath;
            VALUE klass_Vector;
            VALUE klass_Matrix;

            template<class T>
            void Delete(T *d) {
                delete d;
            }
            template<class T>
            VALUE New(VALUE klass) {
                return Data_Wrap_Struct(klass, nullptr, Delete<T>, new T);
            }
            static VALUE Vinitialize(VALUE self, VALUE x, VALUE y, VALUE z, VALUE w) {
                return self;
            }

            void Init() {
                module_DXMath = rb_define_module_under(module, "Math");
                klass_Vector = rb_define_class_under(module_DXMath, "Vector", rb_cObject);
                rb_define_alloc_func(klass_Vector, New<XMVECTOR>);
                

                klass_Matrix = rb_define_class_under(module_DXMath, "Matrix", rb_cObject);
                rb_define_alloc_func(klass_Matrix, New<XMMATRIX>);
            }
        }
    }
}