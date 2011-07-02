#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <stdint.h>
#include <vector>
#include <SDL/SDL.h>

#include "vector.h"

#define PORT 7412

#define PI 3.14159265

#define SPEED 200.0f


//Forward declaration of Player

struct Player;

/**
 * Current animation step.
 * 0.0 -> 1.0
 */
extern float step;

/**
 * Port to broadcast on
 */
extern int port;


extern FILE* verbose;

extern Player * players[4];

extern Player * me;

extern vector_t mouse;

extern bool keys[SDLK_LAST];

#endif /* COMMON_H */
