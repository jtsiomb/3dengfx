obj :=

opt := -g
inc_flags := -I.

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC $(inc_flags) `sdl-config --cflags`
CFLAGS := $(opt) -std=c89 -pedantic -Wall -fPIC $(inc_flags) `sdl-config --cflags`

include 3dengfx/Makefile-part
include gfx/Makefile-part
include n3dmath2/Makefile-part
include dsys/Makefile-part
include common/Makefile-part
include nlibase/Makefile-part
include fxwt/Makefile-part

lib3dengfx.so.0.1.0: $(obj)
	$(CXX) -shared -Wl,-soname,lib3dengfx.so.0 -o $@ $(obj)

lib3dengfx.a: $(obj)
	$(AR) rcs $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	$(RM) $(obj)
