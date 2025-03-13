#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "sdl_util.c"

#define DEBUG 0
#define WEAPON_TEXT_MAX 5000


void sc_cleanup( SDL_Window *window, SDL_GLContext *context);
void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color);
void sc_config_window(ImGuiIO *ioptr, char *input_buffer);
void sc_start_frame();
void sc_debug_window(ImGuiIO *ioptr, char *input_buffer);





int main(int argc, char* argv[]){
    //setup SDL and OpenGL
    SDL_Window *window = sdl_setup("Steam Cleaner", 1027, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    const char* glsl_version = "#version 130";
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);  // enable vsync
    SDL_Log("opengl version: %s", (char*)glGetString(GL_VERSION));


    // setup imgui
    igCreateContext(NULL);
    ImGuiIO* ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    igStyleColorsDark(NULL);
    


    //Application vars
    ImVec4 clear_color = {0.45f, 0.55f, 0.60f, 1.00f};
    char input_buffer[WEAPON_TEXT_MAX] = {0};


    //main loop
    bool quit = false;
    while (!quit){
        quit = sdl_main_loop(window);

        // start imgui frame
        sc_start_frame();

        //application actions
        sc_config_window(ioptr, input_buffer);

        if (DEBUG == 1){
            sc_debug_window(ioptr, input_buffer);
        }



        // render
        sc_render( window, &gl_context, ioptr, &clear_color);
    }

    
    // clean up
    sc_cleanup(window, &gl_context);
}






void sc_config_window(ImGuiIO *ioptr, char *input_buffer){
    //resize window to viewport
    ImVec2 screen_size = {ioptr->DisplaySize.x, ioptr->DisplaySize.y};
    igSetNextWindowSize(screen_size, 0);
    igSetNextWindowPos((ImVec2){0,0}, 0, (ImVec2){0,0});

    //main window config
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    {
        
        igBegin("Steam Cleaner", NULL, flags);
        igText("test text box");

        igInputTextMultiline(" ", input_buffer, WEAPON_TEXT_MAX, (ImVec2){400, 300}, 0, NULL, NULL);
        igSameLine(0,75);
        igText("Test");

        

        igEnd();
    }
    
    return;
}


void sc_cleanup(SDL_Window *window, SDL_GLContext *context){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DeleteContext(*context);
    if (window != NULL){
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();

    return;
}


void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color){
    igRender();
    SDL_GL_MakeCurrent(window, *context);
    glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
    glClearColor(clear_color->x, clear_color->y, clear_color->z, clear_color->w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
    SDL_GL_SwapWindow(window);

}


void sc_start_frame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    igNewFrame();
}



void sc_debug_window(ImGuiIO *ioptr, char *input_buffer){


    //main window config
    ImGuiWindowFlags flags = 0;
    {
        
        igBegin("Debug", NULL, flags );
        igText("Input char length: %d", strlen(input_buffer));
        igText("%.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igEnd();
    }
    
    return;
}