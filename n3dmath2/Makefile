obj := n3dmath2.o n3dmath2_mat.o n3dmath2_vec.o n3dmath2_qua.o n3dmath2_ray.o n3dmath2_qdr.o

include ../Makefile-options

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC -I..

n3dmath2_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$


.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f $(obj) *.d *.o
