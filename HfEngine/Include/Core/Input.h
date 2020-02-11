#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <ThirdParties.h>

HFENGINE_NAMESPACE_BEGIN

namespace Input {

    extern ComPtr<IDirectInput8> native_dinput_object;
    void Initialize();

    class Device : public Utility::ReferredObject {
    public:

        ComPtr<IDirectInputDevice8> native_dinput_device;
        void Acquire() {
            native_dinput_device->Acquire();
        }

        void UnAcqure() {
            native_dinput_device->Unacquire();
        }

        void UnInitialize() {
            UnAcqure();
            native_dinput_device.ReleaseAndGetAddressOf();
        }

        virtual void Release() {
            UnInitialize();
        }
    public:
        template<class ElemType, int BufferSize>
        bool ReadDeviceData(ElemType* data_buffer) {
            HRESULT hr;
            while (true) {
                native_dinput_device->Poll();
                Acquire();
                hr = native_dinput_device->GetDeviceState(BufferSize, data_buffer);
                if (SUCCEEDED(hr))
                    break;
                if ((hr != DIERR_INPUTLOST) || (hr != DIERR_NOTACQUIRED))
                    return false;
                if (FAILED(native_dinput_device->Acquire()))
                    return false;
            }
            return true;
        }

    };

    class Keyboard : public Input::Device {
        int buffer_index;
        char data_buffer[2][256]; //double buffer 
    public:
        Keyboard() {}
        Keyboard(HWND www) { Initialize(www); }
        ~Keyboard() { UnInitialize(); }
        void Initialize(HWND hWnd);
        bool IsKeyPressedNow(int keycode);
        bool IsKeyPressedBefore(int keycode);
        bool ReadDeviceData() {
            buffer_index ^= 1;
            return Device::ReadDeviceData<char, 256>(data_buffer[buffer_index & 1]);
        }
        void UnInitialize() {
            Device::UnInitialize();
        }
        virtual void Release() {
            UnInitialize();
        }
    };
};

HFENGINE_NAMESPACE_END