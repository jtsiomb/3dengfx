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

#include "3dengfx_config.h"

#include <cstdio>
#include <string>
#include <cassert>
#include <cctype>
#include "3dengfx.hpp"
#include "sceneloader.hpp"
#include "3dschunks.h"

using std::string;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

namespace SceneLoader {
	unsigned long ReadCounter;
	Material *mat;
	unsigned long MatCount;
	unsigned long anim_start, anim_end;

	bool eof = false;

	string datapath = "";

	string SceneFileName;
	string ObjectName;

	bool SaveNormalFile = false;
}

using namespace SceneLoader;

struct ChunkHeader {
	ChunkID id;
	dword size;
};

struct Percent {
	int IntPercent;
	scalar_t FloatPercent;

	Percent(int p = 0) {IntPercent = p; FloatPercent = (scalar_t)p / 100.0f; }
	Percent(scalar_t p) {FloatPercent = p; IntPercent = (int)(p * 100.0f); }
};

struct TexMap {
	string filename;
	TextureType type;
	scalar_t intensity;
	scalar_t rotation;
	Vector2 offset;
	Vector2 scale;
};


struct Animation {
	string name;
	int id, father;
	Vector3 pivot;
	// TODO: add the actual animation track
};

const dword HeaderSize = 6;

enum {OBJ_MESH, OBJ_PTLIGHT, OBJ_SPLIGHT, OBJ_CAMERA, OBJ_CURVE};

// local function prototypes
static byte ReadByte(FILE *file);
static word ReadWord(FILE *file);
static dword ReadDword(FILE *file);
static scalar_t ReadFloat(FILE *file);
static Vector3 ReadVector(FILE *file, bool FlipYZ = true);
static string ReadString(FILE *file);
static Color ReadColor(FILE *file);
static Percent ReadPercent(FILE *file);
static ChunkHeader ReadChunkHeader(FILE *file);
static void SkipChunk(FILE *file, const ChunkHeader &chunk);
static void SkipBytes(FILE *file, dword bytes);

static int ReadObject(FILE *file, const ChunkHeader &ch, void **obj);
//static int ReadLight(FILE *file, const ChunkHeader &ch, Light **lt);
static Material ReadMaterial(FILE *file, const ChunkHeader &ch);
static TexMap ReadTextureMap(FILE *file, const ChunkHeader &ch);

static Material *FindMaterial(string name);

static bool LoadAnimation(FILE *file, string name, Animation *anim);
Animation ReadMeshInfo(FILE *file, const ChunkHeader &ch);

static bool LoadNormalsFromFile(const char *fname, Scene *scene);
static void SaveNormalsToFile(const char *fname, Scene *scene);


void SceneLoader::SetDataPath(const char *path) {
	datapath = path;
}

void SceneLoader::SetNormalFileSaving(bool enable) {
	SaveNormalFile = enable;
}



////////////////////////////////////////
//   --==( function LoadScene )==--   //
// ---------------------------------- //
// Creates a Scene instance and loads //
// the data from specified file       //
////////////////////////////////////////

