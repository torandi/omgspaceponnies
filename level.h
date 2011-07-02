#ifndef LEVEL_H
#define LEVEL_H

#include "vector.h"

#define MAP_HEIGHT 24
#define MAP_WIDTH 32


extern int map[MAP_HEIGHT][MAP_WIDTH];
extern int spawn_pos[4][2];

void init_level();
int map_value(int x, int y);
void calc_map_index(const vector_t &pos, int &x, int &y);

#endif
