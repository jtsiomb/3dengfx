obj := 3dgeom.o controller.o base_cam.o curves.o color.o

opt := -O3 -mmmx -msse

CXXFLAGS := -g $(opt) -ansi -pedantic -Wall -fPIC -I. -I../common -I../3dengfx -I..

gfx_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
