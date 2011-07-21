OBJS = render.o network.o texture.o resource.o player.o logic.o render_object.o level.o sha1.o network_lib.o socket.o
SPRITES = dispencer.png tail.png
CFLAGS += -Wall `sdl-config --cflags` -g
LDFLAGS += `sdl-config --libs`  -lGL -lSDL_image

all: omgspaceponies omgserver
 
omgspaceponies: client.o $(OBJS) $(SPRITES)
	$(CXX) client.o $(OBJS) $(LDFLAGS) -o $@

omgserver: server.o $(OBJS)
	$(CXX) server.o $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o *.d omgspaceponies omgserver

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

dispencer.png: $(wildcard gfx_parts/dispencer_*.png)
	./makesprite gfx/dispencer.png
	

tail.png: $(wildcard gfx_parts/tail_*.png)
	./makesprite gfx/tail.png

test_network: sha1.o network_lib.o network_test.o
	$(CXX) network_lib.o sha1.o network_test.o $(LDFLAGS) -o $@
	
