/*
Copyright 2004 John Tsiombikas

This file is part of nlibASE.

nlibASE is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

nlibASE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with nlibASE; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <bzlib.h>
#include "nlibase.h"

/* DEBUG */
#include <assert.h>


#ifndef __USE_POSIX
FILE *fdopen(int fd, const char *modes);
#endif	/* __USE_POSIX */


/* Some static helper functions (impl. at the end) */
static char *FindValue(char *mem, const char *name);
static char *Find(char *mem, const char *name);
static NASE_Vector3 GetVector3(char *mem);
static NASE_Color GetColor(char *mem);
static char *GetString(char *mem);
static NASE_Texture *GetTexture(char *mem);
static NASE_Vertex *GetVertexArray(char *mem);
static NASE_Triangle *GetTriangleArray(char *mem);

/* This function checks to see if the file is compressed with bzip2
 * if not, it does nothing and the loader loads the ASE file as usual.
 * If the file is bzip compressed then it decompresses it in a mem buffer,
 * unmaps and closes the file and sets the NASE_File struct up as if the
 * thing was read from memory to begin with, from then on, the loader goes
 * on to load the file from memory (the decompressed buffer).
 */

static int BZipRemap(NASE_File *file) {
	FILE *fp;
	BZFILE *bzfile;
	int bzerr, bytes, buf_size = 0;
	char *tmpbuf, *buffer = 0;
	const int tmpbuf_size = 512 * 1024;	/* 512 kb */
	struct stat sbuf;

	if(file->fd == -1) return 0;
	
	if(!(fp = fdopen(file->fd, "rb"))) {
		perror("(nlibASE) unexpected error");
		return -1;
	}

	/* BZ2_bzReadOpen(int *bzerror, FILE *f, int verbosity, int small, void *unused, int nUnused) */
	bzfile = BZ2_bzReadOpen(&bzerr, fp, 0, 0, 0, 0);
	if(bzerr != BZ_OK) {
		fprintf(stderr, "(nlibASE) bzReadOpen() failed\n");
		switch(bzerr) {
		case BZ_IO_ERROR:
			fprintf(stderr, "\terror: BZ_IO_ERROR\n");
			break;

		case BZ_PARAM_ERROR:
			fprintf(stderr, "\terror: BZ_PARAM_ERROR\n");
			break;
		}
		return -1;
	}

	tmpbuf = (char*)malloc(tmpbuf_size);
	do {
		char *ptr;
		bytes = BZ2_bzRead(&bzerr, bzfile, tmpbuf, tmpbuf_size);
		if(bzerr == BZ_DATA_ERROR_MAGIC) { /* not bziped */
			free(tmpbuf);
			return 0;
		}
		
		buffer = (char*)realloc(buffer, buf_size + bytes);
		ptr = buffer + buf_size;
		buf_size += bytes;

		memcpy(ptr, tmpbuf, bytes);
	} while(bzerr != BZ_STREAM_END);
	
	BZ2_bzReadClose(&bzerr, bzfile);
	free(tmpbuf);
	fstat(file->fd, &sbuf);
	munmap(file->mem, sbuf.st_size);
	close(file->fd);
	
	file->fd = -1;
	file->mem = buffer;

	return 0;
}	


NASE_File *NASE_OpenFile(const char *fname) {
	NASE_File *file;
	struct stat sbuf;
	
	file = (NASE_File*)malloc(sizeof(NASE_File));

	if((file->fd = open(fname, O_RDONLY)) == -1) {
		char errstr[50];
		sprintf(errstr, "(nlibASE) error opening file %s", fname);
		perror(errstr);
		free(file);
		return 0;
	}

	fstat(file->fd, &sbuf);
	file->mem = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, file->fd, 0);
	if(file->mem == MAP_FAILED) {
		perror("(nlibASE) could not mmap the file");
		close(file->fd);
		free(file);
		return 0;
	}
	
	if(BZipRemap(file) == -1) {
		close(file->fd);
		free(file);
		return 0;
	}

	if(!strstr(file->mem, "ASCIIEXPORT")) {
		NASE_CloseFile(file);
		fprintf(stderr, "(nlibASE) %s is not an ASE file\n", fname);
		return 0;
	}

	return file;
}

