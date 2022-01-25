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
#include "../../include/rastertechnique/scenerastercolortexturetechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/core/coremanager.h"
#include "../../include/shader/shader.h"
#include "../../include/scene/scene.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/shader/sampler.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materialcolortexture.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

SceneRasterColorTextureTechnique::SceneRasterColorTextureTechnique(string &&name, string&& className) : RasterTechnique(move(name), move(className))
	, m_elapsedTime(0.0f)
	, m_addingTime(true)
	, m_renderTargetColor(nullptr)
	, m_renderTargetDepth(nullptr)
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

SceneRasterColorTextureTechnique::~SceneRasterColorTextureTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneRasterColorTextureTechnique::init()
{
	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_L);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_L);
	signalAdd->connect<SceneRasterColorTextureTechnique, &SceneRasterColorTextureTechnique::slotLKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_R);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_R);
	signalAdd->connect<SceneRasterColorTextureTechnique, &SceneRasterColorTextureTechnique::slotLKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_N);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_N);
	signalAdd->connect<SceneRasterColorTextureTechnique, &SceneRasterColorTextureTechnique::slotLKeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_I);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_I);
	signalAdd->connect<SceneRasterColorTextureTechnique, &SceneRasterColorTextureTechnique::slotLKeyPressed>(this);

	m_renderTargetColor = textureM->getElement(move(string("scenelightingcolor")));
	m_renderTargetDepth = textureM->getElement(move(string("scenelightingdepth")));
	m_renderPass        = renderPassM->getElement(move(string("scenelightingrenderpass")));
	m_framebuffer       = framebufferM->getElement(move(string("scenelightingrenderpassFB")));

	m_material = static_cast<MaterialColorTexture*>(materialM->getElement(move(string("DefaultMaterial"))));
	m_material->setColor(vec3(1.0f, 1.0f, 1.0f));
	m_vectorMaterialName.push_back("DefaultMaterial");
	m_vectorMaterial.push_back(m_material);

	const vectorMaterialPtr vectorMaterialData = materialM->getElementByClassName(move(string("MaterialColorTexture")));
	
	Material* materialTemp;
	const uint maxIndex = uint(vectorMaterialData.size());

	forI(maxIndex)
	{
		materialTemp = vectorMaterialData[i];
		addIfNoPresent(materialTemp->getName(), m_vectorMaterialName);
		addIfNoPresent(materialTemp,            m_vectorMaterial);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* SceneRasterColorTextureTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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
		*m_renderPass->refRenderPass(),
		m_framebuffer->getFramebuffer(),
		VkRect2D({ 0, 0, coreM->getWidth(), coreM->getHeight() }),
		m_material->refVectorClearValue());

	vkCmdBeginRenderPass(*commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE); // Start recording the render pass instance

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("vertexBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline
	vkCmdBindIndexBuffer(*commandBuffer, bufferM->getElement(move(string("indexBuffer")))->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	gpuPipelineM->initViewports((float)coreM->getWidth(), (float)coreM->getHeight(), 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(coreM->getWidth(), coreM->getHeight(), 0, 0, commandBuffer);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	vectorNodePtr arrayNode = sceneM->getByMeshType(E_MT_RENDER_MODEL | E_MT_DEBUG_RENDER_MODEL | E_MT_EMITTER_MODEL | E_MT_INSTANCED_RENDER_MODEL);
	if (arrayNode.size() > 0)
	{
		sceneM->sortByMaterial(arrayNode);

		Material* previous = arrayNode[0]->refMaterial();
		Material* current = previous;

		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current->getPipeline()->getPipeline()); // Bound the command buffer with the graphics pipeline

		bool bindMaterialPipeline = false;

		uint maxIndex = uint(arrayNode.size());

		uint32_t elementIndex;
		uint32_t offsetData[3];
		uint32_t sceneDataBufferOffset = static_cast<uint32_t>(gpuPipelineM->getSceneUniformData()->getDynamicAllignment());
		forI(maxIndex)
		{
			if (bindMaterialPipeline)
			{
				vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current->getPipeline()->getPipeline()); // Bound the command buffer with the graphics pipeline
				bindMaterialPipeline = false;
			}

			elementIndex  = sceneM->getElementIndex(arrayNode[i]);
			offsetData[0] = elementIndex * sceneDataBufferOffset;
			offsetData[1] = 0;
			offsetData[2] = static_cast<uint32_t>(current->getMaterialUniformBufferIndex() * dynamicAllignment);

			vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current->getPipelineLayout(), 0, 1, &current->refDescriptorSet(), 3, &offsetData[0]);
			vkCmdDrawIndexed(*commandBuffer, arrayNode[i]->getIndexSize(), 1, arrayNode[i]->getStartIndex(), 0, 0);

			if ((i + 1) < maxIndex)
			{
				previous = current;
				current  = arrayNode[i + 1]->refMaterial();

				if (previous != current)
				{
					bindMaterialPipeline = true;
				}
			}
		}
	}

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

void SceneRasterColorTextureTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneRasterColorTextureTechnique::slotLKeyPressed()
{
	m_active = !m_active;
}

/////////////////////////////////////////////////////////////////////////////////////////////