bool SceneLoader::LoadScene(const char *fname, Scene **scene) {
	if(!LoadMaterials(fname, &mat)) return false;

	FILE *file = fopen(fname, "rb");
	if(!file) {
		return false;
	}
	eof = false;

	SceneFileName = string(fname);

	ChunkHeader chunk;
	Scene *scn = new Scene;		// new scene instance
	
	chunk = ReadChunkHeader(file);
	if(chunk.id != Chunk_3DSMain) {
		fclose(file);
		return false;
	}

	while(!eof) {

		chunk = ReadChunkHeader(file);

		void *objptr;
		int type;

		switch(chunk.id) {
		case Chunk_Main_3DEditor:
			break;	// dont skip

		case Chunk_Edit_AmbientColor:
			scn->SetAmbientLight(ReadColor(file));
			break;

		case Chunk_Edit_Fog:
			// **TODO** find out chunk structure
			break;

		case Chunk_Edit_Object:
			type = ReadObject(file, chunk, &objptr);
			switch(type) {
			case OBJ_MESH:
				{
					Object *object = (Object*)objptr;
					object->SetDynamic(false);
					scn->AddObject(object);
				}
				break;

			case OBJ_CAMERA:
				{
					TargetCamera *cam = (TargetCamera*)objptr;
					scn->AddCamera(cam);
				}
				break;

			case OBJ_PTLIGHT:
				{
					PointLight *lt = (PointLight*)objptr;
					scn->AddLight(lt);
				}
				break;

			case OBJ_SPLIGHT:
				{
					//SpotLight *lt = (SpotLight*)objptr;
					//scn->AddLight(lt);
				}
				break;

			case OBJ_CURVE:
				{
                    CatmullRomSplineCurve *spline = (CatmullRomSplineCurve*)objptr;
					scn->AddCurve(spline);
				}
				break;
			}

			break;

		default:
			SkipChunk(file, chunk);
		}
	}

	fclose(file);
	

	// check if there is a normals file in the same dir and load them, or else calculate them
	if(!LoadNormalsFromFile((SceneFileName + string(".normals")).c_str(), scn)) {
		std::list<Object*>::iterator objiter = scn->GetObjectsList()->begin();
		while(objiter != scn->GetObjectsList()->end()) {
			(*objiter++)->GetTriMeshPtr()->CalculateNormals();
		}
		if(SaveNormalFile) SaveNormalsToFile((SceneFileName + string(".normals")).c_str(), scn);
	}

	*scene = scn;
    return true;
}



bool SceneLoader::LoadObject(const char *fname, const char *ObjectName, Object **obj) {
	if(!LoadMaterials(fname, &mat)) return false;

	FILE *file = fopen(fname, "rb");
	if(!file) {
		return false;
	}
	eof = false;

	ChunkHeader chunk = ReadChunkHeader(file);
	if(chunk.id != Chunk_3DSMain) {
		fclose(file);
		return false;
	}

	while(!eof) {

		chunk = ReadChunkHeader(file);

		void *objptr;
		int type;

		switch(chunk.id) {
		case Chunk_Main_3DEditor:
			break;	// dont skip

		case Chunk_Edit_Object:
			type = ReadObject(file, chunk, &objptr);
			if(type == OBJ_MESH) {
				Object *object = (Object*)objptr;
				if(!strcmp(object->name.c_str(), ObjectName)) {
					object->GetTriMeshPtr()->CalculateNormals();
					*obj = object;
					fclose(file);
                    return true;
				}
			}
			break;

		default:
			SkipChunk(file, chunk);
		}
	}

	fclose(file);
	return false;
}



bool FindChunk(FILE *file, word ChunkID) {

	ChunkHeader chunk = ReadChunkHeader(file);

	while(chunk.id != ChunkID) {
		SkipChunk(file, chunk);
		chunk = ReadChunkHeader(file);
	}

	return chunk.id == ChunkID;
}



bool SceneLoader::LoadMaterials(const char *fname, Material **materials) {
	if(!materials) return false;

	FILE *file = fopen(fname, "rb");
	if(!file) {
		return false;
	}
	eof = false;

	ChunkHeader chunk;

	chunk = ReadChunkHeader(file);
	if(chunk.id != Chunk_3DSMain) {
		fclose(file);
		return false;
	}

	if(!FindChunk(file, Chunk_Main_3DEditor)) {
		fclose(file);
		return false;
	}

	std::vector<Material> mats;
	
	while(!eof) {

		chunk = ReadChunkHeader(file);

		if(chunk.id == Chunk_Edit_Material) {
            Material mat = ReadMaterial(file, chunk);
			mats.push_back(mat);
		} else {
			SkipChunk(file, chunk);
		}
	}

	MatCount = (dword)mats.size();

	if(*materials) delete [] *materials;
	Material *m = new Material[MatCount];

	for(dword i=0; i<MatCount; i++) {
		m[i] = mats[i];
	}

	*materials = m;

	fclose(file);
	return true;
}


