#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <stdint.h>
#include <vector>
#include "player.h"

#define PORT 7412

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

extern std::vector<Player> players;

extern Player me;

extern vector_t mouse;

#endif /* COMMON_H */
