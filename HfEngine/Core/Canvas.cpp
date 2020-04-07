#include <Core/RubyVM.h>
#include <Core/Canvas.h>
#include <Core/GDevice.h>
#include <Core/Basic.h>

HFENGINE_NAMESPACE_BEGIN

void Canvas::CreateViews() {
    auto device = GDevice::GetInstance();
    D3D11_TEXTURE2D_DESC desc;
    native_texture2d->GetDesc(&desc);
    HRESULT hr = S_FALSE;
    if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) &&
        FAILED(hr = device->native_device->CreateShaderResourceView(reinterpret_cast<ID3D11Resource*>(native_texture2d.Get()),
            0, &native_shader_resource_view))) {
        THROW_ERROR_CODE(std::runtime_error, "Failed to create shader resource view, Error code:", hr);
    }
    if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) &&
        FAILED(hr = device->native_device->CreateRenderTargetView(reinterpret_cast<ID3D11Resource*>(native_texture2d.Get()), 0,
            &native_render_target_view))) {
        THROW_ERROR_CODE(std::runtime_error, "Failed to create render target view, Error code:", hr);
    }
    
    D3D11_TEXTURE2D_DESC depthTexDesc;
    ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
    depthTexDesc.Width = width;
    depthTexDesc.Height = height;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    hr = device->native_device->CreateTexture2D(&depthTexDesc, nullptr, native_depth_stencil_texture.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        THROW_ERROR_CODE(std::runtime_error, "Failed to create depth stencil texture", hr);
        return;
    }
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory(&dsvd, sizeof(dsvd));
    dsvd.Format = depthTexDesc.Format;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;
    hr = device->native_device->CreateDepthStencilView(reinterpret_cast<ID3D11Resource *>(native_depth_stencil_texture.Get()), 
                    &dsvd, native_depth_stencil_view.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        THROW_ERROR_CODE(std::runtime_error, "Failed to create depth stencil view", hr);
        return;
    }
    
}

void Canvas::Initialize(const std::wstring& filename) {
    auto device = GDevice::GetInstance();
    D3DX11_IMAGE_LOAD_INFO info;
    RtlZeroMemory(&info, sizeof info);
    info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    info.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    info.MipFilter = D3DX11_FILTER_LINEAR;
    info.MipLevels = D3DX11_DEFAULT;
    HRESULT hr;
    if (FAILED(hr = D3DX11CreateTextureFromFileW(device->native_device.Get(), filename.c_str(), &info, nullptr,
        reinterpret_cast<ID3D11Resource**>(native_texture2d.GetAddressOf()), 0))) {
        if (hr == 0x887c0002) {
            std::string s;
            U16ToU8(filename.c_str(), s);
            THROW_ERROR_CODE(std::runtime_error, ("Failed to create D3D Texture2D from file(file : " + s + " not found), " + "Error code:").c_str(), hr);
        }
        THROW_ERROR_CODE(std::runtime_error, "Failed to create D3D Texture2D from file, Error code:", hr);
    }
    D3D11_TEXTURE2D_DESC desc;
    native_texture2d->GetDesc(&desc);
    _width = desc.Width;
    _height = desc.Height;
    CreateViews();
}

void Canvas::Initialize(int w, int h, const void* init_data) {
    auto device = GDevice::GetInstance();
    _width = w, _height = h;
    D3D11_TEXTURE2D_DESC td;
    RtlZeroMemory(&td, sizeof td);
    td.Width = width;
    td.Height = height;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    HRESULT hr;

    D3D11_SUBRESOURCE_DATA data;
    if (init_data) {
        data.pSysMem = init_data;
        data.SysMemPitch = w * 4;
        data.SysMemSlicePitch = 0;
    }
    if (FAILED(hr = device->native_device->CreateTexture2D(&td, init_data ? &data : nullptr, &native_texture2d))) {
        THROW_ERROR_CODE(std::runtime_error, "Failed to create D3D Texture2D, Error code:", hr);
    }
    
    CreateViews();
}