TexMap ReadTextureMap(FILE *file, const ChunkHeader &ch) {
	assert(ch.id == Chunk_Mat_TextureMap || ch.id == Chunk_Mat_TextureMap2 || ch.id == Chunk_Mat_OpacityMap || ch.id == Chunk_Mat_BumpMap || ch.id == Chunk_Mat_ReflectionMap || ch.id == Chunk_Mat_SelfIlluminationMap);

	TexMap map;
	Percent p = ReadPercent(file);
	map.intensity = p.FloatPercent;	

	switch(ch.id) {
	case Chunk_Mat_TextureMap:
		map.type = TEXTYPE_DIFFUSE;//TextureMap;
		break;

	case Chunk_Mat_TextureMap2:
		map.type = TEXTYPE_DETAIL;//DetailMap;
		break;

	case Chunk_Mat_OpacityMap:
		map.type = TEXTYPE_DETAIL;//OpacityMap; (we don't handle this...)
		break;

	case Chunk_Mat_BumpMap:
		map.type = TEXTYPE_DETAIL;//BumpMap; (nor this)
		break;

	case Chunk_Mat_ReflectionMap:
		map.type = TEXTYPE_ENVMAP;//EnvironmentMap;
		break;

	case Chunk_Mat_SelfIlluminationMap:
		map.type = TEXTYPE_LIGHTMAP;//LightMap;
		break;
	default:
		assert(0);
	}

	ChunkHeader chunk = ReadChunkHeader(file);
	assert(chunk.id == Chunk_Map_FileName);

	map.filename = ReadString(file);

	// convert to lowercase
	char *cstr = new char[map.filename.length()+1];
	char *ptr = cstr - 1;
	
	strcpy(cstr, map.filename.c_str());
	
	while(*++ptr) *ptr = tolower(*ptr);

	map.filename = string(cstr);
	delete [] cstr;
		
	return map;
}





Material ReadMaterial(FILE *file, const ChunkHeader &ch) {

	Material mat;

	assert(ch.id == Chunk_Edit_Material);

	ReadCounter = HeaderSize;
	dword ChunkSize = ch.size;

	while(ReadCounter < ChunkSize) {
		ChunkHeader chunk = ReadChunkHeader(file);

		Percent p;
		TexMap map;

		switch(chunk.id) {
		case Chunk_Mat_Name:
			mat.name = ReadString(file);
			break;

		case Chunk_Mat_AmbientColor:
			mat.ambient_color = ReadColor(file);
			break;

		case Chunk_Mat_DiffuseColor:
			mat.diffuse_color = ReadColor(file);
			break;

		case Chunk_Mat_SpecularColor:
			mat.specular_color = ReadColor(file);
			break;

		case Chunk_Mat_Specular:
			p = ReadPercent(file);
			mat.specular_power = (scalar_t)p.IntPercent;
			//if(mat.specular_power > 0.0f) mat.SpecularEnable = true;
			break;

		case Chunk_Mat_SpecularIntensity:
			p = ReadPercent(file);
			mat.specular_color.r *= p.FloatPercent;
			mat.specular_color.g *= p.FloatPercent;
			mat.specular_color.b *= p.FloatPercent;
			break;

		case Chunk_Mat_Transparency:
			p = ReadPercent(file);
			mat.alpha = 1.0f - p.FloatPercent;
			break;

		case Chunk_Mat_SelfIllumination:
			p = ReadPercent(file);
			mat.emissive_color = Color(p.FloatPercent);
			break;

		case Chunk_Mat_TextureMap:
		case Chunk_Mat_TextureMap2:
		case Chunk_Mat_OpacityMap:
		case Chunk_Mat_SelfIlluminationMap:
			map = ReadTextureMap(file, chunk);
			mat.SetTexture(GetTexture((datapath + map.filename).c_str()), map.type);
            break;

		case Chunk_Mat_ReflectionMap:
			map = ReadTextureMap(file, chunk);
			mat.SetTexture(GetTexture((datapath + map.filename).c_str()), map.type);
			mat.env_intensity = map.intensity;
            break;

		case Chunk_Mat_BumpMap:
			map = ReadTextureMap(file, chunk);
			mat.SetTexture(GetTexture((datapath + map.filename).c_str()), map.type);
			mat.bump_intensity = map.intensity;
            break;

		default:
			SkipChunk(file, chunk);
		}
	}

	return mat;
}            





