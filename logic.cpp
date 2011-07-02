#include <stdio.h>
#include <math.h>

#include "common.h"
#include "logic.h"

static float dash_angle;

void logic(double dt) {
	double s = SPEED * dt;

	if(keys[SDLK_RCTRL])
		me.free_move = true;
	else 
		me.free_move = false;

	//Calculate angle:
	if(!me.free_move) {
		me.angle = atan2(mouse.y - me.pos.y, mouse.x - me.pos.x);
	}

	if(keys[SDLK_SPACE]) {
		me.dash();
		dash_angle = atan2(mouse.y - me.pos.y, mouse.x - me.pos.x);
	}

	if(me.dashing) {
		float dx = s * cos(dash_angle);
		float dy = s * sin(dash_angle);
		me.pos.x += dx;
		me.pos.y += dy;

		if( abs(me.pos.x - me.target.x) < abs(dx) && abs(me.pos.y - me.target.y) < abs(dy)) {
			me.dashing = false;
		}
	}

	if(keys[SDLK_d] || keys[SDLK_e]) {
		me.pos.x += s * cos(me.angle+PI/2.0);
		me.pos.y += s * sin(me.angle+PI/2.0);
	}

	if(keys[SDLK_a]) {
		me.pos.x += s * cos(me.angle+PI*1.5);
		me.pos.y += s * sin(me.angle+PI*1.5);
	}

	if(me.fire) {
		me.fire_end.x = me.pos.x + 300*cos(me.angle);
		me.fire_end.y = me.pos.y + 300*sin(me.angle);
	}
}
