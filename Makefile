OBJS = main.o render.o network.o texture.o resource.o player.o logic.o
CFLAGS += -Wall `sdl-config --cflags`
LDFLAGS += `sdl-config --libs`  -lGL -lSDL_image

all: omgspaceponies

omgspaceponies: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o omgspaceponies 

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

