// HfEngineFont.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <extension.h>
#include <D3DDevice.h>
#include <Texture2D.h>
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "HfEngine.lib")

VALUE klass_Font;

VALUE Font_module_init(VALUE self) {
    //MessageBox(0, L"Font module init", L"tip", MB_OK);
    TTF_Init();
    return self;
}


namespace HfEngineFont {

//some codes are from ProjectGap
//Font
struct FontData {
    int size;
    TTF_Font *font;
    unsigned style;
    FontData() {
        memset(this, 0, sizeof(FontData));
    }
};
static inline FontData *GetFontData(VALUE obj) {
    return Ext::GetNativeObject<FontData>(obj);
}
static VALUE __cdecl set_bold(VALUE self, VALUE bold) {
    FontData *d = GetFontData(self);
    //VALUE bold = rb_funcall2(self, rb_intern("bold"), 0, nullptr);
    rb_iv_set(self,"@bold", bold);
    if (!d || !d->font)return bold;
    if (bold == Qtrue) {
        d->style |= TTF_STYLE_BOLD;
    }
    else {
        d->style &= (~TTF_STYLE_BOLD);
    }
    TTF_SetFontStyle(d->font, d->style);
    return bold;
}
static VALUE __cdecl set_italic(VALUE self, VALUE italic) {
    FontData *d = GetFontData(self);
    //VALUE italic = rb_funcall2(self, rb_intern("italic"), 0, nullptr);
    rb_iv_set(self, "@italic", italic);
    if (!d || !d->font)return italic;
    if (italic == Qtrue) {
        d->style |= TTF_STYLE_ITALIC;
    }
    else {
        d->style &= (~TTF_STYLE_ITALIC);
    }
    TTF_SetFontStyle(d->font, d->style);
    return italic;
}
static VALUE __cdecl set_underline(VALUE self, VALUE underline) {
    FontData *d = GetFontData(self);
    //VALUE underline = rb_funcall2(self, rb_intern("underline"), 0, nullptr);
    rb_iv_set(self, "@underline", underline);
    if (!d || !d->font)return underline;
    if (underline == Qtrue) {
        d->style |= TTF_STYLE_UNDERLINE;
    }
    else {
        d->style &= (~TTF_STYLE_UNDERLINE);
    }
    TTF_SetFontStyle(d->font, d->style);
    return underline;
}
static VALUE __cdecl set_strike_through(VALUE self, VALUE strike_through) {
    FontData *d = GetFontData(self);
    //VALUE strike_through = rb_funcall2(self, rb_intern("strike_through"), 0, nullptr);
    rb_iv_set(self, "@strike_through", strike_through);
    if (!d || !d->font)return strike_through;
    if (strike_through == Qtrue) {
        d->style |= TTF_STYLE_STRIKETHROUGH;
    }
    else {
        d->style &= (~TTF_STYLE_STRIKETHROUGH);
    }
    TTF_SetFontStyle(d->font, d->style);
    return strike_through;
}

static void __create_font(FontData *d, VALUE name, int size) {
    d->font = TTF_OpenFont(RSTRING_PTR(name), size);  //
    if (!d->font) {  //try finding it in C:\Windows\Fonts
        wchar_t buf[MAX_PATH];
        GetWindowsDirectoryW(buf, MAX_PATH);
        lstrcatW(buf, L"/Fonts/");
        std::string fname;
        Ext::U16ToU8(buf, fname);
        fname += RSTRING_PTR(name);
#ifdef DEBUG
        printf("Redirect font file to %s\n", fname.c_str());
#endif
        d->font = TTF_OpenFont(fname.c_str(), size);
    }

    if (!d->font) {
        rb_raise(rb_eRuntimeError, "Fail to create font %s.", RSTRING_PTR(name));
    }
}

static VALUE __cdecl initialize(VALUE self, VALUE name, VALUE _size) {
    FontData *d = GetFontData(self);
    d->size = FIX2INT(_size);
    // d->style = TTF_STYLE_NORMAL; //0
    __create_font(d, name, d->size-2);
    rb_iv_set(self, "@font_name", name);
    rb_iv_set(self, "@size", _size);
    return self;
}

static VALUE __cdecl set_size(VALUE self, VALUE size) {
    FontData *d = GetFontData(self);
    if (!d)return Qnil;
    
    TTF_CloseFont(d->font);
    //d->font = TTF_OpenFont(RSTRING_PTR(rb_funcall2(self, rb_intern("font_name"), 0, nullptr)), d->size - 2);
    d->size = FIX2INT(size);
    __create_font(d, rb_iv_get(self, "@font_name"), d->size-2);
    rb_iv_set(self, "@size", size);
    return size;
}

static void __delete_texture2d(Texture2D *tex) {
    tex->SubRefer(); 
}

static VALUE render_texture2d(VALUE self, VALUE _device, VALUE text, VALUE color) {
    const char *ctext = RSTRING_PTR(text);
    auto device = Ext::GetNativeObject<D3DDevice>(_device);
    auto hfcolor = Ext::GetNativeObject<Utility::Color>(color);
    SDL_Color col{hfcolor->r * 255, hfcolor->g * 255 , hfcolor->b * 255 , hfcolor->a * 255};
    SDL_Surface *sur = TTF_RenderUTF8_Blended(GetFontData(self)->font, ctext, col);
    unsigned *ptr = (unsigned*)sur->pixels;
    /*
    int wh = sur->w*sur->h;
    for (int i = 0; i < wh; i++) {
        ptr[i] = (ptr[i]<<8) | ((ptr[i]&0xff000000) >> 24);  //ARGB->RGBA...(I hate this...)
    }
    if (!sur) {
        rb_raise(rb_eRuntimeError, "Fail to draw text %s, Error: %s\n", RSTRING_PTR(text),SDL_GetError());
        return Qnil;
    }
    */
    /*
    SDL_LockSurface(sur);
    auto native_texture = new Texture2D();
    native_texture->AddRefer();
    native_texture->Initialize(device, sur->w, sur->h, sur->pixels);
    SDL_UnlockSurface(sur);
    SDL_FreeSurface(sur);
    */
    /*
    return Data_Wrap_Struct(Ext::DX::Texture::klass_texture2d, 0, __delete_texture2d, native_texture);
    */
    VALUE klass_tex2d = rb_eval_string("DX::Texture2D");
    VALUE tex = rb_obj_alloc(klass_tex2d);
    auto native_texture = Ext::GetNativeObject<::Texture2D>(tex);
    native_texture->Initialize(device, sur->w, sur->h, sur->pixels);
    SDL_FreeSurface(sur);
    return tex;
}

static void DeleteFontData(FontData *data){
    if (data) {
        if (data->font) {
            TTF_CloseFont(data->font);
            data->font = nullptr;
        }
        delete data;
    }
}   

}

extern "C" __declspec(dllexport) void Init_Font() {
    klass_Font = rb_define_class("Font", rb_cObject);
    using namespace HfEngineFont;   
    using namespace Ext;
    
    //module init, to load dynamic library.
    rb_define_module_function(klass_Font, "init", (Ext::rubyfunc)Font_module_init, 0);

    //
    rb_define_alloc_func(klass_Font, [](VALUE klass)->VALUE {return Data_Wrap_Struct(klass, 0, DeleteFontData, new FontData); });
    rb_define_method(klass_Font, "initialize", (Ext::rubyfunc)initialize, 2);
    rb_define_method(klass_Font, "set_bold", (rubyfunc)set_bold, 1);
    rb_define_method(klass_Font, "set_italic", (rubyfunc)set_italic, 1);
    rb_define_method(klass_Font, "set_underline", (rubyfunc)set_underline, 1);
    rb_define_method(klass_Font, "set_strike_through", (rubyfunc)set_strike_through, 1);
    rb_define_method(klass_Font, "set_size", (rubyfunc)set_size, 1);

    rb_define_method(klass_Font, "render_texture2d", (rubyfunc)render_texture2d, 3);
}