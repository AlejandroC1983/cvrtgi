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

#ifndef _SHADER_H_
#define _SHADER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../Headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/shader/pushconstant.h"
#include "../../include/material/materialenum.h"

// CLASS FORWARDING
class Pipeline;
class ShaderStruct;
class Buffer;

// NAMESPACE
using namespace materialenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Shader : public GenericResource
{
	friend class ShaderManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	Shader(string &&name);

	/** Destructor
	* @return nothing */
	virtual ~Shader();

public:
	/** Shader initialization
	* @return nothing */
	void init();

	// Kill the shader when not required
	void destroyShaderStages();

	/** Initializes the texture samplers present in m_vecSampler
	* @return return false if any of the texture samplers didn't have a Texture assigned, and true otherwise */
	bool initializeSamplerHandlers();

	/** Initializes the image samplers present in m_vecSampler
	* @return return false if any of the image samplers didn't have a Texture assigned, and true otherwise */
	bool initializeImageHandlers();

	/** Looks for a texture sampler with name given as parameter and assigns the texture given as parameter to it
	* (default mip mapping type is VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR)
	* @param textureSamplerName  [in] name of the texture sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @return true if texture sampler found and texture not nullptr, false otherwise */
	bool setTextureToSample(string &&textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType);

	/** Looks for a texture sampler with name given as parameter and assigns the texture given as parameter to it
	* @param textureSamplerName  [in] name of the texture sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @param mipmapMode          [in] type of mip mapping to perform for this sampler
	* @param minMagFilter        [in] flag for sampler minification / magnification filter
	* @return true if texture sampler found and texture not nullptr, false otherwise */
	bool setTextureToSample(string &&textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter);

	/** Looks for an image sampler with name given as parameter and assigns the texture given as parameter to it
	* @param imageSamplerName    [in] name of the image sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @param mipmapMode          [in] type of mip mapping to perform for this sampler
	* @param minMagFilter        [in] flag for sampler minification / magnification filter
	* @return true if image sampler found and texture not nullptr, false otherwise */
	bool setImageToSample(string &&imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType);

	/** Looks for an image sampler with name given as parameter and assigns the texture given as parameter to it
	* @param imageSamplerName    [in] name of the image sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @return true if image sampler found and texture not nullptr, false otherwise */
	bool setImageToSample(string &&imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter);

	/** Returns a pointer to the sampler with name given by the name parameter
	* @param name [in] name of the sampler to look for
	* @return true found, false otherwise */
	const Sampler* getSamplerByName(string&& name) const;

	/** Looks for a shader storage buffer with name given as parameter and assigns the buffer given as parameter to it
	* @param storageBufferName  [in] shader storage buffer to look for
	* @param bufferResourceName [in] name of the buffer resource to assign to the ssbo with name storageBufferName
	* @param descriptorType     [in] flags for the descriptor set to build
	* @return true if a shader storage buffer with the given name was found and storageBuffer not nullptr, false otherwise */
	bool setShaderStorageBuffer(string &&storageBufferName, string&& bufferResourceName, VkDescriptorType descriptorType);

	/** Destroy Vulkan resources present in m_vecTextureSampler and m_vecImageSampler
	* @return nothing */
	void destroySamplers();

	/** Add source code given as parameter to m_headerSourceCode to be added to each shader before compiling it
	* @param code [in] source code to add
	* @return nothing */
	void addHeaderSourceCode(string&& code);

	GETCOPY(bool, m_isCompute, IsCompute)
	GET(vector<VkPipelineShaderStageCreateInfo>, m_arrayShaderStages, ArrayShaderStages)
	REF(vector<VkPipelineShaderStageCreateInfo>, m_arrayShaderStages, ArrayShaderStages)
	GET(vectorSamplerPtr, m_vecTextureSampler, VecTextureSampler)
	GET(vectorSamplerPtr, m_vecImageSampler, VecImageSampler)
	REF(vectorUniformBasePtr, m_vecDirtyUniform, VecDirtyUniform)
	GET(vectorUniformBasePtr, m_vecUniformBase, VecUniformBase)
	GET(vectorAtomicCounterUnitPtr, m_vecAtomicCounterUnit, VecAtomicCounterUnit)
	GET(vectorShaderStorageBufferPtr, m_vectorShaderStorageBuffer, VectorShaderStorageBuffer)
	REF(PushConstant, m_pushConstant, PushConstant)
	GET(string, m_headerSourceCode, HeaderSourceCode)

protected:
	/** Tests if all the current resources used by the shader are available, making it ready for use
	* @return true if ready, false otherwise */
	bool resourceIsReady();

	/** To receive texture resource notifications from manager
	*@param vectorResource[in] vector with the sampler pointers to test
	* @param textureResourceName[in] resource name in the notification
	* @param notificationType[in] enum describing the type of notification
	* @return true if there was a resource affected by the otification, and false otherwise */
	bool textureResourceNotification(string&& textureResourceName, ManagerNotificationType notificationType);

	/** Tests if the resource with name given by textureResourceName is used in the sampler pointer vector given as parameter,
	* setting the sampler's properties appropiatedly if found, according with the notification type given by notificationType
	* @param vectorResource      [in] vector with the sampler pointers to test
	* @param textureResourceName [in] resource name in the notification
	* @param notificationType    [in] enum describing the type of notification
	* @return true if there was a resource affected by the notification, and false otherwise */
	bool textureResourceNotification(vectorSamplerPtr& vectorResource, string&& textureResourceName, ManagerNotificationType notificationType);

	/** Tests if the resource with name given by bufferResourceName is used in m_vectorShaderStorageBuffer,
	* setting the buffer's properties appropiatedly if found, according with the notification type given by notificationType
	* @param bufferResourceName [in] resource name in the notification
	* @param notificationType   [in] enum describing the type of notification
	* @return true if there was a resource affected by the notification, and false otherwise */
	bool bufferResourceNotification(string&& bufferResourceName, ManagerNotificationType notificationType);

	/** Add to m_shader::m_headerSourceCode all the shader header flags as source code that have been specified for this material, like
	* for instance based on the material type (in case it is of type MaterialSurfaceType::MST_ALPHATESTED or MaterialSurfaceType::MST_ALPHABLENDED)
	* @param surfaceType [in] material surface type to add flags for this shader
	* @return nothing */
	void addShaderHeaderSourceCode(MaterialSurfaceType surfaceType);

	bool                                    m_isCompute;                 //!< True if this is a compute shader
	vector<VkPipelineShaderStageCreateInfo> m_arrayShaderStages;         //!< Built shader stages for this shader, done at shader building time
	vectorUniformBasePtr                    m_vecUniformBase;            //!< Contains all the uniforms of this shader
	vectorUniformBasePtr                    m_vecDirtyUniform;           //!< Contains pointer to all the elements in m_vecUniformBase whose value has been modified since the last frame
	vectorSamplerPtr                        m_vecTextureSampler;         //!< Contains all the texture samplers that this shader has (each sampler has a texture it uses to bind and sample in the shader)
	vectorSamplerPtr                        m_vecImageSampler;           //!< Contains all the image samplers that this shader has (each image has a texture it uses to bind and sample from and / or write to in the shader)
	vectorAtomicCounterUnitPtr              m_vecAtomicCounterUnit;      //!< Contains all the atomic counter units that this shader has (each atomic counter unit has an atomic counter it uses to bind to)
	vectorShaderStructPtr                   m_vecShaderStruct;           //!< Contains with all the uniform buffer data of this shader (name of the variable, name of the struct of the variable, binding and set indexes)
	vectorShaderStorageBufferPtr            m_vectorShaderStorageBuffer; //!< Contains all the storage buffer data of this shader
	PushConstant                            m_pushConstant;              //!< Push constant for this shader (if any)
	string                                  m_headerSourceCode;          //!< This string contains source code added at the top of each sahder (vertex / geometry / fragment / compute) when compiling it. It allows per-shader values against ShaderManager::m_globalHeaderSourceCode
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SHADER_H_