thread_local RClass* ClassCanvas;
static mrb_data_type ClassCanvasDataType = mrb_data_type{ "Canvas", [](mrb_state* mrb, void* ptr) {
    static_cast<Canvas*>(ptr)->SubRefer();
} };

/*[DOCUMENT]
method: HEG::Canvas::new(filename : String) | HEG::Canvas::new(w : Fixnum, h : Fixnum, init_data = nil : String/Fixnum) -> canvas : Canvas
note: Create Canvas object from image file or from given init_data
*/
static mrb_value ClassCanvas_new(mrb_state* mrb, mrb_value klass) {
    mrb_int argc = mrb_get_argc(mrb);
    Canvas* cv = nullptr;
    mrb_int w, h;
    LPVOID pdata = nullptr;
    if (argc == 1) {
        mrb_value fn;
        mrb_get_args(mrb, "S", &fn);
        std::wstring fnw;
        U8ToU16(RSTRING_PTR(fn), fnw);
        cv = new Canvas();
        try {
            cv->Initialize(fnw);
            cv->AddRefer();
            return mrb_obj_value(mrb_data_object_alloc(mrb, ClassCanvas, cv, &ClassCanvasDataType));
        }
        catch (std::exception & e) {
            mrb_raise(mrb, E_RUNTIME_ERROR, (std::string("Canvas::new: failed to create canvas: ") + e.what()).c_str());
        }
    }
    else if (argc == 2) {
        mrb_get_args(mrb, "ii", &w, &h);
    }
    else if (argc == 3) {
        mrb_value data_obj;
        mrb_get_args(mrb, "iio", &w, &h, &data_obj);
        if (data_obj.tt == MRB_TT_STRING)
            pdata = (LPVOID)RSTRING_PTR(data_obj);
        else if (data_obj.tt == MRB_TT_FIXNUM)
            pdata = (LPVOID)data_obj.value.i;
        else if (mrb_nil_p(data_obj)) {
            pdata = nullptr;
        }
        else {
            mrb_raise(mrb, E_ARGUMENT_ERROR, "HEG::Canvas::new: init_data can only be a String or a Fixnum(Pointer)");
            return mrb_nil_value();
        }
    }
    else {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "HEG::Canvas::new(filename : String) | HEG::Canvas::new(w : Fixnum, h : Fixnum, init_data = nil : String/Fixnum) -> canvas : Canvas requires 1..3 arguments");
        return mrb_value();
    }
    cv = new Canvas();
    try {
        cv->Initialize((INT)w, (INT)h, pdata);
        cv->AddRefer();
        return mrb_obj_value(mrb_data_object_alloc(mrb, ClassCanvas, cv, &ClassCanvasDataType));
    }
    catch (std::exception & e) {
        mrb_raise(mrb, E_RUNTIME_ERROR, (std::string("Canvas::new: failed to create canvas: ") + e.what()).c_str());
    }
}

/*[DOCUMENT]
method: HEG::Canvas#width
note: Get width of canvas(in pixels)
*/
mrb_value ClassCanvas_width(mrb_state* mrb, mrb_value self) {
    return mrb_fixnum_value(GetNativeObject<Canvas>(self)->width);
}

/*[DOCUMENT]
method: HEG::Canvas#height
note: Get height of canvas(in pixels)
*/
mrb_value ClassCanvas_height(mrb_state* mrb, mrb_value self) {
    return mrb_fixnum_value(GetNativeObject<Canvas>(self)->height);
}

bool InjectCanvasExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* ClassObject = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ClassCanvas = mrb_define_class_under(mrb, HEG, "Canvas", ClassHEGObject);
    mrb_define_class_method(mrb, ClassCanvas, "new", ClassCanvas_new, MRB_ARGS_ANY());
    mrb_define_method(mrb, ClassCanvas, "width", ClassCanvas_width, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassCanvas, "height", ClassCanvas_height, MRB_ARGS_NONE());
    
    return true;
}


HFENGINE_NAMESPACE_END