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
#ifndef _3DSCHUNKS_H_
#define _3DSCHUNKS_H_

enum ChunkID {
	Chunk_Color_Float3					= 0x0010,	// o Floating point color
	Chunk_Color_Byte3					= 0x0011,	// o 24bit color
	Chunk_Color_GammaByte3				= 0x0012,	// o 24bit gamma corrected
	Chunk_Color_GammaFloat3				= 0x0013,	// o Floating point gamma corrected
	Chunk_PercentInt					= 0x0030,	// o Percent Chunk int 0 - 100
	Chunk_PercentFloat					= 0x0031,	// o Percent Chunk float 0 - 1

	Chunk_3DSMain						= 0x4D4D,	// + Root Chunk
	Chunk_Main_3DSVersion				= 0x0002,	//   - 3DS Version
	Chunk_Main_3DEditor					= 0x3D3D,	//   + 3D Editor Chunk
	Chunk_Edit_Unit						= 0x0100,	//     - Unit
	Chunk_Edit_BGBitmap					= 0x1100,	//     - Background Bitmap
	Chunk_Edit_UseBGBitmap				= 0x1101,	//     - Use Background Bitmap
	Chunk_Edit_BGColor					= 0x1200,	//     - Background Color
	Chunk_Edit_UseBGColor				= 0x1201,	//     - Use Background Color
	Chunk_Edit_GradColor				= 0x1300,	//     - Background Gradient
	Chunk_Edit_UseGradColor				= 0x1301,	//     - Use Gradient Color
	Chunk_Edit_ShadowMapBias			= 0x1400,	//     - Shadow map bias
	Chunk_Edit_ShadowMapSize			= 0x1420,	//     - Shadow map size
	Chunk_Edit_ShadowMapSampleRange		= 0x1450,	//     - Shadow map sample range
	Chunk_Edit_RaytraceBias				= 0x1460,	//     - Raytrace bias
	Chunk_Edit_UseRaytrace				= 0x1470,	//     - Use Raytrace
	Chunk_Edit_AmbientColor				= 0x2100,	//     - Ambient Color
	Chunk_Edit_Fog						= 0x2200,	//     + Fog
	Chunk_Fog_FogColor					= 0x2210,	//	     - Fog Color
	Chunk_Edit_UseFog					= 0x2201,	//     - Use Fog
	Chunk_Edit_DistanceQue				= 0x2300,	//     + Distance que
	Chunk_Dist_DimBackground			= 0x2310,	//       - Dim Background
	Chunk_Edit_UseDistanceQue			= 0x2301,	//     - Use distance que
	Chunk_Edit_LayeredFogOptions		= 0x2302,	//     - Layered fog options
	Chunk_Edit_UseLayeredFog			= 0x2303,	//     - Use Layered Fog
	Chunk_Edit_MeshVersion				= 0x3D3E,	//     - Mesh Version

	Chunk_Edit_Object					= 0x4000,	//     + Object
	Chunk_Obj_Hidden					= 0x4010,	//       - Hidden
	Chunk_Obj_DontCastShadows			= 0x4012,	//       - Object doesn't cast shadows
	Chunk_Obj_MatteObject				= 0x4013,	//       - Matte
	Chunk_Obj_ExternalProcessOn			= 0x4015,	//       - External Process on (?)
	Chunk_Obj_DontReceiveShadows		= 0x4017,	//       - doesn't reseive shadows
	Chunk_Obj_TriMesh					= 0x4100,	//       + TriMesh
	Chunk_TriMesh_VertexList			= 0x4110,	//         - Vertex List
	Chunk_TriMesh_FaceDesc				= 0x4120,	//         + Faces description
	Chunk_Face_Material					= 0x4130,	//           - Face Materials*
	Chunk_TriMesh_TexCoords				= 0x4140,	//         - Texture Coordinates
	Chunk_TriMesh_SmoothingGroup		= 0x4150,	//         - Smoothing group
	Chunk_TriMesh_WorldTransform		= 0x4160,	//         - Position and Orientation
	Chunk_TriMesh_Color					= 0x4165,   //         - Object color
	Chunk_TriMesh_ExternalProcessName	= 0x4181,	//         - External Process name (?)
	Chunk_TriMesh_ExternalProcessParams	= 0x4182,	//         - External Process parameters (?)

