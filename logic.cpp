#include <stdio.h>
#include <math.h>

#include "common.h"
#include "logic.h"

static float dash_angle;

void logic(double dt) {
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
		double s = DASH_SPEED * dt;
		float dx = s * cos(dash_angle);
		float dy = s * sin(dash_angle);
		me.pos.x += dx;
		me.pos.y += dy;

		if( abs(me.pos.x - me.target.x) < abs(dx) && abs(me.pos.y - me.target.y) < abs(dy)) {
			me.dashing = false;
		}
	}
}
