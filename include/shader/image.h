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

#ifndef _IMAGE_H_
#define _IMAGE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING
class Texture;
class GPUBuffer;

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////
/**
* This class is a wrapper for an OpenGL Image resource
*/
class Image : public GenericResource
{
public:
	/** Parameter constructor
	* @param id          [in] id of the image
	* @param name        [in] image name
	* @param imageType   [in] image type
	* @param shaderStage [in] enumerated with value the shader stage the uniform being built is used
	* @return nothing */
	Image(const int &id, string &&name, const ResourceInternalType &imageType, VkShaderStageFlagBits shaderStage);

	/** Sets the texture given as parameter as the one to be bound with this image (no compatibility tests are performed between the 
	* image and the texture). All the parameters to be used when reading / storing from to the texture are specified in this method taking
	* default read - only values from layer 0. The storing format taken is the attached texture's.
	* @param texture         [in] texture to set to this image
	* @param unit            [in] image unit where to bind the texture to
	* @param textureLevel    [in] texture level to bind
	* @param layered         [in] if true, then the entire array is attached to the image unit
	* @param textureLayer    [in] if layered is false, this value specifies the texture layer to be bound to the image unit (if layered is false, it's ignored)
	* @param imageAccessType [in] specifies the type of access that will be performed on the image
	* @return true if the texture was successfully set as the one used by this image, and false otherwise */
	bool setTextureToImage(Texture *texture, const int unit = 0, const int &textureLevel = 0, const bool &layered = false, const int &textureLayer = 0, const ImageAccessType &imageAccessType = ImageAccessType::IT_READ_ONLY);

	/** Sets the texture given as parameter as the one to be bound with this image, if the texture manager has a texture with this name
	* available. If it's not the case, the m_hasAssignedTexture flag will be used before rasterization for try to find a texture with name
	* given by m_textureToSampleName and set it to m_texture, setting the m_hasAssignedTexture flag as true
	* @param name            [in] name of the texture to use for this image (will be requested to the texture manager when calling the method, and, if not found, again before rasterization)
	* @param unit            [in] image unit where to bind the texture to
	* @param textureLevel    [in] texture level to bind
	* @param layered         [in] if true, then the entire array is attached to the image unit
	* @param textureLayer    [in] if layered is false, this value specifies the texture layer to be bound to the image unit (if layered is false, it's ignored)
	* @param imageAccessType [in] specifies the type of access that will be performed on the image
	* @return true if the texture was found and assigned to m_texture successfully, false otherwise */
	bool setTextureToImage(string &&name, const int unit = 0, const int &textureLevel = 0, const bool &layered = false, const int &textureLayer = 0, const ImageAccessType &imageAccessType = ImageAccessType::IT_READ_ONLY);

	/** Sets the gpu buffer given as parameter as the one to be bound with this image (no compatibility tests are performed between the
	* image and the gpu buffer's texture internal format). All the parameters to be used when reading / storing from to the texture are specified in this method taking
	* default read - only values from layer 0. The storing format taken is the attached gpu buffer's.
	* @param gpuBuffer       [in] gpu buffer to set to this image
	* @param unit            [in] image unit where to bind the gpu buffer to
	* @param imageAccessType [in] specifies the type of access that will be performed on the image
	* @return true if the gpu buffer was successfully set as the one used by this image, and false otherwise */
	bool setGPUBufferToImage(GPUBuffer *gpuBuffer, const int unit = 0, const ImageAccessType &imageAccessType = ImageAccessType::IT_READ_ONLY);

	/** Sets the gpu buffer given as parameter as the one to be bound with this image, if the gpu buffer manager has a gpu buffer with this name
	* available. If it's not the case, the m_hasAssignedGPUBuffer flag will be used before rasterization for try to find a gpu buffer with name
	* given by m_GPUBufferToSampleName and set it to m_GPUBbuffer, setting the m_hasAssignedGPUBuffer flag as true
	* @param name            [in] name of the gpu buffer to use for this image (will be requested to the gpu buffer manager when calling the method, and, if not found, again before rasterization)
	* @param unit            [in] image unit where to bind the gpu buffer to
	* @param imageAccessType [in] specifies the type of access that will be performed on the image
	* @return true if the gpu buffer was found and assigned to m_GPUBuffer successfully, false otherwise */
	bool setGPUBufferToImage(string &&name, const int unit = 0, const ImageAccessType &imageAccessType = ImageAccessType::IT_READ_ONLY);

	/** Default destructor
	* @return nothing */
	~Image();
	
	GET(int, m_id, Id)
	GET(int, m_unit, Unit)
	GET(ResourceInternalType, m_imageType, ResourceInternalType)
	GET(VkShaderStageFlagBits, m_shaderStage, ShaderStage)
	GET(bool, m_hasAssignedTexture, HasAssignedTexture)
	GET(bool, m_hasAssignedGPUBuffer, HasAssignedGPUBuffer)
	GET_PTR(Texture, m_texture, Texture)
	GET_PTR(GPUBuffer, m_gpuBuffer, GPUBuffer)
	GET_SET(int, m_textureLevel, TextureLevel)
	GET_SET(bool, m_layered, Layered)
	GET_SET(int, m_textureLayer, TextureLayer)
	GET_SET(ImageAccessType, m_accessType, AccessType)
	GET_SET(VkFormat, m_storingFormat, StoringFormat)
	GET(string, m_textureToSampleName, TextureToSampleName)
	GET(string, m_gpuBufferToSampleName, GPUBufferToSampleName)

protected:
	/** Simply resets the member variables of this class that are not provided by the constructor
	* @return nothing */
	void resetNonConstructorParameterValues();

	int                    m_id;                    //!< id of the image
	int			           m_unit;                  //!< Index of the image unit to bind the texture
	ResourceInternalType   m_imageType;             //!< image type
	VkShaderStageFlagBits  m_shaderStage;           //!< enumerated with value the shader stage the uniform being built is used
	bool                   m_hasAssignedTexture;    //!< If false, this image still doesn't have a proper value for the m_texture to sample from
	bool                   m_hasAssignedGPUBuffer;  //!< If false, this image still doesn't have a proper value for the m_gpuBuffer to sample from
	Texture*               m_texture;               //!< texture bound to this image sample (can be a texture or a gpu buffer)
	GPUBuffer*             m_gpuBuffer;             //!< gpu buffer bound to this image sample (can be a texture or a gpu buffer)
	int	                   m_textureLevel;          //!< Level of the texture that is to be bound
	bool                   m_layered;               //!< Whether a layered texture binding is to be established
	int                    m_textureLayer;          //!< If layered is GL_FALSE, specifies the layer of texture to be bound to the image unit. Ignored otherwise.
	ImageAccessType        m_accessType;            //!< Specifies the tpye of access that will be performed on the image
	VkFormat               m_storingFormat;         //!< specifies the format that elements of the image will be treated as for the purposes of formatted stores
	string                 m_textureToSampleName;   //!< name of the texture to sample (for cases when the texture is built after the initialization call of the shader containing this image, will be taken into account before rasterization)
	string                 m_gpuBufferToSampleName; //!< name of the gpu buffer to sample (for cases when the gpu buffer is built after the initialization call of the shader containing this image, will be taken into account before rasterization)
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _IMAGE_H_
