#pragma once
#include "stdafx.h"
#include <DX.h>

class D3DBuffer : public Utility::ReferredObject {
    ComPtr<ID3D11Buffer> native_buffer;
};