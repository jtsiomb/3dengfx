obj := dsys.o fx.o part.o script.o

opt := -O3 -mmmx -msse

CXXFLAGS := -g $(opt) -ansi -pedantic -Wall -fPIC\
			-I../common -I../3dengfx -I../gfx -I..

CFLAGS := $(CXXFLAGS)

dsys_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
