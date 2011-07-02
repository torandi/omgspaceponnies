#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "common.h"
#include "logic.h"
#include "player.h"

static float dash_angle;

void logic(double dt) {
	double s = SPEED * dt;

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
		float dx = s * cos(dash_angle);
		float dy = s * sin(dash_angle);
		me->pos.x += dx;
		me->pos.y += dy;

		struct timeval now;
		gettimeofday(&now, NULL);

		if( (me->pos - me->target).norm() < s*1.5)
			me->dashing = false;
		else if( me->dash_start+0.5 > (now.tv_sec+now.tv_usec/1000000.0))
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

	if(me->fire) {
		me->fire_end.x = me->pos.x + 300*cos(me->angle);
		me->fire_end.y = me->pos.y + 300*sin(me->angle);
	}
}

Player * create_player(char * nick, int id) {
	Player * p = new Player(nick, id);
	p->spawn();
	players[id] = p;
	return p;
}
