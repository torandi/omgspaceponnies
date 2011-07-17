#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "common.h"
#include "logic.h"
#include "player.h"
#include "network.h"
#include "level.h"

#define SEND_DELAY 0.1f
#define REPULSE_LIMIT 0.3f

static double last_send=0;

static bool has_sent_still = false;
static bool has_sent_no_rot = false;
static bool has_sent_no_fire = true;

static void hndl_collision(int mx, int my, int cp_index, const vector_t &cp, float last_angle, double dt);

void logic(double dt) {
	double s = SPEED * dt;

	vector_t last = vector_t(me->pos);
	float last_a = me->angle;

	int mouse_x,mouse_y;
	SDL_GetMouseState( &mouse_x, &mouse_y);
	mouse.x = mouse_x - (window.w/2.0-me->pos.x);
	mouse.y = mouse_y - (window.h/2.0-me->pos.y);
					

	if(me->dead == 0) {
		me->current_base_texture = TEXTURE_BASE;

		me->angle = atan2(mouse.y - me->pos.y, mouse.x - me->pos.x);

		if(keys[SDLK_SPACE]) {
			me->accelerate(vector_t(
				cos(me->angle),
				sin(me->angle)
				) * s * DASH_SPEED_FACTOR);
			me->current_base_texture = TEXTURE_DASH;
		}

		if(keys[SDLK_d] || keys[SDLK_e]) {
			me->accelerate(vector_t(
				cos(me->angle+PI/2.0),
				sin(me->angle+PI/2.0)
				) * s);
			me->current_base_texture = TEXTURE_LEFT;
		}

		if(keys[SDLK_a]) {
			me->accelerate(vector_t(
				cos(me->angle+PI*1.5),
				sin(me->angle+PI*1.5)
				) * s);
			me->current_base_texture = TEXTURE_RIGHT;
		}

		if(keys[SDLK_w] || keys[SDLK_COMMA]) {
			me->accelerate(vector_t(
				cos(me->angle),
				sin(me->angle)
				) * s);
			me->current_base_texture = TEXTURE_FWD;
		}


		me->logic(dt);

		//Collision detect on the collision points:
		for(int i = 0; i < NUM_COLLISION_POINTS; ++i) {
			int mx, my;
			vector_t cp = me->collision_point(i);
			calc_map_index(cp, mx, my);
			if(map_value(mx, my)>0) {
				hndl_collision(mx,my,i, cp, last_a , dt);
			}
		}



		if(me->fire) {
			me->calc_fire(true);
		}


		if(last_send+SEND_DELAY < curtime()) {
			char buffer[1024];

			vector_t delta = me->pos - last;
			float da = (me->angle - last_a)/dt;

			delta.x/=dt;
			delta.y/=dt;

			if(delta.norm() > 1 || me->fire) {
					sprintf(buffer, "omg mov %i %f %f %f %i %f %f %f", me->id, me->pos.x, me->pos.y, me->angle, me->current_base_texture, delta.x, delta.y, da);
				send_msg(buffer);
				has_sent_still = false;
			} else if(!has_sent_still){
				sprintf(buffer, "omg mov %i %f %f %f %i 0 0 %f", me->id, me->pos.x, me->pos.y, me->angle, me->current_base_texture, da);
				send_msg(buffer);
				has_sent_still = true;
			} else if(da > 0.1) {
				has_sent_no_rot = false;
				sprintf(buffer, "omg rot %i %f %f",me->id, me->angle, da); 
				send_msg(buffer);
			} else if(!has_sent_no_rot) {
				has_sent_no_rot = true;
				sprintf(buffer, "omg rot %i %f 0",me->id, me->angle); 
				send_msg(buffer);
			}

			if(me->fire) {
				sprintf(buffer, "omg fir %i", me->id);
				send_msg(buffer);
				has_sent_no_fire = false;
			} else if(!has_sent_no_fire) {
				sprintf(buffer, "omg nof %i", me->id);
				send_msg(buffer);
				has_sent_no_fire = false;
			}
				
			last_send = curtime();
		}
	} else {
		++me->dead;
		if(me->dead > RESPAWN_TIME)
			me->spawn();
	}
	//Update other:
	for(int i=0; i < 4; ++i) {
		if(i != me->id && players[i] != NULL) {
			players[i]->logic(dt);	
		}
	}
}

static void hndl_collision(int mx, int my, int cp_index, const vector_t &cp, float last_angle, double dt) {
	vector_t block = vector_t(mx*64,my*64);

	vector_t repulse = (block - cp).normalized().abs();


	if(repulse.x-repulse.y > REPULSE_LIMIT) {
		me->velocity.x *= -1.0f;
	} else if(repulse.y - repulse.x > REPULSE_LIMIT) {
		me->velocity.y *= -1.0f;
	} else {
		me->velocity *= -1.0f;
	}

	vector_t old_cp = me->collision_point(cp_index, &last_angle);

	vector_t rotation_impuls = old_cp - cp;

	me->velocity+= (rotation_impuls/dt)*0.4;
	
	me->pos += me->velocity*dt + rotation_impuls;
	me->velocity *= ELASTICITY;
	map[my][mx] = 2;
}

Player * create_player(char * nick, int id) {
	printf("Created player %d\n", id);
	Player * p = new Player(nick, id);
	p->spawn();
	players[id] = p;
	return p;
}

double curtime()  {
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return ts.tv_sec + ts.tv_usec/1000000.0;
}

