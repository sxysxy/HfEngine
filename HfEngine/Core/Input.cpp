#include <Core/Input.h>

#pragma comment(lib, "dinput8.lib")

HFENGINE_NAMESPACE_BEGIN

namespace Input {
    ComPtr<IDirectInput8> native_dinput_object;
    bool initialized;
    void Initialize() {
        if (!initialized) {
            HRESULT hr = DirectInput8Create(GetModuleHandle(0), 0x0800,
                IID_IDirectInput8, &native_dinput_object, nullptr);
            if (FAILED(hr))
                THROW_ERROR_CODE(std::runtime_error, "Failed to create DirectInput8, Error code:", hr);
            initialized = true;
        }
    }

    void Keyboard::Initialize(HWND hWnd) {
        if (!native_dinput_object)throw std::runtime_error("Call Input::Initialize first");
        using namespace Utility;
        buffer_index = 0;
        HRESULT hr = native_dinput_object->CreateDevice(GUID_SysKeyboard, &native_dinput_device, nullptr);
        if (FAILED(hr))
            THROW_ERROR_CODE(std::runtime_error, "Failed to create keyboard device object, Error code:", hr);
        hr = native_dinput_device->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(hr))
            THROW_ERROR_CODE(std::runtime_error, "Failed to set keyboard data format, Error code:", hr);
        hr = native_dinput_device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        if (FAILED(hr))
            THROW_ERROR_CODE(std::runtime_error, "Failed to set keyboard cooperation level, Error code:", hr);
        DIPROPDWORD property;
        property.diph.dwSize = sizeof DIPROPDWORD;
        property.diph.dwHeaderSize = sizeof DIPROPHEADER;
        property.diph.dwObj = 0;
        property.diph.dwHow = DIPH_DEVICE;
        property.dwData = 16;
        hr = native_dinput_device->SetProperty(DIPROP_BUFFERSIZE, &property.diph);
        if (FAILED(hr))
            THROW_ERROR_CODE(std::runtime_error, "Failed to set keyboard property, Error code:", hr);
        ReadDeviceData();
    }
    bool Keyboard::IsKeyPressedNow(int keycode) {
        return data_buffer[buffer_index][keycode] & 0x80;
    }
    bool Keyboard::IsKeyPressedBefore(int keycode) {
        return data_buffer[buffer_index ^ 1][keycode] & 0x80;
    }
}

HFENGINE_NAMESPACE_END