obj := nlibase.o

include ../Makefile-options

CFLAGS := $(opt) -std=c89 -pedantic -Wall -fPIC

#nlibase.a: $(obj)
#	ar crus $@ $(obj)

nlibase_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
