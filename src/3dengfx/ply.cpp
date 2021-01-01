#include <vector>
#include <string>
#include <cstdio>
#include <cctype>
#include <cassert>
#include "gfx/3dgeom.hpp"
#include "common/err_msg.h"

#define BUFFER_SIZE		256

using std::vector;
using std::string;

enum PlyFormat {
	PLY_ASCII,
	PLY_LITTLE_ENDIAN,
	PLY_BIG_ENDIAN
};

enum PropType {
	PROP_FLOAT,
	PROP_INT8,
	PROP_INT16,
	PROP_INT32,
	PROP_LIST
};

const size_t prop_size[] = {32, 8, 16, 32, 0};

struct PropTypeMatch {
	char *symb;
	PropType type;
} prop_match[] = {
	{"float",	PROP_FLOAT},
	{"float32",	PROP_FLOAT},
	{"int",		PROP_INT32},
	{"int32",	PROP_INT32},
	{"uint",	PROP_INT32},
	{"uint32",	PROP_INT32},
	{"int16",	PROP_INT16},
	{"uint16",	PROP_INT16},
	{"char",	PROP_INT8},
	{"uchar",	PROP_INT8},
	{"int8",	PROP_INT8},
	{"uint8",	PROP_INT8},
	{"list",	PROP_LIST},
	{0, (PropType)0}
};

struct Property {
	string name;
	PropType type;
	PropType list_type;	// list elements type, if type == PROP_LIST
	size_t size;
};

enum ElementType {ELEM_UNKNOWN, ELEM_VERTEX, ELEM_FACE};

struct Element {
	ElementType type;
	unsigned long count;
	vector<Property> prop;
	long offset;
};

struct Ply {
	PlyFormat fmt;
	vector<Element> elem;
	FILE *fp;
	unsigned long header_skip;
};

static Ply *read_header(FILE *fp);
static Element *seek_elem(Ply *ply, ElementType elem_type);

static const char *ply_filename = 0;	// for error reports

bool file_is_ply(FILE *file) {
	char sig[5] = {0};
	
	fseek(file, 0, SEEK_SET);
	fgets(sig, 5, file);

	return !strcmp(sig, "ply\n");
}

#define FAIL(m) {\
	error("ply(%s): " m, fname);\
	fclose(fp);\
	delete ply;\
	return 0;\
}

TriMesh *load_mesh_ply(const char *fname) {
	const char *sep = " \t\n";
	char buf[BUFFER_SIZE];

	FILE *fp = fopen(fname, "r");
	if(!fp || !file_is_ply(fp)) {
		if(fp) fclose(fp);
		return 0;
	}

	ply_filename = fname;

	Ply *ply = read_header(fp);
	if(!ply) {
		fclose(fp);
		return 0;
	}
	ply->fp = fp;

	vector<Vertex> verts;
	vector<Triangle> tris;

	Element *elem;

	// -- read the vertices
	if(!(elem = seek_elem(ply, ELEM_VERTEX))) {
		FAIL("failed to locate vertex data");
	}

	if(elem->prop[0].type != PROP_FLOAT || elem->prop[1].type != PROP_FLOAT || elem->prop[2].type != PROP_FLOAT) {
		FAIL("weird vertex format, didn't find 3 floats");
	}

	for(unsigned long i=0; i<elem->count; i++) {
		Vertex v;
		if(ply->fmt == PLY_ASCII) {
			fgets(buf, BUFFER_SIZE, fp);

			char *x_str = strtok(buf, sep);
			char *y_str = strtok(0, sep);
			char *z_str = strtok(0, sep);

			if(!x_str || !y_str || !z_str) {
				FAIL("vertex data loading failed, format inconsistent");
			}
			v.pos.x = atof(x_str);
			v.pos.y = atof(y_str);
			v.pos.z = -atof(z_str);
			
		} else {
			FAIL("sorry binary ply loading not implemented yet");
		}

		verts.push_back(v);
	}

	// -- read the face list
	if(!(elem = seek_elem(ply, ELEM_FACE))) {
		FAIL("failed to locate face data");
	}

	if(elem->prop[0].type != PROP_LIST) {
		FAIL("weird face format, didn't find an index list");
	}

	for(unsigned long i=0; i<elem->count; i++) {
		int count;
		unsigned long indices[4];
		
		if(ply->fmt == PLY_ASCII) {
			fgets(buf, BUFFER_SIZE, fp);

			count = atoi(strtok(buf, sep));
			if(count < 3 || count > 4) {
				FAIL("only triangles and quads are supported");
			}

			for(int j=0; j<count; j++) {
				char *substr = strtok(0, sep);
				if(!substr) {
					FAIL("inconsistent face list data");
				}
				indices[j] = atol(substr);
			}
			
		} else {
			FAIL("sorry binary ply loading not implemented yet");
		}

		Triangle tri(indices[0], indices[1], indices[2]);
		tris.push_back(tri);

		if(count == 4) {
			Triangle tri2(indices[0], indices[2], indices[3]);
			tris.push_back(tri2);
		}
	}

	fclose(fp);
	delete ply;

	// ok now we have the vertex/triangle vectors, let's create the mesh and return it
	TriMesh *mesh = new TriMesh(&verts[0], verts.size(), &tris[0], tris.size());
	mesh->calculate_normals();
	return mesh;
}

