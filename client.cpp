#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <getopt.h>
#include <vector>

#include "common.h"
#include "network.h"
#include "client.h"
#include "render.h"
#include "logic.h"
#include "level.h"

bool keys[SDLK_LAST];

float step = 0.0f;
int verbose_flag = 0;
FILE* verbose = NULL;
bool fullscreen = false;

std::vector<Player> players;

Player * me = NULL; 

bool ready = false;

vector_t mouse;

char * myname;

static void setup(){
	render_init(1024, 768, fullscreen);
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
					if(event.button.button == SDL_BUTTON_LEFT)
						me->fire = true;
					else if(event.button.button == SDL_BUTTON_WHEELUP)
						me->shield_angle+=0.3;
					else if(event.button.button == SDL_BUTTON_WHEELDOWN)
						me->shield_angle-=0.3;
					else if(event.button.button == SDL_BUTTON_RIGHT)
						me->full_shield = true;
					break;

				case SDL_MOUSEBUTTONUP:
					if(event.button.button == SDL_BUTTON_LEFT)
						me->fire = false;
					else if(event.button.button == SDL_BUTTON_RIGHT)
						me->full_shield = false;
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
  fprintf(stderr, "./omgponnies [options] nick team(1-4)\n");
  fprintf(stderr, "  -p, --port=PORT Use PORT for communication (default: %d).\n", PORT);
  fprintf(stderr, "  -h, --help      This help text.\n");
}

int main(int argc, char* argv[]){
	int verbose_flag;
  static struct option long_options[] =
  {
		{"port",    required_argument, 0, 'p' },
		{"help",    no_argument,       0, 'h'},
		{"fullscreen",    no_argument,       0, 'f'},
		{"verbose",    no_argument,      &verbose_flag, 'v'},
		{0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while( (c=getopt_long(argc, argv, "p:hfv", long_options, &option_index)) != -1 ) {
	switch(c) {
		case 0:
			break;
		case 'p':
			network_port = atoi(optarg);
			printf("Set port to %i\n", network_port);
			break;
		case 'f':
			fullscreen= true;
			break;
		case 'h':
			show_usage();
			exit(0);
		default:
			break;
	}
  }

  if ( argc - optind != 2 ){
	show_usage();
    exit(1);
  }

	players.push_back(Player(argv[option_index++], atoi(argv[option_index++])+1));
	me = &players[0];


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
