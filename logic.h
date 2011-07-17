#ifndef LOGIC_H
#define LOGIC_H

#include "player.h"

#define DASH_SPEED_FACTOR 2.2f

void logic(double dt);
Player * create_player(char * nick, int id);

#endif
