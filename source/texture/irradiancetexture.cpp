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
#include "../../include/texture/irradiancetexture.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

IrradianceTexture::IrradianceTexture()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

IrradianceTexture::IrradianceTexture(string&& path)
{
	ifstream file(path, ios::in | ios::binary);

	file.read((char*)(&m_width),  sizeof(uint));
	file.read((char*)(&m_height), sizeof(uint));
	file.read((char*)(&m_depth),  sizeof(uint));

	uint i;
	m_vectorDistance.resize(m_depth);
	for (i = 0; i < m_depth; ++i)
	{
		file.read((char*)(&m_vectorDistance[i]), sizeof(float));
	}

	m_size = 0;
	m_vectorTextureSize.resize(m_depth);
	for (i = 0; i < m_depth; ++i)
	{
		file.read((char*)(&m_vectorTextureSize[i]), sizeof(uint));
		m_size += m_vectorTextureSize[i];
	}

	m_vectorTextureData.resize(m_depth);
	for (i = 0; i < m_depth; ++i)
	{
		m_vectorTextureData[i].resize(m_vectorTextureSize[i]);
		file.read((char*)(m_vectorTextureData[i].data()), sizeof(float) * m_vectorTextureSize[i]);
	}

	for (i = 0; i < m_depth; ++i)
	{
		m_vectorTextureFullData.insert(m_vectorTextureFullData.end(), m_vectorTextureData[i].begin(), m_vectorTextureData[i].end());
	}

	m_size *= sizeof(float);

	file.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////
