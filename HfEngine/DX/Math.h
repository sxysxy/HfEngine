#pragma once
#include <stdafx.h>
#include <DirectXMath.h>
#ifndef DIRECTX_MATH_VERSION
#include <xnamath.h>
#else
using namespace DirectX;
#endif

namespace Ext {
    namespace DX {
        namespace Math {
            extern VALUE module_DXMath;
            extern VALUE klass_Vector;
            extern VALUE klass_Matrix;

            struct XMVector {
                XMVECTOR v;
            };

            void Init();
        }
    }
}