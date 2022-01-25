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

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/shader/resourceenum.h"
#include "../../include/material/exposedstructfield.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/material/materialenum.h"
#include "../../include/util/io.h"

// CLASS FORWARDING
class Shader;
class Buffer;

// NAMESPACE
using namespace resourceenum;
using namespace materialenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Material : public GenericResource
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] material instance name
	* @param [in] material class name
	* @return nothing */
	Material(string &&name, string &&className);

public:
	/** Destructor
	* @return nothing */
	virtual ~Material();

	/** Initialization method
	* @return nothing */
	void init();

	/** Will match the exposed resources with those present in the shader, the call to exposeResources needs to be done
	* before caling this method, and loading the corresponding shader as well
	* @return true if all elements in m_vectorExposedStructField were exposed successfully and false otherwise */
	bool matchExposedResources();

	/** Will unmatch all exposed resources with those present in the shader
	* @return nothing */
	void unmatchExposedResources();

	/** Builds the pipeline for this material, taking into account the exposed resources. Calls to exposeResources,
	* matchExposedResources and load the shader need to be done before calling this method
	* @return nothing */
	void buildPipeline();

	/** Called every frame, here all frame and time dependant variables in the material can be updated
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void update(float dt);

	/** Called every frame, here all exposed resource values are tested against those in the shader this material has, if
	* there are different values, then the values in the shader equivalent are updated and the material buffer is set as dirty
	* @return nothing */
	void updateExposedResources();

	/** Called from used on per-shader needs, will update the values of the push constant CPU buffer with the values of
	* the push constant exposed resources if the values of the exposed resources have changed
	* @return nothing */
	void updatePushConstantExposedResources();

	/** Loads the shader for the corresponding material class inheriting from this one. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	virtual bool loadShader();

	/** Buidls the resources used by this material
	* @return true if the shader was loaded successfully, and false otherwise */
	void buildMaterialResources();

	/** Load here all the resources needed for this material (textures and the like)
	* @return nothing */
	virtual void loadResources();

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	virtual void exposeResources();

	/** Set in any inherited class the information present in the m_pipelineData member variable
	* @return nothing */
	virtual void setupPipelineData();

	/** Exposes a member variable of this material to be linked and updated with a struct field variable in the shader this
	* material is attached to.
	* @param internalType    [in] resource internal type
	* @param data            [in] void pointer to the member variable to expose
	* @param structName      [in] name of the struct in the shader this member variable is going to be exposed
	* @param structFieldName [in] name of the struct member field the member variable of this class is going to be linked with
	* @return true if the element was found in the shader resources, and false otherwise */
	bool exposeStructField(ResourceInternalType internalType, void* data, string&& structName, string&& structFieldName);

	/** Exposes a member variable of this material to be linked and updated with a push constant struct field variable
	* (i.e. element in Shader::m_pushConstant::m_vecUniformBase) in the shader this material is attached to.
	* @param internalType    [in] resource internal type
	* @param data            [in] void pointer to the member variable to expose
	* @param structName      [in] name of the struct in the psuh constant shader's vector of push constant struct elements this member variable is going to be exposed
	* @param structFieldName [in] name of the struct member field the member variable of this class is going to be linked with (matching the name of an element in the push constant struct of the shader)
	* @return true if the element was found in the shader's push constant resources (i.e. in Shader::m_pushConstant::m_vecUniformBase), and false otherwise */
	bool pushConstantExposeStructField(ResourceInternalType internalType, void* data, string&& structName, string&& structFieldName);

	/** Assigns the texture given as parameter to the texture sampler whose name is given as parameter, that needs to be present in the
	* shader this material is linked with. Default mip mapping type is VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR.
	* @param textureSamplerName  [in] name of the texture sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @return true if a texture sampler was found in the shader resources, and false otherwise */
	bool assignTextureToSampler(string&& textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType);

	/** Assigns the texture given as parameter to the texture sampler whose name is given as parameter, that needs to be present in the
	* shader this material is linked with
	* @param textureSamplerName  [in] name of the texture sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @param mipmapMode          [in] type of mip mapping to perform for this sampler
	* @param minMagFilter        [in] flag for sampler minification / magnification filter
	* @return true if a texture sampler was found in the shader resources, and false otherwise */
	bool assignTextureToSampler(string&& textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter);

	/** Assigns the texture given as parameter to the image sampler whose name is given as parameter, that needs to be present in the
	* shader this material is linked with
	* @param imageSamplerName    [in] name of the image sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @return true if an image sampler was found in the shader resources, and false otherwise */
	bool assignImageToSampler(string&& imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType);

	/** Assigns the texture given as parameter to the image sampler whose name is given as parameter, that needs to be present in the
	* shader this material is linked with
	* @param imageSamplerName    [in] name of the image sampler to link the texture given as parameter with
	* @param textureResourceName [in] name of the texture resource to link with the image sampler of name imageSamplerName
	* @param descriptorType      [in] flags for the descriptor set to build for the sampler this texture is assigned to (a texture can be assigned to different samplers with different purposes)
	* @param mipmapMode          [in] type of mip mapping to perform for this sampler
	* @param minMagFilter        [in] flag for sampler minification / magnification filter
	* @return true if an image sampler was found in the shader resources, and false otherwise */
	bool assignImageToSampler(string&& imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter);

	/** Assigns the shader storage buffer given as parameter to the shader storage buffer data whose name is given as parameter, that needs to be present in the
	* shader this material is linked with
	* @param storageBufferName  [in] shader storage buffer to link with a shader storage buffer data with name storageBufferName
	* @param bufferResourceName [in] name of the buffer resource to assign to the ssbo with name storageBufferName
	* @param descriptorType     [in] flags for the descriptor set to build for the this shader storage bufer
	* @return true shader storage buffer data was found in the shader resources, and false otherwise */
	bool assignShaderStorageBuffer(string &&storageBufferName, string&& bufferResourceName, VkDescriptorType descriptorType);

	/** Computes the size in bytes of the exposed struct fields for the vector given as parameter
	* @param vectorExposed [in] vector with the exposed struct fields to compute the size in bytes
	* @return size in bytes of the exposed strcut fields present in vectorExposed */
	static int computeExposedStructFieldSize(const vector<ExposedStructField>& vectorExposed);

	/** Writes the exposed data to the material uniform buffer which has all the information for
	* each material used in the scene
	* @return nothing */
	void writeExposedDataToMaterialUB() const;

	/** Writes the push constant exposed data to the push constant cpu buffer in the shader 
	* (i.e. Shader::m_pushConstant::m_CPUBuffer::m_UBHostMemory)
	* @return nothing */
	void pushConstantWriteExposedDataToCPUBuffer() const;

	/** Called in the class destructor, destroys the elements in m_arrayDescriptorSetLayout
	* @return nothing */
	void destroyDescriptorLayout();
	
	/** Called in the class destructor, destroys m_pipelineLayout
	* @return nothing */
	void destroyPipelineLayouts();
	
	/** Called in the class destructor, destroys the elements in m_vectorTextureDescriptorPool
	* @return nothing */
	void destroyDescriptorPool();

	/** Called in the class destructor, destroys the elements in m_vectorTextureDescriptorSet
	* @return nothing */
	void destroyDescriptorSet();

	/** Binds the samplers for textures, with the data present in m_vectorTextureDescriptorSet and Shader::m_vecTextureSampler
	* to the command buffer given as argument 
	* @param commandBuffer [in] command buffer to record to this bind operations
	* @return nothing */
	void bindTextureSamplers(VkCommandBuffer* commandBuffer) const;

	/** Binds the samplers for images, with the data present in m_vectorImageDescriptorSet and Shader::m_vecImageSampler
	* to the command buffer given as argument
	* @param commandBuffer [in] command buffer to record to this bind operations
	* @return nothing */
	void bindImageSamplers(VkCommandBuffer* commandBuffer) const;

	/** Binds the shader storage buffers, with the data present in Shader::m_vectorShaderStorageBuffer
	* to the command buffer given as argument
	* @param commandBuffer [in] command buffer to record to this bind operations
	* @return nothing */
	void bindShaderStorageBuffers(VkCommandBuffer* commandBuffer) const;

	/** Updates the content of the shader's push constant CPU buffer copy
	* (i.e. Shader::m_pushConstant::m_CPUBuffer::m_UBHostMemory) with the updated values of the variables
	* exposed as push contant
	* @return nothing */
	void updatePushConstantCPUBuffer();

	/** Look in m_vectorPushConstantExposedStructField for an element with the data with internal type, struct name and
	* struct field name as the ones given by parameter and return its index
	* @param internalType    [in] exposed push constant's internal type
	* @param structName      [in] exposed push constant's struct name
	* @param structFieldName [in] exposed push constant's struct field name
	* @return nothing */
	int getPushConstantExposedResourceIndex(ResourceInternalType internalType, string structName, string structFieldName) const;

	/** Destroys Vulkan resources m_descriptorSet, m_descriptorPool and m_descriptorSetLayout
	* @return nothing */
	void destroyDescriptorSetResource();

	/** Destroy the allocated resources for this material
	* @return nothing */
	void destroyMaterialResources();

	/** Destroys Vulkan Pipeline::m_pipeline
	* @return nothing */
	void destroyPipelineResource();

	GET_PTR(Shader, m_shader, Shader)
	REF_PTR(Shader, m_shader, Shader)
	GET(string, m_shaderResourceName, ShaderResourceName)
	GETCOPY(VkPipelineLayout, m_pipelineLayout, PipelineLayout)
	GET_RETURN_PTR(Pipeline, m_pipeline, Pipeline)
	REF_RETURN_PTR(Pipeline, m_pipeline, Pipeline)
	GET(vector<VkDescriptorSetLayout>, m_arrayDescriptorSetLayout, ArrayDescriptorSetLayout)
	GET(vector<VkDescriptorSet>, m_vectorTextureDescriptorSet, VectorTextureDescriptorSet)
	GET(vector<VkDescriptorSet>, m_vectorImageDescriptorSet, VectorImageDescriptorSet)
	GETCOPY_SET(bool, m_exposedStructFieldDirty, ExposedStructFieldDirty)
	GETCOPY_SET(bool, m_pushConstantExposedStructFieldDirty, PushConstantExposedStructFieldDirty)
	GETCOPY(int, m_exposedStructFieldSize, ExposedStructFieldSize)
	GETCOPY(int, m_pushConstantExposedStructFieldSize, PushConstantExposedStructFieldSize)
	GETCOPY_SET(int, m_materialUniformBufferIndex, MaterialUniformBufferIndex)
	GET_SET(vector<VkClearValue>, m_vectorClearValue, VectorClearValue)
	REF(vector<VkClearValue>, m_vectorClearValue, VectorClearValue)
	GETCOPY(bool, m_isCompute, IsCompute)
	GETCOPY(bool, m_isEmitter, IsEmitter)
	GETCOPY(MaterialBufferResource, m_resourcesUsed, ResourcesUsed)
	REF(VkDescriptorSet, m_descriptorSet, DescriptorSet)
	GETCOPY(uint, m_materialInstanceIndex, MaterialInstanceIndex)
	GETCOPY_SET(MaterialSurfaceType, m_materialSurfaceType, MaterialSurfaceType)

