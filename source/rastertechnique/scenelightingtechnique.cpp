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
#include "../../include/rastertechnique/scenelightingtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/scene/scene.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materialcolortexture.h"
#include "../../include/material/materiallighting.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/clusterizationmergeclustertechnique.h"
#include "../../include/rastertechnique/litclustertechnique.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
string SceneLightingTechnique::m_lightingMaterialSuffix = "_Lighting";

/////////////////////////////////////////////////////////////////////////////////////////////

SceneLightingTechnique::SceneLightingTechnique(string &&name, string&& className) :
	  RasterTechnique(move(name), move(className))
	, m_renderTargetColor(nullptr)
	, m_renderTargetDepth(nullptr)
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
	, m_indirectCommandBufferMainCamera(nullptr)
{
	m_active = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

SceneLightingTechnique::~SceneLightingTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::init()
{
	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_L);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_L);
	signalAdd->connect<SceneLightingTechnique, &SceneLightingTechnique::slotLKeyPressed>(this);

	m_indirectCommandBufferMainCamera = bufferM->getElement(move(string("indirectCommandBufferMainCamera")));

	m_arrayNode = sceneM->getByMeshType(E_MT_RENDER_MODEL);

	vector<uint> vectorData;
	vectorData.resize(36000000);
	memset(vectorData.data(), 0, vectorData.size() * size_t(sizeof(uint)));

	bufferM->buildBuffer(
		move(string("debugBuffer")),
		vectorData.data(),
		vectorData.size() * sizeof(uint),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint debugValue = 0;
	bufferM->buildBuffer(
		move(string("debugFragmentCounterBuffer")),
		(void*)(&debugValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litClusterTechnique = static_cast<LitClusterTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("LitClusterTechnique"))));
	m_renderTargetColor   = textureM->getElement(move(string("scenelightingcolor")));
	m_renderTargetDepth   = textureM->getElement(move(string("scenelightingdepth")));
	m_renderPass          = renderPassM->getElement(move(string("scenelightingrenderpass")));
	m_framebuffer         = framebufferM->getElement(move(string("scenelightingrenderpassFB")));

	generateLightingMaterials();

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_1);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_1);
	signalAdd->connect<SceneLightingTechnique, &SceneLightingTechnique::slot1KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_2);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_2);
	signalAdd->connect<SceneLightingTechnique, &SceneLightingTechnique::slot2KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_7);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_7);
	signalAdd->connect<SceneLightingTechnique, &SceneLightingTechnique::slot7KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_8);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_8);
	signalAdd->connect<SceneLightingTechnique, &SceneLightingTechnique::slot8KeyPressed>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::prepare(float dt)
{
	// TODO: automatize this
	const uint maxIndex         = uint(m_vectorMaterial.size());
	Camera* shadowMappingCamera = cameraM->getElement(move(string("emitter")));
	vec3 position               = shadowMappingCamera->getPosition();
	mat4 viewprojectionMatrix   = shadowMappingCamera->getProjection() * shadowMappingCamera->getView();
	vec4 offsetAndSize          = vec4(256.0f, 512.0f, 256.0f, 1024.0f);
	vec3 cameraPosition         = m_litClusterTechnique->getCameraPosition();
	vec3  cameraForward         = m_litClusterTechnique->getCameraForward();
	float emitterRadiance       = m_litClusterTechnique->getEmitterRadiance();

	MaterialLighting* materialCasted = nullptr;
	forI(maxIndex)
	{
		materialCasted = static_cast<MaterialLighting*>(m_vectorMaterial[i]);
		materialCasted->setShadowViewProjection(viewprojectionMatrix);
		materialCasted->setLightPosition(vec4(position.x, position.y, position.z, 0.0f));
		materialCasted->setOffsetAndSize(offsetAndSize);
		materialCasted->setLightForwardEmitterRadiance(vec4(cameraForward.x, cameraForward.y, cameraForward.z, emitterRadiance));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* SceneLightingTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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

	MaterialLighting* material = static_cast<MaterialLighting*>(m_vectorMaterial[0]);

	VkRenderPassBeginInfo renderPassBegin = VulkanStructInitializer::renderPassBeginInfo(
		m_renderPass->getRenderPass(),
		m_framebuffer->getFramebuffer(),
		VkRect2D({ 0, 0, coreM->getWidth(), coreM->getHeight() }),
		material->refVectorClearValue());

	vkCmdBeginRenderPass(*commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE); // Start recording the render pass instance

	const VkDeviceSize offsets[1] = { 0 };
	Buffer* instanceDataBuffer = bufferM->getElement(move(string("instanceDataBuffer")));
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("vertexBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline
	vkCmdBindVertexBuffers(*commandBuffer, 1, 1, &instanceDataBuffer->getBuffer(), offsets);
	vkCmdBindIndexBuffer(*commandBuffer, bufferM->getElement(move(string("indexBuffer")))->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	gpuPipelineM->initViewports((float)coreM->getWidth(), (float)coreM->getHeight(), 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(coreM->getWidth(), coreM->getHeight(), 0, 0, commandBuffer);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	forI(m_arrayNode.size())
	{
		string materialName = m_arrayNode[i]->refMaterial()->getName();
		materialName       += m_lightingMaterialSuffix;
		Material* currentMaterial  = materialM->getElement(move(materialName));

		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentMaterial->getPipeline()->getPipeline()); // Bound the command buffer with the graphics pipeline

		uint32_t elementIndex;
		uint32_t offsetData[3];
		uint32_t sceneDataBufferOffset = static_cast<uint32_t>(gpuPipelineM->getSceneUniformData()->getDynamicAllignment());

		elementIndex = sceneM->getElementIndex(m_arrayNode[i]);

		offsetData[0] = elementIndex * sceneDataBufferOffset;
		//offsetData[0] = i * sceneDataBufferOffset;
		offsetData[1] = 0;
		offsetData[2] = static_cast<uint32_t>(currentMaterial->getMaterialUniformBufferIndex() * dynamicAllignment);

		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentMaterial->getPipelineLayout(), 0, 1, &currentMaterial->refDescriptorSet(), 3, &offsetData[0]);
		vkCmdDrawIndexedIndirect(*commandBuffer, m_indirectCommandBufferMainCamera->getBuffer(), i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
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

void SceneLightingTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::slotLKeyPressed()
{
	m_active = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

// TODO: refactor with SceneVoxelizationTechnique::generateVoxelizationMaterials
void SceneLightingTechnique::generateLightingMaterials()
{
	SceneVoxelizationTechnique* technique           = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	vec4 voxelSize                                  = vec4(float(technique->getVoxelizedSceneWidth()), float(technique->getVoxelizedSceneHeight()), float(technique->getVoxelizedSceneHeight()), 0.0f);
	ClusterizationMergeClusterTechnique* technique2 = static_cast<ClusterizationMergeClusterTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationMergeClusterTechnique"))));
	int irradianceFieldGridDensity                  = technique2->getIrradianceFieldGridDensity();

	vec3 min3D;
	vec3 max3D;
	BBox3D& box = sceneM->refBox();

	box.getCenteredBoxMinMax(min3D, max3D);

	vec3 extent3D = max3D - min3D;
	vec4 extent   = vec4(extent3D.x, extent3D.y, extent3D.z, 0.0f);
	vec4 min      = vec4(min3D.x,    min3D.y,    min3D.z,    0.0f);
	vec4 max      = vec4(max3D.x,    max3D.y,    max3D.z,    0.0f);

	// TODO: use descriptor sets to avoid material instances
	const vectorNodePtr arrayNode = sceneM->getByMeshType(E_MT_RENDER_MODEL);
	const uint maxIndex = uint(arrayNode.size());

	const Node* node;
	const Material* material;
	const string materialName = "MaterialColorTexture";
	const MaterialColorTexture* materialCasted;

	vectorString vectorMaterialName;
	vectorMaterialPtr vectorMaterial;

	forI(maxIndex)
	{
		node     = arrayNode[i];
		material = node->getMaterial();

		if ((material != nullptr) && (material->getClassName() == materialName))
		{
			materialCasted        = static_cast<const MaterialColorTexture*>(material);
			string newName        = material->getName() + m_lightingMaterialSuffix;
			Material* newMaterial = materialM->getElement(move(string(newName)));

			if (newMaterial == nullptr)
			{
				MultiTypeUnorderedMap *attributeMaterial = new MultiTypeUnorderedMap();
				attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_reflectanceTextureResourceName), string(materialCasted->getReflectanceTextureName())));
				attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_normalTextureResourceName), string(materialCasted->getNormalTextureName())));

				MaterialSurfaceType surfaceType = materialCasted->getMaterialSurfaceType();
				if (surfaceType != MaterialSurfaceType::MST_OPAQUE)
				{
					attributeMaterial->newElement<AttributeData<MaterialSurfaceType>*>(new AttributeData<MaterialSurfaceType>(string(g_materialSurfaceType), move(surfaceType)));
				}

				newMaterial = materialM->buildMaterial(move(string("MaterialLighting")), move(string(newName)), attributeMaterial);
				vectorMaterialName.push_back(newName);
				vectorMaterial.push_back(newMaterial);
				MaterialLighting* newMaterialCasted = static_cast<MaterialLighting*>(newMaterial);
				newMaterialCasted->setVoxelSize(voxelSize);
				newMaterialCasted->setSceneMin(min);
				newMaterialCasted->setSceneExtent(extent);
				newMaterialCasted->setZFar(vec4(ZFAR, 0.0f, 0.0f, 0.0f));
				newMaterialCasted->setIrradianceFieldGridDensity(irradianceFieldGridDensity);
			}
		}
	}

	m_vectorMaterialName = vectorMaterialName;
	m_vectorMaterial     = vectorMaterial;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::slot1KeyPressed()
{
	const uint maxIndex = uint(m_vectorMaterial.size());
	MaterialLighting* materialCasted = nullptr;

	forI(maxIndex)
	{
		materialCasted = static_cast<MaterialLighting*>(m_vectorMaterial[i]);
		materialCasted->setIrradianceMultiplier(materialCasted->getIrradianceMultiplier() - 100.0f);

		if (i == 0)
		{
			cout << "New value of irradianceMultiplier " << materialCasted->getIrradianceMultiplier() << endl;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::slot2KeyPressed()
{
	const uint maxIndex = uint(m_vectorMaterial.size());
	MaterialLighting* materialCasted = nullptr;

	forI(maxIndex)
	{
		materialCasted = static_cast<MaterialLighting*>(m_vectorMaterial[i]);
		materialCasted->setIrradianceMultiplier(materialCasted->getIrradianceMultiplier() + 100.0f);

		if (i == 0)
		{
			cout << "New value of irradianceMultiplier " << materialCasted->getIrradianceMultiplier() << endl;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::slot7KeyPressed()
{
	const uint maxIndex = uint(m_vectorMaterial.size());
	MaterialLighting* materialCasted = nullptr;

	forI(maxIndex)
	{
		materialCasted = static_cast<MaterialLighting*>(m_vectorMaterial[i]);
		materialCasted->setDirectIrradianceMultiplier(materialCasted->getDirectIrradianceMultiplier() - 100.0f);

		if (i == 0)
		{
			cout << "New value of directIrradianceMultiplier " << materialCasted->getDirectIrradianceMultiplier() << endl;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneLightingTechnique::slot8KeyPressed()
{
	const uint maxIndex = uint(m_vectorMaterial.size());
	MaterialLighting* materialCasted = nullptr;

	forI(maxIndex)
	{
		materialCasted = static_cast<MaterialLighting*>(m_vectorMaterial[i]);
		materialCasted->setDirectIrradianceMultiplier(materialCasted->getDirectIrradianceMultiplier() + 100.0f);

		if (i == 0)
		{
			cout << "New value of directIrradianceMultiplier " << materialCasted->getDirectIrradianceMultiplier() << endl;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////