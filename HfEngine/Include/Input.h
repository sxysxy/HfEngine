#pragma once
#include "stdafx.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

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
        template<class ElemType>
        bool ReadDeviceData(Utility::HFBuffer<ElemType> *data_buffer) {
            HRESULT hr;
            while (true) {
                native_dinput_device->Poll();
                Acquire();
                hr = native_dinput_device->GetDeviceState(data_buffer->size, data_buffer->ptr);
                if(SUCCEEDED(hr))break;
                if(hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)return false;
                if(FAILED(native_dinput_device->Acquire()))return false;
            }
            return true;
        } 
       
    };

    class Keyboard : public Input::Device {
        int buffer_index;
        Utility::HFBuffer<char> data_buffer[2]; //double buffer 
    public:
        Keyboard() {}
        Keyboard(HWND www) { Initialize(www); }
        ~Keyboard() {UnInitialize(); }
        void Initialize(HWND hWnd);
        bool IsKeyPressedNow(int keycode);
        bool IsKeyPressedBefore(int keycode);
        bool ReadDeviceData() {
            buffer_index ^= 1; 
            return Device::ReadDeviceData<char>(&data_buffer[buffer_index&1]);
        }
        void UnInitialize() {
            Device::UnInitialize();
            data_buffer[0].Release();
            data_buffer[1].Release();
        }
        virtual void Release(){ 
            UnInitialize(); 
        }
    };

    class Mouse : public Input::Device {
        Utility::HFBuffer<DIMOUSESTATE> data_buffer;
    public:
        Mouse() {}
        ~Mouse() {UnInitialize(); }
        Mouse(HWND w) {Initialize(w);}
        void Initialize(HWND hwnd);
        bool IsButtonPressed(int keycode);
        void GetPosition(int * x, int * y);
        void GetDeltaParams(int * dx, int * dy, int * dz);
        bool ReadDeviceData() {
            return Input::Device::ReadDeviceData<DIMOUSESTATE>(&data_buffer);
        }
        void UnInitialize() {
            Device::UnInitialize();
            data_buffer.Release();
        }
        virtual void Release() { UnInitialize(); }
    };
};

namespace Ext {
    namespace DX {
        namespace Input {
            extern VALUE module_Input;
            extern VALUE klass_device;
            extern VALUE klass_Keyboard;
            extern VALUE klass_Mouse;
            void Init();
        }
    }
}