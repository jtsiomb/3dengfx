libnames := common 3dengfx gfx nlibase dsys n3dmath2
obj := $(foreach lib,$(libnames),$(lib)/$(lib)_pack.o)

lib3dengfx.so.0.1.0: $(obj)
	$(CXX) -shared -Wl,-soname,lib3dengfx.so.0 -o $@ $(obj)

%.o:
	cd $(@D); make

.PHONY: install
install:
	rm -rf /usr/local/include/3dengfx
	rm -f /usr/local/lib/lib3dengfx*
	mkdir /usr/local/include/3dengfx
	mkdir /usr/local/include/3dengfx/3dengfx
	mkdir /usr/local/include/3dengfx/3dengfx/3dwt
	mkdir /usr/local/include/3dengfx/gfx
	mkdir /usr/local/include/3dengfx/common
	mkdir /usr/local/include/3dengfx/dsys
	mkdir /usr/local/include/3dengfx/nlibase
	mkdir /usr/local/include/3dengfx/n3dmath2
	cp 3dengfx/*.hpp 3dengfx/*.h /usr/local/include/3dengfx/3dengfx/
	cp 3dengfx/3dwt/*.hpp /usr/local/include/3dengfx/3dengfx/3dwt/
	cp gfx/*.hpp gfx/*.inl /usr/local/include/3dengfx/gfx/
	cp common/*.hpp common/*.h common/*.inl /usr/local/include/3dengfx/common/
	cp dsys/*.hpp dsys/*.h /usr/local/include/3dengfx/dsys/
	cp nlibase/*.h /usr/local/include/3dengfx/nlibase/
	cp n3dmath2/*.hpp n3dmath2/*.inl /usr/local/include/3dengfx/n3dmath2/
	cp config.h /usr/local/include/3dengfx/
	cp lib3dengfx.so.0.1.0 /usr/local/lib
	cd /usr/local/lib; ln -s lib3dengfx.so.0.1.0 lib3dengfx.so
	chmod +x 3dengfx-config
	cp 3dengfx-config /usr/local/bin
	ldconfig
	

.PHONY: clean
clean:
	@echo Cleaning everything...
	cd 3dengfx; make clean
	cd gfx; make clean
	cd dsys; make clean
	cd n3dmath2; make clean
	cd nlibase; make clean
	cd common; make clean