////////////////////////////////////////////////////
byte ReadByte(FILE *file) {
	byte val;
	dword numread = fread(&val, 1, 1, file);
	//ReadFile(file, &val, sizeof(byte), &numread, NULL);
	if(numread < 1) eof = true;
	ReadCounter++;
	return val;
}

word ReadWord(FILE *file) {
	word val;
	dword numread = fread(&val, sizeof(word), 1, file);
	//ReadFile(file, &val, sizeof(word), &numread, NULL);
	if(numread < 1) eof = true;
	ReadCounter += sizeof(word);
	return val;
}

dword ReadDword(FILE *file) {
	dword val;
	dword numread = fread(&val, sizeof(dword), 1, file);
	//ReadFile(file, &val, sizeof(dword), &numread, NULL);
	if(numread < 1) eof = true;
	ReadCounter += sizeof(dword);
	return val;
}

scalar_t ReadFloat(FILE *file) {
	float val;
	dword numread = fread(&val, sizeof(float), 1, file);
	//ReadFile(file, &val, sizeof(float), &numread, NULL);
	if(numread < 1) eof = true;
	ReadCounter += sizeof(float);
	return (scalar_t)val;
}

Vector3 ReadVector(FILE *file, bool FlipYZ) {
	Vector3 vector;
	vector.x = ReadFloat(file);
	if(!FlipYZ) vector.y = ReadFloat(file);
	vector.z = ReadFloat(file);
	if(FlipYZ) vector.y = ReadFloat(file);
	return vector;		
}

string ReadString(FILE *file) {
	string str;
	char c;
	while((c = (char)ReadByte(file))) {
		str.push_back(c);
	}
	str.push_back('\0');	// noted while porting: ... JESUS! 
							// must have forgotten to bring my clue along :)
	ReadCounter++;

	return str;
}

Color ReadColor(FILE *file) {
	ChunkHeader chunk = ReadChunkHeader(file);
	if(chunk.id < 0x0010 || chunk.id > 0x0013) return Color(-1.0f, -1.0f, -1.0f);

	Color color;

	if(chunk.id == Chunk_Color_Byte3 || chunk.id == Chunk_Color_GammaByte3) {
		byte r = ReadByte(file);
		byte g = ReadByte(file);
		byte b = ReadByte(file);
		color = Color(r, g, b);
	} else {
		color.r = ReadFloat(file);
		color.g = ReadFloat(file);
		color.b = ReadFloat(file);
	}

	return color;
}

Percent ReadPercent(FILE *file) {
	ChunkHeader chunk = ReadChunkHeader(file);
	Percent p;
	if(chunk.id != Chunk_PercentInt && chunk.id != Chunk_PercentFloat) return p;

	if(chunk.id == Chunk_PercentInt) {
		p = Percent(ReadWord(file));
	} else {
		p = Percent(ReadFloat(file));
	}

	return p;
}


ChunkHeader ReadChunkHeader(FILE *file) {
	ChunkHeader chunk;
	chunk.id = (ChunkID)ReadWord(file);
	chunk.size = ReadDword(file);
	return chunk;
}

void SkipChunk(FILE *file, const ChunkHeader &chunk) {
	fseek(file, chunk.size - HeaderSize, SEEK_CUR);
	ReadCounter += chunk.size - HeaderSize;
}

void SkipBytes(FILE *file, dword bytes) {
	fseek(file, bytes, SEEK_CUR);
	ReadCounter += bytes;
}

Material *FindMaterial(string name) {
	dword i=0;
	while(i < MatCount) {
		if(mat[i].name == name) return &mat[i];
		i++;
	}

	return 0;
}

