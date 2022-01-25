/*
Copyright 2022 Alejandro Cosin & Gustavo Patow

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/shader/resourceenum.h"
#include "../../include/shader/shaderreflection.h"

// NAMESPACE
using namespace commonnamespace;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

namespace resourceenum
{
	const string arrayResourceKeywords[119] =
	{
		"float",
		"vec2",
		"vec3",
		"vec4",
		"double",
		"dvec2",
		"dvec3",
		"dvec4",
		"int",
		"ivec2",
		"ivec3",
		"ivec4",
		"uint",
		"uvec2",
		"uvec3",
		"uvec4",
		"bool",
		"bvec2",
		"bvec3",
		"bvec4",
		"mat2",
		"mat3",
		"mat4",
		"mat2x2",
		"mat2x3",
		"mat2x4",
		"mat3x2",
		"mat3x3",
		"mat3x4",
		"mat4x2",
		"mat4x3",
		"mat4x4",
		"dmat2",
		"dmat3",
		"dmat4",
		"dmat2x2",
		"dmat2x3",
		"dmat2x4",
		"dmat3x2",
		"dmat3x3",
		"dmat3x4",
		"dmat4x2",
		"dmat4x3",
		"dmat4x4",
		"sampler1D",
		"sampler2D",
		"sampler3D",
		"samplerCube",
		"sampler1DShadow",
		"sampler2DShadow",
		"samplerCubeShadow",
		"sampler1DArray",
		"sampler2DArray",
		"sampler1DArrayShadow",
		"sampler2DArrayShadow",
		"isampler1D",
		"isampler2D",
		"isampler3D",
		"isamplerCube",
		"isampler1DArray",
		"isampler2DArray",
		"usampler1D",
		"usampler2D",
		"usampler3D",
		"usamplerCube",
		"usampler1DArray",
		"usampler2DArray",
		"sampler2DRect",
		"sampler2DRectShadow",
		"isampler2DRect",
		"usampler2DRect",
		"samplerBuffer",
		"isamplerBuffer",
		"usamplerBuffer",
		"sampler2DMS",
		"isampler2DMS",
		"usampler2DMS",
		"sampler2DMSArray",
		"isampler2DMSArray",
		"usampler2DMSArray",
		"samplerCubeArray",
		"samplerCubeArrayShadow",
		"isamplerCubeArray",
		"usamplerCubeArray",
		"image1D",
		"iimage1D",
		"uimage1D",
		"image2D",
		"iimage2D",
		"uimage2D",
		"image3D",
		"iimage3D",
		"uimage3D",
		"image2DRect",
		"iimage2DRect",
		"uimage2DRect",
		"imageCube",
		"iimageCube",
		"uimageCube",
		"imageBuffer",
		"iimageBuffer",
		"uimageBuffer",
		"image1DArray",
		"iimage1DArray",
		"uimage1DArray",
		"image2DArray",
		"iimage2DArray",
		"uimage2DArray",
		"imageCubeArray",
		"iimageCubeArray",
		"uimageCubeArray",
		"image2DMS",
		"iimage2DMS",
		"uimage2DMS",
		"image2DMSArray",
		"iimage2DMSArray",
		"uimage2DMSArray",
		"atomic_uint",
		"struct"
	};

	const string arrayImageFormatQualifierKeywords[39] =
	{
		"rgba32f",
		"rgba16f",
		"rg32f",
		"rg16f",
		"r11f_g11f_b10f",
		"r32f",
		"r16f",
		"rgba16",
		"rgb10_a2",
		"rgba8",
		"rg16",
		"rg8",
		"r16",
		"r8",
		"rgba16_snorm",
		"rgba8_snorm",
		"rg16_snorm",
		"rg8_snorm",
		"r16_snorm",
		"r8_snorm",
		"rgba32i",
		"rgba16i",
		"rgba8i",
		"rg32i",
		"rg16i",
		"rg8i",
		"r32i",
		"r16i",
		"r8i",
		"rgba32ui",
		"rgba16ui",
		"rgb10_a2ui",
		"rgba8ui",
		"rg32ui",
		"rg16ui",
		"rg8ui",
		"r32ui",
		"r16ui",
		"r8ui"
	};

	int getResourceInternalTypeSizeInBytes(ResourceInternalType resourceInternalType)
	{
		if (!ShaderReflection::isDataType(resourceInternalType))
		{
			cout << "ERROR: given resourceInternalType is not a data type in getResourceInternalTypeSizeInBytes" << endl;
			return -1;
		}

		switch (resourceInternalType)
		{
			case ResourceInternalType::RIT_FLOAT:
			case ResourceInternalType::RIT_INT:
			case ResourceInternalType::RIT_UNSIGNED_INT:
			case ResourceInternalType::RIT_BOOL:
			{
				return 4;
			}
			case ResourceInternalType::RIT_DOUBLE:
			case ResourceInternalType::RIT_FLOAT_VEC2:
			case ResourceInternalType::RIT_INT_VEC2:
			case ResourceInternalType::RIT_UNSIGNED_INT_VEC2:
			case ResourceInternalType::RIT_BOOL_VEC2:
			{
				return 8;
			}
			case ResourceInternalType::RIT_FLOAT_VEC3:
			case ResourceInternalType::RIT_BOOL_VEC3:
			case ResourceInternalType::RIT_INT_VEC3:
			case ResourceInternalType::RIT_UNSIGNED_INT_VEC3:
			{
				return 12;
			}
			case ResourceInternalType::RIT_DOUBLE_VEC2:
			case ResourceInternalType::RIT_FLOAT_VEC4:
			case ResourceInternalType::RIT_BOOL_VEC4:
			case ResourceInternalType::RIT_INT_VEC4:
			case ResourceInternalType::RIT_UNSIGNED_INT_VEC4:
			case ResourceInternalType::RIT_FLOAT_MAT2x2:
			case ResourceInternalType::RIT_FLOAT_MAT2:
			{
				return 16;
			}
			case ResourceInternalType::RIT_DOUBLE_VEC3:
			case ResourceInternalType::RIT_FLOAT_MAT2x3:
			case ResourceInternalType::RIT_FLOAT_MAT3x2:
			{
				return 24;
			}
			case ResourceInternalType::RIT_DOUBLE_VEC4:
			case ResourceInternalType::RIT_FLOAT_MAT2x4:
			case ResourceInternalType::RIT_FLOAT_MAT4x2:
			case ResourceInternalType::RIT_DOUBLE_MAT2:
			case ResourceInternalType::RIT_DOUBLE_MAT2x2:
			{
				return 32;
			}
			case ResourceInternalType::RIT_FLOAT_MAT3:
			case ResourceInternalType::RIT_FLOAT_MAT3x3:
			{
				return 36;
			}
			case ResourceInternalType::RIT_FLOAT_MAT3x4:
			case ResourceInternalType::RIT_FLOAT_MAT4x3:
			case ResourceInternalType::RIT_DOUBLE_MAT2x3:
			case ResourceInternalType::RIT_DOUBLE_MAT3x2:
			{
				return 48;
			}
			case ResourceInternalType::RIT_FLOAT_MAT4:
			case ResourceInternalType::RIT_FLOAT_MAT4x4:
			case ResourceInternalType::RIT_DOUBLE_MAT2x4:
			case ResourceInternalType::RIT_DOUBLE_MAT4x2:
			{
				return 64;
			}
			case ResourceInternalType::RIT_DOUBLE_MAT3:
			case ResourceInternalType::RIT_DOUBLE_MAT3x3:
			{
				return 72;
			}
			case ResourceInternalType::RIT_DOUBLE_MAT3x4:
			case ResourceInternalType::RIT_DOUBLE_MAT4x3:
			{
				return 96;
			}
			case ResourceInternalType::RIT_DOUBLE_MAT4:
			case ResourceInternalType::RIT_DOUBLE_MAT4x4:
			{
				return 128;
			}
			default:
			{
				return -1;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
