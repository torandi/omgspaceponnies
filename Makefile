OBJS = main.o render.o network.o texture.o resource.o player.o logic.o render_object.o level.o
SPRITES = dispencer.png tail.png
CFLAGS += -Wall `sdl-config --cflags` -g
LDFLAGS += `sdl-config --libs`  -lGL -lSDL_image

all: omgspaceponies

omgspaceponies: $(OBJS) $(SPRITES)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o omgspaceponies 

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

dispencer.png: $(wildcard gfx_parts/dispencer_*.png)
	./makesprite gfx/dispencer.png
	

tail.png: $(wildcard gfx_parts/tail_*.png)
	./makesprite gfx/tail.png
