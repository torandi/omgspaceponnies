#include "render.h"
#include "common.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "texture.h"
#include <math.h>

#define ANIM_MAX 0

#define PLAYER_W 32
#define PLAYER_H 50

static struct {
  float w;
  float h;
} window;

static struct animation_t {
  Texture* texture;
  unsigned int frames;
  unsigned int current;
  float delay;
  float s;
  float acc;
} animation[ANIM_MAX];

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
  glClearColor(0,0,0,0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glShadeModel(GL_FLAT);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* load textures */
 // animation[ANIM_WAIVING] = load_anim("data/waving.png", 17, 25);
}

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);

	bool render = true;
	animation_t* anim = NULL;


	glPushMatrix();
	
	glTranslatef(me.pos.x-PLAYER_W/2.0, me.pos.y-PLAYER_H/2.0f, 0);

	glDisable(GL_TEXTURE_2D);
	glColor3f(1,1,1);

	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(PLAYER_W,0);
		glVertex2f(PLAYER_W,PLAYER_H);
		glVertex2f(0,PLAYER_H);
	glEnd();

	glPopMatrix();

	glColor3f(1,0,0);
	glPointSize(5);
	glBegin(GL_POINTS);
		glVertex2f(me.pos.x,me.pos.y);
	glEnd();

	glColor3f(1,0,1);
	glBegin(GL_LINES);
		glVertex2f(me.pos.x, me.pos.y);
		glVertex2f(mouse.x, mouse.y);
	glEnd();

	SDL_GL_SwapBuffers();

  /*
    anim->texture->bind();
    const unsigned int index = (int)(anim->s * anim->frames);
    Texture::texcoord_t tc = anim->texture->index_to_texcoord(index);
	*/
}
