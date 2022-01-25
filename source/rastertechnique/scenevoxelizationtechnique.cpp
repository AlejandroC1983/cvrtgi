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
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/core/coremanager.h"
#include "../../include/shader/shader.h"
#include "../../include/scene/scene.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/shader/sampler.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materialscenevoxelization.h"
#include "../../include/scene/scene.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/material/materialcolortexture.h"
#include "../../include/node/emitternode.h"
#include "../../include/util/mathutil.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
string SceneVoxelizationTechnique::m_voxelizationMaterialSuffix = "_Voxelization";

/////////////////////////////////////////////////////////////////////////////////////////////

SceneVoxelizationTechnique::SceneVoxelizationTechnique(string &&name, string&& className) : RasterTechnique(move(name), move(className))
	, m_voxelizedSceneWidth(0)
	, m_voxelizedSceneHeight(0)
	, m_voxelizedSceneDepth(0)
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
	, m_voxelizationTexture(nullptr)
	, m_bufferResizingFlag(false)
	, m_firstPassFragmentCountDone(false)
	, m_secondPassFragmentStoreDone(false)
	, m_fragmentCounter(0)
	, m_fragmentOccupiedCounter(0)
	, m_voxelOccupiedBuffer(nullptr)
	, m_voxelFirstIndexBuffer(nullptr)
	, m_fragmentCounterBuffer(nullptr)
	, m_fragmentOccupiedCounterBuffer(nullptr)
	, m_fragmentDataBuffer(nullptr)
	, m_fragmentIrradianceBuffer(nullptr)
	, m_nextFragmentIndexBuffer(nullptr)
	, m_emitterBuffer(nullptr)
	, m_storeInformation(0.0f)
	, m_stepMultiplier(0.0f)
	, m_currentStep(VoxelizationStep::VS_INIT)
{
	m_recordPolicy                  = CommandRecordPolicy::CRP_SINGLE_TIME;
	m_neededSemaphoreNumber         = 2;
	int sceneVoxelizationResolution = gpuPipelineM->getRasterFlagValue(move(string("SCENE_VOXELIZATION_RESOLUTION")));
	m_voxelizedSceneWidth           = sceneVoxelizationResolution;
	m_voxelizedSceneHeight          = sceneVoxelizationResolution;
	m_voxelizedSceneDepth           = sceneVoxelizationResolution;
	m_pixelDiagonal                 = (vec3(float(sqrt(2.0 * (1.0 / double(m_voxelizedSceneWidth))  * (1.0 / double(m_voxelizedSceneWidth)))),
										    float(sqrt(2.0 * (1.0 / double(m_voxelizedSceneHeight)) * (1.0 / double(m_voxelizedSceneHeight)))),
										    float(sqrt(2.0 * (1.0 / double(m_voxelizedSceneDepth))  * (1.0 / double(m_voxelizedSceneDepth))))));

	cout << "Voxelization texture resolution is " << sceneVoxelizationResolution << endl;

	// Information about scene aabb is updated in the Scene::update call in Scene::init
	updateEmitterVoxelizationInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////////

SceneVoxelizationTechnique::~SceneVoxelizationTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneVoxelizationTechnique::init()
{
	// Texture 3D
	m_voxelizationTexture = textureM->buildTexture(
		move(string("voxelizedscene")),
		VK_FORMAT_R8G8B8A8_UNORM,
		VkExtent3D({ uint32_t(m_voxelizedSceneWidth), uint32_t(m_voxelizedSceneHeight), uint32_t(m_voxelizedSceneDepth) }),
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_VIEW_TYPE_3D,
		0);

	Texture* output = textureM->buildTexture(
		move(string("voxelizationoutputimage")),
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VkExtent3D({ uint32_t(m_voxelizedSceneWidth), uint32_t(m_voxelizedSceneHeight), uint32_t(1) }),
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_8_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_VIEW_TYPE_2D,
		0);

	VkPipelineBindPoint* pipelineBindPoint                         = new VkPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
	vector<VkFormat>* vectorAttachmentFormat                       = new vector<VkFormat>;
	vector<VkSampleCountFlagBits>* vectorAttachmentSamplesPerPixel = new vector<VkSampleCountFlagBits>;
	vector<VkImageLayout>* vectorAttachmentFinalLayout             = new vector<VkImageLayout>;
	vector<VkAttachmentReference>* vectorColorReference            = new vector<VkAttachmentReference>;
	vectorAttachmentFormat->push_back(VK_FORMAT_R16G16B16A16_SFLOAT);
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_8_BIT);
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vectorColorReference->push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	MultiTypeUnorderedMap *attributeUM = new MultiTypeUnorderedMap();
	attributeUM->newElement<AttributeData<VkPipelineBindPoint*>*>(new AttributeData<VkPipelineBindPoint*>(string(g_renderPassAttachmentPipelineBindPoint), move(pipelineBindPoint)));
	attributeUM->newElement<AttributeData<vector<VkFormat>*>*>(new AttributeData<vector<VkFormat>*>(string(g_renderPassAttachmentFormat), move(vectorAttachmentFormat)));
	attributeUM->newElement<AttributeData<vector<VkSampleCountFlagBits>*>*>(new AttributeData<vector<VkSampleCountFlagBits>*>(string(g_renderPassAttachmentSamplesPerPixel), move(vectorAttachmentSamplesPerPixel)));
	attributeUM->newElement<AttributeData<vector<VkImageLayout>*>*>(new AttributeData<vector<VkImageLayout>*>(string(g_renderPassAttachmentFinalLayout), move(vectorAttachmentFinalLayout)));
	attributeUM->newElement<AttributeData<vector<VkAttachmentReference>*>*>(new AttributeData<vector<VkAttachmentReference>*>(string(g_renderPassAttachmentColorReference), move(vectorColorReference)));
	m_renderPass = renderPassM->buildRenderPass(move(string("voxelizationrenderpass")), attributeUM);

	vector<string> arrayAttachment;
	arrayAttachment.push_back(output->getName());
	m_framebuffer = framebufferM->buildFramebuffer(move(string("voxelizationoutputFB")), (uint32_t)(m_voxelizedSceneWidth), (uint32_t)(m_voxelizedSceneHeight), move(string(m_renderPass->getName())), move(arrayAttachment));

	// Build shader storage buffer to store number of emitted fragments in the scene
	m_fragmentCounterBuffer = bufferM->buildBuffer(
		move(string("fragmentCounterBuffer")),
		(void*)(&m_fragmentCounter),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Build shader storage buffer to store the number of 3D voxelization volume cells (positions) occupied
	// by at least one emitted fragment
	m_fragmentOccupiedCounterBuffer = bufferM->buildBuffer(
		move(string("fragmentOccupiedCounterBuffer")),
		(void*)(&m_fragmentOccupiedCounter),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vector<uint> vectorData;
	vectorData.resize(m_voxelizedSceneWidth * m_voxelizedSceneHeight * m_voxelizedSceneDepth);
	memset(vectorData.data(), maxValue, vectorData.size() * size_t(sizeof(uint)));

	vector<uint> vectorVoxelOccupied;
	vectorVoxelOccupied.resize(m_voxelizedSceneWidth * m_voxelizedSceneHeight * m_voxelizedSceneDepth / 8);
	memset(vectorVoxelOccupied.data(), 0, vectorVoxelOccupied.size() * size_t(sizeof(uint)));
	// Build shader storage buffer to know if a particular voxel is occupied or is not
	// A single bit is used to know if the hashed value of the 3D position of a voxel is empty or occupied
	m_voxelOccupiedBuffer = bufferM->buildBuffer(
		move(string("voxelOccupiedBuffer")),
		vectorVoxelOccupied.data(),
		vectorVoxelOccupied.size(),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Build shader storage buffer to know first index of each generated fragment
	m_voxelFirstIndexBuffer = bufferM->buildBuffer(
		move(string("voxelFirstIndexBuffer")),
		vectorData.data(),
		vectorData.size() * size_t(sizeof(uint)),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Build shader storage buffer to store each emitted fragment information
	// The buffer will be re-done with the proper size once the number of fragments is known, value gathered in m_fragmentCounter
	m_fragmentDataBuffer = bufferM->buildBuffer(
		move(string("fragmentDataBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Build shader storage buffer to store per-fragment irradiance for all scene emitters
	// The buffer will be re-done with the proper size once the number of fragments is known, value gathered in m_fragmentCounter
	m_fragmentIrradianceBuffer = bufferM->buildBuffer(
		move(string("fragmentIrradianceBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// SSBO for the next fragment in the linked list of fragments generated inthe same 3D voxelization volume
	m_nextFragmentIndexBuffer = bufferM->buildBuffer(
		move(string("nextFragmentIndexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	buildEmitterBuffer();

	generateVoxelizationMaterials();
}

/////////////////////////////////////////////////////////////////////////////////////////////

mat4 orthoRH(float left, float right, float bottom, float top, float zNear, float zFar)
{
	mat4 Result(1);
	Result[0][0] = 2.0f / (right - left);
	Result[1][1] = 2.0f / (top - bottom);
	Result[3][0] = -(right + left) / (right - left);
	Result[3][1] = -(top + bottom) / (top - bottom);
	Result[2][2] = -1.0f / (zFar - zNear);
	Result[3][2] = -zNear / (zFar - zNear);

	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneVoxelizationTechnique::prepare(float dt)
{
	// TODO: Move this code to initialization, since it does not change (but m_storeInformation)
	BBox3D& sceneAABB = sceneM->refBox();

	vec3 m;
	vec3 M;

	sceneAABB.getCenteredBoxMinMax(m, M);

	vec3 center         = (m + M) * 0.5f;
	float maxLen        = glm::max(M.x - m.x, glm::max(M.y - m.y, M.z - m.z)) * 0.5f;
	float maxLen2       = maxLen * 2.0f;
	float halfVoxelSize = maxLen * 0.5f;
	m_projection        = orthoRH(-maxLen, maxLen, -maxLen, maxLen, 0.0f, maxLen2);
	m_projection[1][1] *= -1;
	m_viewX             = lookAt(center - vec3(maxLen, 0.0f, 0.0f), center, vec3(0.0f, 1.0f, 0.0f));
	m_viewY             = lookAt(center - vec3(0.0f, maxLen, 0.0f), center, vec3(-1.0f, 0.0f, 0.0f));
	m_viewZ             = lookAt(center - vec3(0.0f, 0.0f, maxLen), center, vec3(0.0f, 1.0f, 0.0f));
	const uint maxIndex = uint(m_vectorMaterial.size());

	// TODO: use descriptor sets to avoid material instances
	MaterialSceneVoxelization* material;
	forI(maxIndex)
	{
		material = static_cast<MaterialSceneVoxelization*>(m_vectorMaterial[i]);
		material->setProjection(m_projection);
		material->setViewX(m_viewX);
		material->setViewY(m_viewY);
		material->setViewZ(m_viewZ);
		material->setVoxelizationSize(vec3(float(m_voxelizedSceneWidth), float(m_voxelizedSceneWidth), float(m_voxelizedSceneWidth)));
		material->setStoreInformation(m_storeInformation);
		material->setPixelDiagonal(vec4(m_pixelDiagonal.x, m_pixelDiagonal.y, m_pixelDiagonal.z, 0.0f));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneVoxelizationTechnique::postCommandSubmit()
{
	m_needsToRecord = (m_vectorCommand.size() < 1);

	switch (m_currentStep)
	{
		case VoxelizationStep::VS_INIT:
		{
			m_currentStep = VoxelizationStep::VS_FIRST_CB_SUBMITTED;
			m_fragmentCounterBuffer->getContent((void*)(&m_fragmentCounter));

			uint temp = 0;
			m_fragmentCounterBuffer->setContent(&temp);

			if (m_fragmentCounter > 0)
			{
				temp = sizeof(PerFragmentData);
				uint bufferSize = sizeof(PerFragmentData) * m_fragmentCounter;
				bufferM->resize(m_fragmentDataBuffer,       nullptr, bufferSize);
				bufferM->resize(m_fragmentIrradianceBuffer, nullptr, m_fragmentCounter * sizeof(uint));
				bufferM->resize(m_nextFragmentIndexBuffer,  nullptr, m_fragmentCounter * sizeof(uint));
			}
			else
			{
				m_secondPassFragmentStoreDone = true;
			}

			m_storeInformation = 1.0f;

			break;
		}
		case VoxelizationStep::VS_FIRST_CB_SUBMITTED:
		{
			m_currentStep    = VoxelizationStep::VS_SECOND_CB_SUBMITTED;
			m_executeCommand = false;
			m_active         = false;

			m_fragmentOccupiedCounterBuffer->getContent((void*)(&m_fragmentOccupiedCounter));
			//BufferVerificationHelper::verifyVoxelOccupiedBuffer();
			m_voxelizationComplete.emit();
			break;
		}
		default:
		{
			cout << "ERROR: no proper value for m_currentStep in SceneVoxelizationTechnique::postCommandSubmit" << endl;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* SceneVoxelizationTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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

	MaterialSceneVoxelization* material = static_cast<MaterialSceneVoxelization*>(m_vectorMaterial[0]);

	VkRenderPassBeginInfo renderPassBegin = VulkanStructInitializer::renderPassBeginInfo(
		*m_renderPass->refRenderPass(),
		m_framebuffer->getFramebuffer(),
		VkRect2D({ 0, 0, uint32_t(m_voxelizedSceneWidth), uint32_t(m_voxelizedSceneHeight) }),
		material->refVectorClearValue());

	vkCmdBeginRenderPass(*commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE); // Start recording the render pass instance

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("vertexBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline
	vkCmdBindIndexBuffer(*commandBuffer, bufferM->getElement(move(string("indexBuffer")))->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	gpuPipelineM->initViewports(float(m_voxelizedSceneWidth), float(m_voxelizedSceneHeight), 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(m_voxelizedSceneWidth, m_voxelizedSceneHeight, 0, 0, commandBuffer);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	vectorNodePtr arrayNode = sceneM->getByMeshType(E_MT_RENDER_MODEL);
	if (arrayNode.size() > 0)
	{
		sceneM->sortByMaterial(arrayNode);

		string materialName = arrayNode[0]->refMaterial()->getName();
		materialName       += m_voxelizationMaterialSuffix;
		Material* previous  = materialM->getElement(move(materialName));
		Material* current   = previous;

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

			elementIndex = sceneM->getElementIndex(arrayNode[i]);

			// Only models that are meant to be rasterized and are not emitters nor debug elements
			if ((arrayNode[i]->getMeshType() & E_MT_RENDER_MODEL) && !(arrayNode[i]->getMeshType() & E_MT_EMITTER_MODEL))
			{
				offsetData[0] = elementIndex * sceneDataBufferOffset;
				offsetData[1] = 0;
				offsetData[2] = static_cast<uint32_t>(current->getMaterialUniformBufferIndex() * dynamicAllignment);
				vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current->getPipelineLayout(), 0, 1, &current->refDescriptorSet(), 3, &offsetData[0]);
				vkCmdDrawIndexed(*commandBuffer, arrayNode[i]->getIndexSize(), 1, arrayNode[i]->getStartIndex(), 0, 0);
			}

			if ((i + 1) < maxIndex)
			{
				previous = current;

				materialName  = arrayNode[i + 1]->refMaterial()->getName();
				materialName += m_voxelizationMaterialSuffix;
				current       = materialM->getElement(move(materialName));

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

// TODO: refactor with SceneLightingTechnique::generateVoxelizationMaterials
void SceneVoxelizationTechnique::generateVoxelizationMaterials()
{
	// TODO: use descriptor sets to avoid material instances
	const vectorNodePtr& arrayNode = sceneM->getModel();
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
			string newName        = material->getName() + m_voxelizationMaterialSuffix;
			Material* newMaterial = materialM->getElement(move(string(newName)));

			if (newMaterial == nullptr)
			{
				MultiTypeUnorderedMap *attributeMaterial = new MultiTypeUnorderedMap();
				attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_reflectanceTextureResourceName), string(materialCasted->getReflectanceTextureName())));
				attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_normalTextureResourceName), string(materialCasted->getNormalTextureName())));
				newMaterial = materialM->buildMaterial(move(string("MaterialSceneVoxelization")), move(string(newName)), attributeMaterial);
				vectorMaterialName.push_back(newName);
				vectorMaterial.push_back(newMaterial);
			}
		}
	}
	
	m_vectorMaterialName = vectorMaterialName;
	m_vectorMaterial     = vectorMaterial;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneVoxelizationTechnique::updateEmitterVoxelizationInfo()
{
	BBox3D& box               = sceneM->refBox();
	vec3 min                  = box.getMin();
	vec3 max                  = box.getMax();
	float maxDimensionSize    = glm::max((max.x - min.x), glm::max((max.y - min.y), (max.z - min.z)));
	float maxVoxelizationSize = float(glm::max(m_voxelizedSceneWidth, glm::max(m_voxelizedSceneHeight, m_voxelizedSceneDepth)));
	m_stepMultiplier          = maxDimensionSize / maxVoxelizationSize;

	// Pending:
	// 1. Apply first initial offset to properly center the emitter geometry in voxel space (forward direction * some amount of step)
	// 2. Obtain the number of steps until the emitter gets out of the aabb totally (all vertices of the plane)

	vectorNodePtr arrayNode = sceneM->getByMeshType(E_MT_EMITTER_MODEL);
	const uint maxIndex     = uint(arrayNode.size());
	uvec3 voxelizationSize  = uvec3(m_voxelizedSceneWidth, m_voxelizedSceneHeight, m_voxelizedSceneDepth);
	float angleRadians      = glm::radians(45.0f);

	EmitterNode* node;
	forI(maxIndex)
	{
		node = static_cast<EmitterNode*>(arrayNode[i]);
		node->refTransform().addTraslation(node->getInitialTraslation());
		node->setupEmitter(m_stepMultiplier, voxelizationSize, angleRadians);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SceneVoxelizationTechnique::buildEmitterBuffer()
{
	vectorNodePtr arrayNode = sceneM->getByMeshType(E_MT_EMITTER_MODEL);
	const uint maxIndex     = uint(arrayNode.size());

	EmitterNode* node;
	uint finalSize = 1;
	forI(maxIndex)
	{
		node       = static_cast<EmitterNode*>(arrayNode[i]);
		finalSize += uint(node->getBufferData().size());
		finalSize++;
	}

	// vertex data and information about each emitter is all in a single float buffer
	// The format is:
	// Total number of emitters
	//		Number of elements of the first emitter
	//			first triangle of the emitter vertices,
	//          normal of this triangle,
	//          second triangle of the emitter vertices,
	//          normal of this triangle,
	//          ...
	//     Number of elements of the second emitter
	//			first triangle of the emitter vertices,
	//          normal of this triangle,
	//          second triangle of the emitter vertices,
	//          normal of this triangle,
	//          ...
	//     ...
	// Example: N0 E0 V0 V1 V2 N0 V3 V4 V5 N1 V6 V7 V8 N3, ..., E1, V9 V10 V11 N4, ...

	vectorFloat vectorFinal;
	vectorFinal.resize(finalSize);
	uint counter = 0;
	vectorFinal[counter] = float(maxIndex);
	forI(maxIndex)
	{
		node = static_cast<EmitterNode*>(arrayNode[i]);
		const vectorFloat& vecEmitterGeometry = node->getBufferData();
		counter++;
		vectorFinal[counter] = float(vecEmitterGeometry.size());
		counter++;
		memcpy((void*)(&vectorFinal[counter]), vecEmitterGeometry.data(), sizeof(float) * vecEmitterGeometry.size());
		counter += uint(vecEmitterGeometry.size());
	}

	m_emitterBuffer = bufferM->buildBuffer(
		move(string("emitterBuffer")),
		vectorFinal.data(),
		vectorFinal.size() * sizeof(float),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