void NASE_CloseFile(NASE_File *file) {
	struct stat sbuf;

	if(file->fd == -1) {
		/* it is not a real file but just a memory pointer */
		free(file->mem);
	} else {
		fstat(file->fd, &sbuf);
		munmap(file->mem, sbuf.st_size);
		close(file->fd);
	}
	
	free(file);
}

unsigned int NASE_GetMaterialCount(NASE_File *file) {
	char *ptr;

	if(!(ptr = FindValue(file->mem, "*MATERIAL_COUNT"))) {
		return -1;
	}
	return atoi(ptr);
}

NASE_Material *NASE_GetMaterials(NASE_File *file) {
	char *ptr = file->mem;
	NASE_Material *mat;
	int mat_count, i=0;

	if(!(ptr = strstr(file->mem, "*MATERIAL_LIST"))) {
		return 0;
	}

	mat_count = NASE_GetMaterialCount(file);
	mat = (NASE_Material*)malloc(mat_count * sizeof(NASE_Material));

	for(i=0; i<mat_count; i++) {
		NASE_Material *material = NASE_GetMaterial(file, 0, i);
		memcpy(&mat[i], material, sizeof(NASE_Material));
		free(material);
	}

	return mat;
}

NASE_Material *NASE_GetMaterial(NASE_File *file, const char *matname, int id) {
	char *ptr = file->mem;
	NASE_Material *mat;

	if(!(ptr = strstr(file->mem, "*MATERIAL_LIST"))) {
		return 0;
	}

	while((ptr = Find(ptr, "*MATERIAL"))) {
		char *name = 0;
		int matid;
		
		if(matname) {
			name = GetString(FindValue(ptr, "*MATERIAL_NAME"));
		} else {
			matid = atoi(FindValue(ptr, "*MATERIAL"));
		}
		
		if((matname && !strcmp(matname, name)) || (!matname && id == matid)) {
			char *tptr;
			char *next_mat = Find(ptr+1, "*MATERIAL");
			if(!next_mat) {
				next_mat = Find(ptr, "*GEOMOBJECT");
				if(!next_mat) next_mat = ptr + strlen(ptr);
			}
			
			mat = malloc(sizeof(NASE_Material));
			
			mat->id = atoi(FindValue(ptr, "*MATERIAL"));
			mat->name = GetString(FindValue(ptr, "*MATERIAL_NAME"));
			mat->ambient = GetColor(FindValue(ptr, "*MATERIAL_AMBIENT"));
			mat->diffuse = GetColor(FindValue(ptr, "*MATERIAL_DIFFUSE"));
			mat->specular = GetColor(FindValue(ptr, "*MATERIAL_SPECULAR"));
			mat->specular_power = atof(FindValue(ptr, "*MATERIAL_SHINE")) * 100.0f;
			mat->specular_intensity = atof(FindValue(ptr, "*MATERIAL_SHINESTRENGTH"));
			mat->alpha = 1.0f - atof(FindValue(ptr, "*MATERIAL_TRANSPARENCY"));
			mat->self_illum = atof(FindValue(ptr, "*MATERIAL_SELFILLUM"));

			memset(mat->tex, 0, NASE_MAX_TEXTURES * sizeof(NASE_Texture*));
			
			if((tptr = Find(ptr, "*MAP_DIFFUSE")) && tptr < next_mat) {
				mat->tex[NASE_TEX_DIFFUSE] = GetTexture(tptr);
			}
			if((tptr = Find(ptr, "*MAP_REFLECT")) && tptr < next_mat) {
				mat->tex[NASE_TEX_REFLECT] = GetTexture(tptr);
			}
			/* TODO: add the rest */
			
			free(name);
			return mat;
		} else {
			free(name);
		}

		ptr++;
	}
	
	return 0;
}


unsigned int NASE_GetObjectCount(NASE_File *file) {
	unsigned int count = 0;
	char *ptr = file->mem;

	while((ptr = strstr(ptr, "*GEOMOBJECT"))) {
		count++;
		ptr++;
	}

	return count;
}

