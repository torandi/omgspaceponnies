#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>

#include "vector.h"

//Forward declaration of Player
struct Player;

/**
 * Current animation step.
 * 0.0 -> 1.0
 */
extern float step;

extern FILE* verbose;

extern Player * me;

extern vector_t mouse;

extern bool keys[SDLK_LAST];

extern bool ready;

extern char * myname;

#endif /* CLIENT_H */
