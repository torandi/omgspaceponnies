#include "player.h"
#include "common.h"
#include "render.h"
#include "level.h"
#include "texture.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <math.h>
#include <sys/time.h>


Player::Player(const char * n, int _team) {
	nick = std::string(n);
	init(_team);
}

Player::Player(int _team) {
	init(_team);
}

void Player::init(int _team) {
	char texture[64];

	angle = M_PI;
	power = 1.0;
	fire = false;
	team = _team;
	dead = 0;
	da = 0;
	full_shield = false;

	shield_angle = M_PI;

	velocity = vector_t(0.0f,0.0f);

	current_base_texture = TEXTURE_BASE;


	//Load textures:
	vector_t size = vector_t(PLAYER_W, PLAYER_H);

	shield = new Texture("gfx/shield.png",1); 

	//Base
	sprintf(texture,"gfx/player%i/base.png", team+1);
	textures[TEXTURE_BASE] = RenderObject(texture, 1, 25, size);

	//Dash
	sprintf(texture,"gfx/dash.png");
	textures[TEXTURE_DASH] = RenderObject(texture, 1, 25, size);

	//Fwd
	sprintf(texture,"gfx/fwd.png");
	textures[TEXTURE_FWD] = RenderObject(texture, 1, 25, size);

	//Left
	sprintf(texture,"gfx/left.png");
	textures[TEXTURE_LEFT] = RenderObject(texture, 1, 25, size);

	//Right
	sprintf(texture,"gfx/right.png");
	textures[TEXTURE_RIGHT] = RenderObject(texture, 1, 25, size);


	//Tail
	sprintf(texture,"gfx/tail.png");
	textures[TEXTURE_TAIL] = RenderObject(texture, 9, 25, size);

	//Dispencer
	sprintf(texture,"gfx/dispencer.png");
	textures[TEXTURE_DISPENCER] = RenderObject(texture, 6, 25, size);
	
}


void Player::spawn() {
	pos = get_rand_spawn();
	dead = 0;
	power = 1.0;
	da = 0;
	angle = M_PI;
}

void Player::render(double dt) {

	texture_colors[1]=1;
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	glTranslatef(pos.x, pos.y, 0);
	glRotatef(radians_to_degrees(angle+M_PI_2), 0, 0, 1.0);
	glTranslatef(-PLAYER_W*0.5,-PLAYER_H*0.5, 0);

	//Draw textures:

	textures[TEXTURE_BASE].render(dt);
	textures[TEXTURE_TAIL].render(dt);
	if(current_base_texture != TEXTURE_BASE)
		textures[current_base_texture].render(dt);
	if(fire)
		textures[TEXTURE_DISPENCER].render(dt);

	glPopMatrix();


	glMatrixMode(GL_PROJECTION);

	if(!fire) {
		glPushMatrix();
		
		glTranslatef(pos.x-shield->width()/2, pos.y-shield->height()/2,0);

		shield->bind();
		if(full_shield) {
			glBegin(GL_QUADS);
				glTexCoord2f(0,1.0); glVertex2f(0,200.0);
				glTexCoord2f(0,0); glVertex2f(0,0);
				glTexCoord2f(1.0,0); glVertex2f(200.0,0);
				glTexCoord2f(1.0,1.0); glVertex2f(200.0,200.0);
			glEnd();
		} else {
			float sx1, sy1, sx2, sy2;
			shield_coords(shield_angle-M_PI_4,sx1,sy1);
			shield_coords(shield_angle+M_PI_4,sx2,sy2);
			int corner = (int)floor(period(shield_angle)/(M_PI_2));
			float cx, cy;

			switch(corner) {
				case 0:
					cx = 0;
					cy = 1;
					break;
				case 1:
					cx = 0;
					cy = 0;
					break;
				case 2:
					cx = 1;
					cy = 0;
					break;
				case 3:
					cx = 1;
					cy = 1;
					break;
			}
			glBegin(GL_TRIANGLES);
				glTexCoord2f(sx1,sy1); glVertex2f(200.0*sx1,200.0*sy1);
				glTexCoord2f(sx2,sy2); glVertex2f(200.0*sx2,200.0*sy2);
				glTexCoord2f(0.5, 0.5); glVertex2f(100.0, 100.0);
				glTexCoord2f(sx1,sy1); glVertex2f(200.0*sx1,200.0*sy1);
				glTexCoord2f(sx2,sy2); glVertex2f(200.0*sx2,200.0*sy2);
				glTexCoord2f(cx,cy); glVertex2f(200.0*cx,200.0*cy);
			glEnd();
			
		}

		glPopMatrix();
	}
	

	texture_colors[1]=0.5;

}

