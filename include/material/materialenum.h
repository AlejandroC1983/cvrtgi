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

#ifndef _MATERIALENUM_H_
#define _MATERIALENUM_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES

// CLASS FORWARDING

// NAMESPACE

// DEFINES

namespace materialenum
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Enum to know which of the dynamic uniform buffers built by the raster manager are used by each material */
	enum MaterialBufferResource
	{
		MBR_NONE     = 1 << 0, //!< No element
		MBR_MODEL    = 1 << 1, //!< Scene per-model data
		MBR_CAMERA   = 1 << 2, //!< Scene camera information
		MBR_MATERIAL = 1 << 4  //!< Scene material data
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	inline MaterialBufferResource operator|(MaterialBufferResource a, MaterialBufferResource b)
	{
		return static_cast<MaterialBufferResource>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Enum to know which kind of surface is expected to be rendered with a material */
	enum MaterialSurfaceType
	{
		MST_OPAQUE = 0, //!< Opaque geometry
		MST_ALPHATESTED = 1, //!< Opaque, alpha tested geometry
		MST_ALPHABLENDED = 2, //!< Alpha blender geometry
		MST_SIZE = 3  //!< Enum max value
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
}

#endif _RESOURCEENUM_H_
