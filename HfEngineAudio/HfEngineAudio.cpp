// HfEngineAudio.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "HfEngineAudio.h"
#include <string>
#include <SDL.h>
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_mixer.lib")

using namespace Ext;

VALUE module_Audio;
VALUE klass_sound;
VALUE klass_effect;
struct SoundWraper {
    Mix_Music *music;
    SoundWraper() {
        memset(this, 0, sizeof(SoundWraper));
    }
};

struct EffectWraper {
    Mix_Chunk *chunk;
    EffectWraper() {
        chunk = 0;
    }
};

void FreeSoundWraper(SoundWraper *mw) {
    if (mw->music) {
        Mix_FreeMusic(mw->music);
    }
    delete mw;
}

void FreeEffectWraper(EffectWraper *ew) {
    if (ew->chunk) {
        Mix_FreeChunk(ew->chunk);
    }
    delete ew;
}


VALUE Audio_init(VALUE self) {
    SDL_Init(SDL_INIT_AUDIO);
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MID | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_AllocateChannels(16);
    return self;
}

VALUE Audio_play_bgm(int argc, VALUE *argv, VALUE self) {
    if (argc < 1 || argc > 4) {
        rb_raise(rb_eArgError, "Audio.play_bgm(sound, loop = true, [,volume [,pos]]) requires (1..4) arguments, but given %d\n", argc);
        return self;
    }
    if (!rb_obj_is_kind_of(argv[0], klass_sound)) {
        rb_raise(rb_eArgError, "Audio.play_bgm's first parameter should a kind of Audio::Sound, but given a %s\n", 
                            rb_class2name(rb_obj_class(argv[0])));
        return self;
    }
    auto mw = GetNativeObject<SoundWraper>(argv[0]);
    int loop = argc > 1 ? (argv[1] == Qtrue? -1 : 1) : -1;
    int volume = argc > 2 ? max(0, FIX2INT(argv[2])) : Mix_VolumeMusic(-1);
    double pos = argc > 3 ? rb_float_value(rb_to_float(argv[3])) : 0.0;
    Mix_VolumeMusic(volume);
    Mix_SetMusicPosition(pos);
    if (Mix_PlayMusic(mw->music, loop) == -1) {
        rb_raise(rb_eRuntimeError, "Failed to play bgm, Error: %s", Mix_GetError());
    }
    return self;
}

VALUE Audio_playing_bgm(VALUE self) {
    return Mix_PlayingMusic() ? Qtrue : Qfalse;
}

VALUE Audio_pause_bgm(VALUE self) {
    Mix_PauseMusic();
    return self;
}

VALUE Audio_stop_bgm(VALUE self) {
    Mix_HaltMusic();
    return self;
}

VALUE Audio_resume_bgm(VALUE self) {
    Mix_ResumeMusic();
    return self;
}

VALUE Audio_fadeout_bgm(VALUE self, VALUE time) {
    Mix_FadeOutMusic(FIX2INT(time));
    return self;
}

VALUE Audio_play_se(VALUE self, VALUE effect, VALUE volume) { 
    int _volume = FIX2INT(volume);
    Mix_Chunk *chunk = GetNativeObject<EffectWraper>(effect)->chunk;
    Mix_VolumeChunk(chunk, _volume);
    int channel = Mix_PlayChannel(-1, chunk, 0);
    rb_iv_set(self, "@se_channel", INT2FIX(channel));
    return self;
}

VALUE Audio_stop_se(VALUE self) {
    VALUE se_channel = rb_iv_get(self, "@se_channel");
    if (se_channel != Qnil) {
        Mix_HaltChannel(FIX2INT(se_channel));
    }
    return self;
}

VALUE Sound_initialize(VALUE self, VALUE filename) {
    auto mw = GetNativeObject<SoundWraper>(self);
    mw->music = Mix_LoadMUS(RSTRING_PTR(filename));
    if (!mw->music) {
        rb_raise(rb_eArgError, "Failed to load sound file %s\n Error: %s", RSTRING_PTR(filename), Mix_GetError());
        return self;
    }
    return self;
}

VALUE Effect_initialize(VALUE self, VALUE filename) {
    auto ew = GetNativeObject<EffectWraper>(self);
    ew->chunk = Mix_LoadWAV(RSTRING_PTR(filename));
    if (!ew->chunk) {
        rb_raise(rb_eArgError, "Failed to load Effect file %s\n Error: %s", RSTRING_PTR(filename), Mix_GetError());
        return self;
    }
    return self;
}

extern "C" __declspec(dllexport) void Init_Audio() {
    module_Audio = rb_define_module("Audio");
    rb_define_module_function(module_Audio, "init", (Ext::rubyfunc)Audio_init, 0);
    rb_define_module_function(module_Audio, "play_bgm", (rubyfunc)Audio_play_bgm, -1);
    rb_define_module_function(module_Audio, "pause_bgm", (rubyfunc)Audio_pause_bgm, 0);
    rb_define_module_function(module_Audio, "fadeout_bgm", (rubyfunc)Audio_fadeout_bgm, 1);
    rb_define_module_function(module_Audio, "resume_bgm", (rubyfunc)Audio_resume_bgm, 0);
    rb_define_module_function(module_Audio, "playing_bgm?", (rubyfunc)Audio_playing_bgm, 0);
    rb_define_module_function(module_Audio, "stop_bgm", (rubyfunc)Audio_stop_bgm, 0);
    rb_define_module_function(module_Audio, "play_se", (rubyfunc)Audio_play_se, 2);
    rb_define_module_function(module_Audio, "stop_se", (rubyfunc)Audio_stop_se, 0);

    klass_sound = rb_define_class_under(module_Audio, "Sound", rb_cObject);
    rb_define_alloc_func(klass_sound, [](VALUE klass)->VALUE {return Data_Wrap_Struct(klass, 0, FreeSoundWraper, new SoundWraper);});
    rb_define_method(klass_sound, "initialize", (rubyfunc)Sound_initialize, 1);

    klass_effect = rb_define_class_under(module_Audio, "Effect", rb_cObject);
    rb_define_alloc_func(klass_effect, [](VALUE klass)->VALUE {return Data_Wrap_Struct(klass, 0, FreeEffectWraper, new EffectWraper); });
    rb_define_method(klass_effect, "initialize", (rubyfunc)Effect_initialize, 1);
}

