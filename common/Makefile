obj := image.o logger.o config_parser.o timer.o fps_counter.o err_msg.o

include ../Makefile-options

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC
CFLAGS := $(opt) -ansi -pedantic -Wall -fPIC

common_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

color2.o: color2.cpp color2.hpp
curves.o: curves.cpp curves.hpp

image.o: image.c image.h
logger.o: logger.c logger.h
config_parser.o: config_parser.c config_parser.h
timer.o: timer.c timer.h
fps_counter.o: fps_counter.c fps_counter.h
err_msg.o: err_msg.c err_msg.h


.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o
