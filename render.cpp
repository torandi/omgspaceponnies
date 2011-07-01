#include "render.h"
#include "common.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "texture.h"
#include <math.h>

#define ANIM_MAX 0

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
} animation[ANIM_MAX] = {{0}};

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
	SDL_WM_SetCaption("OMGSPACEPONIES!");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();


  /* orthographic projection */
  glOrtho(0, w, 0, h, -1.0, 1.0);
  glScalef(1, -1, 1);
  glTranslated(0, -h, 0);

  center.x = (float)w / 2;
  center.y = (float)h / 2;
  window.w = w;
  window.h = h;

  /* setup opengl */
  glClearColor(1,1,1,1);
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

  SDL_GL_SwapBuffers();

  /*
    anim->texture->bind();
    const unsigned int index = (int)(anim->s * anim->frames);
    Texture::texcoord_t tc = anim->texture->index_to_texcoord(index);
	*/
}
