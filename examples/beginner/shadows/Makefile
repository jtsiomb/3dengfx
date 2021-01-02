obj := main.o
bin := shadows

3dengfx_path := ../../..

CXXFLAGS := -g -ansi -pedantic -Wall -I$(3dengfx_path)/src `$(3dengfx_path)/3dengfx-config --cflags`

$(bin): $(obj) $(3dengfx_path)/lib3dengfx.a
	$(CXX) -o $@ $(obj) $(3dengfx_path)/lib3dengfx.a `$(3dengfx_path)/3dengfx-config --libs-no-3dengfx`

.PHONY: clean
clean:
	$(RM) $(bin) $(obj)
