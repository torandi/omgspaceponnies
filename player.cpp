#include "player.h"
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Player::Player(const char * n) {
	nick = std::string(n);
	init();
}

Player::Player() {
	init();
}

void Player::init() {
	free_move = false;
	angle = PI;
	dashing = false;
	power = 1.0;
	fire = false;
}


void Player::spawn() {
	pos.x = rand() % 600;
	pos.y = rand() % 800;

	printf("Spawned %s at (%f, %f)\n", nick.c_str(), pos.x, pos.y);
}

void Player::dash() {
	dashing = true;
	target = vector_t(mouse);
}

