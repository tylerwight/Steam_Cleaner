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
#include "data.h"
#include <time.h>

#define DEBUG 0
#define BUFF_MAX 1024
#define LOG_BUFF_MAX 8192


typedef struct {
    char buffer[LOG_BUFF_MAX];
    int buffer_length;

} sc_log;


void sc_cleanup( SDL_Window *window, SDL_GLContext *context);
void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color);
void sc_config_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log);
void sc_start_frame();
void sc_debug_window(ImGuiIO *ioptr);


void append_to_log(char *log_buffer, int *log_buffer_len, const char *message) {
    int msg_len = strlen(message);

    if (*log_buffer_len + msg_len < BUFF_MAX - 1) {
        strcat(log_buffer, message);
        strcat(log_buffer, "\n");  // New line for each entry
        *log_buffer_len += msg_len + 1;
    } else {
        strcpy(log_buffer, "Log buffer full!\n");
        *log_buffer_len = strlen(log_buffer);
    }
}

int compare_games(void *sortSpecs, const void *a, const void *b ) {
    const steam_game *gameA = (const steam_game *)a;
    const steam_game *gameB = (const steam_game *)b;
    ImGuiTableSortSpecs *specs = (ImGuiTableSortSpecs *)sortSpecs;

    for (int i = 0; i < specs->SpecsCount; i++) {
        ImGuiTableColumnSortSpecs *col = &specs->Specs[i];

        int result = 0;
        switch (col->ColumnIndex) {
            case 0:  // AppID (int comparison)
                result = gameA->appid - gameB->appid;
                break;
            case 1:  // Name (string comparison)
                result = strcmp(gameA->name, gameB->name);
                break;
            case 2:  // Size on disk (int64_t comparison)
                result = (gameA->size_on_disk > gameB->size_on_disk) - 
                         (gameA->size_on_disk < gameB->size_on_disk);
                break;
            case 3:  // Last played (int64_t comparison)
                result = (gameA->last_played > gameB->last_played) - 
                         (gameA->last_played < gameB->last_played);
                break;
            case 4:  // Path (string comparison)
                result = strcmp(gameA->path, gameB->path);
                break;
        }

        // Apply ascending/descending order
        if (col->SortDirection == ImGuiSortDirection_Descending) {
            result = -result;
        }

        // If comparison result is non-zero, return it
        if (result != 0) {
            return result;
        }
    }
    
    return 0; // If all compared columns are equal
}


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
    sc_log sc_log = {0};
    steam_data sc_steam = {0};
    //sc_populate_steam_data(&sc_steam);

    //main loop
    bool quit = false;
    while (!quit){
        quit = sdl_main_loop(window);

        // start imgui frame
        sc_start_frame();

        //application actions
        sc_config_window(ioptr, &sc_steam, &sc_log);


        if (DEBUG == 1){
            sc_debug_window(ioptr);
        }



        // render
        sc_render( window, &gl_context, ioptr, &clear_color);
    }

    
    // clean up
    sc_cleanup(window, &gl_context);
}








void sc_config_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log){
    //resize window to viewport
    ImVec2 screen_size = {ioptr->DisplaySize.x, ioptr->DisplaySize.y};
    igSetNextWindowSize(screen_size, 0);
    igSetNextWindowPos((ImVec2){0,0}, 0, (ImVec2){0,0});

    //main window config
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    {
        static bool loaded = false;
        //bool *selected = calloc(sc_steam->games_count, sizeof(bool));
        igBegin("Steam Cleaner", NULL, flags);
        if (loaded == false){
            if (igButton("Load Games", (ImVec2){100, 50})){
                sc_populate_steam_data(sc_steam);
                loaded = true;
            }
        }
        if (loaded == true){
            if (igButton("Select all", (ImVec2){100, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    sc_steam->games[i].selected = true;
                }
            }
            igSameLine(0, 10);
            if (igButton("Deselect all", (ImVec2){100, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    sc_steam->games[i].selected = false;
                }
            }
        }



        if (igBeginTable("ItemTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
                                         ImGuiTableFlags_Sortable |
                                         ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | 
                                         ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                                         (ImVec2){ioptr->DisplaySize.x - 10, ioptr->DisplaySize.y - 150}, 0)) {
            // Create table headers
            igTableSetupColumn("AppID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 100, 0);
            igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 200, 0);
            igTableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 150, 0);
            igTableSetupColumn("Last Played", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 150, 0);
            igTableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0, 0);
            igTableHeadersRow();

            ImGuiTableSortSpecs* sortSpecs = igTableGetSortSpecs();
            if (sortSpecs && sortSpecs->SpecsDirty) {
                sortSpecs->SpecsDirty = false;
                // Sort games array based on selected column
                qsort_s(sc_steam->games, sc_steam->games_count, sizeof(steam_game), 
                    compare_games, (void *)sortSpecs);   
            }

    
    
            for (int i = 0; i < sc_steam->games_count-1; i++) {
                igTableNextRow(0, 0);
                igTableSetColumnIndex(0);
                //igText(sc_steam->games[i].appid_str);
                igCheckbox(sc_steam->games[i].appid_str, &sc_steam->games[i].selected);
                igTableSetColumnIndex(1);
                igText(sc_steam->games[i].name);
                igTableSetColumnIndex(2);
                igText(sc_steam->games[i].size_on_disk_str);
                igTableSetColumnIndex(3);
                igText(sc_steam->games[i].last_played_pretty);
                igTableSetColumnIndex(4);
                igText(sc_steam->games[i].path);
            }
            igEndTable();
        }

        if (loaded == 1){
            if (igButton("Uninstall Selected", (ImVec2){150, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    if (sc_steam->games[i].selected == 1){
                        char output_buffer[BUFF_MAX] = {0};
                        snprintf(output_buffer, BUFF_MAX, "%s%s", "Trying to uninstall: ", sc_steam->games[i].name);
                        append_to_log(sc_log->buffer, &sc_log->buffer_length,output_buffer);

                        char shell_command[BUFF_MAX] = {0};
                        snprintf(shell_command, BUFF_MAX, "%s%s", "start /wait steam://uninstall/", sc_steam->games[i].appid_str);
                        append_to_log(sc_log->buffer, &sc_log->buffer_length, shell_command);
                        //ShellExecute(NULL, "open", shell_command, NULL, NULL, SW_SHOWNORMAL);
                        system(shell_command);
                    }


                }
                

            }
            igSameLine(0, 10);
        }

        
        igBeginChild_Str("log_scroll", (ImVec2){0, 0}, 0, ImGuiWindowFlags_HorizontalScrollbar);
        igText(sc_log->buffer);  // Display log messages
        igEndChild();

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



void sc_debug_window(ImGuiIO *ioptr){


    //main window config
    ImGuiWindowFlags flags = 0;
    {
        
        igBegin("Debug", NULL, flags );
        //igText("Input char length: %d", strlen(log_buffer));
        igText("%.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igEnd();
    }
    
    return;
}