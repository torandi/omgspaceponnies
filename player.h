#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include "render_object.h"
#include <string>

enum texture_t {
	TEXTURE_BASE,
	TEXTURE_DASH,
	TEXTURE_LEFT,
	TEXTURE_RIGHT,
	TEXTURE_TAIL,
	TEXTURE_DISPENCER,
	TEXTURE_LAST
};

struct Player {
	vector_t pos;
	vector_t target;
	unsigned long dash_start;

	vector_t fire_end;

	bool dashing, fire, dead;

	int id;

	std::string nick;


	texture_t current_base_texture;	

	RenderObject textures[TEXTURE_LAST];

	bool free_move;
	float angle;
	float power;


	Player(const char * nick, int _id);
	Player(int _id);


	void dash();
	void spawn();

	void render(double dt);

private:
	void init(int _id);
};

#endif
