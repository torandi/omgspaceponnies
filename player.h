#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include <string>

struct Player {
	vector_t pos;
	std::string texture, nick;


	Player(const char * nick);
	Player();

	void spawn();
};

#endif
