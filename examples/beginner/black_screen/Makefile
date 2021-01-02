obj := main.o

3dengfx_path := ../../..

CXXFLAGS := -O3 -ansi -pedantic -Wall -I$(3dengfx_path)/src `../../../3dengfx-config --cflags`

black_screen: $(obj) $(3dengfx_path)/lib3dengfx.a
	$(CXX) -o $@ $(obj) $(3dengfx_path)/lib3dengfx.a `../../../3dengfx-config --libs-no-3dengfx`

.PHONY: clean
clean:
	$(RM) -f *.o *.d black_screen
