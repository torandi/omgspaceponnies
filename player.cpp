#include "player.h"
#include "common.h"
#include "render.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <math.h>
#include <sys/time.h>

Player::Player(const char * n, int _id) {
	nick = std::string(n);
	init(_id);
}

Player::Player(int _id) {
	init(_id);
}

void Player::init(int _id) {
	free_move = false;
	angle = PI;
	dashing = false;
	power = 1.0;
	fire = false;
	id = _id;
	dead = false;

	dx = 0;
	dy = 0;

	current_base_texture = TEXTURE_BASE;

	char texture[64];

	//Load textures:
	vector_t size = vector_t(PLAYER_W, PLAYER_H);

	//Base
	sprintf(texture,"gfx/player%i/base.png", id+1);
   textures[TEXTURE_BASE] = RenderObject(texture, 1, 25, size);

	//Dash
	sprintf(texture,"gfx/dash.png", id+1);
   textures[TEXTURE_DASH] = RenderObject(texture, 1, 25, size);

	//Left
	sprintf(texture,"gfx/left.png", id+1);
   textures[TEXTURE_LEFT] = RenderObject(texture, 1, 25, size);

	//Right
	sprintf(texture,"gfx/right.png", id+1);
   textures[TEXTURE_RIGHT] = RenderObject(texture, 1, 25, size);


	//Tail
	sprintf(texture,"gfx/tail.png", id+1);
   textures[TEXTURE_TAIL] = RenderObject(texture, 9, 25, size);

	//Dispencer
	sprintf(texture,"gfx/dispencer.png", id+1);
   textures[TEXTURE_DISPENCER] = RenderObject(texture, 6, 25, size);

}


void Player::spawn() {
	pos.x = rand() % 600;
	pos.y = rand() % 800;

	printf("Spawned %s at (%f, %f)\n", nick.c_str(), pos.x, pos.y);
}

void Player::dash() {
	dashing = true;
	target = vector_t(mouse);
	struct timeval now;
	gettimeofday(&now, NULL);
	dash_start = now.tv_sec + now.tv_usec/1000000.0;
}

void Player::render(double dt) {
	animation_t* anim = NULL;

	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();

	glTranslatef(pos.x, pos.y, 0);
	glRotatef(radians_to_degrees(angle+PI/2.0), 0, 0, 1.0);
	glTranslatef(-PLAYER_W*0.5,-PLAYER_H*0.5, 0);
	
	//Draw textures:
	textures[TEXTURE_BASE].render(dt);
	if(current_base_texture != TEXTURE_BASE)
		textures[current_base_texture].render(dt);
	textures[TEXTURE_TAIL].render(dt);
	if(fire)
		textures[TEXTURE_DISPENCER].render(dt);

	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);

	glDisable(GL_TEXTURE_2D);

	glColor3f(1,0,0);
	glPointSize(5);
	glBegin(GL_POINTS);
		glVertex2f(pos.x,pos.y);
	glEnd();

	glColor3f(1,0,1);
	glBegin(GL_LINES);
		glVertex2f(pos.x, pos.y);
		glVertex2f(mouse.x, mouse.y);
	glEnd();

	if(fire) {
		glBegin(GL_LINES);
			glLineWidth(1);
			for(int i=0;i<12;++i) {
				float dx = i*cos(angle+PI/2.0) - 6*cos(angle+PI/2.0) + cos(angle)*PLAYER_H/2.0;
				float dy = i*sin(angle+PI/2.0) - 6*sin(angle+PI/2.0) + sin(angle)*PLAYER_H/2.0;
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
	pos.x += dt*dx;
	pos.y += dt*dy;
	angle += dt*da;
}