///////////////////// Read Object Function //////////////////////
int ReadObject(FILE *file, const ChunkHeader &ch, void **obj) {
	ReadCounter = HeaderSize;	// reset the global read counter

	string name = ReadString(file);

	ChunkHeader chunk;
	chunk = ReadChunkHeader(file);
	if(chunk.id == Chunk_Obj_TriMesh) {
		// object is a trimesh... load it
		Vertex *varray;
		Triangle *tarray;
		dword VertexCount=0, TriCount=0;
		Material mat;
		Base base;
		Vector3 translation;

		bool curve = true;

		dword ObjChunkSize = ch.size;

		while(ReadCounter < ObjChunkSize) {	// make sure we only read subchunks of this object chunk
			assert(!eof);
			chunk = ReadChunkHeader(file);

            switch(chunk.id) {
			case Chunk_TriMesh_VertexList:
				VertexCount = (dword)ReadWord(file);
				varray = new Vertex[VertexCount];

				for(dword i=0; i<VertexCount; i++) {
					varray[i].pos = ReadVector(file);
				}

				break;

			case Chunk_TriMesh_FaceDesc:
				curve = false;	// it is a real object not a curve since it has triangles
				TriCount = (dword)ReadWord(file);
				tarray = new Triangle[TriCount];

				for(dword i=0; i<TriCount; i++) {
					tarray[i].vertices[0] = (Index)ReadWord(file);	// 
					tarray[i].vertices[2] = (Index)ReadWord(file);	// flip order to CW
					tarray[i].vertices[1] = (Index)ReadWord(file);	//
					ReadWord(file);	// discard edge visibility flags
				}
				break;

			case Chunk_Face_Material:
				mat = *FindMaterial(ReadString(file));

				SkipBytes(file, ReadWord(file)<<1);
				break;

			case Chunk_TriMesh_TexCoords:
				assert((dword)ReadWord(file) == VertexCount);

				for(dword i=0; i<VertexCount; i++) {
					varray[i].tex[0].u = varray[i].tex[1].u = ReadFloat(file);
					varray[i].tex[0].v = varray[i].tex[1].v = ReadFloat(file);
				}
				break;

			case Chunk_TriMesh_SmoothingGroup:
				// **TODO** abide by smoothing groups duplicate vertices, weld others etc
				SkipChunk(file, chunk);
				break;

			case Chunk_TriMesh_WorldTransform:
				base.i = ReadVector(file);
				base.k = ReadVector(file);	// flip
				base.j = ReadVector(file);
				translation = ReadVector(file);
				break;

			default:
				SkipChunk(file, chunk);
			}
		}

		if(curve) {
			CatmullRomSplineCurve *spline = new CatmullRomSplineCurve;
			spline->name = name;
			for(dword i=0; i<VertexCount; i++) {
				spline->AddControlPoint(varray[i].pos);
			}

			*obj = spline;
			return OBJ_CURVE;
		} else {

			Animation anim;
			if(LoadAnimation(file, name, &anim)) {
				// DEBUG
				//fprintf(stderr, "found animation for obj: %s\n", name.c_str());
				//fprintf(stderr, "\thier-id: %d, parent: %d", anim.id, anim.father);
				//fprintf(stderr, "\tpivot: (%.2f, %.2f, %.2f)\n", (float)anim.pivot.x, (float)anim.pivot.y, (float)anim.pivot.z);
				
				for(dword i=0; i<VertexCount; i++) {
					varray[i].pos -= anim.pivot;
				}
			}

			base.i.Normalize();
			base.j.Normalize();
			base.k.Normalize();
						
			base.k = CrossProduct(base.i, base.j);
			base.j = CrossProduct(base.k, base.i);
			Matrix3x3 RotXForm = base.CreateRotationMatrix();
			
			
			for(dword i=0; i<VertexCount; i++) {
				varray[i].pos += -translation;
				varray[i].pos.Transform(RotXForm.Transposed());
			}
			

            Object *object = new Object;
			object->name = name;
			object->GetTriMeshPtr()->SetData(varray, VertexCount, tarray, TriCount);
			object->SetMaterial(mat);
			object->SetRotation(Quaternion());
			object->Rotate(RotXForm);
			object->SetPosition(translation);
			*obj = object;

			return OBJ_MESH;
		}
	} else {

		if(chunk.id == Chunk_Obj_Light) {

			dword ObjChunkSize = ch.size;

			Vector3 pos = ReadVector(file);
			Color color = ReadColor(file);

			Vector3 SpotTarget;
			scalar_t InnerCone, OuterCone;
			bool spot = false;
			bool att = false;
			bool CastShadows = false;
			scalar_t AttEnd = 10000.0f;
			scalar_t Intensity = 1.0f;

			while(ReadCounter < ObjChunkSize) {

				chunk = ReadChunkHeader(file);

				switch(chunk.id) {
				case Chunk_Light_SpotLight:
					spot = true;
					SpotTarget = ReadVector(file);
					InnerCone = ReadFloat(file) / 180.0f;
					OuterCone = ReadFloat(file) / 180.0f;
					break;

				case Chunk_Light_Attenuation:
					att = true;
					break;
				
				case Chunk_Light_AttenuationEnd:
					AttEnd = ReadFloat(file);
					break;

				case Chunk_Light_Intensity:
					Intensity = ReadFloat(file);
					break;

				case Chunk_Spot_CastShadows:
					CastShadows = true;
					break;

				default:
					SkipChunk(file, chunk);
				}
			}

			Light *light;
			if(spot) {
				//light = new TargetSpotLight(pos, SpotTarget, InnerCone, OuterCone);
				light = new PointLight(pos);
			} else {
				light = new PointLight(pos);
			}
			light->SetColor(color);
			//light->SetShadowCasting(CastShadows);
			light->SetIntensity(Intensity);
			if(att) {
				light->SetAttenuation(0, 1.0/(AttEnd/3.0f), 0);
			}
			light->name = name;

			*obj = light;
			return OBJ_PTLIGHT;//spot ? OBJ_SPLIGHT : OBJ_PTLIGHT;
		}

		if(chunk.id == Chunk_Obj_Camera) {
			TargetCamera *cam = new TargetCamera;
			Vector3 pos = ReadVector(file);
			Vector3 targ = ReadVector(file);
			scalar_t roll = ReadFloat(file);
			scalar_t FOV = ReadFloat(file);

			Vector3 up = VECTOR3_J;
			Vector3 view = targ - pos;

			Quaternion q(view.Normalized(), roll);
			up.Transform(q);
			//up.Rotate(view.Normalized(), roll);

			cam->SetPosition(pos);
			cam->SetTarget(targ);
			cam->SetUpVector(up);
			//cam->SetCamera(pos, targ, up);
			cam->name = name;

			/* convert the fov from diagonal to vertical
			 * 	tan(a) = vfov / hfov <=> tan(a) = 1 / aspect
			 *	vfov = sin(a) * dfov
			 *	
			 * [vfov = vertical FOV, hfov = horizontal FOV, dfov = diagonal FOV]
			 */
			scalar_t angle = atan(1.0 / cam->GetAspect());
			cam->SetFOV(sin(angle) * DEG_TO_RAD(FOV));
			
			*obj = cam;
			return OBJ_CAMERA;
		}
	}

	return -1;  // should have already left by now, if not something is wrong
}