	Chunk_Obj_Light						= 0x4600,	//       + Light
	Chunk_Light_SpotLight				= 0x4610,	//         + SpotLight
	Chunk_Spot_Raytrace					= 0x4627,	//           - Raytrace
	Chunk_Spot_CastShadows				= 0x4630,	//           - Light casts shadows
	Chunk_Spot_ShadowMap				= 0x4641,	//           - Shadow Map
	Chunk_Spot_ShowCone					= 0x4650,	//           - Show Cone
	Chunk_Spot_Rectangular				= 0x4651,	//           - Rectangular shaped spotlight
	Chunk_Spot_OverShoot				= 0x4652,	//           - Overshoot
	Chunk_Spot_ProjMap					= 0x4653,	//           - Projector Map
	Chunk_Spot_Roll						= 0x4656,	//           - Roll around dir
	Chunk_Spot_RaytraceBias				= 0x4658,	//           - Raytrace Bias
	Chunk_Light_Off						= 0x4620,	//         - Light is disabled
	Chunk_Light_Attenuation				= 0x4625,	//         - Attenuation enabled
	Chunk_Light_AttenuationStart		= 0x4659,	//         - Attenuation Start Range
	Chunk_Light_AttenuationEnd			= 0x465A,	//         - Attenuation End Range
	Chunk_Light_Intensity				= 0x465B,	//         - Light Intensity

	Chunk_Obj_Camera					= 0x4700,	//       - Camera
	Chunk_Edit_ViewSettings				= 0x7001,	//     - View Settings
	Chunk_Edit_ViewDesc2				= 0x7011,	//     - View Description 2
	Chunk_Edit_ViewDesc1				= 0x7012,	//     - View Description 1
	Chunk_Edit_MeshWindows				= 0x7020,	//     - Mesh Windows (?)

	Chunk_Edit_Material					= 0xAFFF,	//     + Material Block
	Chunk_Mat_Name						= 0xA000,	//       - Material Name
	Chunk_Mat_AmbientColor				= 0xA010,	//       - Ambient Color
	Chunk_Mat_DiffuseColor				= 0xA020,	//       - Diffuse Color
	Chunk_Mat_SpecularColor				= 0xA030,	//       - Specular Color
	Chunk_Mat_Specular					= 0xA040,	//       - Shininness (Specular Power)
	Chunk_Mat_SpecularIntensity			= 0xA041,	//       - Shininness Strength (specular intensity)
	Chunk_Mat_Transparency				= 0xA050,	//       - Transparency (alpha)
	Chunk_Mat_TransparencyFalloff		= 0xA052,	//       - Transparency Falloff
	Chunk_Mat_ReflectionBlur			= 0xA053,	//       - Reflection Blur
	Chunk_Mat_TwoSided					= 0xA081,	//       - Two Sided
	Chunk_Mat_AddTransparency			= 0xA083,	//       - ?
	Chunk_Mat_SelfIllumination			= 0xA084,	//       - Self Illumination (emissive)
	Chunk_Mat_Wireframe					= 0xA085,	//       - Render in wireframe
	Chunk_Mat_WireframeThickness		= 0xA087,	//       - Wire thickness
	Chunk_Mat_FaceMapping				= 0xA088,	//       - Apply maps to faces seperatly (ignore uv)
	Chunk_Mat_InTranc					= 0xA08A,	// ?
	Chunk_Mat_Soften					= 0xA08C,	//       - Soft Shading
	Chunk_Mat_WireUnits					= 0xA08E,	//       - Wire units (?)
	Chunk_Mat_RenderType				= 0xA100,	//       - Render Type
	Chunk_Mat_BumpMapPercent			= 0xA252,	//       - Bump map intensity
	Chunk_Mat_TextureMap				= 0xA200,	//       + Texture Map
	Chunk_Mat_TextureMap2				= 0xA33A,	//       + Texture Map 2
	Chunk_Mat_OpacityMap				= 0xA210,	//       + Opacity Map
	Chunk_Mat_BumpMap					= 0xA230,	//       + Bump Map
	Chunk_Mat_SpecularMap				= 0xA33C,	//       + Specular Intensity map
	Chunk_Mat_SpecularColorMap			= 0xA204,	//       + Specular color (texture) map
	Chunk_Mat_SelfIlluminationMap		= 0xA33D,	//       + Self Illumination Map
	Chunk_Mat_ReflectionMap				= 0xA220,	//       + Reflection Map
	Chunk_Mat_TextureMask				= 0xA33E,	//       - Texture Mask
	Chunk_Mat_Texture2Mask				= 0xA340,	//       - Texture 2 Mask
	Chunk_Mat_OpacityMask				= 0xA342,	//       - Opacity Mask
	Chunk_Mat_BumpMask					= 0xA344,	//       - Bump Mask
	Chunk_Mat_SpecularMask				= 0xA346,	//       - Specular Mask
	Chunk_Mat_SpecularColorMask			= 0xA348,	//       - Specular color mask
	Chunk_Mat_SelfIlluminationMask		= 0xA34A,	//       - Self Illumination mask
	Chunk_Mat_ReflectionMask			= 0xA34C,	//       - Reflection mask
	
