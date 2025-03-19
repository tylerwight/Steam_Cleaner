#ifndef UI_H
#define UI_H
#include "steam_cleaner.h"
#include "data.h"
// #include <stdio.h>
// #include <stdbool.h>
// #include <time.h>
// #include <SDL2/SDL.h>
// #include <windows.h>
// #include <GL/gl.h>
// #include <GL/glu.h>
// #include "data.h"
// #include "config.h"




void sc_cleanup( SDL_Window *window, SDL_GLContext *context);
void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color);
void sc_config_window(ImGuiIO *ioptr, sc_steam_cleaner *app); //Main window configuration in imgui
void sc_start_frame();
void sc_debug_window(ImGuiIO *ioptr, steam_data *sc_steam);
void sc_uninstall_window(ImGuiIO *ioptr, sc_steam_cleaner *app);
int compare_games(void *sortSpecs, const void *a, const void *b );



#endif
