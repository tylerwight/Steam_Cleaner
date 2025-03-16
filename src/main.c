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
//#define BUFF_MAX 1024





void sc_cleanup( SDL_Window *window, SDL_GLContext *context);
void sc_render(SDL_Window *window, SDL_GLContext *context, ImGuiIO *ioptr, ImVec4 *clear_color);
void sc_config_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log, sc_uninstall_stack **uninstall_list);
void sc_start_frame();
void sc_debug_window(ImGuiIO *ioptr, steam_data *sc_steam);
void sc_uninstall_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log, sc_uninstall_stack **uninstall_list);


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

bool sc_uninstalling = false;
bool sc_loaded = false;


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
    sc_uninstall_stack *uninstall_list = NULL;

    //main loop
    bool quit = false;
    while (!quit){
        quit = sdl_main_loop(window);

        // start imgui frame
        sc_start_frame();

        //application actions
        sc_config_window(ioptr, &sc_steam, &sc_log, &uninstall_list);

        if (sc_uninstalling == true){
            sc_uninstall_window(ioptr, &sc_steam, &sc_log, &uninstall_list);
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








void sc_config_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log, sc_uninstall_stack **uninstall_list){
    //resize window to viewport
    ImVec2 screen_size = {ioptr->DisplaySize.x, ioptr->DisplaySize.y};
    igSetNextWindowSize(screen_size, 0);
    igSetNextWindowPos((ImVec2){0,0}, 0, (ImVec2){0,0});
    static bool select_all = false;

    static bool select_bigger_than = false;
    static int size;

    static bool select_older_than = false;
    static int month, day, year;

    //main window config
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    {
        //bool *selected = calloc(sc_steam->games_count, sizeof(bool));
        igBegin("Steam Cleaner", NULL, flags);
        if (sc_loaded == false){
            if (igButton("Load Games", (ImVec2){100, 50})){
                sc_populate_steam_data(sc_steam);
                sc_loaded = true;
            }
        }
        if (sc_loaded == true){
            if (igButton("Select games", (ImVec2){100, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    if (select_all == true){
                        sc_steam->games[i].selected = true;
                    }
                    if (select_bigger_than == true){
                        int64_t size_in_bytes = (int64_t)size * 1073741824;
                        if (sc_steam->games[i].size_on_disk > size_in_bytes){
                            sc_steam->games[i].selected = true;
                        }

                    }

                }



            }
            igSameLine(0, 10);
            if (igButton("Deselect all", (ImVec2){100, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    sc_steam->games[i].selected = false;
                }
            }
            igSameLine(0, 10);
            igCheckbox("Select all", &select_all);

            if (select_all == true){
                select_bigger_than = false;
                select_older_than = false;
            } else{
                igSameLine(0, 10);
                
                igCheckbox("Bigger than...", &select_bigger_than);
                if (select_bigger_than == true){
                    igSameLine(0, 10);
                    static char slider_text[BUFF_MAX];
                    snprintf(slider_text, BUFF_MAX, "%dGB", size);
                    igSetNextItemWidth(100);
                    igSliderInt(" ", &size, 0, 200, slider_text, 0);
                }
                // igSameLine(0, 10);
                // igCheckbox("Older than...", &select_older_than);
    
                
                if (select_older_than == true){
                    igSameLine(0, 10);
                    static char slider_text[BUFF_MAX];
                    //snprintf(slider_text, BUFF_MAX, "%ddate", size);
                    igSetNextItemWidth(100);
                    //igSliderInt(" ", &date, 0, 200, slider_text, 0);
                    igCombo_Str("Month", &month, "January\0February\0March\0April\0May\0June\0July\0August\0September\0October\0November\0December\0", 5);
                    igCombo_Str("Day", &day, "1\0 2\0 3\0 4\05\06\07\08\09\010\011\012\013\014\015\016\017\018\019\020\021\022\023\024\025\026\027\028\029\030\031\0", 5);
                    igCombo_Str("Year", &year, "2005\02006\02007\02008\02009\02010\02011\02012\02013\02014\02015\02016\02017\02018\02019\02020\02021\02022\02023\02024\02025\0", 5);
                    char buf[100] = {0};
                    snprintf(buf, 100, "%d/%d/%d", month, day, year);
                    igText(buf);
                    
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

        if (sc_loaded == 1 && sc_uninstalling == 0){
            if (igButton("Uninstall Selected", (ImVec2){150, 50})){
                for (int i = 0; i < sc_steam->games_count-1; i++){
                    if (sc_steam->games[i].selected == 1){
                        sc_uninstall_stack input = {0};
                        input.game = &sc_steam->games[i];
                        us_push(uninstall_list, &input);
                        char output_buffer[BUFF_MAX] = {0};
                        snprintf(output_buffer, BUFF_MAX, "Added %s to the uninstall list", sc_steam->games[i].name);
                        append_to_log(sc_log->buffer, &sc_log->buffer_length,output_buffer);
                    }


                }
                printf("created uninstall stack of these items:\n");
                us_print(*uninstall_list);
                sc_uninstalling = 1;
                

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


void sc_uninstall_window(ImGuiIO *ioptr, steam_data *sc_steam, sc_log *sc_log, sc_uninstall_stack **uninstall_list){
    //main window config
    ImGuiWindowFlags flags = 0;
    ImVec2 window_size = {0};
    igGetWindowSize(&window_size);
    {
        
        igBegin("Uninstalling", NULL, flags );

        if (*uninstall_list == NULL){
            if (sc_uninstalling == true){
                sc_uninstalling = false;
                sc_repopulate_steam_games(sc_steam);
            }
            igTextWrapped("No games in uninstall list. This window shouldn't be open, this is a bug.");
            igEnd();
            return;
            
        }

        igSetWindowSize_Str("Uninstalling", (ImVec2){600,200}, ImGuiCond_FirstUseEver);
        igTextWrapped("Because of Steam's config, I cannot uninstall the games automatically without prompting you. We could just delete the game files without Steam, but this would leave bits left over in your registry.\n");
        igTextWrapped("I also can't click 'yes' for you when prompted.\n\nSo in lieu of a better option. Click this button to tell steam to uninstall next game, then confirm on steam, and repeat until all games in the list are gone.");
        
        char tmp_buffer[BUFF_MAX] = {0};
        char game_name[BUFF_MAX] = {0};

        snprintf(game_name, BUFF_MAX, "%s", (*uninstall_list)->game->name);
        snprintf(tmp_buffer, BUFF_MAX, "%s%s", "Uninstall ", game_name);

        if (igButton(tmp_buffer, (ImVec2){igGetWindowWidth() - 15,50} )){
            memset(tmp_buffer, 0, sizeof(tmp_buffer));
            snprintf(tmp_buffer, BUFF_MAX, "Uninstalling %s. Click the confirmation on the steam popup.", game_name);
            append_to_log(sc_log->buffer, &sc_log->buffer_length, tmp_buffer);

            char shell_command[BUFF_MAX] = {0};
            snprintf(shell_command, BUFF_MAX, "%s%s", "steam://uninstall/", (*uninstall_list)->game->appid_str);
            append_to_log(sc_log->buffer, &sc_log->buffer_length, shell_command);
            ShellExecute(NULL, "open", shell_command, NULL, NULL, SW_SHOWNORMAL);
            us_pop(uninstall_list);
        }

        if (igButton("Cancel", (ImVec2){0,0})){
            us_clear(uninstall_list);
            append_to_log(sc_log->buffer, &sc_log->buffer_length, "Canceling and clearing uninstall list");
            sc_uninstalling = 0;

        }
        igEnd();
    }
    
    return;
}


void sc_debug_window(ImGuiIO *ioptr, steam_data *sc_steam){


    //main window config
    ImGuiWindowFlags flags = 0;
    
    {
        
        igBegin("Debug", NULL, flags );
        //igText("Input char length: %d", strlen(log_buffer));
        if (igButton("repopulate", (ImVec2){0,0})){
            sc_repopulate_steam_games(sc_steam);
        }
        igText("%.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igEnd();
    }
    
    return;
}