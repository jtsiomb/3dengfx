#include <stdio.h>
#include <string.h>
#include "3dengfx_config.h"

#define CFLAGS	"pkg-config --cflags freetype2"
#define LIBS	"pkg-config --libs freetype2"

#if GFX_LIBRARY == SDL
#define GFX_CFLAGS	"sdl-config --cflags"
#define GFX_LIBS	"sdl-config --libs"
#elif GFX_LIBRARY == GTK
#define GFX_CFLAGS	"pkg-config --cflags gtk+-2.0 gtkglext-1.0"
#define GFX_LIBS	"pkg-config --libs gtk+-2.0 gtkglext-1.0"
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
	
	if((p = popen(GFX_CFLAGS, "r"))) {
		while((c = fgetc(p)) != -1) {
			if(c != '\n') putchar(c);
		}
		putchar(' ');
		pclose(p);
	}

	if((p = popen(CFLAGS, "r"))) {
		while((c = fgetc(p)) != -1) putchar(c);
		putchar(' ');
		pclose(p);
	}
}

void print_libs(void) {
	printf("-l3dengfx ");
	print_libs_no_3dengfx();
}

void print_libs_no_3dengfx(void) {
	FILE *p;
	int c;
		
	printf("-lGL -lGLU -l3ds -lbz2 %s %s ", LD_JPEG, LD_PNG);

	if((p = popen(GFX_LIBS, "r"))) {
		while((c = fgetc(p)) != -1) {
			if(c != '\n') putchar(c);
		}
		putchar(' ');
		pclose(p);
	}

	if((p = popen(LIBS, "r"))) {
		while((c = fgetc(p)) != -1) putchar(c);
		putchar(' ');
		pclose(p);
	}

}
