#include "player.h"
#include "common.h"
#include "render.h"
#include "level.h"
#include "texture.h"
#include "server.h"
#include "client.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <math.h>
#include <sys/time.h>

#define REPULSE_LIMIT 0.01f

Player::Player(const char * n, int _team) {
	nick = std::string(n);
	init(_team);
}

void Player::init(int _team) {

	angle = M_PI;
	power = 1.0;
	fire = false;
	team = _team;
	dead = 1;
	full_shield = false;
	score = 0;
	flash_power = 0;

	shield_angle = M_PI;

	velocity = vector_t(0.0f,0.0f);
	if(!IS_SERVER) 
		init_gfx();
}

void Player::init_gfx(){
	char texture[64];

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
	angle = M_PI;
	client->send_spawn();
}

void Player::spawn_remote(vector_t new_pos) {
	pos = new_pos;
	dead = 0;
	power = 1.0;
	angle = M_PI;
}

void Player::render(double dt) {

	texture_colors[1]=1;
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	glTranslatef(pos.x, pos.y, 0);
	glPushMatrix();
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

	render_text(nick.c_str(), nick_font, vector_t(-(2*nick.length()*NICK_FONT_SIZE)/7,-PLAYER_H/1.4f));
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

void Player::logic(double dt, float last_angle) {
	if(dead == 0) {
		vector_t last = vector_t(pos);

		pos += velocity * dt;

		if(fire) {
			calc_fire();
		}

		power+=(power*0.3+1)*dt*PWR_REGEN_FACTOR;
		if(power > 1.0) 
			power = 1.0;

		if(!IS_SERVER && client->me->id != id)
			last_angle = angle;
		bool hit = true;
		for(int tries=0;tries < 3 && hit; ++tries) {
			hit = false;
			for(int i = 0; i < NUM_COLLISION_POINTS; ++i) {
				int mx, my;
				vector_t cp = collision_point(i);
				calc_map_index(cp, mx, my);
				if(map_value(mx, my)>0) {
					hit = true;
					hndl_collision(mx,my,i, cp, last_angle , dt);
				}
			}
		}
		if(hit) {
			printf("GAH! STILL IN COLLISION!\n");
			angle = last_angle;
			pos = last;
			velocity*=ELASTICITY;
		}
	}
}

void Player::hndl_collision(int mx, int my, int cp_index, const vector_t &cp, float last_angle, double dt) {
	vector_t block = vector_t(mx*64,my*64);

	vector_t repulse = (block - cp).normalized().abs();


	if(repulse.x-repulse.y > REPULSE_LIMIT) {
		velocity.x *= -1.0f;
	} else if(repulse.y - repulse.x > REPULSE_LIMIT) {
		velocity.y *= -1.0f;
	} else {
		velocity *= -1.0f;
	}

	vector_t old_cp = collision_point(cp_index, &last_angle);

//	vector_t rotation_impuls = old_cp - cp;
//	velocity+= (rotation_impuls/dt)*0.4;
	
	pos += velocity*dt;// + rotation_impuls;
	velocity *= ELASTICITY;

	angle = last_angle;
}

bool Player::use_power(float amount) {
	if(power >= amount) {
		power -= amount;
		return true;
	} else {
		if(flash_power <= 0) 
			flash_power = 1;
		return false;
	}
}

bool Player::calc_player_hit(Player * player) {
	if(player->id != id && player->dead == 0) {
		vector_t d;
		d.x = fire_end.x - player->pos.x;
		d.y = fire_end.y - player->pos.y;
		if(d.norm() < PLAYER_W/2.0)  {
			if(IS_SERVER) {
				player->dead = 1;
				if(player->team != team)
					add_score(KILL_SCORE);
				else
					add_score(TEAM_KILL_SCORE);
				printf("Killed player %s\n", player->nick.c_str());
				server->network_kill(this, player);
			}
			return true;
		}
	}
	return false;
}

void Player::calc_fire() {
	fire_end = pos;
	int len = 0;
	while(len < MAX_FIRE_LENGHT) {
		len += 32;
		vector_t prev(fire_end);
		fire_end.x+= 32*cos(angle);
		fire_end.y+= 32*sin(angle);

		
		bool hit=false;
		if(IS_SERVER) {
			for(std::map<Player*, int>::iterator it=server->players.begin(); it!=server->players.end(); ++it) {
				hit = calc_player_hit(it->first);
			}
		} else {
			for(std::map<int, Player*>::iterator it=client->players.begin(); it!=client->players.end(); ++it) {
				hit |= calc_player_hit(it->second);
			}
		}
		if(hit)
			break;

		int mx, my;
		calc_map_index(fire_end, mx, my);
		if(map_value(mx, my) > 0) {
			fire_end = prev;
			//map[my][mx]+=10;
			break;
		}
	}

}

void Player::add_score(int _score) {
	score+=_score;
	if(IS_SERVER)
		server->network_score(this);
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
