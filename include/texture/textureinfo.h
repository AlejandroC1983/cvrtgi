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

#ifndef _TEXTUREINFO_H_
#define _TEXTUREINFO_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"
#include "../headers.h"

// CLASS FORWARDING
class IrradianceTexture;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** Simple helper to abstract texture mip map level information from the library / resource used to load it */
struct TextureMipMapInfo
{
	/** Default constructor
	* @return nothing */
	TextureMipMapInfo()
	{

	}

	TextureMipMapInfo(uint32_t width, uint32_t height, uint32_t depth, uint32_t size):
		  m_width(width)
		, m_height(height)
		, m_depth(depth)
		, m_size(size)
	{

	}

	uint32_t m_width;  //!< Texture width
	uint32_t m_height; //!< Texture height
	uint32_t m_depth;  //!< Texture depth
	uint32_t m_size;   //!< Texture mip map level size
};

/////////////////////////////////////////////////////////////////////////////////////////////

/** Simple helper to abstract texture information from the library / resource used to load it */
class TextureInfo
{
public:
	/** Default constructor
	* @return nothing */
	TextureInfo();

	/** Parameter constructor
	* @param texture [in] gli texture to get information from
	* @return nothing */
	TextureInfo(gli::texture2D* texture);

	/** Parameter constructor
	* @param irradianceTexture [in] .irrt texture to get information from
	* @return nothing */
	TextureInfo(IrradianceTexture* irradianceTexture);

	REF_PTR(void, m_data, Data)
	GETCOPY(uint32_t, m_size, Size)
	GET(vector<TextureMipMapInfo>, m_vectorMipMap, VectorMipMap)

protected:
	void*                     m_data;         //!< Texture data
	uint32_t                  m_size;         //!< Texture whole size (all mipmap data together)
	vector<TextureMipMapInfo> m_vectorMipMap; //!< Vector with the number of mip map levels 
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _TEXTUREINFO_H_
