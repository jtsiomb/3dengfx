obj :=

opt := -g
inc_flags := -Isrc

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC $(inc_flags) `sdl-config --cflags`
CFLAGS := $(opt) -std=c89 -pedantic -Wall -fPIC $(inc_flags) `sdl-config --cflags`

include src/3dengfx/Makefile-part
include src/gfx/Makefile-part
include src/n3dmath2/Makefile-part
include src/dsys/Makefile-part
include src/common/Makefile-part
include src/nlibase/Makefile-part
include src/fxwt/Makefile-part

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
