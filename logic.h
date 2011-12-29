#ifndef LOGIC_H
#define LOGIC_H

#include "player.h"
#include <SDL/SDL.h>

#define DASH_SPEED_FACTOR 2.2f

extern bool keys[SDLK_LAST];
extern vector_t mouse;


void logic(double dt);
Player * create_player(char * nick, int id);

#endif
