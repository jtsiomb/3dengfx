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
#ifndef _NLIBASE_TYPES_H_
#define _NLIBASE_TYPES_H_

#define NASE_MAX_TEXTURES	8

enum {
	NASE_TEX_DIFFUSE	= 0,
	NASE_TEX_REFLECT	= 1,
	NASE_TEX_REFRACT	= 2,
	NASE_TEX_BUMP		= 3
};

typedef enum NASE_TexClass {
	NASE_TC_BITMAP,
	NASE_TC_CHESS
} NASE_TexClass;
	

typedef struct NASE_File {
	const char *filename;
	char *mem;
	int fd;
} NASE_File;


typedef struct NASE_Vector3 {
	float x, y, z;
} NASE_Vector3;

typedef struct NASE_Color {
	float r, g, b;
} NASE_Color;


typedef struct NASE_Quaternion {
	float s;
	NASE_Vector3 v;
} NASE_Quaternion;


typedef struct NASE_PRS {
	NASE_Vector3 pos;
	NASE_Quaternion rot;
	NASE_Vector3 scale;
} NASE_PRS;


typedef struct NASE_TexCoord {
	float u, v;
} NASE_TexCoord;


typedef struct NASE_Vertex {
	NASE_Vector3 pos;
	NASE_Vector3 normal;
	NASE_TexCoord tex;
} NASE_Vertex;


typedef struct NASE_Triangle {
	int v[3];
	NASE_Vector3 normal;
} NASE_Triangle;


typedef struct NASE_Mesh {
	NASE_Vertex *varray;
	NASE_Triangle *tarray;
	int vcount, tcount;
} NASE_Mesh;

typedef struct NASE_Texture {
	char *name;
	char *filename;
	NASE_TexClass tex_class;
} NASE_Texture;
	

typedef struct NASE_Material {
	int id;
	char *name;
	NASE_Color ambient, diffuse, specular;
	float specular_power, specular_intensity;
	float alpha, self_illum;
	NASE_Texture *tex[NASE_MAX_TEXTURES];
} NASE_Material;


typedef struct NASE_Object {
	char *name;
	NASE_Mesh mesh;
	int mat_ref;
	NASE_PRS prs;
} NASE_Object;


typedef struct NASE_Scene {
	char *name;
	NASE_Color ambient_light, background;
	int fps, frames, ticks_per_frame;
	
	NASE_Material *materials;
	NASE_Object *objects;
	/* .. lights, cameras, splines, etc... */
} NASE_Scene;	


#endif	/* _NLIBASE_TYPES_H_ */
