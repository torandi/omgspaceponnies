#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <SDL/SDL.h>

#include "common.h"
#include "logic.h"
#include "player.h"
#include "level.h"
#include "client.h"

#define SEND_DELAY 0.05f

bool keys[SDLK_LAST];
vector_t mouse;

static double last_send=0;

static bool has_sent_still = false;
static bool has_sent_no_fire = true;


void logic(double dt) {
	double s = SPEED * dt;

	vector_t last = vector_t(client->me->pos);
	float last_a = client->me->angle;

	int mouse_x,mouse_y;
	SDL_GetMouseState( &mouse_x, &mouse_y);
	mouse.x = mouse_x - (window.w/2.0-client->me->pos.x);
	mouse.y = mouse_y - (window.h/2.0-client->me->pos.y);
					

	if(client->me->dead == 0) {
		client->me->current_base_texture = TEXTURE_BASE;

		client->me->angle = atan2(mouse.y - client->me->pos.y, mouse.x - client->me->pos.x);

		if(keys[SDLK_SPACE]) {
			if(client->me->use_power(DASH_POWER*dt)) {
				client->me->accelerate(vector_t(
					cos(client->me->angle),
					sin(client->me->angle)
					) * s * DASH_SPEED_FACTOR);
				client->me->current_base_texture = TEXTURE_DASH;
			}
		}

		if(keys[SDLK_d] || keys[SDLK_e]) {
			client->me->accelerate(vector_t(
				cos(client->me->angle+M_PI/2.0),
				sin(client->me->angle+M_PI/2.0)
				) * s);
			client->me->current_base_texture = TEXTURE_LEFT;
		}

		if(keys[SDLK_a]) {
			client->me->accelerate(vector_t(
				cos(client->me->angle+M_PI*1.5),
				sin(client->me->angle+M_PI*1.5)
				) * s);
			client->me->current_base_texture = TEXTURE_RIGHT;
		}

		if(keys[SDLK_w] || keys[SDLK_COMMA]) {
			client->me->accelerate(vector_t(
				cos(client->me->angle),
				sin(client->me->angle)
				) * s);
			client->me->current_base_texture = TEXTURE_FWD;
		}

		client->me->logic(dt, last_a);

		if(client->me->fire) {
			client->me->full_shield = false;
			if(client->me->use_power(FIRE_POWER*dt))
				client->me->calc_fire();
			else
				client->me->fire = false;
		}

		if(client->me->full_shield) {
			if(!client->me->use_power(SHIELD_POWER*dt))
				client->me->full_shield = false;
		}


		if(last_send+SEND_DELAY < curtime()) {
			char buffer[1024];

			vector_t delta = client->me->pos - last;
			float da = (client->me->angle - last_a)/dt;

			delta.x/=dt;
			delta.y/=dt;


			if(delta.norm() > 1 || client->me->fire) {
				client->send_move(delta);
				has_sent_still = false;
			} else if(!has_sent_still){
				client->send_move(vector_t(0,0));
				has_sent_still = true;
			} else if(da > 0.1) {
				client->send_rotate();
			}

			if(client->me->fire) {
				client->send_fire();
				has_sent_no_fire = false;
			} else if(!has_sent_no_fire) {
				client->send_fire();
				has_sent_no_fire = false;
			}
				
			last_send = curtime();
		}
	} else {
		++client->me->dead;
		if(client->me->dead > RESPAWN_TIME)
			client->me->spawn();
	}
}

double curtime()  {
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return ts.tv_sec + ts.tv_usec/1000000.0;
}

