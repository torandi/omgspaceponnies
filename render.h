#ifndef RENDER_H
#define RENDER_H

#define PLAYER_W 84
#define PLAYER_H 132

#include "texture.h"
#include <GL/gl.h>

extern char * msg;

void render_init(int w, int h, bool fullscreen);
void render(double dt);
void render_splash();

struct animation_t {
  Texture* texture;
  unsigned int frames;
  unsigned int current;
  float delay;
  float s;
  float acc;
};

animation_t load_anim(const char* filename, unsigned int frames, unsigned int fps);
Texture::texcoord_t prepare_animation(animation_t * anim, double dt);
float radians_to_degrees(double rad);

extern GLfloat rbcolors[12][3];				// Rainbow Of Colors
extern float texture_colors[3];

#endif /* render_H */