NASE_Object *NASE_GetObject(NASE_File *file, const char *objname, int num) {
	char *objmem = file->mem;
	int i = 0;

	if(!strstr(file->mem, "*GEOMOBJECT")) {
		return 0;
	}

	while((objmem = strstr((objmem+1), "*GEOMOBJECT"))) {
		char *name = GetString(FindValue(objmem, "*NODE_NAME"));

		if((objname && !strcmp(objname, name)) || (!objname && i == num)) {
			NASE_Object *obj;
			char *ptr;
			/*
			char *next_geom = strstr((objmem+1), "*GEOMOBJECT");
			char *next_mesh = Find(objmem, "*MESH");

			if(!next_mesh || (next_geom && next_mesh > next_geom)) {
				free(name);
				continue;
			}
			*/
			obj = (NASE_Object)malloc(sizeof(NASE_Object));

			obj->name = name;
			/* get PRS */
			{
				NASE_Vector3 axis = GetVector3(FindValue(objmem, "*TM_ROTAXIS"));
				float angle = atof(FindValue(objmem, "*TM_ROTANGLE"));
				float sin_half_angle = sin(angle / 2.0f);
				obj->prs.rot.s = cos(angle / 2.0f);
				obj->prs.rot.v.x = axis.x * sin_half_angle;
				obj->prs.rot.v.y = axis.y * sin_half_angle;
				obj->prs.rot.v.z = axis.z * sin_half_angle;

				obj->prs.pos = GetVector3(FindValue(objmem, "*TM_POS"));
				obj->prs.scale = GetVector3(FindValue(objmem, "*TM_SCALE"));
			}

			/* get vertices */
			obj->mesh.vcount = atoi(FindValue(objmem, "*MESH_NUMVERTEX"));
			obj->mesh.tcount = atoi(FindValue(objmem, "*MESH_NUMFACES"));
			obj->mesh.varray = GetVertexArray(objmem);
			obj->mesh.tarray = GetTriangleArray(objmem);
			
			ptr = FindValue(objmem, "*MATERIAL_REF");
			obj->mat_ref = ptr ? atoi(ptr) : -1;
			
			return obj;			
		} else {
			free(name);
		}
	}
	return 0;
}


void NASE_FreeObject(NASE_Object *obj) {
	NASE_FreeObjectData(obj);
	free(obj);
}

void NASE_FreeMaterial(NASE_Material *mat) {
	NASE_FreeMaterialData(mat);
	free(mat);
}

void NASE_FreeObjectData(NASE_Object *obj) {
	free(obj->mesh.varray);
	free(obj->mesh.tarray);
	free(obj->name);
}

void NASE_FreeMaterialData(NASE_Material *mat) {
	int i;
	for(i=0; i<NASE_MAX_TEXTURES; i++) {
		if(mat->tex[i]) {
			free(mat->tex[i]->name);
			free(mat->tex[i]->filename);
			free(mat->tex[i]);
		}
	}
	free(mat->name);
}

/* ----------------------------------------------------- */
/* --------------- static helper functions ------------- */
/* ----------------------------------------------------- */


/* Find the *value* of this entry */
static char *FindValue(char *mem, const char *name) {
	char *ptr = strstr(mem, name);
	if(!ptr) return 0;
	if(!isspace(*(ptr + strlen(name)))) {
		return FindValue(ptr + 1, name);
	}

	while(!isspace(*ptr++));
	while(isspace(*ptr)) ptr++;

	return ptr;
}

/* Find exact (no trailing non-space characters) */
static char *Find(char *mem, const char *name) {
	char *ptr = strstr(mem, name);
	if(!ptr) return 0;
	if(!isspace(*(ptr + strlen(name)))) {
		return Find(ptr + 1, name);
	}

	return ptr;
}

/* Reads stuff as a 3D Vector, note that in 3D Studio MAX the 
 * y and z axes are swapped, so we compensate by swapping them.
 */
static NASE_Vector3 GetVector3(char *mem) {
	NASE_Vector3 vec;
	char *ptr = mem;
	
	vec.x = atof(ptr);
	while(!isspace(*ptr++));
	vec.z = atof(ptr);
	while(!isspace(*ptr++));
	vec.y = atof(ptr);

	return vec;
}


static NASE_Color GetColor(char *mem) {
	NASE_Color col;
	char *ptr = mem;

	col.r = atof(ptr);
	while(!isspace(*ptr++));
	col.g = atof(ptr);
	while(!isspace(*ptr++));
	col.b = atof(ptr);

	return col;
}

/* Strings like names, filenames, etc in ASE files are enclosed in quotes
 * this function duplicates the string without the quotes and returns a
 * pointer to the newly allocated string.
 */
