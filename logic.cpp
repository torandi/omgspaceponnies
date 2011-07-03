#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "common.h"
#include "logic.h"
#include "player.h"
#include "network.h"
#include "level.h"

#define SEND_DELAY 0.1f

static float dash_angle;

static double last_send=0;

static bool has_sent_still = false;
static bool has_sent_no_rot = false;
static bool has_sent_no_fire = true;

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

		if(keys[SDLK_RCTRL])
			me->free_move = true;
		else 
			me->free_move = false;

		//Calculate angle:
		if(!me->free_move) {
			me->angle = atan2(mouse.y - me->pos.y, mouse.x - me->pos.x);
		}

		if(keys[SDLK_SPACE]) {
			me->dash();
			dash_angle = atan2(mouse.y - me->pos.y, mouse.x - me->pos.x);
		}

		if(me->dashing) {
			float dx = s * cos(dash_angle) * 2.2;
			float dy = s * sin(dash_angle) * 2.2;
			me->pos.x += dx;
			me->pos.y += dy;

			struct timeval now;
			gettimeofday(&now, NULL);

			if( (me->pos - me->target).norm() < s*1.5)
				me->dashing = false;
			else if( me->dash_start+1 > (now.tv_sec+now.tv_usec/1000000.0))
				me->current_base_texture = TEXTURE_DASH;
		}

		if(keys[SDLK_d] || keys[SDLK_e]) {
			me->pos.x += s * cos(me->angle+PI/2.0);
			me->pos.y += s * sin(me->angle+PI/2.0);
			me->current_base_texture = TEXTURE_LEFT;
		}

		if(keys[SDLK_a]) {
			me->pos.x += s * cos(me->angle+PI*1.5);
			me->pos.y += s * sin(me->angle+PI*1.5);
			me->current_base_texture = TEXTURE_RIGHT;
		}

		if(keys[SDLK_w] || keys[SDLK_COMMA]) {
			me->pos.x += s * cos(me->angle);
			me->pos.y += s * sin(me->angle);
			me->current_base_texture = TEXTURE_DASH;
		}


		if(keys[SDLK_o] || keys[SDLK_s]) {
			me->pos.x += s * cos(me->angle+PI);
			me->pos.y += s * sin(me->angle+PI);
		}

		bool loop = false;
		float da=me->angle - last_a;
		vector_t diff = me->pos - last;
		int l = 10;
		do {
			loop = false;
			int mx, my;
			float ax, ay;

			ax = abs((PLAYER_H/2.1) * cos(me->angle)) + abs((PLAYER_W/2.1)*sin(me->angle));
			ay = abs((PLAYER_W/2.1) * cos(me->angle)) + abs((PLAYER_H/2.1)*sin(me->angle));
			calc_map_index(vector_t(me->pos.x-ax,me->pos.y), mx, my);
			if(map_value(mx, my)>0) {
				goto collision;
			}
			calc_map_index(vector_t(me->pos.x+ax,me->pos.y), mx, my);
			if(map_value(mx, my)>0) {
				goto collision;
			}
			calc_map_index(vector_t(me->pos.x,me->pos.y+ay), mx, my);
			if(map_value(mx, my)>0) {
				goto collision;
			}
			calc_map_index(vector_t(me->pos.x,me->pos.y-ay), mx, my);
			if(map_value(mx, my)>0) {
				goto collision;
			}
			break;

			collision:
				me->pos.x -= diff.x;
				me->pos.y -= diff.y;
				me->angle -= da;
				me->dashing = false;
				loop = true;

				--l;

		} while(loop && l > 0);

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

