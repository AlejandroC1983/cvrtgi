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

#ifndef _GPUPIPELINE_H_
#define _GPUPIPELINE_H_


// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/commonnamespace.h"
#include "../../include/core/coreenum.h"
#include "../../include/util/singleton.h"

// CLASS FORWARDING
class UniformBuffer;
class RasterTechnique;

// NAMESPACE
using namespace commonnamespace;
using namespace coreenum;

// DEFINES
#define gpuPipelineM s_pGPUPipeline->instance()
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT // Number of samples needs to be the same at image creation, used at renderpass creation (in attachment) and pipeline creation

/////////////////////////////////////////////////////////////////////////////////////////////

class GPUPipeline : public Singleton<GPUPipeline>
{
public:
	/** Default constructor
	* @return nothing */
	GPUPipeline();

	/** Default destructor
	* @return nothing */
	~GPUPipeline();

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();

	/** Initializes all the rasterization techniques, scene index and vertex buffers, and the material
	* and uniform buffer objetcs
	* @return nothing */
	void init();

	/** Calls destructor
	* @return nothing */
	void shutdown();

	/** Returns a pointer to the element in m_vectorRasterTechnique with name equal to the one given as parameter, if not
	* found, it returns nullptr
	* @param name [in] name of the technique to look for in m_vectorRasterTechnique
	* @return pointer to the element in m_vectorRasterTechnique with name equal to the one given as parameter, nullptr otherwise */
	RasterTechnique* getRasterTechniqueByName(string &&name);

	/** Build the scene geoemtry index and vertex buffer
	* @param arrayIndices    [in] array with index data
	* @param arrayVertexData [in] array with vertex data (expanded per-vertex attributes)
	* @return nothing */
	void createVertexBuffer(vectorUint& arrayIndices, vectorFloat& arrayVertexData);

	/** Build the scene vertex specification with viIpBind and m_viIpAttrb
	* @param dataStride [in] per vertex total stride
	* @return nothing */
	void setVertexInput(uint32_t dataStride);

	/** Update scene element transform buffer and camera buffer and upload data to GPU
	* @return nothing */
	void update();

	/** Sets the viewport extent and origin coordinates in screen space, as well as the minimum and maximum depth
	* @param width    [in] viewport width
	* @param height   [in] viewport height
	* @param offsetX  [in] viewport offset
	* @param offsetY  [in] viewport offset
	* @param minDepth [in] viewport minimum depth
	* @param maxDepth [in] viewport maximum depth
	* @param cmd      [in] command to record to
	* @return nothing */
	void initViewports(float width, float height, float offsetX, float offsetY, float minDepth, float maxDepth, VkCommandBuffer* cmd);

	/** Sets the scissor extent and origin coordinates in screen space
	* @param width    [in] scissor width
	* @param height   [in] scissor height
	* @param offsetX  [in] scissor offset
	* @param offsetY  [in] scissor offset
	* @param minDepth [in] scissor minimum depth
	* @param maxDepth [in] scissor maximum depth
	* @param cmd      [in] command to record to
	* @return nothing */
	void initScissors(uint width, uint height, int offsetX, int offsetY, VkCommandBuffer* cmd);
	
	/** Build all scene geometry buffer to upload to GPU
	* @param arrayIndices    [in] array with indices
	* @param arrayVertexData [in] array with vertex data (expanded per-vertex attribute)
	* @return nothing */
	void buildSceneBufferData(vectorUint& arrayIndices, vectorFloat& arrayVertexData);

	/** Build pipeline caches for building compute and graphics pipelines
	* @return nothing */
	void createPipelineCache();

	/** Build the uniform buffer with world space transform information for all the scene elements
	* @return nothing */
	void createSceneDataUniformBuffer();

	/** Build the uniform buffer with camera information
	* @return nothing */
	void createSceneCameraUniformBuffer();

	/** Builds a descriptor set of type given by descriptorType. The generated resources a VkDescriptorSetLayout,
	* a VkDescriptorPool, and a VkDescriptorSet, are returned as parameters.
	* @param vectorDescriptorType [in] Vector with the types of the descriptors to build (must match in size vectorDescriptorType)
	* @param vectorBindingIndex   [in] Vector with the binding indices for each one of the descriptor set elements (must match in size vectorDescriptorType)
	* @param vectorStageFlags     [in] Vector with the state flags for the descriptor sets to build
	* @param descriptorSetLayout  [in] Descriptor set layout generated for the descriptor set to build
	* @param descriptorPool       [in] Descriptor pool generated for the descriptor set to build
	* @return descriptor set generated */
	VkDescriptorSet buildDescriptorSet(vector<VkDescriptorType> vectorDescriptorType,
		vector<uint32_t> vectorBindingIndex,
		vector<VkShaderStageFlags> vectorStageFlags,
		VkDescriptorSetLayout& descriptorSetLayout,
		VkDescriptorPool& descriptorPool);

