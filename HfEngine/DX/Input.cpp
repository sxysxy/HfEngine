#include "../Include/Input.h"
#include "../Include/HFWindow.h"
#include "../Include/extension.h"
#include "../Include/DX.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Input {
    ComPtr<IDirectInput8> native_dinput_object;
    bool initialized;
    void Initialize() {
        if(!initialized){
            HRESULT hr = DirectInput8Create(GetModuleHandle(0), 0x0800,
                IID_IDirectInput8, &native_dinput_object, nullptr);
            if(FAILED(hr))
                MAKE_ERRMSG<std::runtime_error>("Failed to create DirectInput8, Error code:", hr);
            initialized = true;
        }
    }

    void Keyboard::Initialize(HWND hWnd) {
        if (!native_dinput_object)throw std::runtime_error("Call Input::Initialize first");
        using namespace Utility;
        data_buffer[0] = ReferPtr<HFBuffer<char>>::New(256);
        data_buffer[1] = ReferPtr<HFBuffer<char>>::New(256);
        buffer_index = 0;
        HRESULT hr = native_dinput_object->CreateDevice(GUID_SysKeyboard, &native_dinput_device, nullptr);
        if (FAILED(hr))
            MAKE_ERRMSG<std::runtime_error>("Failed to create keyboard device object, Error code:", hr);
        hr = native_dinput_device->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(hr))
            MAKE_ERRMSG<std::runtime_error>("Failed to set keyboard data format, Error code:", hr);
        hr = native_dinput_device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        if(FAILED(hr))
            MAKE_ERRMSG<std::runtime_error>("Failed to set keyboard cooperation level, Error code:", hr);
        DIPROPDWORD property;
        property.diph.dwSize = sizeof DIPROPDWORD;
        property.diph.dwHeaderSize = sizeof DIPROPHEADER;
        property.diph.dwObj = 0;
        property.diph.dwHow = DIPH_DEVICE;
        property.dwData = 16;
        hr = native_dinput_device->SetProperty(DIPROP_BUFFERSIZE, &property.diph);
        if (FAILED(hr)) 
            MAKE_ERRMSG<std::runtime_error>("Failed to set keyboard property, Error code:", hr);
        ReadDeviceData();
    }
    bool Keyboard::IsKeyPressedNow(int keycode) {
        return data_buffer[buffer_index]->ptr[keycode] & 0x80;
    }
    bool Keyboard::IsKeyPressedBefore(int keycode) {
        return data_buffer[buffer_index^1]->ptr[keycode] & 0x80;
    }
    void Mouse::Initialize(HWND hwnd){
        HRESULT hr;
        if (FAILED(hr = Input::native_dinput_object->CreateDevice(GUID_SysMouse, &native_dinput_device, nullptr))) {
            MAKE_ERRMSG<std::runtime_error>("Failed to create native direct input device(Mouse) object, Error code:", hr);
        }
        if (FAILED(hr = native_dinput_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
            MAKE_ERRMSG<std::runtime_error>("Failed to set mouse cooperation level, Error code:", hr);
        }
        if (FAILED(hr = native_dinput_device->SetDataFormat(&c_dfDIMouse))) {
            MAKE_ERRMSG<std::runtime_error>("Failed to set mouse data format, Error code:", hr);
        }
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = sizeof(DIMOUSESTATE);
        hr = native_dinput_device->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
        if (FAILED(hr)) {
            MAKE_ERRMSG<std::runtime_error>("Failed to set mouse property Error code:", hr);
        }
        data_buffer.Initialize(1);
        ReadDeviceData();
    }
    bool Mouse::IsButtonPressed(int keycode){
       return data_buffer.ptr->rgbButtons[keycode] & 0x80;
    }
    void Mouse::GetPosition(int *x, int *y) {
        POINT p;
        GetCursorPos(&p);
        if(x)*x = p.x; if(y)*y = p.y;
    }
    void Mouse::GetDeltaParams(int *dx, int *dy, int *dz) {
        if(dx)*dx = data_buffer.ptr->lX;
        if(dy)*dy = data_buffer.ptr->lY;
        if(dz)*dz = data_buffer.ptr->lZ;
    }
}

namespace Ext {
    namespace DX {
        namespace Input {
            VALUE module_Input;
            VALUE klass_device;
            VALUE klass_Keyboard;
            VALUE klass_Mouse;
            VALUE InputInitialize(VALUE self) {
                try{
                    ::Input::Initialize();
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                    return Qfalse;
                }
                return Qtrue;
            }

            void Keyboard_delete(::Input::Keyboard *k) {
                k -> SubRefer();
            }

            VALUE Keyboard_new(VALUE k) {
                auto kbd = new ::Input::Keyboard;
                kbd->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Keyboard_delete, kbd);
            }

