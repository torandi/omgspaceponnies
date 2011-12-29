CLIENT_OBJS = client.o client_main.o render.o texture.o resource.o player.o logic.o render_object.o level.o sha1.o network_lib.o socket.o protocol.o server.o
SERVER_OBJS = client.o server_main.o render.o texture.o resource.o player.o logic.o render_object.o level.o sha1.o network_lib.o socket.o protocol.o server.o
SPRITES = dispencer.png tail.png
CFLAGS += -Wall `sdl-config --cflags` -g 
LDFLAGS += `sdl-config --libs`  -lGL -lSDL_image

all: omgspaceponies omgserver
 
omgspaceponies:  $(CLIENT_OBJS) $(SPRITES)
	$(CXX) $(CLIENT_OBJS) $(LDFLAGS) -o $@

omgserver: $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) $(LDFLAGS) -o $@

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

test_network: sha1.o network_lib.o network_test.o protocol.o socket.o
	$(CXX) network_lib.o sha1.o protocol.o socket.o network_test.o $(LDFLAGS) -o $@
	
