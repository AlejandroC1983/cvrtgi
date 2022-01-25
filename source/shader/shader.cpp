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
#include "../../include/shader/shader.h"
#include "../../include/core/coremanager.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/shader/sampler.h"
#include "../../include/shader/shaderstoragebuffer.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/util/containerutilities.h"
#include "../../include/shader/atomiccounterunit.h"
#include "../../include/shader/uniformBase.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(string &&name) : GenericResource(move(name), move(string("Shader")), GenericResourceType::GRT_SHADER)
	, m_isCompute(false)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader::~Shader()
{
	destroySamplers();
	deleteVectorInstances(m_vecUniformBase);
	deleteVectorInstances(m_vecDirtyUniform);
	deleteVectorInstances(m_vecAtomicCounterUnit);
	deleteVectorInstances(m_vecShaderStruct);
	deleteVectorInstances(m_vectorShaderStorageBuffer);

	destroyShaderStages();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Shader::init()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Shader::destroyShaderStages()
{
	forIT(m_arrayShaderStages)
	{
		vkDestroyShaderModule(coreM->getLogicalDevice(), it->module, NULL);
	}

	m_arrayShaderStages.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::initializeSamplerHandlers()
{
	bool result = true;

	forIT(m_vecTextureSampler)
	{
		if ((*it)->getTexture() == nullptr)
		{
			result = false;
		}

		(*it)->setSamplerHandle(textureM->buildSampler(0.0f, (float)((*it)->getTexture()->getMipMapLevels()), (*it)->getMipmapMode(), (*it)->getMinMagFilter()));
	}

	return result;
}
/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::initializeImageHandlers()
{
	bool result = true;

	forIT(m_vecImageSampler)
	{
		if ((*it)->getTexture() == nullptr)
		{
			result = false;
		}

		if ((*it)->getTexture() == nullptr)
		{
			cout << "ERROR no texture in Shader::initializeImageHandlers() for shader with name " << m_name << endl;
		}
		else
		{
			(*it)->setSamplerHandle(textureM->buildSampler(0.0f, (float)((*it)->getTexture()->getMipMapLevels()), (*it)->getMipmapMode(), (*it)->getMinMagFilter()));
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::setTextureToSample(string &&textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType)
{
	return setTextureToSample(move(textureSamplerName), move(textureResourceName), descriptorType, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_LINEAR);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::setTextureToSample(string &&textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter)
{
	// TODO: refactor and merge with setImageToSample
	forIT(m_vecTextureSampler)
	{
		if ((*it)->getName() == textureSamplerName)
		{
			(*it)->setTextureToSample(move(string(textureResourceName)));
			(*it)->setDescriptorType(descriptorType);
			(*it)->setMipmapMode(mipmapMode);
			(*it)->setMinMagFilter(minMagFilter);
			Texture* texture = textureM->getElement(move(textureResourceName));
			if (texture != nullptr)
			{
				(*it)->setTextureToSample(texture);
				(*it)->setReady(true);
				setReady(resourceIsReady());
			}
			else
			{
				(*it)->setReady(false);
				setReady(false);
			}
			return true;
		}
	}

	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::setImageToSample(string &&imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType)
{
	return setImageToSample(move(imageSamplerName), move(textureResourceName), descriptorType, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_LINEAR);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::setImageToSample(string &&imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter)
{
	// TODO: refactor and merge using the vector as parameter
	forIT(m_vecImageSampler)
	{
		if ((*it)->getName() == imageSamplerName)
		{
			(*it)->setTextureToSample(move(string(textureResourceName)));
			(*it)->setDescriptorType(descriptorType);
			(*it)->setMipmapMode(mipmapMode);
			(*it)->setMinMagFilter(minMagFilter);
			Texture* texture = textureM->getElement(move(textureResourceName));
			if (texture != nullptr)
			{
				(*it)->setTextureToSample(texture);
				(*it)->setReady(true);
				setReady(resourceIsReady());
			}
			else
			{
				(*it)->setReady(false);
				setReady(false);
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

const Sampler* Shader::getSamplerByName(string&& name) const
{
	forIT(m_vecTextureSampler)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::setShaderStorageBuffer(string &&storageBufferName, string&& bufferResourceName, VkDescriptorType descriptorType)
{
	// TODO: refactor and merge using the vector as parameter
	forIT(m_vectorShaderStorageBuffer)
	{
		if ((*it)->getName() == storageBufferName)
		{
			(*it)->setBufferName(move(bufferResourceName));
			(*it)->setDescriptorType(descriptorType);
			Buffer* buffer = bufferM->getElement(move(string(bufferResourceName)));
			if (buffer != nullptr)
			{
				(*it)->setBuffer(buffer);
				(*it)->setReady(true);
				setReady(resourceIsReady());
			}
			else
			{
				(*it)->setReady(false);
				setReady(false);
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Shader::destroySamplers()
{
	forI(m_vecTextureSampler.size())
	{
		if (m_vecTextureSampler[i]->getSamplerHandle() != VK_NULL_HANDLE)
		{
			vkDestroySampler(coreM->getLogicalDevice(), m_vecTextureSampler[i]->getSamplerHandle(), nullptr);
		}
	}

	forI(m_vecImageSampler.size())
	{
		if (m_vecImageSampler[i]->getSamplerHandle() != VK_NULL_HANDLE)
		{
			vkDestroySampler(coreM->getLogicalDevice(), m_vecImageSampler[i]->getSamplerHandle(), nullptr);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Shader::addHeaderSourceCode(string&& code)
{
	m_headerSourceCode += code;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::resourceIsReady()
{
	// TODO: refactor and merge using the vector as parameter
	uint maxIndex = uint(m_vecTextureSampler.size());

	forI(maxIndex)
	{
		if (!m_vecTextureSampler[i]->getReady())
		{
			setReady(false);
			return false;
		}
	}

	maxIndex = uint(m_vecImageSampler.size());
	forI(maxIndex)
	{
		if (!m_vecImageSampler[i]->getReady())
		{
			setReady(false);
			return false;
		}
	}

	maxIndex = uint(m_vectorShaderStorageBuffer.size());
	forI(maxIndex)
	{
		if (!m_vectorShaderStorageBuffer[i]->getReady())
		{
			setReady(false);
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::textureResourceNotification(string&& textureResourceName, ManagerNotificationType notificationType)
{
	bool resultTexture = textureResourceNotification(m_vecTextureSampler, move(string(textureResourceName)), notificationType);
	bool resultImage   = textureResourceNotification(m_vecImageSampler, move(textureResourceName), notificationType);

	return (resultTexture || resultImage);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::textureResourceNotification(vectorSamplerPtr& vectorResource, string&& textureResourceName, ManagerNotificationType notificationType)
{
	// TODO: find a way to refector with bufferResourceNotification
	
	bool result = false;
	bool builtCondition;

	switch (notificationType)
	{
		case ManagerNotificationType::MNT_ADDED:
		{
			builtCondition = false;
			break;
		}
		case ManagerNotificationType::MNT_REMOVED:
		{
			builtCondition = true;
			break;
		}
	}

	uint maxIndex = uint(vectorResource.size());
	Texture* texture;
	forI(maxIndex)
	{
		texture = vectorResource[i]->refTexture();

		if ((texture != nullptr) &&
			(vectorResource[i]->getReady() == builtCondition) &&
			(texture->getName() == textureResourceName))
		{
			switch (notificationType)
			{
				case ManagerNotificationType::MNT_ADDED:
				{
					vectorResource[i]->setTextureToSample(texture);
					vectorResource[i]->setReady(true);
					setReady(resourceIsReady());
					break;
				}
				case ManagerNotificationType::MNT_REMOVED:
				{
					vectorResource[i]->setTextureToSample(nullptr);
					vectorResource[i]->setReady(false);
					setReady(false);
					break;
				}
			}
			result = true;
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Shader::bufferResourceNotification(string&& bufferResourceName, ManagerNotificationType notificationType)
{
	// TODO: find a way to refector with textureResourceNotification

	bool result = false;
	bool builtCondition;

	switch (notificationType)
	{
		case ManagerNotificationType::MNT_ADDED:
		{
			builtCondition = false;
			break;
		}
		case ManagerNotificationType::MNT_CHANGED:
		case ManagerNotificationType::MNT_REMOVED:
		{
			builtCondition = true;
			break;
		}
		default:
		{
			builtCondition = false;
			break;
		}
	}

	uint maxIndex = uint(m_vectorShaderStorageBuffer.size());

	Buffer* buffer;
	forI(maxIndex)
	{
		buffer = m_vectorShaderStorageBuffer[i]->refBuffer();

		if ((buffer != nullptr) &&
			(m_vectorShaderStorageBuffer[i]->getReady() == builtCondition) &&
			(buffer->getName() == bufferResourceName))
		{
			switch (notificationType)
			{
				case ManagerNotificationType::MNT_ADDED:
				case ManagerNotificationType::MNT_CHANGED:
				{
					m_vectorShaderStorageBuffer[i]->setBuffer(buffer);
					m_vectorShaderStorageBuffer[i]->setReady(true);
					setReady(resourceIsReady());
					break;
				}
				case ManagerNotificationType::MNT_REMOVED:
				{
					m_vectorShaderStorageBuffer[i]->setBuffer(nullptr);
					m_vectorShaderStorageBuffer[i]->setReady(false);
					setReady(false);
					break;
				}
			}
			
			result = true;
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Shader::addShaderHeaderSourceCode(MaterialSurfaceType surfaceType)
{
	if (surfaceType == MaterialSurfaceType::MST_ALPHATESTED)
	{
		addHeaderSourceCode(move(string("#define MATERIAL_TYPE_ALPHATESTED 1\n")));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
