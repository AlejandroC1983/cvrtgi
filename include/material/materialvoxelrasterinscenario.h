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

#ifndef _MATERIALVOXELRASTERINSCENARIO_H_
#define _MATERIALVOXELRASTERINSCENARIO_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/scene/scene.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialVoxelRasterInScenario: public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialVoxelRasterInScenario(string &&name) : Material(move(name), move(string("MaterialVoxelRasterInScenario")))
		, m_numOcupiedVoxel(0)
		, m_padding0(0)
		, m_padding1(0)
		, m_padding2(0)
	{
		m_resourcesUsed = (MaterialBufferResource::MBR_CAMERA | MaterialBufferResource::MBR_MATERIAL);

		m_vectorClearValue.resize(2);
		m_vectorClearValue[0].color.float32[0] = 0.7215f;
		m_vectorClearValue[0].color.float32[1] = 0.059f;
		m_vectorClearValue[0].color.float32[2] = 0.04f;
		m_vectorClearValue[0].color.float32[3] = 1.0f;

		// Specify the depth/stencil clear value
		m_vectorClearValue[1].depthStencil.depth   = 1.0f;
		m_vectorClearValue[1].depthStencil.stencil = 0;

		SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
		m_voxelizationSize = vec4(float(technique->getVoxelizedSceneWidth()), technique->getVoxelizedSceneHeight(), technique->getVoxelizedSceneDepth(), 0.0f);

		BBox3D& sceneAABB = sceneM->refBox();

		vec3 m;
		vec3 M;
		sceneAABB.getCenteredBoxMinMax(m, M);
		vec3 center        = (m + M) * 0.5f;
		float maxValue0    = max(2.0f, 3.0f);
		float voxelSizeMax = max(float(M.z - m.z), maxValue0) * 0.5f;
		m_sceneMin         = vec4(m.x,          m.y,          m.z,          0.0f);
		vec3 sceneExtent3D = sceneAABB.getMax() - sceneAABB.getMin();
		m_sceneExtent      = vec4(sceneExtent3D.x, sceneExtent3D.y, sceneExtent3D.z, 1.0f);
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeTemp;

		void* vertShaderCode     = InputOutput::readFile("../data/vulkanshaders/voxelrenderinscenario.vert", &sizeTemp);
		void* geometryShaderCode = InputOutput::readFile("../data/vulkanshaders/voxelrenderinscenario.geom", &sizeTemp);
		void* fragShaderCode     = InputOutput::readFile("../data/vulkanshaders/voxelrenderinscenario.frag", &sizeTemp);

		m_shaderResourceName = "voxelrenderinscenario";
		m_shader             = shaderM->buildShaderVGF(move(string(m_shaderResourceName)), (const char*)vertShaderCode, (const char*)geometryShaderCode, (const char*)fragShaderCode, m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		assignShaderStorageBuffer(move(string("voxelrasterinscenariodebugbuffer")),    move(string("voxelrasterinscenariodebugbuffer")),    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")),  move(string("voxelHashedPositionCompactedBuffer")),  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),              move(string("IndirectionIndexBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),               move(string("IndirectionRankBuffer")),               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("fragmentDataBuffer")),                  move(string("fragmentDataBuffer")),                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelFirstIndexCompactedBuffer")),      move(string("voxelFirstIndexCompactedBuffer")),      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("litTestVoxelBuffer")),                  move(string("litTestVoxelBuffer")),                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanCurvatureBuffer")),                 move(string("meanCurvatureBuffer")),                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanNormalBuffer")),                    move(string("meanNormalBuffer")),                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),        move(string("voxelClusterOwnerIndexBuffer")),        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationFinalBuffer")),           move(string("clusterizationFinalBuffer")),           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationDebugVoxelIndexBuffer")), move(string("clusterizationDebugVoxelIndexBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("litTestClusterBuffer")),                move(string("litTestClusterBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("nextFragmentIndexBuffer")),             move(string("nextFragmentIndexBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		exposeStructField(ResourceInternalType::RIT_FLOAT_MAT4,   (void*)(&m_viewProjection),   move(string("myMaterialData")), move(string("viewProjection")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_voxelizationSize), move(string("myMaterialData")), move(string("voxelizationSize")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMin),         move(string("myMaterialData")), move(string("sceneMin")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtent),      move(string("myMaterialData")), move(string("sceneExtent")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numOcupiedVoxel),  move(string("myMaterialData")), move(string("numOcupiedVoxel")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding0),         move(string("myMaterialData")), move(string("padding0")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding1),         move(string("myMaterialData")), move(string("padding1")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding2),         move(string("myMaterialData")), move(string("padding2")));
	}

	void setupPipelineData()
	{
		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo;
		depthStencilStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.pNext                 = nullptr;
		depthStencilStateInfo.flags                 = 0;
		depthStencilStateInfo.depthTestEnable       = VK_TRUE;
		depthStencilStateInfo.depthWriteEnable      = VK_TRUE;
		depthStencilStateInfo.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable     = VK_FALSE;
		depthStencilStateInfo.back.failOp           = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.back.passOp           = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.back.depthFailOp      = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.back.compareOp        = VK_COMPARE_OP_NEVER;
		depthStencilStateInfo.back.compareMask      = 0;
		depthStencilStateInfo.back.reference        = 0;
		depthStencilStateInfo.back.writeMask        = 0;
		depthStencilStateInfo.minDepthBounds        = 0;
		depthStencilStateInfo.maxDepthBounds        = 0;
		depthStencilStateInfo.front                 = depthStencilStateInfo.back;

		m_pipeline.refPipelineData().setDepthStencilStateInfo(depthStencilStateInfo);

		m_pipeline.refPipelineData().setRenderPass(renderPassM->getElement(move(string("scenelightingrenderpass")))->getRenderPass());

		VkPipelineRasterizationStateCreateInfo rasterStateInfo = m_pipeline.refPipelineData().getRasterStateInfo();
		rasterStateInfo.depthClampEnable = VK_FALSE;
		rasterStateInfo.cullMode         = VK_CULL_MODE_BACK_BIT;
		rasterStateInfo.frontFace        = VK_FRONT_FACE_CLOCKWISE;
		m_pipeline.refPipelineData().setRasterStateInfo(rasterStateInfo);

		// The VkVertexInputBinding m_vertexInputBindingDescription, stores the rate at which the information will be
		// injected for vertex input.
		m_vertexInputBindingDescription.binding   = 0;
		m_vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_vertexInputBindingDescription.stride    = 4;

		// The VkVertexInputAttribute - Description) structure, store the information that helps in interpreting the data.
		m_vertexInputAttributeDescription[0].binding  = 0;
		m_vertexInputAttributeDescription[0].location = 0;
		m_vertexInputAttributeDescription[0].format   = VK_FORMAT_R32_UINT;
		m_vertexInputAttributeDescription[0].offset   = 0;

		VkPipelineVertexInputStateCreateInfo inputState = m_pipeline.refPipelineData().getVertexInputStateInfo();
		inputState.vertexBindingDescriptionCount   = 1;
		inputState.pVertexBindingDescriptions      = &m_vertexInputBindingDescription;
		inputState.vertexAttributeDescriptionCount = 1;
		inputState.pVertexAttributeDescriptions    = m_vertexInputAttributeDescription;

		m_pipeline.refPipelineData().updateVertexInputStateInfo(inputState);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = m_pipeline.refPipelineData().getInputAssemblyInfo();
		inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		m_pipeline.refPipelineData().updateinputAssemblyStateCreateInfo(inputAssemblyStateCreateInfo);
	}

	GET_SET(mat4, m_viewProjection, ViewProjection)
	GET_SET(vec4, m_voxelizationSize, VoxelizationSize)
	GET_SET(vec4, m_sceneMin, SceneMin)
	GET_SET(vec4, m_sceneExtent, SceneExtent)
	SET(uint, m_numOcupiedVoxel, NumOcupiedVoxel)

protected:
	mat4                              m_viewProjection;                     //!< Matrix to transform each cube's center coordinates in the geometry shader
	vec4                              m_voxelizationSize;                   //!< Voxelizatoin size
	vec4                              m_sceneMin;                           //!< Scene minimum value
	vec4                              m_sceneExtent;                        //!< Extent of the scene. Also, w value is used to distinguish between rasterization of reflectance or normal information
	uint                              m_numOcupiedVoxel;                    //!< Number of occupied voxel
	uint                              m_padding0;                           //!< Padding value to achieve 4 32-bit data in the material in the shader
	uint                              m_padding1;                           //!< Padding value to achieve 4 32-bit data in the material in the shader
	uint                              m_padding2;                           //!< Padding value to achieve 4 32-bit data in the material in the shader
	VkVertexInputBindingDescription   m_vertexInputBindingDescription;      //!< This material uses a different vertex input format
	VkVertexInputAttributeDescription m_vertexInputAttributeDescription[1]; //!< This material uses a different vertex input format
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALVOXELRASTERINSCENARIO_H_
