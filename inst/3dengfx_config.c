#include <stdio.h>
#include <string.h>
#include <3dengfx_config.h>

#if GFX_LIBRARY == SDL
#define GFX_CFLAGS	"sdl-config --cflags"
#define GFX_LIBS	"echo `sdl-config --libs` -lGL"

#elif GFX_LIBRARY == GTK
#define GFX_CFLAGS	"pkg-config --cflags gtk+-2.0 gtkglext-1.0"
#define GFX_LIBS	"pkg-config --libs gtk+-2.0 gtkglext-1.0"

#elif GFX_LIBRARY == NATIVE
#if NATIVE_LIB == NATIVE_X11
#define GFX_CFLAGS	"echo -I/usr/X11R6/include -I/usr/include/X11"

#ifdef USE_XF86VIDMODE
#define GFX_LIBS	"echo -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lGL"
#else
#define GFX_LIBS	"echo -L/usr/X11R6/lib -lX11 -lXext -lGL"
#endif	// USE_XF86VIDMODE

#elif NATIVE_LIB == NATIVE_WIN32
#define GFX_CFLAGS	"echo -mwindows"
#define GFX_LIBS	"echo -lopengl32 -lgdi32"

#endif

#elif GFX_LIBRARY == GLUT
#define GFX_CFLAGS	"echo"
#define GFX_LIBS	"echo -lglut"

#endif	/* GFX_LIBRARY == ? */

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

#ifndef FXWT_NO_FREETYPE
#define FT_CFLAGS	"pkg-config --cflags freetype2"
#define FT_LIBS		"pkg-config --libs freetype2"
#else
#define FT_CFLAGS	""
#define FT_LIBS		""
#endif	/* freetype */

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

	if((p = popen(FT_CFLAGS, "r"))) {
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
		
	printf("%s %s ", LD_JPEG, LD_PNG);

	if((p = popen(GFX_LIBS, "r"))) {
		while((c = fgetc(p)) != -1) {
			if(c != '\n') putchar(c);
		}
		putchar(' ');
		pclose(p);
	}

	if((p = popen(FT_LIBS, "r"))) {
		while((c = fgetc(p)) != -1) putchar(c);
		putchar(' ');
		pclose(p);
	}

}
