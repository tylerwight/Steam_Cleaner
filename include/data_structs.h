#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H
#include "config.h"
#include <stdint.h>

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

#endif