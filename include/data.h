#ifndef DATA_H
#define DATA_H
#include "steam_cleaner.h"
#include "config.h"
// #include <stdio.h>
// #include <windows.h>
// #include <stdbool.h>
// #include <time.h>
// #include <SDL2/SDL.h>
// #include "config.h"




typedef struct{
    char name[BUFF_MAX];
    char path[BUFF_MAX];
    int appid;
    int64_t size_on_disk;
    int64_t last_played;
    char appid_str[BUFF_MAX];
    char size_on_disk_str[BUFF_MAX];
    char last_played_str[BUFF_MAX];
    char last_played_pretty[BUFF_MAX];
    bool selected;
} steam_game;

typedef struct{
    char install_path[BUFF_MAX];
    char **library_paths;
    int library_paths_count;
    steam_game *games;
    int games_count;
} steam_data;

typedef struct {
    char buffer[LOG_BUFF_MAX];
    int buffer_length;

} sc_log;

struct sc_uninstall_stack{
    steam_game *game;
    struct sc_uninstall_stack *next;
};
typedef struct sc_uninstall_stack sc_uninstall_stack;

typedef struct{
    bool uninstalling;
    bool loaded;
    steam_data *steam_data;
    sc_log log;
    sc_uninstall_stack *uninstall_list;
}sc_steam_cleaner;

void sc_populate_steam_data(steam_data *sc_steam); //SC = Steam Cleaner
void sc_populate_steam_libraries(steam_data *sc_steam, const char *lib_vdf_path);
void sc_populate_steam_games(steam_data *sc_steam);
void sc_add_steam_library(steam_data *sc_steam, const char *path);
void sc_add_steam_game(steam_data *sc_steam, steam_game *game);
void sc_free_steam_data(steam_data *sc_steam);
void sc_repopulate_steam_games(steam_data *sc_steam);


sc_uninstall_stack *us_create_node(sc_uninstall_stack *input); //US = uninstall stack. A stack data structure to keep track games to uninstall
void us_push(sc_uninstall_stack **head, sc_uninstall_stack *input); 
void us_pop(sc_uninstall_stack **head);
void us_print(sc_uninstall_stack *head);
void us_clear(sc_uninstall_stack **head);



#endif