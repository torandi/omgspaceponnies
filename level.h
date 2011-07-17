#ifndef LEVEL_H
#define LEVEL_H

#include "vector.h"
#include <vector>

#define MAP_HEIGHT 24
#define MAP_WIDTH 32

#define ELASTICITY 0.5 //Percent of velocity to keep on collision with wall


extern int map[MAP_HEIGHT][MAP_WIDTH];
extern std::vector<vector_t> spawn_pts;

void init_level();
int map_value(int x, int y);
void calc_map_index(const vector_t &pos, int &x, int &y);
vector_t get_rand_spawn();

#endif
