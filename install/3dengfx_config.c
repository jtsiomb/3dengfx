#include <stdio.h>
#include <string.h>
#include "3dengfx_config.h"

#if GFX_LIBRARY == SDL
#define CFLAGS	"-I/usr/include/SDL -D_REENTRANT"
#define LIBS	"-lSDL -lpthread"
#elif GFX_LIBRARY == GTK
#define CFLAGS	"-DXTHREADS -I/usr/include/gtkglext-1.0 -I/usr/lib/gtkglext-1.0/include -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/X11R6/include -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/atk-1.0"
#define LIBS	"-Wl,--export-dynamic -L/usr/X11R6/lib -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 -lGLU -lGL -lstdc++ -lXmu -lXt -lSM -lICE -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0"
#endif	/* GFX_LIBRARY */

#ifndef IMGLIB_NO_PNG
#define LD_PNG	"-lpng"
#else
#define LD_PNG	""
#endif	/* IMGLIB_NO_PNG */

#ifndef IMGLIB_NO_JPEG
#define LD_JPEG	"-ljpeg"
#else
#define LD_JPEG	""
#endif	/* IMGLIB_NO_JPEG */

#ifdef USING_CG_TOOLKIT
#define LD_CG	"-lCg -lCgGL"
#else
#define LD_CG	""
#endif	/* USING_CG_TOOLKIT */

int main(int argc, char **argv) {
	int i;

	for(i=1; i<argc; i++) {
		if(!strcmp(argv[i], "--prefix")) {
			printf("%s\n", PREFIX);
			continue;
		}
		
		if(!strcmp(argv[i], "--cflags")) {
			printf("-I%s/include/3dengfx %s\n", PREFIX, CFLAGS);
			continue;
		}

		if(!strcmp(argv[i], "--libs")) {
			printf("-l3dengfx -lGL -lGLU %s %s %s %s\n", LD_JPEG, LD_PNG, LD_CG, LIBS);
			continue;
		}
		
		if(!strcmp(argv[i], "--libs-no-3dengfx")) {
			printf("-lGL -lGLU %s %s %s %s\n", LD_JPEG, LD_PNG, LD_CG, LIBS);
			continue;
		}

		if(!strcmp(argv[i], "--version")) {
			printf("%s\n", VER_STR);
			continue;
		}

		fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
		return -1;
	}

	return 0;
}
