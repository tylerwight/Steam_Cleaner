#ifndef DATA_H
#define DATA_H
#include "steam_cleaner.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include "config.h"
#include "data_structs.h"








void convert_unix_to_date(int64_t unix_time, char *buffer, size_t buffer_size);
void byte_to_human(uint64_t size_in_bytes, char *output, size_t output_size);

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