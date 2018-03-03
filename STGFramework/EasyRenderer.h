#ifdef __ANDROID__
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"
#else
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cstring>

#include "../Utility/ReferPtr.h"