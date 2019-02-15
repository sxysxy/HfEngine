#pragma once

#include <SDL_mixer.h>
#include <extension.h>

extern VALUE module_Audio;
extern VALUE klass_sound;
extern "C" __declspec(dllexport) void Init_Audio();