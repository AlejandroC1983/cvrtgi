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
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/core/coremanager.h"
#include "../../include/shader/shaderreflection.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
uint ShaderManager::m_nextInstanceSuffix = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderManager::ShaderManager()
{
	m_managerName = g_shaderManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderManager::~ShaderManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::obtainMaxPushConstantsSize()
{
	const VkPhysicalDeviceLimits& physicalDeviceLimits = coreM->getPhysicalDeviceProperties().limits;
	m_maxPushConstantsSize                             = physicalDeviceLimits.maxPushConstantsSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::addGlobalHeaderSourceCode(string&& code)
{
	m_globalHeaderSourceCode += code;
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint ShaderManager::getNextInstanceSuffix()
{
	return m_nextInstanceSuffix++;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShaderV(string&& instanceName, const char *vertexShaderText, MaterialSurfaceType surfaceType)
{
	assert(vertexShaderText != nullptr);
	vectorCharPtr arrayShader = { vertexShaderText, nullptr, nullptr, nullptr };
	return buildShader(move(instanceName), arrayShader, surfaceType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShaderVG(string&& instanceName, const char *vertexShaderText, const char *geometryShaderText, MaterialSurfaceType surfaceType)
{
	assert((vertexShaderText != nullptr) || (geometryShaderText != nullptr));
	vectorCharPtr arrayShader = { vertexShaderText, geometryShaderText, nullptr, nullptr };
	return buildShader(move(instanceName), arrayShader, surfaceType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShaderVF(string&& instanceName, const char *vertexShaderText, const char *fragmentShaderText, MaterialSurfaceType surfaceType)
{
	assert((vertexShaderText != nullptr) || (fragmentShaderText != nullptr));
	vectorCharPtr arrayShader = { vertexShaderText, nullptr, fragmentShaderText, nullptr };
	return buildShader(move(instanceName), arrayShader, surfaceType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShaderVGF(string&& instanceName, const char *vertexShaderText, const char *geometryShaderText, const char *fragmentShaderText, MaterialSurfaceType surfaceType)
{
	assert((vertexShaderText != nullptr) || (geometryShaderText != nullptr) || (fragmentShaderText != nullptr));
	vectorCharPtr arrayShader = { vertexShaderText, geometryShaderText, fragmentShaderText, nullptr };
	return buildShader(move(instanceName), arrayShader, surfaceType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShaderC(string&& instanceName, const char *computeShaderText, MaterialSurfaceType surfaceType)
{
	assert(computeShaderText != nullptr);
	vectorCharPtr arrayShader = { nullptr, nullptr, nullptr, computeShaderText };
	return buildShader(move(instanceName), arrayShader, surfaceType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Shader* ShaderManager::buildShader(string&& instanceName, vectorCharPtr arrayShader, MaterialSurfaceType surfaceType)
{
	if (existsElement(move(instanceName)))
	{
		return getElement(move(instanceName));
	}

	Shader* shader = new Shader(move(string(instanceName)));
	shader->addShaderHeaderSourceCode(surfaceType);
	vector<vector<uint>> arraySPVShaderStage;
	shader->m_arrayShaderStages = buildShaderStages(arrayShader, shader->getHeaderSourceCode(), arraySPVShaderStage);

	if ((arrayShader.size() == 4) && 
		(arrayShader[0] == nullptr) && (arrayShader[1] == nullptr) && 
		(arrayShader[2] == nullptr) && (arrayShader[3] != nullptr))
	{
		shader->m_isCompute = true;
	}

	forI(arraySPVShaderStage.size())
	{
		ShaderReflection::extractResources(move(arraySPVShaderStage[i]),
			shader->m_vecUniformBase,
			shader->m_vecTextureSampler,
			shader->m_vecImageSampler,
			shader->m_vecAtomicCounterUnit,
			shader->m_vecShaderStruct,
			shader->m_vectorShaderStorageBuffer,
			&shader->m_pushConstant);
	}

	shader->init();
	if (shader->m_pushConstant.m_vecUniformBase.size() > 0)
	{
		shader->m_pushConstant.m_CPUBuffer.buildCPUBuffer(shader->m_pushConstant.m_vecUniformBase);
	}

	addElement(move(string(instanceName)), shader);
	shader->m_name = move(instanceName);

	return shader;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderManager::GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *pShader, vector<unsigned int> &spirv)
{
	glslang::TProgram* program = new glslang::TProgram;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	initializeResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	EShLanguage stage = getLanguage(shaderType);
	glslang::TShader* shader = new glslang::TShader(stage);

	shaderStrings[0] = pShader;
	shader->setStrings(shaderStrings, 1);

	if (!shader->parse(&Resources, 100, false, messages))
	{
		puts(shader->getInfoLog());
		puts(shader->getInfoDebugLog());
		return false;
	}

	program->addShader(shader);

	// Link the program and report if errors...
	if (!program->link(messages))
	{
		puts(shader->getInfoLog());
		puts(shader->getInfoDebugLog());
		return false;
	}

	glslang::GlslangToSpv(*program->getIntermediate(stage), spirv);
	delete program;
	delete shader;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

EShLanguage ShaderManager::getLanguage(const VkShaderStageFlagBits shaderType)
{
	switch (shaderType)
	{
		case VK_SHADER_STAGE_VERTEX_BIT:
		{
			return EShLangVertex;
		}
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		{
			return EShLangTessControl;
		}
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		{
			return EShLangTessEvaluation;
		}
		case VK_SHADER_STAGE_GEOMETRY_BIT:
		{
			return EShLangGeometry;
		}
		case VK_SHADER_STAGE_FRAGMENT_BIT:
		{
			return EShLangFragment;
		}
		case VK_SHADER_STAGE_COMPUTE_BIT:
		{
			return EShLangCompute;
		}
		default:
		{
			printf("Unknown shader type specified: %d. Exiting!", shaderType);
			exit(1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::initializeResources(TBuiltInResource &Resources)
{
	// TODO: update with real data
	Resources.maxLights                                   = 32;
	Resources.maxClipPlanes                               = 6;
	Resources.maxTextureUnits                             = 32;
	Resources.maxTextureCoords                            = 32;
	Resources.maxVertexAttribs                            = 64;
	Resources.maxVertexUniformComponents                  = 4096;
	Resources.maxVaryingFloats                            = 64;
	Resources.maxVertexTextureImageUnits                  = 32;
	Resources.maxCombinedTextureImageUnits                = 80;
	Resources.maxTextureImageUnits                        = 32;
	Resources.maxFragmentUniformComponents                = 4096;
	Resources.maxDrawBuffers                              = 32;
	Resources.maxVertexUniformVectors                     = 128;
	Resources.maxVaryingVectors                           = 8;
	Resources.maxFragmentUniformVectors                   = 16;
	Resources.maxVertexOutputVectors                      = 16;
	Resources.maxFragmentInputVectors                     = 15;
	Resources.minProgramTexelOffset                       = -8;
	Resources.maxProgramTexelOffset                       = 7;
	Resources.maxClipDistances                            = 8;
	Resources.maxComputeWorkGroupCountX                   = 65535;
	Resources.maxComputeWorkGroupCountY                   = 65535;
	Resources.maxComputeWorkGroupCountZ                   = 65535;
	Resources.maxComputeWorkGroupSizeX                    = 1024;
	Resources.maxComputeWorkGroupSizeY                    = 1024;
	Resources.maxComputeWorkGroupSizeZ                    = 64;
	Resources.maxComputeUniformComponents                 = 1024;
	Resources.maxComputeTextureImageUnits                 = 16;
	Resources.maxComputeImageUniforms                     = 8;
	Resources.maxComputeAtomicCounters                    = 8;
	Resources.maxComputeAtomicCounterBuffers              = 1;
	Resources.maxVaryingComponents                        = 60;
	Resources.maxVertexOutputComponents                   = 64;
	Resources.maxGeometryInputComponents                  = 64;
	Resources.maxGeometryOutputComponents                 = 128;
	Resources.maxFragmentInputComponents                  = 128;
	Resources.maxImageUnits                               = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs     = 8;
	Resources.maxCombinedShaderOutputResources            = 8;
	Resources.maxImageSamples                             = 0;
	Resources.maxVertexImageUniforms                      = 0;
	Resources.maxTessControlImageUniforms                 = 0;
	Resources.maxTessEvaluationImageUniforms              = 0;
	Resources.maxGeometryImageUniforms                    = 0;
	Resources.maxFragmentImageUniforms                    = 8;
	Resources.maxCombinedImageUniforms                    = 8;
	Resources.maxGeometryTextureImageUnits                = 16;
	Resources.maxGeometryOutputVertices                   = 256;
	Resources.maxGeometryTotalOutputComponents            = 1024;
	Resources.maxGeometryUniformComponents                = 1024;
	Resources.maxGeometryVaryingComponents                = 64;
	Resources.maxTessControlInputComponents               = 128;
	Resources.maxTessControlOutputComponents              = 128;
	Resources.maxTessControlTextureImageUnits             = 16;
	Resources.maxTessControlUniformComponents             = 1024;
	Resources.maxTessControlTotalOutputComponents         = 4096;
	Resources.maxTessEvaluationInputComponents            = 128;
	Resources.maxTessEvaluationOutputComponents           = 128;
	Resources.maxTessEvaluationTextureImageUnits          = 16;
	Resources.maxTessEvaluationUniformComponents          = 1024;
	Resources.maxTessPatchComponents                      = 120;
	Resources.maxPatchVertices                            = 32;
	Resources.maxTessGenLevel                             = 64;
	Resources.maxViewports                                = 16;
	Resources.maxVertexAtomicCounters                     = 0;
	Resources.maxTessControlAtomicCounters                = 0;
	Resources.maxTessEvaluationAtomicCounters             = 0;
	Resources.maxGeometryAtomicCounters                   = 0;
	Resources.maxFragmentAtomicCounters                   = 8;
	Resources.maxCombinedAtomicCounters                   = 8;
	Resources.maxAtomicCounterBindings                    = 1;
	Resources.maxVertexAtomicCounterBuffers               = 0;
	Resources.maxTessControlAtomicCounterBuffers          = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers       = 0;
	Resources.maxGeometryAtomicCounterBuffers             = 0;
	Resources.maxFragmentAtomicCounterBuffers             = 1;
	Resources.maxCombinedAtomicCounterBuffers             = 1;
	Resources.maxAtomicCounterBufferSize                  = 16384;
	Resources.maxTransformFeedbackBuffers                 = 4;
	Resources.maxTransformFeedbackInterleavedComponents   = 64;
	Resources.maxCullDistances                            = 8;
	Resources.maxCombinedClipAndCullDistances             = 8;
	Resources.maxSamples                                  = 4;
	Resources.limits.nonInductiveForLoops                 = 1;
	Resources.limits.whileLoops                           = 1;
	Resources.limits.doWhileLoops                         = 1;
	Resources.limits.generalUniformIndexing               = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing               = 1;
	Resources.limits.generalSamplerIndexing               = 1;
	Resources.limits.generalVariableIndexing              = 1;
	Resources.limits.generalConstantMatrixVectorIndexing  = 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////

vector<VkPipelineShaderStageCreateInfo> ShaderManager::buildShaderStages(const vectorCharPtr& arrayShader, const string& shaderHeaderSourceCode, vector<vector<uint>>& arraySPVShaderStage)
{
	//Index 0 is the vertex shader information
	//Index 1 is the geometry shader information
	//Index 2 is the fragment shader information
	//Index 3 is the compute shader information
	vector<VkPipelineShaderStageCreateInfo> arrayShaderStage;

	string tempFullShaderSource;

	glslang::InitializeProcess();

	forI(arrayShader.size())
	{
		if (arrayShader[i] != nullptr)
		{
			vector<uint> arrayShaderStageSPV;
			VkPipelineShaderStageCreateInfo shaderStage = {};

			shaderStage.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.pNext               = NULL;
			shaderStage.pSpecializationInfo = NULL;
			shaderStage.flags               = 0;
			shaderStage.stage               = (i == 0) ? VK_SHADER_STAGE_VERTEX_BIT : (i == 1) ? VK_SHADER_STAGE_GEOMETRY_BIT : (i == 2) ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_COMPUTE_BIT;
			shaderStage.pName               = "main"; // Assuming the entry function is always "main()"

			tempFullShaderSource            = m_globalHeaderSourceCode;
			tempFullShaderSource           += shaderHeaderSourceCode;
			tempFullShaderSource           += arrayShader[i];
			bool retVal                     = GLSLtoSPV(shaderStage.stage, tempFullShaderSource.c_str(), arrayShaderStageSPV);

			if (!retVal)
			{
				outputSourceWithLineNumber(tempFullShaderSource);
			}

			assert(retVal);

			VkShaderModuleCreateInfo moduleCreateInfo;
			moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.pNext    = NULL;
			moduleCreateInfo.flags    = 0;
			moduleCreateInfo.codeSize = arrayShaderStageSPV.size() * sizeof(unsigned int);
			moduleCreateInfo.pCode    = arrayShaderStageSPV.data();

			VkResult result = vkCreateShaderModule(coreM->getLogicalDevice(), &moduleCreateInfo, NULL, &shaderStage.module);
			assert(result == VK_SUCCESS);

			arrayShaderStage.push_back(shaderStage);
			arraySPVShaderStage.push_back(arrayShaderStageSPV);
		}
	}

	glslang::FinalizeProcess();

	return arrayShaderStage;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::assignSlots()
{
	textureM->refElementSignal().connect<ShaderManager, &ShaderManager::slotElement>(this);
	bufferM->refElementSignal().connect<ShaderManager, &ShaderManager::slotElement>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType)
{
	if (!gpuPipelineM->getPipelineInitialized())
	{
		return;
	}

	vectorShaderPtr vectorModifiedShader;

	if (managerName == g_textureManager)
	{
		map<string, Shader*>::iterator it = m_mapElement.begin();
		for (it; it != m_mapElement.end(); ++it)
		{
			if (it->second->textureResourceNotification(move(string(elementName)), notificationType))
			{
				vectorModifiedShader.push_back(it->second);
			}
		}
	}

	if (managerName == g_bufferManager)
	{
		map<string, Shader*>::iterator it = m_mapElement.begin();
		for (it; it != m_mapElement.end(); ++it)
		{
			if (it->second->bufferResourceNotification(move(string(elementName)), notificationType))
			{
				vectorModifiedShader.push_back(it->second);
			}
		}
	}

	if (gpuPipelineM->getPipelineInitialized())
	{
		forIT(vectorModifiedShader)
		{
			emitSignalElement(move(string((*it)->getName())), notificationType);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderManager::outputSourceWithLineNumber(const string& sourceCode)
{
	string delimiter       = "\n";
	string::size_type pos  = 0;
	string::size_type prev = 0;
	uint counter           = 1;

	cout << "-----------------------------------------------------------\n";
	while ((pos = sourceCode.find(delimiter, prev)) != string::npos)
	{
		cout << counter;
		if (counter < 100)
		{
			cout << "   ";
		}
		else
		{
			cout << "  ";
		}

		cout << sourceCode.substr(prev, pos - prev) << endl;
		counter++;
		prev = pos + 1;
	}
	cout << "-----------------------------------------------------------\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////