void Player::shield_coords(float a, float &x, float &y) {
	a = period(a+M_PI_2);
	if(a <= M_PI_4 || a >= 7.0*M_PI_4) {
		x = 1.0;
		y = tan(a);
	} else if(a > M_PI_4 && a <= 3.0*M_PI_4) {
		x = cos(a)/sin(a);
		y = 1;
	} else if(a > 3.0*M_PI_4 && a <= 5.0*M_PI_4) {
		x = -1.0;
		y = -tan(a);
	} else {
		x = -cos(a)/sin(a);
		y = -1.0;
	}
	x = (x+1.0)*0.5;
	y = (y+1.0)*0.5;
}

void Player::render_fire(double dt) {
	glDisable(GL_TEXTURE_2D);

	if(fire) {
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		for(int i=0;i<12;++i) {
			float dx = i*cos(angle+M_PI_2) - 6*cos(angle+M_PI_2) + cos(angle)*PLAYER_H/2.0;
			float dy = i*sin(angle+M_PI_2) - 6*sin(angle+M_PI_2) + sin(angle)*PLAYER_H/2.0;
			glColor3f(rbcolors[i][0],rbcolors[i][1],rbcolors[i][2]);

			glVertex2f(pos.x+dx, pos.y+dy);
			glVertex2f(fire_end.x+dx, fire_end.y+dy);
			glVertex2f(pos.x+dx, pos.y+dy);
			glVertex2f(pos.x+cos(angle)*PLAYER_H*0.1,pos.y+sin(angle)*PLAYER_H*0.1);
		}
		glEnd();
		glLineWidth(1.0f);
	}

	glEnable(GL_TEXTURE_2D);
}

void Player::logic(double dt) {
	pos += velocity * dt;
	angle += dt*da;

	if(fire) {
		calc_fire(false);
	}

	if(dead > 0) {
		++dead;
		if(dead > RESPAWN_TIME)
			spawn();
	}

	power+=(power*0.3+1)*dt*PWR_REGEN_FACTOR;
	if(power > 1.0) 
		power = 1.0;
}

bool Player::use_power(float amount) {
	if(power >= amount) {
		power -= amount;
		return true;
	} else {
		if(this == me && flash_power <= 0) 
			flash_power = 1;
		return false;
	}
}

void Player::calc_fire(bool detect_kill) {
	//fire_end.x = ((pos.x/64)+1)*64;
	//fire_end.y = ((pos.y/64)+1)*64;
	fire_end = pos;
	int len = 0;
	while(len < MAX_FIRE_LENGHT) {
		len += 32;
		vector_t prev(fire_end);
		fire_end.x+= 32*cos(angle);
		fire_end.y+= 32*sin(angle);

		for(int i=0; i < NUM_PLAYERS; ++i) {
			if(i != id && players[i] != NULL && players[i]->dead == 0) {
				vector_t d;
				d.x = fire_end.x - players[i]->pos.x;
				d.y = fire_end.y - players[i]->pos.y;
				if(d.norm() < PLAYER_W/2.0)  {
					players[i]->dead = 1;
					printf("Killed player %i\n", i);
					char buffer[128];
					sprintf(buffer, "omg kil %i", i);
				}
			}

			int mx, my;
			calc_map_index(fire_end, mx, my);
			if(map_value(mx, my) > 0) {
				fire_end = prev;
				//map[my][mx]+=10;
				break;
			}
		}
	}

}

void Player::accelerate(const vector_t &dv) {
	velocity+=dv;
	if(velocity.norm() > MAX_VELOCITY) {
		velocity = velocity.normalized() * MAX_VELOCITY;
	}
}

/**
 * Fetches the specified collision point
 * Set a to an angle to use that instead of this->angle
 */
vector_t Player::collision_point(int i, const float * a) const{
	float ax, ay;
	if(a == NULL)
		a = &angle; 
	ax = PLAYER_W/2.0;
	ay = PLAYER_H/2.0;
	vector_t v(0,0);
	switch(i) {
		case 0:
			v.x -= ax-17.0;
			v.y -= ay-13.0;
			break;
		case 1:
			v.y -= ay-10.0;
			break;
		case 2:
			v.x += ax-14.0;
			v.y -= ay-16.0;
			break;
		case 3:
			v.x -= ax-20.0;
			break;
		case 4:
			v.x += ax-20.0;
			break;
		case 5:
			v.x -= ax-8.0;
			v.y += ay-23.0;
			break;
		case 6:
			v.y += ay;
			break;
		case 7:
			v.x += ax-12.0;
			v.y += ay-18.0;
			break;
	}
	return pos+v.rotate(*a-M_PI*0.5);
}
