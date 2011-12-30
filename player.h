#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include "texture.h"
#include "render_object.h"
#include <string>

#define MAX_FIRE_LENGHT 1024

#define RESPAWN_TIME 300

#define NUM_COLLISION_POINTS 8

#define MAX_VELOCITY 1500.0f

#define PWR_REGEN_FACTOR 0.15
#define DASH_POWER 0.5f
#define FIRE_POWER 1.0f
#define SHIELD_POWER 0.4f

#define KILL_SCORE 200
#define TEAM_KILL_SCORE -400

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

	vector_t velocity; //Current velocity
	vector_t fire_end;
	bool fire;
	int dead;
	int id;
	int team;
	int score;
	float shield_angle;
	bool full_shield;
	int flash_power;
	std::string nick;
	Texture * shield;
	texture_t current_base_texture;	
	RenderObject textures[TEXTURE_LAST];
	float angle;
	float power;

	Player(const char * nick, int _team);
	void spawn();
	void spawn_remote(vector_t new_pos);
	void render(double dt);
	void render_fire(double dt);
	void logic(double dt, float last_angle=0);
	bool calc_player_hit(Player * player);
	void calc_fire();
	bool check_collision(const vector_t &tl, const vector_t &br);
	bool use_power(float amount);
	void add_score(int _score);
	void accelerate(const vector_t &dv);
	vector_t collision_point(int i, const float * a = NULL) const;
	void init_gfx();

private:
	void init(int _team);
	void shield_coords(float a, float &x, float &y);

	void hndl_collision(int mx, int my, int cp_index, const vector_t &cp, float last_angle, double dt);
};

#endif
