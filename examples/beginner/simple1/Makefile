obj := simple1.o
englib := ../../../lib3dengfx.a
bin := simple1

CXXFLAGS := -ansi -pedantic -Wall -I../../../src `../../../3dengfx-config --cflags`

$(bin): $(obj) $(englib)
	$(CXX) -o $@ $(obj) $(englib) `../../../3dengfx-config --libs-no-3dengfx`

.PHONY: clean
clean:
	$(RM) $(obj) $(bin)