/* Starts from the begining, finds the keyframer chunk and loads
 * keyframing info for the specified object if it exists
 */
static bool LoadAnimation(FILE *file, string name, Animation *anim) {
	long prev_fp = ftell(file);
	unsigned long prev_read_counter = ReadCounter;

	fseek(file, 0, SEEK_SET);
	ReadCounter = 0;

	while(!eof) {
		ChunkHeader chunk = ReadChunkHeader(file);
		switch(chunk.id) {
		case Chunk_3DSMain: 
			break;

		case Chunk_Main_Keyframer:
			break;

		case Chunk_Key_MeshInfo:
			*anim = ReadMeshInfo(file, chunk);
			if(anim->name == name) {
				fseek(file, prev_fp, SEEK_SET);
				ReadCounter = prev_read_counter;
				return true;
			}
			break;

		case Chunk_Key_Frames:
			anim_start = ReadDword(file);
			anim_end = ReadDword(file);
			break;

		default:
			SkipChunk(file, chunk);
		}
	}

	fseek(file, prev_fp, SEEK_SET);
	ReadCounter = prev_read_counter;
	return false;
}

// reads the meshinfo chunk and subchunks...
Animation ReadMeshInfo(FILE *file, const ChunkHeader &ch) {
	assert(ch.id == Chunk_Key_MeshInfo);
	Animation anim;

	ReadCounter = HeaderSize;

	while(ReadCounter < ch.size) {
		ChunkHeader chunk = ReadChunkHeader(file);

		switch(chunk.id) {
		case Chunk_Info_Object:
			anim.name = ReadString(file);
			ReadWord(file);		// skip flags1
			ReadWord(file);		// skip flags2 (TODO: reconsider keeping motion blur flag)
			anim.father = (short)ReadWord(file);
			break;

		case Chunk_Info_ObjectPivot:
			anim.pivot = ReadVector(file);
			break;

		case Chunk_Info_HierarchyPosition:
			anim.id = (short)ReadWord(file);
			break;

		default:
			SkipChunk(file, chunk);
		}
	}
	
	return anim;
}

