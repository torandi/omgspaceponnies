#include "player.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Player::Player(const char * n) {
	nick = std::string(n);
}

Player::Player() {

}

void Player::spawn() {
	pos.x = rand() % 600;
	pos.y = rand() % 800;

	printf("Spawned %s at (%f, %f)\n", nick.c_str(), pos.x, pos.y);
}
