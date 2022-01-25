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

#ifndef _RESOURCE_ENUM_H_
#define _RESOURCE_ENUM_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

namespace resourceenum
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Array with the string equivalent of each keyword that could be neede to take into account when doing
	* shader reflection. For ease of use, this strings are hashed and the put in m_mapResourceInternalType for
	* direct traslation into enum type */
	extern const string arrayResourceKeywords[119];

	/** Array with the string equivalent of each image format qualifier possible value */
	extern const string arrayImageFormatQualifierKeywords[39];

	/////////////////////////////////////////////////////////////////////////////////////////////

	enum class ResourceInternalType
	{
		RIT_FLOAT = 0,
		RIT_FLOAT_VEC2,
		RIT_FLOAT_VEC3,
		RIT_FLOAT_VEC4,
		RIT_DOUBLE,
		RIT_DOUBLE_VEC2,
		RIT_DOUBLE_VEC3,
		RIT_DOUBLE_VEC4,
		RIT_INT,
		RIT_INT_VEC2,
		RIT_INT_VEC3,
		RIT_INT_VEC4,
		RIT_UNSIGNED_INT,
		RIT_UNSIGNED_INT_VEC2,
		RIT_UNSIGNED_INT_VEC3,
		RIT_UNSIGNED_INT_VEC4,
		RIT_BOOL,
		RIT_BOOL_VEC2,
		RIT_BOOL_VEC3,
		RIT_BOOL_VEC4,
		RIT_FLOAT_MAT2,
		RIT_FLOAT_MAT3,
		RIT_FLOAT_MAT4,
		RIT_FLOAT_MAT2x2,
		RIT_FLOAT_MAT2x3,
		RIT_FLOAT_MAT2x4,
		RIT_FLOAT_MAT3x2,
		RIT_FLOAT_MAT3x3,
		RIT_FLOAT_MAT3x4,
		RIT_FLOAT_MAT4x2,
		RIT_FLOAT_MAT4x3,
		RIT_FLOAT_MAT4x4,
		RIT_DOUBLE_MAT2,
		RIT_DOUBLE_MAT3,
		RIT_DOUBLE_MAT4,
		RIT_DOUBLE_MAT2x2,
		RIT_DOUBLE_MAT2x3,
		RIT_DOUBLE_MAT2x4,
		RIT_DOUBLE_MAT3x2,
		RIT_DOUBLE_MAT3x3,
		RIT_DOUBLE_MAT3x4,
		RIT_DOUBLE_MAT4x2,
		RIT_DOUBLE_MAT4x3,
		RIT_DOUBLE_MAT4x4,
		RIT_SAMPLER_1D,
		RIT_SAMPLER_2D,
		RIT_SAMPLER_3D,
		RIT_SAMPLER_CUBE,
		RIT_SAMPLER_1D_SHADOW,
		RIT_SAMPLER_2D_SHADOW,
		RIT_SAMPLER_CUBE_SHADOW,
		RIT_SAMPLER_1D_ARRAY,
		RIT_SAMPLER_2D_ARRAY,
		RIT_SAMPLER_1D_ARRAY_SHADOW,
		RIT_SAMPLER_2D_ARRAY_SHADOW,
		RIT_INT_SAMPLER_1D,
		RIT_INT_SAMPLER_2D,
		RIT_INT_SAMPLER_3D,
		RIT_INT_SAMPLER_CUBE,
		RIT_INT_SAMPLER_1D_ARRAY,
		RIT_INT_SAMPLER_2D_ARRAY,
		RIT_UNSIGNED_INT_SAMPLER_1D,
		RIT_UNSIGNED_INT_SAMPLER_2D,
		RIT_UNSIGNED_INT_SAMPLER_3D,
		RIT_UNSIGNED_INT_SAMPLER_CUBE,
		RIT_UNSIGNED_INT_SAMPLER_1D_ARRAY,
		RIT_UNSIGNED_INT_SAMPLER_2D_ARRAY,
		RIT_SAMPLER_2D_RECT,
		RIT_SAMPLER_2D_RECT_SHADOW,
		RIT_INT_SAMPLER_2D_RECT,
		RIT_UNSIGNED_INT_SAMPLER_2D_RECT,
		RIT_SAMPLER_BUFFER,
		RIT_INT_SAMPLER_BUFFER,
		RIT_UNSIGNED_INT_SAMPLER_BUFFER,
		RIT_SAMPLER_2D_MULTISAMPLE,
		RIT_INT_SAMPLER_2D_MULTISAMPLE,
		RIT_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
		RIT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		RIT_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		RIT_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		RIT_SAMPLER_CUBE_ARRAY,
		RIT_SAMPLER_CUBE_ARRAY_SHADOW,
		RIT_INT_SAMPLER_CUBE_ARRAY,
		RIT_UNSIGNED_INT_SAMPLER_CUBE_ARRAY,
		RIT_IMAGE_1D,
		RIT_INT_IMAGE_1D,
		RIT_UNSIGNED_INT_IMAGE_1D,
		RIT_IMAGE_2D,
		RIT_INT_IMAGE_2D,
		RIT_UNSIGNED_INT_IMAGE_2D,
		RIT_IMAGE_3D,
		RIT_INT_IMAGE_3D,
		RIT_UNSIGNED_INT_IMAGE_3D,
		RIT_IMAGE_2D_RECT,
		RIT_INT_IMAGE_2D_RECT,
		RIT_UNSIGNED_INT_IMAGE_2D_RECT,
		RIT_IMAGE_CUBE,
		RIT_INT_IMAGE_CUBE,
		RIT_UNSIGNED_INT_IMAGE_CUBE,
		RIT_IMAGE_BUFFER,
		RIT_INT_IMAGE_BUFFER,
		RIT_UNSIGNED_INT_IMAGE_BUFFER,
		RIT_IMAGE_1D_ARRAY,
		RIT_INT_IMAGE_1D_ARRAY,
		RIT_UNSIGNED_INT_IMAGE_1D_ARRAY,
		RIT_IMAGE_2D_ARRAY,
		RIT_INT_IMAGE_2D_ARRAY,
		RIT_UNSIGNED_INT_IMAGE_2D_ARRAY,
		RIT_IMAGE_CUBE_ARRAY,
		RIT_INT_IMAGE_CUBE_ARRAY,
		RIT_UNSIGNED_INT_IMAGE_CUBE_ARRAY,
		RIT_IMAGE_2D_MULTISAMPLE,
		RIT_INT_IMAGE_2D_MULTISAMPLE,
		RIT_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
		RIT_IMAGE_2D_MULTISAMPLE_ARRAY,
		RIT_INT_IMAGE_2D_MULTISAMPLE_ARRAY,
		RIT_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,
		RIT_UNSIGNED_INT_ATOMIC_COUNTER,
		RIT_STRUCT,
		RIT_SIZE
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Image format qualifier internal type */
	enum class ImageInternalFormat
	{
		IIF_RGBA32F = 0,
		IIF_RGBA16F,
		IIF_RG32F,
		IIF_RG16F,
		IIF_R11F_G11F_B10F,
		IIF_R32F,
		IIF_R16F,
		IIF_RGBA16,
		IIF_RGB10_A2,
		IIF_RGBA8,
		IIF_RG16,
		IIF_RG8,
		IIF_R16,
		IIF_R8,
		IIF_RGBA16_SNORM,
		IIF_RGBA8_SNORM,
		IIF_RG16_SNORM,
		IIF_RG8_SNORM,
		IIF_R16_SNORM,
		IIF_R8_SNORM,
		IIF_RGBA32I,
		IIF_RGBA16I,
		IIF_RGBA8I,
		IIF_RG32I,
		IIF_RG16I,
		IIF_RG8I,
		IIF_R32I,
		IIF_R16I,
		IIF_R8I,
		IIF_RGBA32UI,
		IIF_RGBA16UI,
		IIF_RGB10_A2UI,
		IIF_RGBA8UI,
		IIF_RG32UI,
		IIF_RG16UI,
		IIF_RG8UI,
		IIF_R32UI,
		IIF_R16UI,
		IIF_R8UI,
		IIF_SIZE
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Returns the size in bytes of the ResourceInternalType given as parameter, which has to
	* be a data type, i.e. (ShaderReflection::isDataType return true) 
	* @param resourceInternalType [in] type to compute the size in bytes for
	* @return size in bytes of the ResourceInternalType given as parameter if is a data type,
	* and -1 otherwise */
	int getResourceInternalTypeSizeInBytes(ResourceInternalType resourceInternalType);

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Image access type */
	enum class ImageAccessType
	{
		IT_READ_ONLY = 0,
		IT_WRITE_ONLY,
		IT_READ_WRITE,
		IT_SIZE
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Manager notification type */
	enum class ManagerNotificationType
	{
		MNT_REMOVED = 0, //!< The resource in the notification has been removed from the manager
		MNT_ADDED,       //!< The resource in the notification has been added to the manager
		MNT_CHANGED,     //!< The resource in the notification has been modified
		MNT_SIZE         //!< Max enum value
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Enum class describing the type of generic resource. Note: the value of the enum is used
	* to sort them for resource updating */
	enum class GenericResourceType
	{
		GRT_UNDEFINED       = 0,
		GRT_TEXTURE         = 1,
		GRT_RENDERPASS      = 2,
		GRT_BUFFER          = 3,
		GRT_FRAMEBUFFER     = 4,
		GRT_SHADER          = 5,
		GRT_MATERIAL        = 6,
		GRT_RASTERTECHNIQUE = 7,
		GRT_UNIFORMBUFFER   = 8,
		GRT_IMAGE           = 9,
		GRT_SAMPLER         = 10,
		GRT_PIPELINE        = 11,
		GRT_SHADERSTRUCT    = 12,
		GRT_UNIFORMBASE     = 13,
		GRT_NODE            = 14,
		GRT_CAMERA          = 15,
		GRT_SIZE            = 16,
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Enum class describing the type of camera */
	enum class CameraType
	{
		CT_FIRST_PERSON = 0,
		CT_ARC_BALL     = 1,
		CT_SIZE         = 2,
	};

	/** Enum class for the frustum planes */
	enum class FrustumPlaneIndex
	{
		FPI_LEFT   = 0,
		FPI_RIGHT  = 1,
		FPI_TOP    = 2,
		FPI_BOTTOM = 3,
		FPI_BACK   = 4,
		FPI_FRONT  = 5,
		FPI_TOTAL  = 6
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
}

#endif _RESOURCE_ENUM_H_
