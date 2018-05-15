#pragma once

/*extension for ruby lib*/
#include "stdafx.h"

namespace Ext {
    typedef VALUE(*rubyfunc)(...);
    void ApplyExtensions();

    static void U8ToU16(const char *s, std::wstring &out, UINT cp = CP_UTF8) {
        size_t len = MultiByteToWideChar(cp, 0, s, (int)strlen(s), NULL, 0);
        wchar_t *t = new wchar_t[len + 1];
        MultiByteToWideChar(cp, 0, s, (int)strlen(s), t, (int)len);
        t[len] = '\0';
        out = t;
        delete t;
    }

    static void U16ToU8(const wchar_t *ws, std::string &out, UINT cp = CP_UTF8) {
        size_t len = WideCharToMultiByte(cp, 0, ws, (int)wcslen(ws), NULL, NULL, NULL, NULL);
        char *t = new char[len + 1];
        WideCharToMultiByte(cp, 0, ws, (int)lstrlenW(ws), t, (int)len, NULL, NULL);
        t[len] = '\0';
        out = t;
        delete t;
    }

    template<class T>
    inline T *GetNativeObject(VALUE self) {
        T *obj;
        Data_Get_Struct(self, T, obj);
        return obj;
    }
    template<class T>
    inline static T *GetNULLPTRableNativeObject(VALUE obj) {
        return NIL_P(obj) ? nullptr : GetNativeObject<T>(obj);
    }

    template<class T>
    void RefObjDelete(T *o) {
        o->SubRefer();
    }

    //Create New Object(Utility::ReferredObject) with destructor. It will increase its reference count
    template<class T>
    static VALUE RefObjNew(VALUE k) {
        T *o = new T;
        o->AddRefer();
        return Data_Wrap_Struct(k, nullptr, RefObjDelete<T>, o);
    }

    //Create New Object(Utility::ReferredObject) without destructor. It will not increase the reference count
    template<class T>
    static VALUE RefObjNewNoDelete(VALUE k) {
        T *o = new T;
        return Data_Wrap_Struct(k, nullptr, nullptr, o);
    }

    struct ArgType {
        VALUE klass;
        bool nilok;
        ArgType(const VALUE &k, const bool &b = false) {
            klass = k;
            nilok = b;
        }
    };

#ifdef _DEBUG
    void CheckArgs(int argc, const VALUE *argv, const std::initializer_list<ArgType> &klasses);
    inline void CheckAllFloat(const std::initializer_list<VALUE> &args) {
        const VALUE *x = args.begin();
        while (x != args.end()) {
            if (!RB_FLOAT_TYPE_P(*x)) {
                rb_raise(rb_eArgError, "Param No.%d should be a Float", x - args.begin() + 1);
            }
            x++;
        }
    }

    inline void CheckAllFixnum(const std::initializer_list<VALUE> &args) {
        const VALUE *x = args.begin();
        while (x != args.end()) {
            if (!RB_FIXNUM_P(*x))
                rb_raise(rb_eArgError, "Param No.%d should be a Fixnum", x - args.begin() + 1);
            x++;
        }
    }

    inline void CheckArgs(const std::initializer_list<VALUE> &objs, const std::initializer_list<ArgType> &klasses) {
        CheckArgs((int)(objs.end() - objs.begin()), objs.begin(), klasses);
    }
#else
#define CheckArgs(...) void(0);
#define CheckAllFloat(...) void(0);
#define CheckAllFixnum(...) void(0);
#endif
    

	namespace DX {
		 
	}
    
    extern VALUE rb_mModule;
    extern VALUE module_release;
}