	// map subchunks								// -----------------------
	Chunk_Map_FileName					= 0xA300,	//         - Filename
	Chunk_Map_Params					= 0xA351,	//         - Parameters
	Chunk_Map_BlurPercent				= 0xA353,	//         - Blur ammount
	Chunk_Map_VScale					= 0xA354,	//         - Texture V Scale
	Chunk_Map_UScale					= 0xA356,	//         - Texture U Scale
	Chunk_Map_UOffset					= 0xA358,	//         - Texture U Offset
	Chunk_MAP_VOffset					= 0xA35A,	//         - Texture V Offset
	Chunk_Map_RotationAngle				= 0xA35C,	//         - Texture Rotation Angle
	Chunk_Map_RGBLumAlphaTint1			= 0xA360,	//         - RGB Luminance Alpha Tint 1
	Chunk_Map_RGBLumAlphaTint2			= 0xA362,	//         - RGB Luminance Alpha Tint 2
	Chunk_Map_RGBTintR					= 0xA364,	//         - RGB Tint R
	Chunk_Map_RGBTintG					= 0xA366,	//         - RGB Tint G
	Chunk_Map_RGBTintB					= 0xA368,	//         - RGB Tint B
	// map subchunks end							// -----------------------

	Chunk_Main_Keyframer				= 0xB000,	//     + Keyframer Chunk
	Chunk_Key_AmbientInfo				= 0xB001,	//       - Ambient animation info
	Chunk_Key_MeshInfo					= 0xB002,	//       - Mesh animation info
	Chunk_Key_CameraInfo				= 0xB003,	//       - Camera animation info
	Chunk_Key_CameraTargetInfo			= 0xB004,	//       - Camera Target animation info
	Chunk_Key_OmniLightInfo				= 0xB005,	//       - Omni Light animation info
	Chunk_Key_SpotLightTargetInfo		= 0xB006,	//       - Spotlight target animation info
	Chunk_Key_SpotLightInfo				= 0xB007,	//       - Spotlight animation info
	Chunk_Key_Frames					= 0xB008,	//       - Animation Frames
	
	// animation information subchunks				// -----------------------
	Chunk_Info_Object					= 0xB010,	//         - Object information
	Chunk_Info_ObjectPivot				= 0xB013,	//         - Object Pivot
	Chunk_Info_ObjectMorphAngle			= 0xB015,	//         - Object Morph Angle
	Chunk_Info_PositionTrack			= 0xB020,	//         - Position Track
	Chunk_Info_RotationTrack			= 0xB021,	//         - Rotation Track
	Chunk_Info_ScaleTrack				= 0xB022,	//         - Scaling Track
	Chunk_Info_FOVTrack					= 0xB023,	//         - FOV Track
	Chunk_Info_RollTrack				= 0xB024,	//         - Roll Track
	Chunk_Info_ColorTrack				= 0xB025,	//         - Color Track
	Chunk_Info_MorphTrack				= 0xB026,	//         - Morph Track
	Chunk_Info_HotSpotTrack				= 0xB027,	//         - HotSpot Track
	Chunk_Info_FalloffTrack				= 0xB028,	//         - Falloff Track
	Chunk_Info_HideTrack				= 0xB029,	//         - Hide Track
	Chunk_Info_HierarchyPosition		= 0xB030	//         - Hierarchy Position
};

#endif	// _3DSCHUNKS_H_
