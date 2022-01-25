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
#include "../../include/material/material.h"
#include "../../include/material/materialmanager.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/texture/texture.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/shader/shader.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/sampler.h"
#include "../../include/shader/shaderreflection.h"
#include "../../include/shader/uniformBase.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/shader/shaderstoragebuffer.h"
#include "../../include/buffer/buffer.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Material::Material(string &&name, string &&className) : GenericResource(move(name), move(className), GenericResourceType::GRT_MATERIAL)
	, m_shader(nullptr)
	, m_pipelineLayout(VK_NULL_HANDLE)
	, m_exposedStructFieldDirty(false)
	, m_pushConstantExposedStructFieldDirty(false)
	, m_exposedStructFieldSize(0)
	, m_pushConstantExposedStructFieldSize(0)
	, m_materialUniformBufferIndex(-1)
	, m_isCompute(false)
	, m_isEmitter(false)
	, m_resourcesUsed(MaterialBufferResource::MBR_MODEL | MaterialBufferResource::MBR_CAMERA | MaterialBufferResource::MBR_MATERIAL)
	, m_materialInstanceIndex(shaderM->getNextInstanceSuffix())
	, m_materialSurfaceType(MaterialSurfaceType::MST_OPAQUE)
{
	m_vectorClearValue.resize(2);
	m_vectorClearValue[0].color.float32[0] = 1.0f;
	m_vectorClearValue[0].color.float32[1] = 1.0f;
	m_vectorClearValue[0].color.float32[2] = 1.0f;
	m_vectorClearValue[0].color.float32[3] = 1.0f;

	// Specify the depth/stencil clear value
	m_vectorClearValue[1].depthStencil.depth   = 1.0f;
	m_vectorClearValue[1].depthStencil.stencil = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Material::~Material()
{
	destroyMaterialResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::init()
{
	loadResources();
	loadShader();

	if (m_shader != nullptr)
	{
		m_isCompute = m_shader->getIsCompute();
	}

	buildMaterialResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::matchExposedResources()
{
	bool result = true;
	const vectorUniformBasePtr& vecUniformBase = m_shader->getVecUniformBase();

	forI(m_vectorExposedStructField.size())
	{
		ExposedStructField& exposedStructField = m_vectorExposedStructField[i];
		bool foundStructFieldResource = false;

		forJ(vecUniformBase.size())
		{
			UniformBase* uniform = vecUniformBase[j];

			if ((exposedStructField.getInternalType()    == uniform->getResourceInternalType()) &&
				(exposedStructField.getStructName()      == uniform->getStructName()) &&
				(exposedStructField.getStructFieldName() == uniform->getName()))
			{
				exposedStructField.setStructFieldResource(uniform);
				foundStructFieldResource = true;
			}
		}

		if (!foundStructFieldResource)
		{
			cout << "ERROR: No match found for struct field resource " << exposedStructField.getStructName() << "." << exposedStructField.getStructName() << endl;
		}

		if (exposedStructField.getStructFieldResource() == nullptr)
		{
			result = false;
		}
	}

	const vectorUniformBasePtr& vecPushConstantUniformBase = m_shader->refPushConstant().refVecUniformBase();
	forI(m_vectorPushConstantExposedStructField.size())
	{
		ExposedStructField& exposedStructField = m_vectorPushConstantExposedStructField[i];

		forJ(vecPushConstantUniformBase.size())
		{
			UniformBase* uniform = vecPushConstantUniformBase[j];

			if ((exposedStructField.getInternalType()    == uniform->getResourceInternalType()) &&
				(exposedStructField.getStructName()      == uniform->getStructName()) &&
				(exposedStructField.getStructFieldName() == uniform->getName()))
			{
				exposedStructField.setStructFieldResource(uniform);
			}
		}

		if (exposedStructField.getStructFieldResource() == nullptr)
		{
			result = false;
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::unmatchExposedResources()
{
	m_vectorDirtyExposedStructField.clear();

	forI(m_vectorExposedStructField.size())
	{
		m_vectorExposedStructField[i].setStructFieldResource(nullptr);
	}

	forI(m_vectorPushConstantExposedStructField.size())
	{
		m_vectorPushConstantExposedStructField[i].setStructFieldResource(nullptr);
	}

	m_exposedStructFieldSize             = 0;
	m_pushConstantExposedStructFieldSize = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::buildPipeline()
{
	// This are the steps to use resources in shaders
	// Pipeline side:
	// 1. For each data type to use (textures, uniform buffer, etc), make a VkDescriptorSetLayoutBinding
	// 2. Take all generated VkDescriptorSetLayoutBindings and make a VkDescriptorSetLayout
	// 3. Take the generated VkDescriptorSetLayout and generate a VkPipelineLayout, used when building a pipeline for the corresponding shader
	// Shader side:
	// 1. Build a descriptor pool, with the types of descriptor sets that will be contain (and how many will be generated), using a struct of
	//    type VkDescriptorPoolSize. Specify how many allocations will be done.
	// 2. Generate the corresponding descriptor set
	// 3. For this generated descriptor set, fill the details of the descriptors it has within: for each binding, allocate a VkWriteDescriptorSet
	//    (Note: for uniform buffers, an extra structure of type VkDescriptorBufferInfo inside one of the fields of the VkWriteDescriptorSet is needed
	// 4. Update the descriptor set with this information given by the VkWriteDescriptorSet data
	// 5. Use this descriptor set together with the generated VkPipelineLayout in descriptor set binding commands like vkCmdBindDescriptorSets

	uint numResourcesUsed = getNumDynamicUniformBufferResourceUsed();

	m_arrayDescriptorSetLayout.resize(numResourcesUsed);

	m_arrayDescriptorSetLayoutIsShared.resize(numResourcesUsed);

	forI(numResourcesUsed)
	{
		m_arrayDescriptorSetLayoutIsShared[i] = true;
	}

	vector<VkDescriptorType>   vectorDescriptorType;
	vector<uint32_t>           vectorBindingIndex;
	vector<VkShaderStageFlags> vectorStageFlags;
	vector<void*>              vectorDescriptorInfo;
	vector<int>                vectorDescriptorInfoHint;

	uint numDescriptorInfo = numResourcesUsed;
	numDescriptorInfo     += uint(m_shader->getVecTextureSampler().size());
	numDescriptorInfo     += uint(m_shader->getVecImageSampler().size());
	numDescriptorInfo     += uint(m_shader->getVectorShaderStorageBuffer().size());

	vectorDescriptorInfo.resize(numDescriptorInfo);
	uint descriptorInfoCounter = 0;

	forI(numResourcesUsed)
	{
		vectorDescriptorType.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	}

	if (m_isCompute)
	{
		forI(numResourcesUsed)
		{
			vectorStageFlags.push_back(VK_SHADER_STAGE_COMPUTE_BIT);
		}
	}
	else
	{
		// TODO: automatize this part, "oring" each general buffer used for every shader stage truly used in each shader
		if (m_resourcesUsed & MaterialBufferResource::MBR_MODEL)
		{
			vectorStageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);
		}
		if (m_resourcesUsed & MaterialBufferResource::MBR_CAMERA)
		{
			vectorStageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);
		}
		if (m_resourcesUsed & MaterialBufferResource::MBR_MATERIAL)
		{
			vectorStageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_MODEL)
	{
		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&(gpuPipelineM->refSceneUniformData()->refBufferInfo()));
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(0);
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_CAMERA)
	{
		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&(gpuPipelineM->refSceneCameraUniformData()->refBufferInfo()));
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(0);
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_MATERIAL)
	{
		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&(materialM->refMaterialUniformData()->refBufferInfo()));
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(0);
	}
	
	forI(numResourcesUsed)
	{
		vectorBindingIndex.push_back(i);
	}

	uint numImageDescriptorInfo = 0;
	vector<VkDescriptorImageInfo> vectorDescriptorImageInfo;
	numImageDescriptorInfo += uint(m_shader->getVecTextureSampler().size());
	numImageDescriptorInfo += uint(m_shader->getVecImageSampler().size());
	vectorDescriptorImageInfo.resize(numImageDescriptorInfo);
	uint imageDescriptorInfoCounter = 0;

	VkDescriptorImageInfo descriptorImageInfo;
	const vectorSamplerPtr& arraySampler = m_shader->getVecTextureSampler();
	forIT(arraySampler)
	{
		m_arrayDescriptorSetLayoutIsShared.push_back(false);
		descriptorImageInfo = { (*it)->getSamplerHandle(), (*it)->getTexture()->getView(), (*it)->getTexture()->getImageLayout() };
		vectorDescriptorImageInfo[imageDescriptorInfoCounter] = descriptorImageInfo;
		imageDescriptorInfoCounter++;

		vectorDescriptorType.push_back((*it)->getDescriptorType());
		vectorBindingIndex.push_back((*it)->getBindingIndex());
		vectorStageFlags.push_back(m_isCompute ? VK_SHADER_STAGE_COMPUTE_BIT : (*it)->getShaderStage());

		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&vectorDescriptorImageInfo[imageDescriptorInfoCounter - 1]);
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(1);
	}

	const vectorSamplerPtr& arrayImage = m_shader->getVecImageSampler();
	forIT(arrayImage)
	{
		m_arrayDescriptorSetLayoutIsShared.push_back(false);
		descriptorImageInfo = { (*it)->getSamplerHandle(), (*it)->getTexture()->getView(), (*it)->getTexture()->getImageLayout() };
		vectorDescriptorImageInfo[imageDescriptorInfoCounter] = descriptorImageInfo;
		imageDescriptorInfoCounter++;

		vectorDescriptorType.push_back((*it)->getDescriptorType());
		vectorBindingIndex.push_back((*it)->getBindingIndex());
		vectorStageFlags.push_back(m_isCompute ? VK_SHADER_STAGE_COMPUTE_BIT : (*it)->getShaderStage());

		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&vectorDescriptorImageInfo[imageDescriptorInfoCounter - 1]);
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(1);
	}

	const vectorShaderStorageBufferPtr& arrayShaderStorageBuffer = m_shader->getVectorShaderStorageBuffer();
	forIT(arrayShaderStorageBuffer)
	{
		m_arrayDescriptorSetLayoutIsShared.push_back(false);
		vectorDescriptorType.push_back((*it)->getDescriptorType());
		vectorBindingIndex.push_back((*it)->getBindingIndex());
		vectorStageFlags.push_back(m_isCompute ? VK_SHADER_STAGE_COMPUTE_BIT : (*it)->getShaderStage());

		vectorDescriptorInfo[descriptorInfoCounter] = (void*)(&(*it)->refBuffer()->refDescriptorBufferInfo());
		descriptorInfoCounter++;
		vectorDescriptorInfoHint.push_back(0);
	}

	m_descriptorSet = gpuPipelineM->buildDescriptorSet(
		vectorDescriptorType,
		vectorBindingIndex,
		vectorStageFlags,
		m_descriptorSetLayout,
		m_descriptorPool);

	gpuPipelineM->updateDescriptorSet(
		m_descriptorSet,
		vectorDescriptorType,
		vectorDescriptorInfo,
		vectorDescriptorInfoHint,
		vectorBindingIndex);

	// Create the pipeline layout with the help of descriptor layout.
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext                  = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges    = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount         = 1;
	pPipelineLayoutCreateInfo.pSetLayouts            = &m_descriptorSetLayout;

	// If a push constant was declared and has at least one element, add to the pipeline
	// NOTE: for now, only one push constant range will be used
	VkPushConstantRange pushConstantRange{};
	if (m_shader->refPushConstant().refVecUniformBase().size() > 0)
	{
		pushConstantRange.stageFlags                     = m_shader->refPushConstant().getShaderStages();
		pushConstantRange.offset                         = 0;
		pushConstantRange.size                           = m_pushConstantExposedStructFieldSize;
		pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pPipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
	}

	VkResult  result;
	result = vkCreatePipelineLayout(coreM->getLogicalDevice(), &pPipelineLayoutCreateInfo, NULL, &m_pipelineLayout);
	assert(result == VK_SUCCESS);

	if (m_isCompute)
	{
		VkComputePipelineCreateInfo computePipelineCreateInfo;
		computePipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext              = nullptr;
		computePipelineCreateInfo.flags              = 0;
		computePipelineCreateInfo.stage              = m_shader->refArrayShaderStages()[0];
		computePipelineCreateInfo.layout             = m_pipelineLayout;
		computePipelineCreateInfo.basePipelineHandle = nullptr;
		computePipelineCreateInfo.basePipelineIndex  = 0;
		
		result = vkCreateComputePipelines(coreM->getLogicalDevice(), gpuPipelineM->getPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &m_pipeline.refPipeline());
	}
	else
	{
		m_pipeline.setPipelineShaderStage(m_shader->refArrayShaderStages());
		m_pipeline.setPipelineLayout(m_pipelineLayout);

		result = vkCreateGraphicsPipelines(coreM->getLogicalDevice(), gpuPipelineM->getPipelineCache(), 1, &m_pipeline.refPipelineData().getPipelineInfo(), nullptr, &m_pipeline.refPipeline());
	}
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::update(float dt)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::updateExposedResources()
{
	m_vectorDirtyExposedStructField.clear();

	forIT(m_vectorExposedStructField)
	{
		if (ShaderReflection::setResourceCPUValue((*it)))
		{
			addIfNoPresent(&(*it), m_vectorDirtyExposedStructField);
		}
	}

	m_exposedStructFieldDirty = (m_vectorDirtyExposedStructField.size() > 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::updatePushConstantExposedResources()
{
	m_vectorPushConstantDirtyExposedStructField.clear();

	forIT(m_vectorPushConstantExposedStructField)
	{
		if (ShaderReflection::setResourceCPUValue((*it)))
		{
			addIfNoPresent(&(*it), m_vectorPushConstantDirtyExposedStructField);
		}
	}

	m_pushConstantExposedStructFieldDirty = (m_vectorPushConstantDirtyExposedStructField.size() > 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::loadShader()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::buildMaterialResources()
{
	exposeResources();
	m_exposedStructFieldSize             = computeExposedStructFieldSize(m_vectorExposedStructField);
	m_pushConstantExposedStructFieldSize = computeExposedStructFieldSize(m_vectorPushConstantExposedStructField);
	m_shader->initializeSamplerHandlers();
	m_shader->initializeImageHandlers();
	if (!matchExposedResources())
	{
		cout << "ERRROR: Material::matchExposedResources returned false" << endl;
	}
	setupPipelineData();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::loadResources()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::exposeResources()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::setupPipelineData()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::exposeStructField(ResourceInternalType internalType, void* data, string&& structName, string&& structFieldName)
{
	if ((data == nullptr) || !ShaderReflection::isDataType(internalType))
	{
		return false;
	}

	m_vectorExposedStructField.push_back(ExposedStructField(internalType, move(structName), move(structFieldName), data));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::pushConstantExposeStructField(ResourceInternalType internalType, void* data, string&& structName, string&& structFieldName)
{
	if ((data == nullptr) || !ShaderReflection::isDataType(internalType))
	{
		return false;
	}

	m_vectorPushConstantExposedStructField.push_back(ExposedStructField(internalType, move(structName), move(structFieldName), data));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::assignTextureToSampler(string&& textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType)
{
	return assignTextureToSampler(move(textureSamplerName), move(textureResourceName), descriptorType, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_LINEAR);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::assignTextureToSampler(string&& textureSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter)
{
	return m_shader->setTextureToSample(move(textureSamplerName), move(textureResourceName), descriptorType, mipmapMode, minMagFilter);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::assignImageToSampler(string&& imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType)
{
	return m_shader->setImageToSample(move(imageSamplerName), move(textureResourceName), descriptorType, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_LINEAR);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::assignImageToSampler(string&& imageSamplerName, string&& textureResourceName, VkDescriptorType descriptorType, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter)
{
	return m_shader->setImageToSample(move(imageSamplerName), move(textureResourceName), descriptorType, mipmapMode, minMagFilter);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::assignShaderStorageBuffer(string &&storageBufferName, string&& bufferResourceName, VkDescriptorType descriptorType)
{
	return m_shader->setShaderStorageBuffer(move(storageBufferName), move(bufferResourceName), descriptorType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::writeExposedDataToMaterialUB() const
{
	if (m_materialUniformBufferIndex == -1)
	{
		cout << "ERROR: m_materialUniformBufferIndex is -1 in Material::writeExposedDataToMaterialUB" << endl;
		return;
	}

	UniformBuffer* ubo = materialM->refMaterialUniformData();
	ubo->refCPUBuffer().resetDataAtCell(m_materialUniformBufferIndex);

	forIT(m_vectorExposedStructField)
	{
		ShaderReflection::appendExposedStructFieldDataToUniformBufferCell(&ubo->refCPUBuffer(), m_materialUniformBufferIndex, &(*it));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::pushConstantWriteExposedDataToCPUBuffer() const
{
	CPUBuffer* cpuBuffer = &m_shader->refPushConstant().refCPUBuffer();

	int index;
	forIT(m_vectorPushConstantExposedStructField)
	{
		index = getPushConstantExposedResourceIndex(it->getInternalType(), it->getStructName(), it->getStructFieldName());
		cpuBuffer->resetDataAtCell(index);
		ShaderReflection::appendExposedStructFieldDataToUniformBufferCell(cpuBuffer, index, &(*it));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

int Material::computeExposedStructFieldSize(const vector<ExposedStructField>& vectorExposed)
{
	int result = 0;

	forIT(vectorExposed)
	{
		result += resourceenum::getResourceInternalTypeSizeInBytes(it->getInternalType());
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyDescriptorLayout()
{
	for (int i = 0; i < m_arrayDescriptorSetLayout.size(); i++)
	{
		if (!m_arrayDescriptorSetLayoutIsShared[i])
		{
			vkDestroyDescriptorSetLayout(coreM->getLogicalDevice(), m_arrayDescriptorSetLayout[i], NULL);
		}
	}

	m_arrayDescriptorSetLayoutIsShared.clear();
	m_arrayDescriptorSetLayout.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyPipelineLayouts()
{
	if (m_pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(coreM->getLogicalDevice(), m_pipelineLayout, NULL);

		m_pipelineLayout = VK_NULL_HANDLE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyDescriptorPool()
{
	forIT(m_vectorTextureDescriptorPool)
	{
		vkDestroyDescriptorPool(coreM->getLogicalDevice(), (*it), NULL);
	}

	forIT(m_vectorImageDescriptorPool)
	{
		vkDestroyDescriptorPool(coreM->getLogicalDevice(), (*it), NULL);
	}

	forIT(m_vectorShaderStorageDescriptorPool)
	{
		vkDestroyDescriptorPool(coreM->getLogicalDevice(), (*it), NULL);
	}

	m_vectorTextureDescriptorPool.clear();
	m_vectorImageDescriptorPool.clear();
	m_vectorShaderStorageDescriptorPool.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyDescriptorSet()
{
	VkResult result;

	forI(m_vectorTextureDescriptorSet.size())
	{
		result = vkFreeDescriptorSets(coreM->getLogicalDevice(), m_vectorTextureDescriptorPool[i], (uint32_t)1, &m_vectorTextureDescriptorSet[i]);
		assert(result);
	}

	forI(m_vectorImageDescriptorSet.size())
	{
		result = vkFreeDescriptorSets(coreM->getLogicalDevice(), m_vectorImageDescriptorPool[i], (uint32_t)1, &m_vectorImageDescriptorSet[i]);
		assert(result);
	}

	forI(m_vectorShaderStorageDescriptorSet.size())
	{
		result = vkFreeDescriptorSets(coreM->getLogicalDevice(), m_vectorShaderStorageDescriptorPool[i], (uint32_t)1, &m_vectorShaderStorageDescriptorSet[i]);
		assert(result);
	}

	m_vectorTextureDescriptorSet.clear();
	m_vectorImageDescriptorSet.clear();
	m_vectorShaderStorageDescriptorSet.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::bindTextureSamplers(VkCommandBuffer* commandBuffer) const
{
	const vectorSamplerPtr& arraySampler = m_shader->getVecTextureSampler();
	forI(arraySampler.size())
	{
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, arraySampler[i]->getSetIndex(), 1, &(m_vectorTextureDescriptorSet[i]), 0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::bindImageSamplers(VkCommandBuffer* commandBuffer) const
{
	const vectorSamplerPtr& arrayImage = m_shader->getVecImageSampler();
	forI(arrayImage.size())
	{
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, arrayImage[i]->getSetIndex(), 1, &(m_vectorImageDescriptorSet[i]), 0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::bindShaderStorageBuffers(VkCommandBuffer* commandBuffer) const
{
	const vectorShaderStorageBufferPtr& arrayShaderStorageBuffer = m_shader->getVectorShaderStorageBuffer();
	forI(arrayShaderStorageBuffer.size())
	{
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, arrayShaderStorageBuffer[i]->getSetIndex(), 1, &(m_vectorShaderStorageDescriptorSet[i]), 0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::updatePushConstantCPUBuffer()
{
	updatePushConstantExposedResources();

	if(m_pushConstantExposedStructFieldDirty)
	{
		pushConstantWriteExposedDataToCPUBuffer();
		m_pushConstantExposedStructFieldDirty = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

int Material::getPushConstantExposedResourceIndex(ResourceInternalType internalType, string structName, string structFieldName) const
{
	const ExposedStructField* structField;
	forI(m_vectorPushConstantExposedStructField.size())
	{
		structField = &m_vectorPushConstantExposedStructField[i];
		if ((structField->getInternalType() == internalType) && (structField->getStructName() == structName) && (structField->getStructFieldName() == structFieldName))
		{
			return i;
		}
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyDescriptorSetResource()
{
	VkResult result = vkFreeDescriptorSets(coreM->getLogicalDevice(), m_descriptorPool, (uint32_t)1, &m_descriptorSet);
	assert(result == VK_SUCCESS);

	vkDestroyDescriptorPool(coreM->getLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(coreM->getLogicalDevice(), m_descriptorSetLayout, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyPipelineResource()
{
	m_pipeline.destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Material::destroyMaterialResources()
{
	destroyPipelineResource();
	destroyDescriptorSetResource();
	unmatchExposedResources();
	destroyDescriptorLayout();
	destroyPipelineLayouts();
	destroyDescriptorSet();
	destroyDescriptorPool();
	m_pipeline.destroyResources();
	m_vectorExposedStructField.clear();
	m_vectorDirtyExposedStructField.clear();
	m_vectorPushConstantExposedStructField.clear();
	m_vectorPushConstantDirtyExposedStructField.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Material::shaderResourceNotification(string&& shaderResourceName, ManagerNotificationType notificationType)
{
	if (shaderResourceName == m_shaderResourceName)
	{
		switch (notificationType)
		{
			case ManagerNotificationType::MNT_REMOVED:
			{
				m_shader = nullptr;
				setReady(false);
				return true;
			}
			case ManagerNotificationType::MNT_ADDED:
			{
				m_shader = shaderM->getElement(move(shaderResourceName));
				return true;
			}
			case ManagerNotificationType::MNT_CHANGED:
			{
				if (getReady())
				{
					setReady(false);
				}
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint Material::getNumDynamicUniformBufferResourceUsed()
{
	int result = 0;

	if (m_resourcesUsed & MaterialBufferResource::MBR_NONE)
	{
		return 0;
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_MODEL)
	{
		result++;
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_CAMERA)
	{
		result++;
	}

	if (m_resourcesUsed & MaterialBufferResource::MBR_MATERIAL)
	{
		result++;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
