#include <stdio.h>
#include <math.h>

#include "common.h"
#include "logic.h"

void logic(double dt) {
	//Calculate angle:
	if(!me.free_move) {
		me.angle = atan2(mouse.y - me.pos.y, mouse.x - me.pos.x)+PI/2.0;
	}
}
