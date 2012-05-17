#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include "vector.h"
#include "texture.h"
#include "render_object.h"
#include <string>

#define MAX_FIRE_LENGHT 1024

#define RESPAWN_TIME 10 
//300

#define NUM_COLLISION_POINTS 8

#define MAX_VELOCITY 1500.0f

#define PWR_REGEN_FACTOR 10.f
//0.15
#define DASH_POWER 0.5f
#define FIRE_POWER 1.0f
#define SHIELD_POWER 0.4f
#define SHIELD_HIT_POWER_DRAIN 0.4f
#define SHIELD_RADIUS 100.f

#define KILL_SCORE 200
#define TEAM_KILL_SCORE -400

#define MIN_POWER_SYNC_DELAY 1000.f

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

class Player {
public:
	vector_t pos;

	vector_t s1, s2;

	vector_t velocity; //Current velocity
	vector_t fire_end;
	double last_power_sync;
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
	int health;

	Player(const char * nick, int _team);
	void spawn();
	void spawn_remote(vector_t new_pos);
	void render(double dt);
	void render_fire(double dt);
	void logic(double dt, float last_angle=0);
	bool calc_player_hit(Player * player, double dt);
	void calc_fire(double dt);
	bool hit(Player * player, double dt);
	void sync_power();
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
	bool shield_hit(Player * player);
	bool find_shield_intersection(Player * player, vector_t * ret);
};

#endif
