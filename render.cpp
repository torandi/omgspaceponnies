#include "render.h"
#include "common.h"
#include "player.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "texture.h"
#include <math.h>
#include "render_object.h"
#include "level.h"

char * msg;
float flash_power = 0;

#define DEBUG

GLfloat rbcolors[12][3]=				// Rainbow Of Colors
{
{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

static void glCircle3i(GLint x, GLint y, GLint radius);

static int next_wall_color = 0;

window_t window;

static RenderObject splash;
static RenderObject box;
static RenderObject backdrop;

float texture_colors[3] = {1,1,1};

animation_t load_anim(const char* filename, unsigned int frames, unsigned int fps){
  animation_t tmp;
  tmp.texture = new Texture(filename, frames);
  tmp.frames = frames;
  tmp.current = 0;
  tmp.delay = (float)frames / fps;
  tmp.s = 0.0f;
  tmp.acc = 0.0f;
  return tmp;
}

Texture::texcoord_t prepare_animation(animation_t * anim, double dt) {
	anim->texture->bind();
	const unsigned int index = (int)(anim->s * anim->frames);
	Texture::texcoord_t tc = anim->texture->index_to_texcoord(index);
	anim->acc = fmod(anim->acc + dt, anim->delay);
	anim->s = anim->acc / anim->delay;
	return tc;
}


void render_init(int w, int h, bool fullscreen){
  /* create window */
  SDL_Init(SDL_INIT_VIDEO);
  int flags = SDL_OPENGL | SDL_DOUBLEBUF;
  if ( fullscreen ) flags |= SDL_FULLSCREEN;
  SDL_SetVideoMode(w, h, 0, flags);
  SDL_WM_SetCaption("OMGSPACEPONIES!","OMGSPACEPONIES!");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();


  /* orthographic projection */
  glOrtho(0, w, 0, h, -1.0, 1.0);
  glScalef(1, -1, 1);
  glTranslated(0, -h, 0);

  window.w = w;
  window.h = h;

  /* setup opengl */
  glClearColor(1,1,0,0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  glShadeModel(GL_FLAT);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* load textures */
  splash = RenderObject("gfx/splash.png", 1, 0,vector_t(1024,768));
  backdrop = RenderObject("gfx/space.jpg", 1, 0,vector_t(1600,1200));
  box = RenderObject("gfx/box.png", 1, 0,vector_t(64,64));
}

void render_splash() {
	glClear(GL_COLOR_BUFFER_BIT);
	splash.render(0);
	SDL_GL_SwapBuffers();
}

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);

	


	glPushMatrix();

	glTranslatef(-window.w*0.28+(window.w/2.0-me->pos.x)*0.2,-window.h*0.28+ (window.h/2.0-me->pos.y)*0.2, 0);
	
	texture_colors[1] = 0.3;

	backdrop.render(dt);
	texture_colors[1] = 0.5;

	glPopMatrix();

	glPushMatrix();

	//Center on player
	glTranslatef(window.w/2.0-me->pos.x, window.h/2.0-me->pos.y,0);

	for(int i=0; i < NUM_PLAYERS; ++i) {
		if(players[i] != NULL && players[i]->dead == 0) {
			players[i]->render(dt);
			#ifdef DEBUG
				glPointSize(2.0f);
				glColor3f(1,1,1);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_POINTS);
					for(int c = 0; c<NUM_COLLISION_POINTS; ++c) {
						vector_t v = players[i]->collision_point(c);
						glVertex2f(v.x, v.y);
					}
				glEnd();
				glEnable(GL_TEXTURE_2D);
			#endif
		}
	}

	//Draw walls

	for(int x=0; x<MAP_WIDTH; ++x) {
		for(int y=0; y<MAP_HEIGHT; ++y) {
			if(map_value(x, y) > 0) {
				glPushMatrix();
				#ifdef DEBUG
					if(map_value(x,y) == 2) {
						texture_colors[0]=0;
						texture_colors[1]=1;
						texture_colors[2]=0;
						map[y][x] = 1;
					} else {
						texture_colors[0]=1;
						texture_colors[1]=0.5;
						texture_colors[2]=1;
					}
				#endif
				glTranslatef(x*64-32,y*64-32,0);
				box.render(dt);
				glPopMatrix();
			}	
		}
	}

	glPopMatrix();

	/**********
	 * HUD
	 *********/

	//Power bar

	glPushMatrix();
	glTranslatef(window.w-210, window.h-40,0);

	glDisable(GL_TEXTURE_2D);
	glColor3f(1,1,1);
	glLineWidth(2.0f);

	float pw = 200.0f;
	float ph = 30.0f;

	glBegin(GL_LINES);
		glVertex2f(0.0,0.0); glVertex2f(pw,0.0);
		glVertex2f(0.0,0.0); glVertex2f(0.0,ph);
		glVertex2f(pw,0.0); glVertex2f(pw,ph);
		glVertex2f(0.0,ph); glVertex2f(pw,ph);
	glEnd();

	if(flash_power <= 0) {
		glColor4f(1,1,1,0.1);
	} else {
		flash_power -= dt*20.0; 
		glColor4f(1,1,1,0.8);
	}

	glBegin(GL_TRIANGLES);
		glVertex2f(0.0,0.0); glVertex2f(pw,0.0); glVertex2f(0.0,ph);
		glVertex2f(pw,0.0); glVertex2f(0.0,ph); glVertex2f(pw,ph);
	glEnd();


	float ll = pw*me->power; //Line length
	float lw = ph/12.0; //Line width
	
	glLineWidth(lw);

	glBegin(GL_LINES);
		for(int i=0;i<12; ++i) {
			glColor4f(rbcolors[i][0],rbcolors[i][1],rbcolors[i][2],0.7);
			glVertex2f(0.0,lw*(i+0.5)); glVertex2f(ll,lw*(i+0.5));
		}
	glEnd();

	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
	

	SDL_GL_SwapBuffers();
}

float radians_to_degrees(double rad) {
	return (float) (rad * (180/PI));
}

static void glCircle3i(GLint x, GLint y, GLint radius) { 
	float angle; 
	glBegin(GL_LINE_LOOP); 
		for(int i = 0; i < 100; i++) { 
			angle = i*2*PI/100.0f; 
			glVertex3f(x + (cos(angle) * radius), y + (sin(angle) * radius),0.0f); 
		} 
	glEnd(); 
} 