	/** Updates the descriptor sets given by vectorDescriptorSet, of type buffer / image / texel buffer
	* (given as void pointers in vectorDescriptorInfo).
	* @param descriptorSet            [in] Descriptor set to update
	* @param vectorDescriptorType     [in] Vector with the descriptor types to update
	* @param vectorDescriptorInfo     [in] Vector with the descriptor ifo (buffer / image / texel) codified as void pointers
	* @param vectorDescriptorInfoHint [in] Vector to know what reeosurce is represented in the same index in vectorDescriptorInfo
	* @param vectorBinding            [in] Vector with the binding indices
	* @return nothing */
	void updateDescriptorSet(
		const VkDescriptorSet&          descriptorSet,
		const vector<VkDescriptorType>& vectorDescriptorType,
		const vector<void*>&            vectorDescriptorInfo,
		const vector<int>&              vectorDescriptorInfoHint,
		const vector<uint32_t>&         vectorBinding);

	/** Destroys m_pipelineCache
	* @return nothing */
	void destroyPipelineCache();

	/** Returns the index of the technique given as parameter in
	* m_vectorRasterTechnique, or -1 if not present
	* @param technique [in] technique to look for the index in m_vectorRasterTechnique
	* @return index of the technique given as parameter in m_vectorRasterTechnique , or -1 if not present */
	int getRasterTechniqueIndex(RasterTechnique* technique);

	/** Add to m_mapRasterFlag the flag with name flagName and value given as parameter
	* @param flagName [in] name of the flag to add
	* @param value    [in] value fo the flag to add
	* @return true if the flag didn't exist previously, false if already exists (value is set in any case) */
	bool addRasterFlag(string&& flagName, int value);

	/** Remove from m_mapRasterFlag the flag with name flagName
	* @param flagName [in] name of the flag to remove
	* @return true if the flag was removed successfully, false otherwise (the flag wasn't found) */
	bool removeRasterFlag(string&& flagName);

	/** Set valueto the raster flag in m_mapRasterFlag
	* @param flagName [in] name of the flag whose value to set
	* @param value    [in] vlaue set to the flag
	* @return true if the value was set successfully, false otherwise (the flag didn't exist) */
	bool setRasterFlag(string&& flagName, int value);

	/** Return the value fo the raster flag with name flagName
	* @param flagName [in] name of the flag whose value to return
	* @return value of the raster flag given by flagName if exists, -1 otherwise */
	int getRasterFlagValue(string&& flagName);

	/** Getter of m_viIpAttrb
	* @return pointer to m_viIpAttrb */
	VkVertexInputAttributeDescription* refVertexInputAttributeDescription();

	/** Builds basic resources needed for scene loading like a default material
	* @return nothing */
	void preSceneLoadResources();

	REF(VkVertexInputBindingDescription, m_viIpBind, ViIpBind)
	GET(VkPipelineCache, m_pipelineCache, PipelineCache)
	GET_PTR(UniformBuffer, m_sceneUniformData, SceneUniformData)
	REF_PTR(UniformBuffer, m_sceneUniformData, SceneUniformData)
	GET_PTR(UniformBuffer, m_sceneCameraUniformData, SceneCameraUniformData)
	REF_PTR(UniformBuffer, m_sceneCameraUniformData, SceneCameraUniformData)
	GET(vectorRasterTechniquePtr, m_vectorRasterTechnique, VectorRasterTechnique)
	REF(vectorRasterTechniquePtr, m_vectorRasterTechnique, VectorRasterTechnique)
	GET(bool, m_pipelineInitialized, PipelineInitialized)

protected:
	VkVertexInputBindingDescription   m_viIpBind;               //!< Stores the vertex input rate
	VkVertexInputAttributeDescription m_viIpAttrb[4];           //!< Store metadata helpful in data interpretation
	VkPipelineCache                   m_pipelineCache;          //!< Pipeline cache
	UniformBuffer*                    m_sceneUniformData;       //!< Uniform buffer with the per scene elements data
	UniformBuffer*                    m_sceneCameraUniformData; //!< Uniform buffer with the scene camera data
	vectorRasterTechniquePtr          m_vectorRasterTechnique;  //!< Vector with the raster techniques currently in use
	vector<bool>                      m_vectorReRecordFlags;    //!< Vector to cache the results returned by post queue submit and record method, to know if any technique is asking for re-recording
	bool                              m_pipelineInitialized;    //!< True if the call to GPUPipeline::init() has been done
	map<string, int>                  m_mapRasterFlag;          //!< Map to set rasterizatin flags used by the different raster techniques
};

static GPUPipeline* s_pGPUPipeline;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _GPUPIPELINE_H_
