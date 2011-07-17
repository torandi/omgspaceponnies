#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include "render_object.h"
#include <string>

#define MAX_FIRE_LENGHT 1024

#define RESPAWN_TIME 60

#define NUM_COLLISION_POINTS 8

#define MAX_VELOCITY 1500.0f

enum texture_t {
	TEXTURE_BASE,
	TEXTURE_DASH,
	TEXTURE_FWD,
	TEXTURE_LEFT,
	TEXTURE_RIGHT,
	TEXTURE_TAIL,
	TEXTURE_DISPENCER,
	TEXTURE_LAST
};

struct Player {
	vector_t pos;

	float da; //Rotation speed
	vector_t velocity; //Current velocity

	vector_t fire_end;

	bool fire;

	int dead;

	int id;

	std::string nick;


	texture_t current_base_texture;	

	RenderObject textures[TEXTURE_LAST];

	bool free_move;
	float angle;
	float power;


	Player(const char * nick, int _id);
	Player(int _id);


	void spawn();

	void render(double dt);

	void logic(double dt);

	void calc_fire(bool detect_kill);
	
	bool check_collision(const vector_t &tl, const vector_t &br);

	void accelerate(const vector_t &dv);
	vector_t collision_point(int i);

private:
	void init(int _id);
};

#endif
