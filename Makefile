obj :=

opt := -g
inc_flags := -Isrc

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC $(inc_flags) `./3dengfx-config --cflags`
CFLAGS := $(opt) -std=c89 -pedantic -Wall -fPIC $(inc_flags) `./3dengfx-config --cflags`

include src/3dengfx/Makefile-part
include src/gfx/Makefile-part
include src/n3dmath2/Makefile-part
include src/dsys/Makefile-part
include src/common/Makefile-part
include src/nlibase/Makefile-part
include src/fxwt/Makefile-part
include src/sim/Makefile-part

lib3dengfx.so.0.1.0: 3dengfx-config $(obj)
	$(CXX) -shared -Wl,-soname,lib3dengfx.so.0 -o $@ $(obj)

lib3dengfx.a: 3dengfx-config $(obj)
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

.PHONY: cleandep
cleandep:
	$(RM) src/3dengfx/*.d src/3dengfx/*.d.*
	$(RM) src/gfx/*.d src/gfx/*.d.*
	$(RM) src/n3dmath2/*.d src/n3dmath2/*.d.*
	$(RM) src/dsys/*.d src/dsys/*.d.*
	$(RM) src/common/*.d src/common/*.d.*
	$(RM) src/nlibase/*.d src/nlibase/*.d.*
	$(RM) src/fxwt/*.d src/fxwt/*.d.*
	$(RM) src/sim/*.d src/sim/*.d.*
