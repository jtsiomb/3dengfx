obj := scene_view.o events.o

3dengfx_path := ../..

CXXFLAGS := -g -ansi -pedantic -Wall -I$(3dengfx_path)/src `../../3dengfx-config --cflags`

scene_view: $(obj) $(3dengfx_path)/lib3dengfx.a
	$(CXX) -o $@ $(obj) $(3dengfx_path)/lib3dengfx.a `../../3dengfx-config --libs-no-3dengfx`

.PHONY: clean
clean:
	$(RM) -f *.o *.d scene_view