static char *GetString(char *mem) {
	char *end, *str;
	unsigned int size;
	
	while(*mem++ != '\"');
	if(*(mem-1) == '\n') return 0;
	end = mem;

	while(*end != '\"') {
		if(*end == '\n') return 0;
		end++;
	}

	size = end - mem;
	str = (char*)malloc(size + 1);
	memcpy(str, mem, size);
	str[size] = 0;

	return str;
}

static NASE_Texture *GetTexture(char *mem) {
	NASE_Texture *tex;
	char *class_str, *fname, *tmp;
	
	tex = malloc(sizeof(NASE_Texture));
	
	tex->name = GetString(FindValue(mem, "*MAP_NAME"));
	class_str = GetString(FindValue(mem, "*MAP_CLASS"));
	if(strcmp(class_str, "Bitmap") != 0) {
		char err_str[50];
		sprintf(err_str, "(nlibASE) warning, texture %s is of unsupported class\n", tex->name);
		free(tex->name);
		free(class_str);
		free(tex);
		return 0;
	} else {
		free(class_str);
		tex->tex_class = NASE_TC_BITMAP;
	}

	fname = GetString(FindValue(mem, "*BITMAP"));
	/* lose the path, since it is absolute */
	if((tmp = strrchr(fname, '\\'))) {
		tmp++;
		tex->filename = malloc(strlen(tmp)+1);
		strcpy(tex->filename, tmp);
		free(fname);
	} else {
		tex->filename = fname;
	}
	return tex;
}


static NASE_Vertex *GetVertexArray(char *mem) {
	NASE_Vertex *varray;
	char *ptr = mem;
	int vcount = atoi(FindValue(mem, "*MESH_NUMVERTEX"));
	char *next_obj = Find(ptr + 1, "*GEOMOBJECT");
	if(!next_obj) next_obj = ptr + strlen(ptr);

	varray = malloc(vcount * sizeof(NASE_Vertex));
	
	while((ptr = Find(ptr, "*MESH_VERTEX")) && ptr < next_obj) {
		int index = atoi(ptr = FindValue(ptr, "*MESH_VERTEX"));
		while(!isspace(*ptr++));
		varray[index].pos = GetVector3(ptr);
	}

	ptr = mem;
	while((ptr = Find(ptr, "*MESH_TVERT")) && ptr < next_obj) {
		NASE_Vector3 tv;
		int index = atoi(ptr = FindValue(ptr, "*MESH_TVERT"));
		if(index >= vcount) break;
		
		while(!isspace(*ptr++));
		tv = GetVector3(ptr);

		varray[index].tex.u = tv.x;
		varray[index].tex.v = tv.z;
	}

	/*
	ptr = mem;
	while((ptr = Find(ptr, "*MESH_VERTEXNORMAL")) && ptr < next_obj) {
		int index = atoi(ptr = FindValue(ptr, "*MESH_VERTEXNORMAL"));
		while(!isspace(*ptr++));
		varray[index].normal = GetVector3(ptr);
	}
	*/

	return varray;
}

static NASE_Triangle *GetTriangleArray(char *mem) {
	NASE_Triangle *tarray;
	char *ptr = mem;
	
	int tcount = atoi(FindValue(mem, "*MESH_NUMFACES"));
	char *next_obj = Find(ptr+1, "*GEOMOBJECT");
	if(!next_obj) next_obj = ptr + strlen(ptr);

	tarray = malloc(tcount * sizeof(NASE_Triangle));
	
	while((ptr = Find(ptr, "*MESH_FACE")) && ptr < next_obj) {
		int index = atoi(ptr = FindValue(ptr, "*MESH_FACE"));
		tarray[index].v[0] = atoi(FindValue(ptr, "A:"));
		tarray[index].v[2] = atoi(FindValue(ptr, "B:"));
		tarray[index++].v[1] = atoi(FindValue(ptr, "C:"));
	}

	/*
	ptr = mem;
	while((ptr = Find(ptr, "*MESH_FACENORMAL")) && ptr < next_obj) {
		int index = atoi(ptr = FindValue(ptr, "*MESH_FACENORMAL"));
		while(!isspace(*ptr)) ptr++;
		while(isspace(*ptr)) ptr++;
		tarray[index].normal = GetVector3(ptr);
	}
	*/

	return tarray;
}

