obj := scene_view.o events.o

3dengfx_path := ../..

CXXFLAGS := -g -ansi -pedantic -Wall -I$(3dengfx_path)/src `sdl-config --cflags`
LIBS := -lGL -lGLU -lpng -ljpeg -l3ds
#LIBS += -lCg -lCgGL

scene_view: $(obj) $(3dengfx_path)/lib3dengfx.a
	$(CXX) -o $@ $(obj) $(3dengfx_path)/lib3dengfx.a `sdl-config --libs` $(LIBS)

.PHONY: clean
clean:
	$(RM) -f *.o *.d scene_view
