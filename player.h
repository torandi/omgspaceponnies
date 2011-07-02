#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include <string>

struct Player {
	vector_t pos;
	vector_t target;

	vector_t fire_end;

	bool dashing, fire;

	std::string texture, nick;

	bool free_move;
	float angle;
	float power;


	Player(const char * nick);
	Player();


	void dash();
	void spawn();

private:
	void init();
};

#endif
