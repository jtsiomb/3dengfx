#include <stdio.h>
#include <string.h>
#include "3dengfx_config.h"

#if GFX_LIBRARY == SDL
#define CFLAGS_CMD	"sdl-config --cflags"
#define LIBS_CMD	"sdl-config --libs"
#define CFLAGS		"-I/usr/include/SDL -D_REENTRANT"
#define LIBS		"-lSDL -lpthread"
#elif GFX_LIBRARY == GTK
#define CFLAGS_CMD	"pkg-config --cflags gtk+-2.0 gtkglext-1.0"
#define LIBS_CMD	"pkg-config --libs gtk+-2.0 gtkglext-1.0"
#define CFLAGS	"-DXTHREADS -I/usr/include/gtkglext-1.0 -I/usr/lib/gtkglext-1.0/include -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/X11R6/include -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/atk-1.0"
#define LIBS	"-Wl,--export-dynamic -L/usr/X11R6/lib -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 -lstdc++ -lXmu -lXt -lSM -lICE -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0"
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

void print_cflags(void);
void print_libs(void);
void print_libs_no_3dengfx(void);

int main(int argc, char **argv) {
	int i;

	for(i=1; i<argc; i++) {
		if(!strcmp(argv[i], "--prefix")) {
			printf("%s", PREFIX);
			continue;
		}
		
		if(!strcmp(argv[i], "--cflags")) {
			print_cflags();
			continue;
		}

		if(!strcmp(argv[i], "--libs")) {
			print_libs();
			continue;
		}
		
		if(!strcmp(argv[i], "--libs-no-3dengfx")) {
			print_libs_no_3dengfx();
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


void print_cflags(void) {
	FILE *p;
	int c;

	printf("-I%s/include/3dengfx ", PREFIX);
	
	if((p = popen(CFLAGS_CMD, "r"))) {
		while((c = fgetc(p)) != -1) putchar(c);
		putchar(' ');
		pclose(p);
	} else {
		printf("%s ", LIBS);
	}
}

void print_libs(void) {
	printf("-l3dengfx ");
	print_libs_no_3dengfx();
}

void print_libs_no_3dengfx(void) {
	FILE *p;
	int c;
		
	printf("-lGL -lGLU %s %s %s ", LD_JPEG, LD_PNG, LD_CG);

	if((p = popen(LIBS_CMD, "r"))) {
		while((c = fgetc(p)) != -1) putchar(c);
		putchar(' ');
		pclose(p);
	} else {
		printf("%s ", LIBS);
	}
}