protected:
	/** To track changes in the shader manager, in case those changes affect the shader used in this material
	* @param shaderResourceName [in] name of the shader modified
	* @param notificationType   [in] type of notification coming from the manager
	* @return true if the notification has affected resources used by this material, false otherwise */
	bool shaderResourceNotification(string&& shaderResourceName, ManagerNotificationType notificationType);

	/** Returns the number of dynamic uniform buffers used by the material. This buffers are generated by the raster manager
	* @return number of dynamic uniform buffers used by the material */
	uint getNumDynamicUniformBufferResourceUsed();

	Shader*                       m_shader;                                    //!< Shader used by this material
	string                        m_shaderResourceName;                        //!< Name of the shader resource present in m_shader (for the material class to know about m_shader changes)
	VkPipelineLayout              m_pipelineLayout;                            //!< Pipeline layout used, basically all the descriptor layouts used in the shader
	bool                          m_exposedStructFieldDirty;                   //!< True if any element end ups in m_vectorDirtyExposedStructField during the material update step
	bool                          m_pushConstantExposedStructFieldDirty;       //!< True if any element end ups in m_vectorPushConstantDirtyExposedStructField
	int                           m_exposedStructFieldSize;                    //!< Size in bytes of the exposed struct fields of this material
	int                           m_pushConstantExposedStructFieldSize;        //!< Size in bytes of the exposed struct fields in the psuh constant struct, if any, for this material
	int							  m_materialUniformBufferIndex;                //!< Index in the uniform buffer which has all the information about each material in the scene, this is where the material will write updated exposed values
	Pipeline                      m_pipeline;                                  //!< Material pipeline
	vector<VkDescriptorSetLayout> m_arrayDescriptorSetLayout;                  //!< Vector of descriptor set layous, the forst two of them are the scene transform and the material uniform buffers, and then all samplers used in the material
	vector<bool>                  m_arrayDescriptorSetLayoutIsShared;          //!< Vector to know if the descriptor set layout elements present in the equivalent index of m_arrayDescriptorSetLayout are shared or owned by this material
	vector<VkDescriptorSet>       m_vectorTextureDescriptorSet;                //!< Vector of the texture descriptor sets of this material
	vector<VkDescriptorPool>      m_vectorTextureDescriptorPool;               //!< Vector of the texture descriptor pool of this material
	vector<VkDescriptorSet>       m_vectorImageDescriptorSet;                  //!< Vector of the image descriptor sets of this material
	vector<VkDescriptorPool>      m_vectorImageDescriptorPool;                 //!< Vector of the image descriptor pool of this material
	vector<VkDescriptorSet>       m_vectorShaderStorageDescriptorSet;          //!< Vector of the image descriptor sets of this material
	vector<VkDescriptorPool>      m_vectorShaderStorageDescriptorPool;         //!< Vector of the image descriptor pool of this material
	vector<ExposedStructField>    m_vectorExposedStructField;                  //!< Vector with the exposed member variables that will be linked with sctruct members in the shader
	vector<ExposedStructField*>   m_vectorDirtyExposedStructField;             //!< Vector with those exposed struct fields with different values form the preious update
	vector<ExposedStructField>    m_vectorPushConstantExposedStructField;      //!< Vector with the exposed member variables that will be linked with sctruct members in the shader in the push constant struct (if any)
	vector<ExposedStructField*>   m_vectorPushConstantDirtyExposedStructField; //!< Vector with those exposed struct fields with different values form the preious update in the push constant struct (if any)
	vector<VkClearValue>          m_vectorClearValue;                          //!< Vector with the clear values for this material
	bool                          m_isCompute;                                 //!< True if the material has a compute shader assigned iosntead of the rasterization pipeline shader (default)
	bool                          m_isEmitter;                                 //!< True f the material represents an emitter
	MaterialBufferResource        m_resourcesUsed;                             //!< Enum to know the dynamic uniform buffers generated by raster manager used by this material
	VkDescriptorSet               m_descriptorSet;                             //!< Descriptor set used for this material
	VkDescriptorPool              m_descriptorPool;                            //!< Descriptor set pool used for the descriptor set construction
	VkDescriptorSetLayout         m_descriptorSetLayout;                       //!< Descriptor set layout used for the descriptor set construction
	uint                          m_materialInstanceIndex;                     //!< Material instance index
	MaterialSurfaceType           m_materialSurfaceType;                       //!< Material surface type
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIAL_H_
