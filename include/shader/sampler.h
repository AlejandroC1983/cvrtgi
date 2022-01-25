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

#ifndef _SAMPLER_H_
#define _SAMPLER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING
class Texture;

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Sampler : public GenericResource
{
public:
	/** Parameter constructor for texture samplers
	* @param name         [in] texture sampler name
	* @param samplerType  [in] texture sampler type
	* @param shaderStage  [in] enumerated with value the shader stage the uniform being built is used
	* @param bindingIndex [in] binding index of this uniform buffer for the shader it's owned by
	* @param setIndex     [in] set index inside the binding index given by bindingIndex of this uniform buffer for the shader it's owned by
	* @return nothing */
	Sampler(string &&name, const ResourceInternalType &samplerType, VkShaderStageFlagBits shaderStage, int bindingIndex, int setIndex);

	/** Parameter constructor for image samplers
	* @param name         [in] image sampler name
	* @param samplerType  [in] image sampler type
	* @param shaderStage  [in] enumerated with value the shader stage the uniform being built is used
	* @param bindingIndex [in] binding index of this uniform buffer for the shader it's owned by
	* @param setIndex     [in] set index inside the binding index given by bindingIndex of this uniform buffer for the shader it's owned by
	* @param format       [in] format used in the shader for the image this sampler represents
	* @return nothing */
	Sampler(string &&name, const ResourceInternalType &samplerType, VkShaderStageFlagBits shaderStage, int bindingIndex, int setIndex, ResourceInternalType samplerFormat);

	/** Sets the texture given as parameter as the one to be bound with this sampler, if the texture type and sampler type are compatible
	* @param texture [in] texture to set to this sampler
	* @return true if the texture was successfully set as the one used by this sampler, and false otherwise */
	bool setTextureToSample(Texture *texture);

	/** Sets the texture given as parameter as the one to be bound with this sampler, if the texture type and sampler type are compatible, if
	* the texture manager has a texture with this name available. If it's not the case, the m_hasAssignedTexture flag will be used
	* before rasterization for try to find a texture with name given by m_textureToSampleName and set it to m_texture, setting the
	* m_hasAssignedTexture flag as true
	* @param name [in] name of the texture to use for sampling (will be requested to the texture manager when calling the method, and, if not found, again before rasterization)
	* @return true if the texture was found and assigned to m_texture successfully (sampler and texture may not be compatible), and false otherwise */
	bool setTextureToSample(string &&name);

	/** Default destructor
	*@return nothing */
	~Sampler();
	
	GETCOPY_SET(VkSampler, m_samplerHandle, SamplerHandle)
	GET(ResourceInternalType, m_samplerType, SamplerType)
	GET(VkShaderStageFlagBits, m_shaderStage, ShaderStage)
	GETCOPY(int, m_bindingIndex, BindingIndex)
	GETCOPY(int, m_setIndex, SetIndex)
	GET(bool, m_hasAssignedTexture, HasAssignedTexture)
	GET_PTR(Texture, m_texture, Texture)
	REF_PTR(Texture, m_texture, Texture)
	GET_SET(string, m_textureToSampleName, TextureToSampleName)
	GET(bool, m_isImageSampler, IsImageSampler)
	GETCOPY_SET(VkDescriptorType, m_descriptorType, DescriptorType)
	GETCOPY_SET(VkSamplerMipmapMode, m_mipmapMode, MipmapMode)
	GETCOPY_SET(VkFilter, m_minMagFilter, MinMagFilter)

protected:
	VkSampler             m_samplerHandle;       //!< Handle to the sampler
	ResourceInternalType  m_samplerType;         //!< sampler type
	VkShaderStageFlagBits m_shaderStage;         //!< enumerated with value the shader stage the uniform being built is used
	bool                  m_hasAssignedTexture;  //!< If false, this sampler still doesn't have a proper value for the m_texture to sample from
	int                   m_bindingIndex;        //!< Binding index for this uniform buffer in the shader its owned by
	int                   m_setIndex;            //!< Set index inside m_bindingIndex for this uniform buffer in the shader its owned by
	Texture*              m_texture;             //!< texture bound to this sampler
	string                m_textureToSampleName; //!< name of the texture to sample (for cases when the texture is built after the initialization call of the shader containing this sampler, will be taken into account before rasterization)
	bool                  m_isImageSampler;      //!< True if this is an image sampler, false if it's a texture sampler
	ResourceInternalType  m_samplerFormat;       //!< If m_isImageSampler is true, then this is the image sampler format
	VkDescriptorType      m_descriptorType;      //!< Flags for the descriptor set built for this sampler
	VkSamplerMipmapMode   m_mipmapMode;          //!< Mip map mode for the sampler
	VkFilter              m_minMagFilter;        //!< Flag for sampler minification / magnification filter
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SAMPLER_H_
