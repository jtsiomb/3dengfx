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

#ifndef _NLIBASE_H_
#define _NLIBASE_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include "nlibase_types.h"

NASE_File *NASE_OpenFile(const char *fname);
void NASE_CloseFile(NASE_File *file);

unsigned int NASE_GetMaterialCount(NASE_File *file);
NASE_Material *NASE_GetMaterials(NASE_File *file);
NASE_Material *NASE_GetMaterial(NASE_File *file, const char *matname, int id);

unsigned int NASE_GetObjectCount(NASE_File *file);
NASE_Object *NASE_GetObjects(NASE_File *file);
NASE_Object *NASE_GetObject(NASE_File *file, const char *objname, int num);

/* "Destructors" in a sense ... */
void NASE_FreeObject(NASE_Object *obj);
void NASE_FreeMaterial(NASE_Material *mat);

void NASE_FreeObjectData(NASE_Object *obj);
void NASE_FreeMaterialData(NASE_Material *mat);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _NLIBASE_H_ */