////////////////////////////////////////////////////////////////////////////////
// functions to save/load normals from file

void WriteByte(FILE *file, byte val) {
	fwrite(&val, 1, 1, file);
}

void WriteDword(FILE *file, dword val) {
	fwrite(&val, sizeof(dword), 1, file);
}

void WriteFloat(FILE *file, scalar_t val) {
	float single_val = (float)val;
	fwrite(&single_val, sizeof(float), 1, file);
}

void WriteString(FILE *file, string str) {
	for(dword i=0; i<(dword)str.size(); i++) {
		fputc((int)str[i], file);
	}
}


void SaveNormalsToFile(const char *fname, Scene *scene) {

	FILE *file = fopen(fname, "wb");
	if(!file) return;

	WriteDword(file, (dword)scene->GetObjectsList()->size());

	std::list<Object*>::iterator objiter = scene->GetObjectsList()->begin();
	while(objiter != scene->GetObjectsList()->end()) {
		WriteString(file, (*objiter)->name);
		dword VertexCount = (*objiter)->GetTriMeshPtr()->GetVertexArray()->GetCount();
		WriteDword(file, VertexCount);

		const Vertex *varray = (*objiter)->GetTriMeshPtr()->GetVertexArray()->GetData();
		for(dword i=0; i<VertexCount; i++) {
			WriteFloat(file, varray[i].normal.x);
			WriteFloat(file, varray[i].normal.y);
			WriteFloat(file, varray[i].normal.z);
		}

		objiter++;
	}

	fclose(file);
}

bool LoadNormalsFromFile(const char *fname, Scene *scene) {

	//HANDLE file = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	FILE *file = fopen(fname, "rb");
	if(!file) return false;

	eof = false;
	ReadCounter = 0;

	fseek(file, 0, SEEK_END);
	dword FileSize = ftell(file);

	fseek(file, 0, SEEK_SET);

	dword ObjectCount = ReadDword(file);
	if(ObjectCount != scene->GetObjectsList()->size()) {	// detect changes
		fclose(file);
		return false;
	}
    
	while(SceneLoader::ReadCounter < FileSize) {
		string name = ReadString(file);
		dword VertexCount = ReadDword(file);
		
		Object *obj = scene->GetObject(name.c_str());
		if(!obj) {
			fclose(file);
			return false;
		}

		if(VertexCount != obj->GetTriMeshPtr()->GetVertexArray()->GetCount()) {	// again detect changes
			fclose(file);
			return false;
		}
        
		Vertex *varray = obj->GetTriMeshPtr()->GetModVertexArray()->GetModData();
		for(dword i=0; i<VertexCount; i++) {
			varray[i].normal = ReadVector(file, false);
		}
	}

	fclose(file);
	return true;
}

