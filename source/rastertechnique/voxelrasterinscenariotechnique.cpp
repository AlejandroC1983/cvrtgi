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
#include "../../include/rastertechnique/voxelrasterinscenariotechnique.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/core/coremanager.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/material/materialvoxelrasterinscenario.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

VoxelRasterInScenarioTechnique::VoxelRasterInScenarioTechnique(string &&name, string&& className) : RasterTechnique(move(name), move(className))
	, m_material(nullptr)
	, m_numOccupiedVoxel(0)
	, m_voxelShadowMappingTexture(nullptr)
	, m_voxelrasterinscenariodebugbuffer(nullptr)
	, m_renderTargetColor(nullptr)
	, m_renderTargetDepth(nullptr)
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
{
	m_active = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VoxelRasterInScenarioTechnique::~VoxelRasterInScenarioTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::init()
{
	m_voxelShadowMappingTexture = textureM->getElement(move(string("shadowmappingvoxeltexture")));

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_R);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_R);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotRKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_N);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_N);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotNKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_V);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_V);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotVKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_H);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_H);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotHKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_M);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_M);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotMKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_C);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_C);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotCKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_K);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_K);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotKKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_T);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_T);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotTKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_Y);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_Y);
	signalAdd->connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotYKeyPressed>(this);

	vector<uint> vectorData;
	vectorData.resize(128);
	memset(vectorData.data(), 0, vectorData.size() * size_t(sizeof(uint)));
	m_voxelrasterinscenariodebugbuffer = bufferM->buildBuffer(
		move(string("voxelrasterinscenariodebugbuffer")),
		vectorData.data(),
		vectorData.size() * sizeof(uint),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_material = static_cast<MaterialVoxelRasterInScenario*>(materialM->buildMaterial(move(string("MaterialVoxelRasterInScenario")), move(string("MaterialVoxelRasterInScenario")), nullptr));

	m_vectorMaterialName.push_back("MaterialVoxelRasterInScenario");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));

	BufferPrefixSumTechnique* techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	techniquePrefixSum->refPrefixSumComplete().connect<VoxelRasterInScenarioTechnique, &VoxelRasterInScenarioTechnique::slotPrefixSumCompleted>(this);

	m_renderTargetColor = textureM->getElement(move(string("scenelightingcolor")));
	m_renderTargetDepth = textureM->getElement(move(string("scenelightingdepth")));
	m_renderPass        = renderPassM->getElement(move(string("scenelightingrenderpass")));
	m_framebuffer       = framebufferM->getElement(move(string("scenelightingrenderpassFB")));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::prepare(float dt)
{
	mat4 viewMatrix       = cameraM->refMainCamera()->getView();
	mat4 projectionMatrix = cameraM->refMainCamera()->getProjection();
	mat4 viewProjection   = projectionMatrix * viewMatrix;

	m_material->setViewProjection(viewProjection);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* VoxelRasterInScenarioTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	commandBufferType = CommandBufferType::CBT_GRAPHICS_QUEUE;

	VkCommandBuffer* commandBuffer;
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, coreM->getGraphicsQueueQueryPool(), m_queryIndex0);
#endif

	VkRenderPassBeginInfo renderPassBegin = VulkanStructInitializer::renderPassBeginInfo(
		m_renderPass->getRenderPass(),
		m_framebuffer->getFramebuffer(),
		VkRect2D({ 0, 0, coreM->getWidth(), coreM->getHeight() }),
		m_material->refVectorClearValue());

	vkCmdBeginRenderPass(*commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("voxelHashedPositionCompactedBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipeline()->getPipeline());

	gpuPipelineM->initViewports((float)coreM->getWidth(), (float)coreM->getHeight(), 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(coreM->getWidth(), coreM->getHeight(), 0, 0, commandBuffer);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData[2];
	offsetData[0] = 0;
	offsetData[1] = static_cast<uint32_t>(m_material->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipelineLayout(), 0, 1, &m_material->refDescriptorSet(), 2, &offsetData[0]);

	vkCmdDraw(*commandBuffer, m_numOccupiedVoxel, 1, 0, 0);

	vkCmdEndRenderPass(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, coreM->getGraphicsQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotPrefixSumCompleted()
{
	BufferPrefixSumTechnique* techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_numOccupiedVoxel = techniquePrefixSum->getFirstIndexOccupiedElement();
	bufferM->resize(m_voxelrasterinscenariodebugbuffer, nullptr, m_numOccupiedVoxel * 8 * sizeof(uint));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotRKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 1.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotNKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 0.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotVKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 0.5f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotLKeyPressed()
{
	m_active = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotHKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 2.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotMKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 3.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotCKeyPressed()
{
	if (!m_active)
	{
		m_active = true;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 4.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotKKeyPressed()
{
	if (!m_active)
	{
		m_active        = true;
		m_needsToRecord = !m_needsToRecord;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 5.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotTKeyPressed()
{
	if (!m_active)
	{
		m_active        = true;
		m_needsToRecord = !m_needsToRecord;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 6.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelRasterInScenarioTechnique::slotYKeyPressed()
{
	if (!m_active)
	{
		m_active        = true;
		m_needsToRecord = !m_needsToRecord;
	}

	vec4 sceneExtent = m_material->getSceneExtent();
	sceneExtent.w    = 7.0f;

	m_material->setSceneExtent(sceneExtent);
}

/////////////////////////////////////////////////////////////////////////////////////////////
