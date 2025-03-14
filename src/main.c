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
#define BUFF_MAX 1024

typedef struct{
    char name[BUFF_MAX];
    char path[BUFF_MAX];
    int appid;
    int64_t size_on_disk;
    int64_t last_played;
} steam_game;

typedef struct{
    char install_path[BUFF_MAX];
    char **library_paths;
    int library_paths_count;
    steam_game *games;
    int games_count;
} steam_data;


void sc_cleanup( SDL_Window *window, SDL_GLContext *context);
void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color);
void sc_config_window(ImGuiIO *ioptr, char *input_buffer);
void sc_start_frame();
void sc_debug_window(ImGuiIO *ioptr, char *input_buffer);




void sc_populate_steam_data();
void sc_populate_steam_libraries(steam_data *sc_steam, const char *lib_vdf_path);
void sc_populate_steam_games(steam_data *sc_steam);

void sc_add_steam_library(steam_data *sc_steam, const char *path);
void sc_add_steam_game(steam_data *sc_steam, const char *name, const char *path, int appid, int64_t size_on_disk, int64_t last_played);






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
    char input_buffer[BUFF_MAX] = {0};

    sc_populate_steam_data();

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



void sc_populate_steam_games(steam_data *sc_steam){
    for (int i = 0 ; i <= sc_steam->library_paths_count-1; i++){
        char acting_path[BUFF_MAX] = {0};
        char search_path[BUFF_MAX] = {0};
        snprintf(acting_path, BUFF_MAX, "%s%s", sc_steam->library_paths[i], "\\\\steamapps");
        snprintf(search_path, BUFF_MAX, "%s\\*.acf", acting_path);

        printf("acting path: %s\n", acting_path);



        WIN32_FIND_DATA found_files;
        HANDLE h_find = FindFirstFile(search_path, &found_files);

        if (h_find == INVALID_HANDLE_VALUE){
            printf("Found no games in library, skipping. lib: %s\n", acting_path);
            continue;
        }

        do{
            char current_file_path[BUFF_MAX] = {0};
            snprintf(current_file_path, BUFF_MAX, "%s\\%s", acting_path, found_files.cFileName);
            printf("\tFound ACF File: %s\n", current_file_path);


            FILE *game_file = fopen(current_file_path, "r");
            if (!game_file){
                perror("failed to open appmanifest file");
                exit(1);
            }
        
            char name[BUFF_MAX] = {0};
            char path[BUFF_MAX] = {0};
            char appid[BUFF_MAX] = {0};
            char size_on_disk[BUFF_MAX] = {0};
            char last_played[BUFF_MAX] = {0};
            
            char line[BUFF_MAX];
            while (fgets(line, sizeof(line), game_file)){
                if (sscanf(line, "%*[\t ]\"name\" \"%[^\"]\"", name) == 1){
                    //printf("\t\tfound name: %s\n", name);
                }
                if (sscanf(line, "%*[\t ]\"installdir\" \"%[^\"]\"", path) == 1){
                    //printf("\t\tfound path: %s\n", path);
                }
                if (sscanf(line, "%*[\t ]\"appid\" \"%[^\"]\"", appid) == 1){
                    //printf("\t\tfound appid: %s\n", appid);
                }
                if (sscanf(line, "%*[\t ]\"SizeOnDisk\" \"%[^\"]\"", size_on_disk) == 1){
                    //printf("\t\tfound size_on_disk: %s\n", size_on_disk);
                }
                if (sscanf(line, "%*[\t ]\"LastPlayed\" \"%[^\"]\"", last_played) == 1){
                    //printf("\t\tfound last_played: %s\n", last_played);
                }
                
            }
            //printf("adding game with disk size: %s (converted: %lld)\n", size_on_disk, strtoll(size_on_disk, NULL, 10));
            sc_add_steam_game(sc_steam, name, path, atoi(appid), strtoll(size_on_disk, NULL, 10), strtoll(last_played, NULL, 10));
            
            fclose(game_file);
        } while (FindNextFile(h_find, &found_files) != 0);
    }
}


