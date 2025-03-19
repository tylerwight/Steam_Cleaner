
#include "steam_cleaner.h"
#include "sdl_helpers.h"
#include "data.h"
#include "ui.h"
#include "config.h"




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
    steam_data sc_steam = {0};
    sc_steam_cleaner main_application = {0};
    main_application.steam_data = &sc_steam;
    main_application.loaded = false;
    main_application.uninstalling = false;

    //main loop
    bool quit = false;
    while (!quit){
        quit = sdl_main_loop(window);

        // start imgui frame
        sc_start_frame();

        //application actions
        sc_config_window(ioptr, &main_application);

        if (main_application.uninstalling == true){
            sc_uninstall_window(ioptr, &main_application);
        }
        


        if (DEBUG == 1){
            sc_debug_window(ioptr, &sc_steam);
        }



        // render
        sc_render( window, &gl_context, ioptr, &clear_color);
    }

    
    // clean up
    sc_cleanup(window, &gl_context);
}