            VALUE Keyboard_initialize(VALUE self, VALUE wnd) {
                if(!rb_obj_is_kind_of(wnd, Ext::HFWindow::klass))rb_raise(rb_eArgError, 
                    "Input::Keyboard::initialize: The only param should be a HFWindow");
                auto keyboard = GetNativeObject<::Input::Keyboard>(self);
                auto window = GetNativeObject<Ext::HFWindow::RHFWindow>(wnd);
                try{
                    keyboard->Initialize(window->native_handle);
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            VALUE Keyboard_read_device_data(VALUE self) {
                auto keyboard = GetNativeObject<::Input::Keyboard>(self);
                return keyboard->ReadDeviceData() ? Qtrue : Qfalse;
            }

            VALUE Keyboard_is_pressed_now(VALUE self, VALUE key) {
                auto keyboard = GetNativeObject<::Input::Keyboard>(self);
                int code = FIX2INT(key);
                if(code > 255)rb_raise(rb_eArgError, "Invalid key code value");
                return keyboard->IsKeyPressedNow(code) ? Qtrue : Qfalse;
            }

            VALUE keyboard_is_pressed_before(VALUE self, VALUE key) {
                auto keyboard = GetNativeObject<::Input::Keyboard>(self);
                int code = FIX2INT(key);
                if (code > 255)rb_raise(rb_eArgError, "Invalid key code value");
                return keyboard->IsKeyPressedBefore(code)? Qtrue : Qfalse;
            }

            void Mouse_Delete(::Input::Mouse *m) {
                m->SubRefer();
            }

            VALUE Mouse_initialize(VALUE self, VALUE wnd) {
                if (!rb_obj_is_kind_of(wnd, Ext::HFWindow::klass))rb_raise(rb_eArgError,
                    "Input::Mouse::initialize: The only param should be a HFWindow");
                auto mouse = GetNativeObject<::Input::Mouse>(self);
                auto window = GetNativeObject<Ext::HFWindow::RHFWindow>(wnd);
                try {
                    mouse->Initialize(window->native_handle);
                }
                catch (std::runtime_error re) {
                    rb_raise(rb_eRuntimeError, re.what());
                }
                return self;
            }

            VALUE Mouse_get_position(VALUE self) {
                auto mouse = GetNativeObject<::Input::Mouse>(self);
                int x, y;
                mouse->GetPosition(&x, &y);
                return rb_ary_new3(2, INT2FIX(x), INT2FIX(y));
            }

            VALUE Mouse_is_pressed(VALUE self, VALUE keycode) {
                auto mouse = GetNativeObject<::Input::Mouse>(self);
                int code = FIX2INT(keycode);
                if (code > 3)rb_raise(rb_eArgError, "Invalid key code value");
                return mouse->IsButtonPressed(code) ? Qtrue : Qfalse;
            }

            VALUE Mouse_get_delta_params(VALUE self) {
                auto mouse = GetNativeObject<::Input::Mouse>(self);
                int dx, dy, dz;
                mouse->GetDeltaParams(&dx, &dy, &dz);
                return rb_ary_new3(3, INT2FIX(dx), INT2FIX(dy), INT2FIX(dz));
            }
            template<class T>
            VALUE Device_update(VALUE self) {
                auto device = GetNativeObject<T>(self);
                return device->ReadDeviceData()? Qtrue : Qfalse;
            }

            void Init() {
                module_Input = rb_define_module_under(module, "Input");
                klass_device = rb_define_class_under(module_Input, "Device", rb_cObject);
                rb_include_module(klass_device, module_release);
                klass_Keyboard = rb_define_class_under(module_Input, "Keyboard", klass_device);
                rb_define_alloc_func(klass_Keyboard, Keyboard_new);
                rb_define_method(klass_Keyboard, "initialize", (rubyfunc)Keyboard_initialize, 1);
                rb_define_method(klass_Keyboard, "is_pressed_now", (rubyfunc)Keyboard_is_pressed_now, 1);
                rb_define_method(klass_Keyboard, "is_pressed_before", (rubyfunc)keyboard_is_pressed_before, 1);
                rb_define_method(klass_Keyboard, "update", (rubyfunc)Device_update<::Input::Keyboard>, 0);

                klass_Mouse = rb_define_class_under(module_Input, "Mouse", klass_device);
                rb_define_alloc_func(klass_Mouse, [](VALUE k)->VALUE{
                    auto *m = new ::Input::Mouse;
                    m->AddRefer();
                    return Data_Wrap_Struct(k, nullptr, Mouse_Delete, m);
                });
                rb_define_method(klass_Mouse, "initialize", (rubyfunc)Mouse_initialize, 1);
                rb_define_method(klass_Mouse, "get_position", (rubyfunc)Mouse_get_position, 0);
                rb_define_method(klass_Mouse, "is_pressed", (rubyfunc)Mouse_is_pressed, 1);
                rb_define_method(klass_Mouse, "get_delta_params", (rubyfunc)Mouse_get_delta_params, 0);
                rb_define_method(klass_Mouse, "update", (rubyfunc)Device_update<::Input::Mouse>, 0);
                rb_define_const(module, "MOUSE_LEFT", INT2FIX(0));

                
            }
        }
    }
}