void sc_add_steam_game(steam_data *sc_steam, const char *name, const char *path, int appid, int64_t size_on_disk, int64_t last_played){
    sc_steam->games = realloc(sc_steam->games, sizeof(steam_game) * (sc_steam->games_count + 1));
    if (sc_steam->games == NULL){
        perror("Failed to allocate memory for game data");
        exit(1);
    }

    steam_game tmp_game = {0};
    strcat(tmp_game.name, name);
    strcat(tmp_game.path, path);
    tmp_game.appid = appid;
    tmp_game.size_on_disk = size_on_disk;
    tmp_game.last_played = last_played;


    sc_steam->games[sc_steam->games_count] = tmp_game;
    // printf("===Added game:\n nam %s\n path %s\n appd %d\n disk %lld\n played %lld\n", sc_steam->games[sc_steam->games_count].name,
    //                                          sc_steam->games[sc_steam->games_count].path,
    //                                          sc_steam->games[sc_steam->games_count].appid,
    //                                          sc_steam->games[sc_steam->games_count].size_on_disk,
    //                                          sc_steam->games[sc_steam->games_count].last_played);

    sc_steam->games_count += 1;
}




void sc_add_steam_library(steam_data *sc_steam, const char *path){
    sc_steam->library_paths = realloc(sc_steam->library_paths, sizeof(char *) * (sc_steam->library_paths_count + 1));
    if (sc_steam->library_paths == NULL){
        perror("Failed to allocate memory for library paths");
        exit(1);
    }

    sc_steam->library_paths[sc_steam->library_paths_count] = strdup(path);
    if (sc_steam->library_paths[sc_steam->library_paths_count] == NULL){
        perror("Failed to duplicate string");
        exit(1);
    }

    sc_steam->library_paths_count += 1;
}

void sc_populate_steam_libraries(steam_data *sc_steam, const char *lib_vdf_path){
    printf("reading lib file at %s\n", lib_vdf_path);
    DWORD attributes = GetFileAttributes(lib_vdf_path);
    if (attributes == INVALID_FILE_ATTRIBUTES){
        perror("could not find any Steam Libraries. Is steam installed?");
        exit(1);
    } else{
        printf("found steam library file, reading...\n"); 
    }

    FILE *lib_vdf_file = fopen(lib_vdf_path, "r");
    if (!lib_vdf_file){
        perror("failed to open lib vdf file");
        exit(1);
    }

    char line[BUFF_MAX];
    char lib_path[BUFF_MAX];

    while (fgets(line, sizeof(line), lib_vdf_file)){
        if (sscanf(line, "%*[\t ]\"path\" \"%[^\"]\"", lib_path) == 1){
            printf("found steam library path: %s\n", lib_path);
            sc_add_steam_library(sc_steam, lib_path);
        }
    }

    fclose(lib_vdf_file);
    
    if (sc_steam->library_paths_count == 0){
        perror("didn't populate any libraries, exiting");
        exit(1);
    }


}

void sc_populate_steam_data(){
    steam_data sc_steam = {0};
    DWORD steam_path_size = {BUFF_MAX};
    LONG result = RegGetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath", RRF_RT_REG_SZ, 0, &sc_steam.install_path, &steam_path_size);
    if (strlen(sc_steam.install_path) < 1){
        perror("couldn't find steam install folder in registry, is steam installed?");
        exit(1);
    }

    char tmp_path[] = "\\steamapps\\libraryfolders.vdf";
    if (strlen(sc_steam.install_path) + strlen(tmp_path) > BUFF_MAX){
        perror("vdf path too large");
        exit(1);
    }

    char vdf_path[BUFF_MAX] = {0};
    strcat(vdf_path, sc_steam.install_path);
    strcat(vdf_path, tmp_path);
    printf("vdf_path: %s\n", vdf_path);
    sc_populate_steam_libraries(&sc_steam, vdf_path);
    sc_populate_steam_games(&sc_steam);

    printf("=====listing libraries:======\n");
    for (int i = 0; i <= sc_steam.library_paths_count-1; i++){
        printf("Library path: %s\n", sc_steam.library_paths[i]);
    }
    printf("=====listing games:======\n");
    for (int i = 0; i <= sc_steam.games_count-1; i++){
        printf("name: %s\n path: %s\n appid: %d\n disksize: %lld\n played: %lld\n", sc_steam.games[i].name,
                                                                                    sc_steam.games[i].path,
                                                                                    sc_steam.games[i].appid,
                                                                                    sc_steam.games[i].size_on_disk,
                                                                                    sc_steam.games[i].last_played);
    }


    

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

        igInputTextMultiline(" ", input_buffer, BUFF_MAX, (ImVec2){400, 300}, 0, NULL, NULL);
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