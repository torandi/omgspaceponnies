#include "render.h"
#include "common.h"
#include "player.h"
#include "texture.h"
#include "render_object.h"
#include "level.h"
#include "client.h"

#include <GL/gl.h>
#include <FTGL/ftgl.h>
#include <SDL/SDL.h>
#include <string>
#include <queue>
#include <math.h>

float flash_power = 0;

#define DEBUG

GLfloat rbcolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

static void glCircle3i(GLint x, GLint y, GLint radius);
static void render_walls(double dt);

FTGLTextureFont * nick_font, *announcement_font, *console_font, *misc_font;
const float text_matrix[] = 	{ 1.0f,  0.0f, 0.0f, 0.0f,
										0.0f, -1.0f, 0.0f, 0.0f,
										0.0f,  0.0f, 1.0f, 0.0f,
										0.0f,  0.0f, 0.0f, 1.0f };


static int next_wall_color = 0;

std::string msg;

window_t window;

#define ANNOUNCEMENT_ANIM_TIME 1.5f
#define ANNOUNCMENT_TIME 1.0f

#define NUM_LOG_MESSAGES 10

static std::queue<std::string> announcements;
static std::string messages[NUM_LOG_MESSAGES];
static std::string * log_pos = messages;

static std::string current_announcement = "";
static float announcement_state = -1.0f;

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

	msg="Loading...";

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

  /* load fonts */
	nick_font = new FTTextureFont("fonts/nick.ttf");
	nick_font->FaceSize(NICK_FONT_SIZE);
	announcement_font = new FTTextureFont("fonts/announcement.ttf");
	announcement_font->FaceSize(ANNOUNCEMENT_FONT_SIZE);
	console_font= new FTTextureFont("fonts/console.ttf");
	console_font->FaceSize(CONSOLE_LOG_FONT_SIZE);
	misc_font = new FTTextureFont("fonts/misc.ttf");
	misc_font->FaceSize(12.0f);
}

void render_splash() {
	glClear(GL_COLOR_BUFFER_BIT);
	splash.render(0);
	
	misc_font->FaceSize(25.0f);
	render_text(msg.c_str(), misc_font, vector_t(2.0f,window.h-2.0f));
	
	SDL_GL_SwapBuffers();
}

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(-window.w*0.28+(window.w/2.0-client->me->pos.x)*0.2,-window.h*0.28+ (window.h/2.0-client->me->pos.y)*0.2, 0);
	
	texture_colors[1] = 0.3;
	backdrop.render(dt);
	texture_colors[1] = 0.5;

	glPopMatrix();

	glPushMatrix();


	//Center on player
	glTranslatef(window.w/2.0-client->me->pos.x, window.h/2.0-client->me->pos.y,0);

	//Render player fire
	std::map<int, Player*>::iterator it;
	for(it=client->players.begin(); it!=client->players.end(); ++it) {
		if(it->second->dead == 0) {
			it->second->render_fire(dt);
		}
	}

	//Render walls
	render_walls(dt);

	//Render players
	for(it=client->players.begin(); it!=client->players.end(); ++it) {
		if(it->second->dead == 0) {
			it->second->render(dt);
			#ifdef DEBUG
				glPointSize(2.0f);
				glColor3f(1,1,1);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_POINTS);
					for(int c = 0; c<NUM_COLLISION_POINTS; ++c) {
						vector_t v = it->second->collision_point(c);
						glVertex2f(v.x, v.y);
					}
				glEnd();
				glEnable(GL_TEXTURE_2D);
			#endif
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


	float ll = pw*client->me->power; //Line length
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

	/* Render announcement */
	if(announcement_state <= 0 && !announcements.empty()) {
		current_announcement = announcements.front();
		printf("Show announcement: %s\n", current_announcement.c_str());
		announcements.pop();
		announcement_state = 0.001f;
	}

	if(announcement_state > (2*ANNOUNCEMENT_ANIM_TIME+ANNOUNCMENT_TIME)) {
		announcement_state = -1.0f;
	} else if(announcement_state > 0) {
		glPushMatrix();
		glTranslatef((window.w/2.0f)-(ANNOUNCEMENT_FONT_SIZE*current_announcement.length()/4.0f), window.h/2.0f,0.0f);
		glMultMatrixf(text_matrix);
		if(announcement_state > (ANNOUNCEMENT_ANIM_TIME+ANNOUNCMENT_TIME)) { 
			//Fade out announcement
			glColor4f(1,1,1,1.0f-0.9f*(announcement_state - (ANNOUNCEMENT_ANIM_TIME+ANNOUNCMENT_TIME))/ANNOUNCEMENT_ANIM_TIME);
		} else if(announcement_state <= (ANNOUNCEMENT_ANIM_TIME)) {
			//Fade in
			glColor4f(1,1,1,0.1f+0.9f*announcement_state/ANNOUNCEMENT_ANIM_TIME);
		}
		announcement_font->Render(current_announcement.c_str());
		glPopMatrix();
		glColor4f(1,1,1,1);
		announcement_state+=dt;
	}


	SDL_GL_SwapBuffers();
}

void render_text(const char * str, FTTextureFont * font, vector_t pos) {
	glPushMatrix();
		glTranslatef(pos.x,pos.y,0.0f);
		glMultMatrixf(text_matrix);
		font->Render(str);
	glPopMatrix();
}

float radians_to_degrees(double rad) {
	return (float) (rad * (180/M_PI));
}

static void render_walls(double dt) {
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
}

float period(float rad) {
	if(rad > 0) 
		return fmod(rad, 2.0*M_PI);
	else {
		do {
			rad+=2.0*M_PI;
		} while(rad < 0);
		return rad;
	}
}

void queue_announcement(std::string announcement) {
	printf("[ANNOUNCEMENT] %s\n", announcement.c_str());
	announcements.push(announcement);
}

void log_message(std::string message) {
	printf("[LOG] %s\n", message.c_str());
	*(log_pos++) = message;
	if(log_pos - messages >= NUM_LOG_MESSAGES)
		log_pos = messages;
}

static void glCircle3i(GLint x, GLint y, GLint radius) { 
	float angle; 
	glBegin(GL_LINE_LOOP); 
		for(int i = 0; i < 100; i++) { 
			angle = i*2*M_PI/100.0f; 
			glVertex3f(x + (cos(angle) * radius), y + (sin(angle) * radius),0.0f); 
		} 
	glEnd(); 
} 
