#ifndef SDL_HELPERS_H
#define SDL_HELPERS_H
#include "steam_cleaner.h"
#include <SDL2/SDL.h>


bool sdl_main_loop(SDL_Window *window);
SDL_Window* sdl_setup(char *name, int size_x, int size_y, Uint32 flags);


#endif