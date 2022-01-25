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

#ifndef _IRRADIANCETEXTURE_H_
#define _IRRADIANCETEXTURE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** Simple helper to abstract texture information from the library / resource used to load it */
class IrradianceTexture
{
public:
	/** Default constructor
	* @return nothing */
	IrradianceTexture();

	/** Parameter constructor
	* @param path [in] path to the .irrt texture
	* @return nothing */
	IrradianceTexture(string&& path);

	GETCOPY(uint, m_width, Width)
	GETCOPY(uint, m_height, Height)
	GETCOPY(uint, m_depth, Depth)
	GETCOPY(uint, m_size, Size)
	GET(vector<float>, m_vectorDistance, VectorDistance)
	GET(vector<uint>, m_vectorTextureSize, VectorTextureSize)
	GET(vector<float>, m_vectorTextureFullData, VectorTextureFullData)

protected:
	uint                  m_width;                 //!< Texture width
	uint                  m_height;                //!< Texture height
	uint                  m_depth;                 //!< Texture depth (number of irradiance layers computed and placed together in this texture)
	uint                  m_size;                  //!< Size in bytes of m_vectorTextureFullData
	vector<float>         m_vectorDistance;        //!< Irradiance computatoin distance associated to each one of the textures present in m_vectorTextureFullData
	vector<uint>          m_vectorTextureSize;     //!< Size of each one of the textures in the m_vectorTextureFullData vector
	vector<float>         m_vectorTextureFullData; //!< Full texture data (all irradiance textures consecutively)
	vector<vector<float>> m_vectorTextureData;     //!< Texture data (all irradiance textures consecutively)
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _IRRADIANCETEXTURE_H_
