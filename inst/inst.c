#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "3dengfx_config.h"

const char *modules[] = {
	"",
	"/3dengfx",
	"/gfx",
	"/n3dmath2",
	"/fxwt",
	"/nlibase",
	"/sim",
	"/common",
	"/dsys",
	0
};

int main(int argc, char **argv) {
	int inst = 1;

	if(argc > 1 && !strcmp(argv[1], "-r")) {
		inst = 0;
	}

	if(inst) {
		int perm = 0755;
		const char **modptr = modules;

		while(*modptr) {
			static char buf[512];

			sprintf(buf, PREFIX "/include/3dengfx%s", *modptr);			
			if(mkdir(buf, perm) == -1 && errno != EEXIST) return -1;
			
			sprintf(buf, "cp src%s/*.h src%s/*.hpp src%s/*.inl " PREFIX "/include/3dengfx%s/ 2>/dev/null", *modptr, *modptr, *modptr, *modptr);
			system(buf);

			modptr++;
		}

		system("cp lib3dengfx.so.0.1.0 " PREFIX "/lib/");
		system("cp lib3dengfx.a " PREFIX "/lib/");
		system("cp 3dengfx-config " PREFIX "/bin/");

		chdir(PREFIX "/lib");
		symlink("lib3dengfx.so.0.1.0", "lib3dengfx.so");
		system("ldconfig");
	} else {
		/* remove */
		const char **modptr = modules + 1;

		system("rm -rf " PREFIX "/include/3dengfx");
		system("rm -f " PREFIX "/lib/lib3dengfx.*");
		system("rm -f " PREFIX "/bin/3dengfx-config");
	}
	
	return 0;
}
