OBJS = main.o render.o network.o
CFLAGS += -Wall `sdl-config --cflags`
LDFLAGS += `sdl-config --libs`  -lGL

all: omgspaceponies

wandercat: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

	clean:
	rm -rf *.o omgspaceponies 

	%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