static Ply *read_header(FILE *fp) {
	const char *sep = " \t\n";
	char buf[BUFFER_SIZE];
	
	fseek(fp, 0, SEEK_SET);

	Ply *ply = new Ply;
	memset(ply, 0, sizeof(Ply));

	bool vertex_ok = false, face_ok = false;

	while(fgets(buf, BUFFER_SIZE, fp)) {
		char *field = strtok(buf, sep);
		if(!field) continue;

		if(!strcmp(field, "format")) {
			char *fmt = strtok(0, sep);
			if(!fmt) {
				error("ply(%s): invalid format field", ply_filename);
				delete ply;
				return 0;
			}

			if(!strcmp(fmt, "ascii")) {
				ply->fmt = PLY_ASCII;
			} else if(!strcmp(fmt, "binary_little_endian")) {
				ply->fmt = PLY_LITTLE_ENDIAN;
			} else if(!strcmp(fmt, "binary_big_endian")) {
				ply->fmt = PLY_BIG_ENDIAN;
			} else {
				error("ply(%s): invalid format field", ply_filename);
				delete ply;
				return 0;
			}
			
		} else if(!strcmp(field, "element")) {
			char *elem_name = strtok(0, sep);
			if(!elem_name) {
				warning("ply(%s): invalid element definition", ply_filename);
				continue;
			}
			
			char *count_str = strtok(0, sep);
			if(!count_str || !isdigit(*count_str)) {
				error("ply(%s): element not followed by a count", ply_filename);
				delete ply;
				return 0;
			}

			unsigned long count = atol(count_str);

			Element elem;
			elem.type = ELEM_UNKNOWN;
			elem.count = count;
						
			if(!strcmp(elem_name, "vertex")) {
				elem.type = ELEM_VERTEX;
				vertex_ok = true;
			}
			
			if(!strcmp(elem_name, "face")) {
				elem.type = ELEM_FACE;
				face_ok = true;
			}

			// determine element properties
			while((buf[0] = fgetc(fp)) == 'p') {
				if(!fgets(buf + 1, BUFFER_SIZE - 1, fp)) {
					error("ply(%s): unexpected end of file while reading element properties", ply_filename);
					delete ply;
					return 0;
				}
				char *ptr = strtok(buf, sep);
				if(!ptr || strcmp(ptr, "property")) {
					error("ply(%s): looking for \"propery\", got \"%s\"", ply_filename, ptr ? ptr : "NULL");
					delete ply;
					return 0;
				}

				Property prop;
				prop.size = 0;
				
				/*
				char *name = strtok(0, sep);
				if(!name) {
					error("ply(%s): invalid property entry, no name specified", ply_filename);
					delete ply;
					return 0;
				}
				prop.name = name;
				*/

				char *type = strtok(0, sep);
				if(!type) {
					error("ply(%s): invalid property entry, no type specified", ply_filename);
					delete ply;
					return 0;
				}

				PropTypeMatch *mptr = prop_match;
				while(mptr->symb) {
					if(!strcmp(type, mptr->symb)) {
						prop.type = mptr->type;
						prop.size = prop_size[prop.type];
						break;
					}
					mptr++;
				}

				if(prop.type == PROP_LIST) {
					type = strtok(0, sep);
					if(!type) {
						error("ply(%s): invalid property entry, no list subtype specified", ply_filename);
						delete ply;
						return 0;
					}
					
					mptr = prop_match;
					while(mptr->symb) {
						if(!strcmp(type, mptr->symb)) {
							prop.list_type = mptr->type;
							prop.size = prop_size[prop.list_type];
							break;
						}
						mptr++;
					}
				}

				if(!prop.size) {
					error("ply(%s): unknown property type \"%s\"", ply_filename, type);
					delete ply;
					return 0;
				}

				elem.prop.push_back(prop);
			}

			ungetc(buf[0], fp);

			ply->elem.push_back(elem);

		} else if(!strcmp(field, "end_header")) {
			if(!vertex_ok || !face_ok) {
				error("ply(%s): some important element is unspecified (vertex or face list)", ply_filename);
				delete ply;
				return 0;
			}
			ply->header_skip = ftell(fp);
			break;
		}
	}

	return ply;
}

static Element *seek_elem(Ply *ply, ElementType elem_type) {
	fseek(ply->fp, ply->header_skip, SEEK_SET);
	
	if(ply->fmt == PLY_ASCII) {
		char buf[BUFFER_SIZE];

		for(size_t i=0; i<ply->elem.size(); i++) {
			if(ply->elem[i].type == elem_type) {
				return &ply->elem[i];
			}

			// it's not the one we want, skip it.
			for(size_t j=0; j<ply->elem[i].count; j++) {
				fgets(buf, BUFFER_SIZE, ply->fp);
			}
		}
		return 0;

	} else {
		error("ply(%s): seek failed, binary ply loading not implemented yet", ply_filename);
		return 0;
	}
}
