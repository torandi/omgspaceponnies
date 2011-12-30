#ifndef RENDER_H
#define RENDER_H

#define PLAYER_W 84
#define PLAYER_H 132

#define NICK_FONT_SIZE 18.0f
#define ANNOUNCEMENT_FONT_SIZE 48.0f
#define CONSOLE_LOG_FONT_SIZE 12.0f

#include "texture.h"
#include <GL/gl.h>
#include <FTGL/ftgl.h>
#include <string>

extern char * msg;

extern float flash_power;
extern FTGLTextureFont * nick_font, *announcement_font, *console_font;

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
//Make sure a angle is betweedn 0 and 2PI
float period(float rad);

extern const float text_matrix[];

void queue_announcement(std::string announcement);
void log_message(std::string message);

extern GLfloat rbcolors[12][3];				// Rainbow Of Colors
extern float texture_colors[3];

struct window_t {
  float w;
  float h;
};

extern window_t window;

#endif /* render_H */
