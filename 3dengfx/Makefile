obj :=  3denginefx.o textures.o camera.o except.o material.o\
	object.o texman.o light.o load_geom.o\
	ggen.o 3dscene.o sceneloader.o gfxprog.o\
	3dwt/3dwt.o

opt := -O3 -mmmx -msse

CXXFLAGS := -g $(opt) -ansi -pedantic -Wall -fPIC\
			-I. -I../common -I../gfx -I../nlibase -I../dsys -I.. `sdl-config --cflags`

3dengfx_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$


.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f $(obj) *.d *.o
