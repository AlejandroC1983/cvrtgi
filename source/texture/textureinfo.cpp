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
#include "../../include/texture/textureinfo.h"
#include "../../include/texture/irradiancetexture.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

TextureInfo::TextureInfo() :
	m_data(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

TextureInfo::TextureInfo(gli::texture2D* texture)
{
	m_size                        = uint32_t(texture->size());
	m_data                        = texture->data();
	const uint32_t numMipMapLevel = uint32_t(texture->levels());

	for (uint32_t i = 0; i < numMipMapLevel; ++i)
	{
		m_vectorMipMap.push_back(TextureMipMapInfo(uint32_t((*texture)[i].dimensions().x),
			                                       uint32_t((*texture)[i].dimensions().y),
			                                       uint32_t((*texture)[i].dimensions().z),
			                                       uint32_t((*texture)[i].size())));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

TextureInfo::TextureInfo(IrradianceTexture* irradianceTexture)
{
	m_size                        = uint32_t(irradianceTexture->getSize());
	m_data                        = (void*)(irradianceTexture->getVectorTextureFullData().data());
	const uint32_t numMipMapLevel = 1;

	for (uint32_t i = 0; i < numMipMapLevel; ++i)
	{
		m_vectorMipMap.push_back(TextureMipMapInfo(uint32_t(irradianceTexture->getWidth()),
			                                       uint32_t(irradianceTexture->getHeight()),
			                                       uint32_t(irradianceTexture->getDepth()),
			                                       uint32_t(irradianceTexture->getSize())));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
