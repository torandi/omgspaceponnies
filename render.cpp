#include "render.h"
#include "common.h"
#include "player.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "texture.h"
#include <math.h>

#define ANIM_MAX 0

GLfloat rbcolors[12][3]=				// Rainbow Of Colors
{
{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

static void glCircle3i(GLint x, GLint y, GLint radius);

static struct {
  float w;
  float h;
} window;


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
    /*vertices[ 0] = tc.a[0];
    vertices[ 1] = 1-tc.a[1];
    vertices[ 5] = tc.b[0];
    vertices[ 6] = 1-tc.b[1];
    vertices[10] = tc.c[0];
    vertices[11] = 1-tc.c[1];
    vertices[15] = tc.d[0];
    vertices[16] = 1-tc.d[1];*/
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
 // animation[ANIM_WAIVING] = load_anim("data/waving.png", 17, 25);
}

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);

	bool render = true;

	for(int i=0; i < 4; ++i) {
		if(players[i] != NULL) {
			players[i]->render(dt);
		}
	}

	SDL_GL_SwapBuffers();

  /*
	*/
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
