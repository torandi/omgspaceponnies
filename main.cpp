#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <getopt.h>
#include <vector>

#include "common.h"
#include "network.h"
#include "render.h"
#include "logic.h"
#include "level.h"

bool keys[SDLK_LAST];

#define REF_FPS 30
#define REF_DT (1.0/REF_FPS)

float step = 0.0f;
int verbose_flag = 0;
FILE* verbose = NULL;
int port = PORT;

Player * players[4] = {NULL, NULL, NULL, NULL};

Player * me = NULL; 

bool ready = false;

vector_t mouse;

char * myname;

static void setup(){
	render_init(1024, 768, false);
	init_level();
	init_network();
}

static void cleanup(){
  SDL_Quit();
}

static void poll(bool* run){

	SDL_Event event;
	while ( SDL_PollEvent(&event) ){
		if(ready) {
			switch (event.type){
				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == 1)
						me->fire = true;
					break;
				case SDL_MOUSEBUTTONUP:
					if(event.button.button == 1)
						me->fire = false;
					break;

				case SDL_MOUSEMOTION:
					mouse.x = event.motion.x;
					mouse.y = event.motion.y;
					break;

				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE:
							*run = false;
							break;
						default:
							keys[event.key.keysym.sym] = true;
					}
					break;
				
				case SDL_KEYUP:
					keys[event.key.keysym.sym] = false;
					break;
					

				case SDL_QUIT:
					*run = false;
					break;
			}
		} else {
			switch (event.type){
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) {
						*run = false;
						return;
					}
					break;
				
				case SDL_QUIT:
					*run = false;
					break;
			}
		}
	}
}


static void show_usage(){
  fprintf(stderr, "./omgponnies [options] nick\n");
  fprintf(stderr, "  -p, --port=PORT Use PORT for communication (default: %d).\n", PORT);
  fprintf(stderr, "  -h, --help      This help text.\n");
}

int main(int argc, char* argv[]){
	int verbose_flag;
  static struct option long_options[] =
  {
		{"port",    required_argument, 0, 'p' },
		{"help",    no_argument,       0, 'h'},
		{"verbose",    no_argument,      &verbose_flag, 'v'},
		{0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while( (c=getopt_long(argc, argv, "p:hv", long_options, &option_index)) != -1 ) {
	switch(c) {
		case 0:
			break;
		case 'p':
			port = atoi(optarg);
			printf("Set port to %i\n", port);
			break;
		case 'h':
			show_usage();
			exit(0);
		default:
			break;
	}
  }

	myname = "PLAJUR";

  /* verbose dst */
  if ( verbose_flag ){
	  verbose = stdout;
  } else {
	  verbose = fopen("/dev/null","w");
  }

	srand(time(NULL));

	setup();

  bool run = true;

	while(!ready && run) {
		render_splash();
		usleep(1000);
		poll(&run);
		network();
	}
  
  struct timeval ref;
  gettimeofday(&ref, NULL);

  while ( run ){
    struct timeval ts;
	gettimeofday(&ts, NULL);

    /* calculate dt */
    double dt = (ts.tv_sec - ref.tv_sec) * 1000000.0;
    dt += ts.tv_usec - ref.tv_usec;
    dt /= 1000000;

    /* do stuff */
    poll(&run);
	 network();
	 logic(dt);
	 render(dt);
		 
    /* framelimiter */
    const int delay = (REF_DT - dt) * 1000000;
    if ( delay > 0 ){
      usleep(delay);
    }

    /* store reference time */
    ref = ts;
  }

  cleanup();
}
