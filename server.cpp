#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "logic.h"
#include "level.h"
#include "player.h"
#include "server_network.h"


float step = 0.0f;
int verbose_flag = 0;
FILE* verbose = NULL;
int port = PORT;

std::vector<Player> players;

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

  /* verbose dst */
  if ( verbose_flag ){
	  verbose = stdout;
  } else {
	  verbose = fopen("/dev/null","w");
  }

	srand(time(NULL));

	init_level();

	bool run = true;

  struct timeval ref;
  gettimeofday(&ref, NULL);

  std::vector<Player>::iterator it;

  while ( run ){
    struct timeval ts;
	gettimeofday(&ts, NULL);

    /* calculate dt */
    double dt = (ts.tv_sec - ref.tv_sec) * 1000000.0;
    dt += ts.tv_usec - ref.tv_usec;
    dt /= 1000000;

    /* do stuff */
	 network();
	//Update other:
	for(it=players.begin(); it!=players.end();++it) {
		it->logic(dt);	
	}
		 
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
