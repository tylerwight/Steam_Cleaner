#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define BUFF_MAX 1024
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

void sc_populate_steam_data(steam_data *sc_steam);
void sc_populate_steam_libraries(steam_data *sc_steam, const char *lib_vdf_path);
void sc_populate_steam_games(steam_data *sc_steam);
void sc_add_steam_library(steam_data *sc_steam, const char *path);
void sc_add_steam_game(steam_data *sc_steam, steam_game *game);



#endif