/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include "load_geom.hpp"
#include "nlibase.h"
#include "texman.hpp"

char *tex_path, *geom_path;

void SetDataPath(const char *path, DataPathType dtype) {
	if(dtype == DPATH_TEX) {
		if(tex_path) delete [] tex_path;
		tex_path = new char[strlen(path)+1];
		strcpy(tex_path, path);
	} else if(dtype == DPATH_GEOM) {
		if(geom_path) delete [] geom_path;
		geom_path = new char[strlen(path)+1];
		strcpy(geom_path, path);
	}
}

const char *GetDataPath(DataPathType dtype) {
	if(dtype == DPATH_TEX) {
		return tex_path;
	} else if(dtype == DPATH_GEOM) {
		return geom_path;
	}

	return 0;
}


Object *LoadObject(const char *name, char *src, int flags) {
	
	NASE_File *ase;

	if(flags & LGEOM_FILE) {

		if(geom_path) {
			char *filename = new char[strlen(src) + strlen(geom_path) + 1];
			strcpy(filename, geom_path);
			strcat(filename, src);
			src = filename;
		}
		
		if(!(ase = NASE_OpenFile(src))) {
			return 0;
		}

		if(geom_path) {
			delete [] src;
		}
	} else if(flags & LGEOM_MEM) {
		ase = new NASE_File;
		ase->mem = src;
	} else {
		return 0;
	}

	NASE_Object *nobj = NASE_GetObject(ase, name, 0);
	if(!nobj) {
		if(flags & LGEOM_FILE) {
			NASE_CloseFile(ase);
		} else {
			delete ase;
		}
		return 0;
	}

	NASE_Material *nmat = 0;
	if(nobj->mat_ref != -1) {
		if(!(nmat = NASE_GetMaterial(ase, 0, nobj->mat_ref))) {
			NASE_FreeObject(nobj);
			if(flags & LGEOM_FILE) {
				NASE_CloseFile(ase);
			} else {
				delete ase;
			}
			return 0;
		}
	}
	
	Object *obj = new Object;
	
	if(nmat) {
		Material *mat = obj->GetMaterialPtr();
		mat->ambient_color = Color(nmat->ambient.r, nmat->ambient.g, nmat->ambient.b);
		mat->diffuse_color = Color(nmat->diffuse.r, nmat->diffuse.g, nmat->diffuse.b);
		mat->specular_color = Color(nmat->specular.r, nmat->specular.g, nmat->specular.b);
		mat->specular_color *= nmat->specular_intensity;
		mat->specular_power = nmat->specular_power;
		mat->env_intensity = 1.0f;	// FIXME: load actual env intensity
		mat->bump_intensity = 1.0f;	// FIXME: see above

		if(nmat->tex[NASE_TEX_DIFFUSE]) {
			char *fname;
			if(tex_path) {
				fname = new char[strlen(tex_path) + strlen(nmat->tex[NASE_TEX_DIFFUSE]->filename) + 1];
				strcpy(fname, tex_path);
				strcat(fname, nmat->tex[NASE_TEX_DIFFUSE]->filename);
			} else {
				fname = nmat->tex[NASE_TEX_DIFFUSE]->filename;
			}
			mat->tex[TEXTYPE_DIFFUSE] = GetTexture(fname);
			
			if(tex_path) {
				delete [] fname;
			}
		}
		if(nmat->tex[NASE_TEX_REFLECT]) {
			char *fname;
			if(tex_path) {
				fname = new char[strlen(tex_path) + strlen(nmat->tex[NASE_TEX_REFLECT]->filename) + 1];
				strcpy(fname, tex_path);
				strcat(fname, nmat->tex[NASE_TEX_REFLECT]->filename);
			} else {
				fname = nmat->tex[NASE_TEX_REFLECT]->filename;
			}
			mat->tex[TEXTYPE_ENVMAP] = GetTexture(fname);
			
			if(tex_path) {
				delete [] fname;
			}
		}
		/*if(nmat->tex[NASE_TEX_BUMP]) {
			mat->tex[TEXTYPE_BUMPMAP] = GetTexture(nmat->tex[NASE_TEX_BUMP]->filename);
		}*/
	}

	NASE_Mesh *nmesh = &nobj->mesh;
	Vertex *varray = new Vertex[nmesh->vcount];
	for(int i=0; i<nmesh->vcount; i++) {
		varray[i].pos.x = nmesh->varray[i].pos.x;
		varray[i].pos.y = nmesh->varray[i].pos.y;
		varray[i].pos.z = nmesh->varray[i].pos.z;
		varray[i].tex[0].u = varray[i].tex[1].u = nmesh->varray[i].tex.u;
		varray[i].tex[0].v = varray[i].tex[1].v = nmesh->varray[i].tex.v;
		varray[i].normal.x = nmesh->varray[i].normal.x;
		varray[i].normal.y = nmesh->varray[i].normal.y;
		varray[i].normal.z = nmesh->varray[i].normal.z;
	}
	
	Triangle *tarray = new Triangle[nmesh->tcount];
	for(int i=0; i<nmesh->tcount; i++) {
		for(int j=0; j<3; j++) {
			tarray[i].vertices[j] = nmesh->tarray[i].v[j];
		}
		tarray[i].normal.x = nmesh->tarray[i].normal.x;
		tarray[i].normal.y = nmesh->tarray[i].normal.y;
		tarray[i].normal.z = nmesh->tarray[i].normal.z;
	}
	
	obj->SetTriMesh(TriMesh(varray, nmesh->vcount, tarray, nmesh->tcount));
	obj->GetTriMeshPtr()->CalculateNormals();

	delete [] tarray;
	delete [] varray;
	
	/* get local PRS */
	obj->SetPosition(Vector3(nobj->prs.pos.x, nobj->prs.pos.y, nobj->prs.pos.z));
	Vector3 v;
	v.x = nobj->prs.rot.v.x;
	v.y = nobj->prs.rot.v.y;
	v.z = nobj->prs.rot.v.z;
	obj->SetRotation(Quaternion(nobj->prs.rot.s, v));
	obj->SetScaling(Vector3(nobj->prs.scale.x, nobj->prs.scale.y, nobj->prs.scale.z));
	

	if(nmat) {
		NASE_FreeMaterial(nmat);
	}
	NASE_FreeObject(nobj);

	if(flags & LGEOM_FILE) {
		NASE_CloseFile(ase);
	} else {
		delete ase;
	}

	return obj;
}	
