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
#include "../../include/shader/shaderreflection.h"
#include "../../include/shader/shader.h"
#include "../../include/util/containerutilities.h"
#include "../../include/shader/uniform.h"
#include "../../include/shader/shaderstruct.h"
#include "../../include/shader/sampler.h"
#include "../../include/shader/image.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/shader/shaderstoragebuffer.h"
#include "../../include/uniformbuffer/cpubuffer.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION
map<uint, ResourceInternalType> ShaderReflection::m_mapResourceInternalType = initResourceInternalTypeMap();
map<uint, ImageInternalFormat>  ShaderReflection::m_mapImageInternalType = initImageInternalType();

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::extractResources(vector<uint32_t>&&            shaderBinary,
	                                    vectorUniformBasePtr&         vecUniformBase,
	                                    vectorSamplerPtr&             vecSampler,
	                                    vectorSamplerPtr&             vecImage,
	                                    vectorAtomicCounterUnitPtr&   vecAtomicCounterUnit,
	                                    vectorShaderStructPtr&        vecShaderStruct,
										vectorShaderStorageBufferPtr& vectorShaderStorageBuffer,
										PushConstant*                 pushConstant)
{
	// TODO: improve this implementation
	spirv_cross::CompilerGLSL glsl(move(shaderBinary));
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();
	VkShaderStageFlagBits shaderStage      = obtainShaderStage(glsl);

	uint set;
	uint binding;

	// Get sampler information
	Sampler* sampler;
	string samplerTypeString;
	ResourceInternalType samplerTypeEnum;
	for (auto &resource : resources.sampled_images)
	{
		set               = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding           = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type    = glsl.get<SPIRType>(resource.base_type_id);
		samplerTypeString = glsl.type_to_glsl(type);
		samplerTypeEnum   = stringTypeToEnum(move(samplerTypeString));
		sampler           = new Sampler(move(string(resource.name)), samplerTypeEnum, shaderStage, binding, set);
		vecSampler.push_back(sampler);
	}

	// Get image information
	string imageFormatString;
	ImageInternalFormat imageFormatEnum;
	for (auto &resource : resources.storage_images)
	{
		set               = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding           = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type    = glsl.get<SPIRType>(resource.base_type_id);
		imageFormatString = glsl.format_to_glsl(type.image.format);
		imageFormatEnum   = stringImageFormatQualifierToEnum(move(imageFormatString));
		samplerTypeString = glsl.type_to_glsl(type);
		samplerTypeEnum   = stringTypeToEnum(move(samplerTypeString));
		sampler           = new Sampler(move(string(resource.name)), samplerTypeEnum, shaderStage, binding, set, samplerTypeEnum);
		vecImage.push_back(sampler);
	}

	glsl.compile();
	const vector<Variant>& arrayIds = glsl.getIds();
	const vector<Meta>& arrayMetaInformation = glsl.getMetaInformation();

	ShaderStruct* shaderStruct;
	string structTypeString;
	string structFieldNameString;
	string structFieldTypeString;
	ResourceInternalType structFieldType;
	UniformBase* result;
	string structVariableNameString;
	for (auto &resource : resources.uniform_buffers)
	{
		set              = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding          = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type   = glsl.get<SPIRType>(resource.base_type_id);
		structTypeString = glsl.type_to_glsl(type);

		// Get name of the declared variable in the sahder of type this struct
		SPIRType* finalType = nullptr;
		structVariableNameString = "";
		forI(arrayIds.size())
		{
			if (arrayIds[i].get_type() == TypeVariable)
			{
				auto &var = arrayIds[i].get<SPIRVariable>();
				finalType = &glsl.get<SPIRType>(var.basetype);
				if (glsl.type_to_glsl(*finalType, var.self) == resource.name)
				{
					structVariableNameString = arrayMetaInformation[var.self].decoration.alias;
					break;
				}
			}
		}

		assert(structVariableNameString != "");
		assert(finalType != nullptr);

		shaderStruct = new ShaderStruct(move(string(resource.name)),
			move(string("ShaderStruct")),
			ResourceInternalType::RIT_STRUCT,
			shaderStage,
			binding,
			set,
			move(string(string(structVariableNameString))),
			move(string(string(resource.name))));
		vecShaderStruct.push_back(shaderStruct);

		int counter = 0;
		for (const uint32_t& elem : finalType->member_types)
		{
			structFieldNameString      = glsl.getMetaInformation()[finalType->self].members.at(counter).alias;
			SPIRType &structMemberType = glsl.get<SPIRType>(elem);
			structFieldTypeString      = glsl.type_to_glsl(structMemberType);
			structFieldType            = stringTypeToEnum(move(structFieldTypeString));
			if (isDataType(structFieldType))
			{
				result = makeUniform(structFieldType, shaderStage, move(string(structFieldNameString)), move(string(string(structVariableNameString))), move(string(string(resource.name))), shaderStruct);

				if (result != nullptr)
				{
					vecUniformBase.push_back(result);
				}
			}
			counter++;
		}
	}

	// Push constant
	if (resources.push_constant_buffers.size() > 0)
	{
		pushConstant->setShaderStages(pushConstant->getShaderStages() | shaderStage);
	}

	for (auto &resource : resources.push_constant_buffers)
	{
		set              = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding          = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type   = glsl.get<SPIRType>(resource.base_type_id);
		structTypeString = glsl.type_to_glsl(type);

		// Get name of the declared variable in the sahder of type this struct
		SPIRType* finalType = nullptr;
		structVariableNameString = "";
		forI(arrayIds.size())
		{
			if (arrayIds[i].get_type() == TypeVariable)
			{
				auto &var = arrayIds[i].get<SPIRVariable>();
				finalType = &glsl.get<SPIRType>(var.basetype);
				if (glsl.type_to_glsl(*finalType, var.self) == structTypeString)
				{
					structVariableNameString = arrayMetaInformation[var.self].decoration.alias;
					break;
				}
			}
		}

		assert(structVariableNameString != "");
		assert(finalType != nullptr);

		shaderStruct = new ShaderStruct(move(string(resource.name)),
			move(string("ShaderStruct")),
			ResourceInternalType::RIT_STRUCT,
			shaderStage,
			binding,
			set,
			move(string(string(structVariableNameString))),
			move(string(string(structTypeString))));
		vecShaderStruct.push_back(shaderStruct);

		int counter = 0;
		for (const uint32_t& elem : finalType->member_types)
		{
			structFieldNameString      = glsl.getMetaInformation()[finalType->self].members.at(counter).alias;
			SPIRType &structMemberType = glsl.get<SPIRType>(elem);
			structFieldTypeString      = glsl.type_to_glsl(structMemberType);
			structFieldType            = stringTypeToEnum(move(structFieldTypeString));
			if (isDataType(structFieldType))
			{
				result = makeUniform(structFieldType, shaderStage, move(string(structFieldNameString)), move(string(string(structVariableNameString))), move(string(string(resource.name))), shaderStruct);

				if (result != nullptr)
				{
					pushConstant->refVecUniformBase().push_back(result);
				}
			}
			counter++;
		}
	}
	
	// Constant buffers
	for (auto &resource : resources.push_constant_buffers)
	{
		set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type = glsl.get<SPIRType>(resource.base_type_id);
		structTypeString = glsl.type_to_glsl(type);

		// Get name of the declared variable in the sahder of type this struct
		SPIRType* finalType = nullptr;
		structVariableNameString = "";
		forI(arrayIds.size())
		{
			if (arrayIds[i].get_type() == TypeVariable)
			{
				auto &var = arrayIds[i].get<SPIRVariable>();
				finalType = &glsl.get<SPIRType>(var.basetype);
				if (glsl.type_to_glsl(*finalType, var.self) == structTypeString)
				{
					structVariableNameString = arrayMetaInformation[var.self].decoration.alias;
					break;
				}
			}
		}

		assert(structVariableNameString != "");
		assert(finalType != nullptr);

		vectorString vectorStructFieldName;
		vectorString vectorStructFieldType;
		vector<ResourceInternalType> vectorInternalType;
		int counter = 0;
		for (const uint32_t& elem : finalType->member_types)
		{
			structFieldNameString = glsl.getMetaInformation()[finalType->self].members.at(counter).alias;
			SPIRType &structMemberType = glsl.get<SPIRType>(elem);
			structFieldTypeString = glsl.type_to_glsl(structMemberType);
			structFieldType = stringTypeToEnum(move(structFieldTypeString));
			vectorStructFieldName.push_back(move(structFieldNameString));
			vectorStructFieldType.push_back(move(structFieldTypeString));
			vectorInternalType.push_back(structFieldType);
			counter++;
		}
	}

	ShaderStorageBuffer* shaderStorageBuffer;
	vectorString vectorStructFieldName;
	vectorString vectorStructFieldType;
	vector<ResourceInternalType> vectorInternalType;
	for (auto &resource : resources.storage_buffers)
	{
		set              = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		binding          = glsl.get_decoration(resource.id, spv::DecorationBinding);
		SPIRType& type   = glsl.get<SPIRType>(resource.base_type_id);
		structTypeString = glsl.type_to_glsl(type);

		// Get name of the declared variable in the sahder of type this struct
		SPIRType* finalType = nullptr;
		structVariableNameString = "";
		forI(arrayIds.size())
		{
			if (arrayIds[i].get_type() == TypeVariable)
			{
				auto &var = arrayIds[i].get<SPIRVariable>();
				finalType = &glsl.get<SPIRType>(var.basetype);
				if (glsl.type_to_glsl(*finalType, var.self) == resource.name)
				{
					structVariableNameString = arrayMetaInformation[var.self].decoration.alias;
					break;
				}
			}
		}

		if (structVariableNameString == "")
		{
			// In case of shader storage buffers, just set the name of the what-would-be the struct name variable
			structVariableNameString = structTypeString;
		}

		assert(structVariableNameString != "");
		assert(finalType != nullptr);

		shaderStorageBuffer = new ShaderStorageBuffer(move(string(resource.name)),
			shaderStage,
			binding,
			set,
			move(string(string(structVariableNameString))),
			move(string(string(resource.name))));
		vectorShaderStorageBuffer.push_back(shaderStorageBuffer);

		vectorStructFieldName.clear();
		vectorStructFieldType.clear();
		vectorInternalType.clear();
		int counter = 0;
		for (const uint32_t& elem : finalType->member_types)
		{
			structFieldNameString      = glsl.getMetaInformation()[finalType->self].members.at(counter).alias;
			SPIRType &structMemberType = glsl.get<SPIRType>(elem);
			structFieldTypeString      = glsl.type_to_glsl(structMemberType);
			structFieldType            = stringTypeToEnum(move(structFieldTypeString));
			vectorStructFieldName.push_back(move(structFieldNameString));
			vectorStructFieldType.push_back(move(structFieldTypeString));
			vectorInternalType.push_back(structFieldType);
			counter++;
		}

		shaderStorageBuffer->setVectorStructFieldName(vectorStructFieldName);
		shaderStorageBuffer->setVectorStructFieldType(vectorStructFieldType);
		shaderStorageBuffer->setVectorInternalType(vectorInternalType);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

ResourceInternalType ShaderReflection::stringTypeToEnum(string&& value)
{
	uint hashedValue = uint(hash<string>()(value.c_str()));

	auto it = m_mapResourceInternalType.find(hashedValue);

	if (it == m_mapResourceInternalType.end())
	{
		return ResourceInternalType::RIT_SIZE;
	}

	return it->second;
}

/////////////////////////////////////////////////////////////////////////////////////////////

ImageInternalFormat ShaderReflection::stringImageFormatQualifierToEnum(string&& value)
{
	uint hashedValue = uint(hash<string>()(value.c_str()));

	auto it = m_mapImageInternalType.find(hashedValue);

	if (it == m_mapImageInternalType.end())
	{
		return ImageInternalFormat::IIF_SIZE;
	}

	return it->second;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::isStruct(const ResourceInternalType &type)
{
	switch (type)
	{
		case ResourceInternalType::RIT_STRUCT:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::isDataType(const ResourceInternalType &type)
{
	switch (type)
	{
		case ResourceInternalType::RIT_FLOAT:
		case ResourceInternalType::RIT_FLOAT_VEC2:
		case ResourceInternalType::RIT_FLOAT_VEC3:
		case ResourceInternalType::RIT_FLOAT_VEC4:
		case ResourceInternalType::RIT_DOUBLE:
		case ResourceInternalType::RIT_DOUBLE_VEC2:
		case ResourceInternalType::RIT_DOUBLE_VEC3:
		case ResourceInternalType::RIT_DOUBLE_VEC4:
		case ResourceInternalType::RIT_INT:
		case ResourceInternalType::RIT_INT_VEC2:
		case ResourceInternalType::RIT_INT_VEC3:
		case ResourceInternalType::RIT_INT_VEC4:
		case ResourceInternalType::RIT_UNSIGNED_INT:
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC2:
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC3:
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC4:
		case ResourceInternalType::RIT_BOOL:
		case ResourceInternalType::RIT_BOOL_VEC2:
		case ResourceInternalType::RIT_BOOL_VEC3:
		case ResourceInternalType::RIT_BOOL_VEC4:
		case ResourceInternalType::RIT_FLOAT_MAT2:
		case ResourceInternalType::RIT_FLOAT_MAT3:
		case ResourceInternalType::RIT_FLOAT_MAT4:
		case ResourceInternalType::RIT_FLOAT_MAT2x2:
		case ResourceInternalType::RIT_FLOAT_MAT2x3:
		case ResourceInternalType::RIT_FLOAT_MAT2x4:
		case ResourceInternalType::RIT_FLOAT_MAT3x2:
		case ResourceInternalType::RIT_FLOAT_MAT3x3:
		case ResourceInternalType::RIT_FLOAT_MAT3x4:
		case ResourceInternalType::RIT_FLOAT_MAT4x2:
		case ResourceInternalType::RIT_FLOAT_MAT4x3:
		case ResourceInternalType::RIT_FLOAT_MAT4x4:
		case ResourceInternalType::RIT_DOUBLE_MAT2:
		case ResourceInternalType::RIT_DOUBLE_MAT3:
		case ResourceInternalType::RIT_DOUBLE_MAT4:
		case ResourceInternalType::RIT_DOUBLE_MAT2x2:
		case ResourceInternalType::RIT_DOUBLE_MAT2x3:
		case ResourceInternalType::RIT_DOUBLE_MAT2x4:
		case ResourceInternalType::RIT_DOUBLE_MAT3x2:
		case ResourceInternalType::RIT_DOUBLE_MAT3x3:
		case ResourceInternalType::RIT_DOUBLE_MAT3x4:
		case ResourceInternalType::RIT_DOUBLE_MAT4x2:
		case ResourceInternalType::RIT_DOUBLE_MAT4x3:
		case ResourceInternalType::RIT_DOUBLE_MAT4x4:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::ShaderReflection::isSampler(const ResourceInternalType &type)
{
	switch (type)
	{
		case ResourceInternalType::RIT_SAMPLER_1D:
		case ResourceInternalType::RIT_SAMPLER_2D:
		case ResourceInternalType::RIT_SAMPLER_3D:
		case ResourceInternalType::RIT_SAMPLER_CUBE:
		case ResourceInternalType::RIT_SAMPLER_1D_SHADOW:
		case ResourceInternalType::RIT_SAMPLER_2D_SHADOW:
		case ResourceInternalType::RIT_SAMPLER_CUBE_SHADOW:
		case ResourceInternalType::RIT_SAMPLER_1D_ARRAY:
		case ResourceInternalType::RIT_SAMPLER_2D_ARRAY:
		case ResourceInternalType::RIT_SAMPLER_1D_ARRAY_SHADOW:
		case ResourceInternalType::RIT_SAMPLER_2D_ARRAY_SHADOW:
		case ResourceInternalType::RIT_INT_SAMPLER_1D:
		case ResourceInternalType::RIT_INT_SAMPLER_2D:
		case ResourceInternalType::RIT_INT_SAMPLER_3D:
		case ResourceInternalType::RIT_INT_SAMPLER_CUBE:
		case ResourceInternalType::RIT_INT_SAMPLER_1D_ARRAY:
		case ResourceInternalType::RIT_INT_SAMPLER_2D_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_1D:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_2D:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_3D:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_CUBE:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case ResourceInternalType::RIT_SAMPLER_2D_RECT:
		case ResourceInternalType::RIT_SAMPLER_2D_RECT_SHADOW:
		case ResourceInternalType::RIT_INT_SAMPLER_2D_RECT:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_2D_RECT:
		case ResourceInternalType::RIT_SAMPLER_BUFFER:
		case ResourceInternalType::RIT_INT_SAMPLER_BUFFER:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_BUFFER:
		case ResourceInternalType::RIT_SAMPLER_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_INT_SAMPLER_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case ResourceInternalType::RIT_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case ResourceInternalType::RIT_SAMPLER_CUBE_ARRAY:
		case ResourceInternalType::RIT_SAMPLER_CUBE_ARRAY_SHADOW:
		case ResourceInternalType::RIT_INT_SAMPLER_CUBE_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_SAMPLER_CUBE_ARRAY:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::isImage(const ResourceInternalType &type)
{
	switch (type)
	{
		case ResourceInternalType::RIT_IMAGE_1D:
		case ResourceInternalType::RIT_INT_IMAGE_1D:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_1D:
		case ResourceInternalType::RIT_IMAGE_2D:
		case ResourceInternalType::RIT_INT_IMAGE_2D:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_2D:
		case ResourceInternalType::RIT_IMAGE_3D:
		case ResourceInternalType::RIT_INT_IMAGE_3D:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_3D:
		case ResourceInternalType::RIT_IMAGE_2D_RECT:
		case ResourceInternalType::RIT_INT_IMAGE_2D_RECT:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_2D_RECT:
		case ResourceInternalType::RIT_IMAGE_CUBE:
		case ResourceInternalType::RIT_INT_IMAGE_CUBE:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_CUBE:
		case ResourceInternalType::RIT_IMAGE_BUFFER:
		case ResourceInternalType::RIT_INT_IMAGE_BUFFER:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_BUFFER:
		case ResourceInternalType::RIT_IMAGE_1D_ARRAY:
		case ResourceInternalType::RIT_INT_IMAGE_1D_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_1D_ARRAY:
		case ResourceInternalType::RIT_IMAGE_2D_ARRAY:
		case ResourceInternalType::RIT_INT_IMAGE_2D_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_2D_ARRAY:
		case ResourceInternalType::RIT_IMAGE_CUBE_ARRAY:
		case ResourceInternalType::RIT_INT_IMAGE_CUBE_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_CUBE_ARRAY:
		case ResourceInternalType::RIT_IMAGE_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_INT_IMAGE_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
		case ResourceInternalType::RIT_IMAGE_2D_MULTISAMPLE_ARRAY:
		case ResourceInternalType::RIT_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
		case ResourceInternalType::RIT_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::isAtomicCounter(const ResourceInternalType &type)
{
	switch (type)
	{
		case ResourceInternalType::RIT_UNSIGNED_INT_ATOMIC_COUNTER:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setResourceCPUValue(ExposedStructField& exposedStructField)
{
	const UniformBase* uniformBase = exposedStructField.getStructFieldResource();
	if (uniformBase == nullptr)
	{
		cout << "ERROR: uniformBase is nullptr in ShaderReflection::setResourceCPUValue" << endl;
		return false;
	}

	return testUpdateResourceCPUValue(exposedStructField);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::appendExposedStructFieldDataToUniformBufferCell(CPUBuffer* cpuBuffer, int cellIndex, const ExposedStructField* exposedStructField)
{
	switch (exposedStructField->getInternalType())
	{
		case ResourceInternalType::RIT_FLOAT:
		{
			float* value = (float*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<float>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_VEC2:
		{
			vec2* value = (vec2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<vec2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_VEC3:
		{
			vec3* value = (vec3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<vec3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_VEC4:
		{
			vec4* value = (vec4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<vec4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE:
		{
			double* value = (double*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<double>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_VEC2:
		{
			dvec2* value = (dvec2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<dvec2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_VEC3:
		{
			dvec3* value = (dvec3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<dvec3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_VEC4:
		{
			dvec4* value = (dvec4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<dvec4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_INT:
		{
			int* value = (int*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<int>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_INT_VEC2:
		{
			ivec2* value = (ivec2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<ivec2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_INT_VEC3:
		{
			ivec3* value = (ivec3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<ivec3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_INT_VEC4:
		{
			ivec4* value = (ivec4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<ivec4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_UNSIGNED_INT:
		{
			uint* value = (uint*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<uint>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC2:
		{
			uvec2* value = (uvec2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<uvec2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC3:
		{
			uvec3* value = (uvec3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<uvec3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC4:
		{
			uvec4* value = (uvec4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<uvec4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_BOOL:
		{
			bool* value = (bool*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<bool>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_BOOL_VEC2:
		{
			bvec2* value = (bvec2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<bvec2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_BOOL_VEC3:
		{
			bvec3* value = (bvec3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<bvec3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_BOOL_VEC4:
		{
			bvec4* value = (bvec4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<bvec4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT2:
		case ResourceInternalType::RIT_FLOAT_MAT2x2:
		{
			mat2* value = (mat2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT3:
		case ResourceInternalType::RIT_FLOAT_MAT3x3:
		{
			mat3* value = (mat3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT4:
		case ResourceInternalType::RIT_FLOAT_MAT4x4:
		{
			mat4* value = (mat4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x3:
		{
			mat2x3* value = (mat2x3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2x3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x4:
		{
			mat2x4* value = (mat2x4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2x4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x2:
		{
			mat3x2* value = (mat3x2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3x2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x4:
		{
			mat3x4* value = (mat3x4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3x4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x2:
		{
			mat4x2* value = (mat4x2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4x2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x3:
		{
			mat4x3* value = (mat4x3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4x3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2:
		case ResourceInternalType::RIT_DOUBLE_MAT2x2:
		{
			mat2* value = (mat2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3:
		case ResourceInternalType::RIT_DOUBLE_MAT3x3:
		{
			mat3* value = (mat3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4:
		case ResourceInternalType::RIT_DOUBLE_MAT4x4:
		{
			mat4* value = (mat4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x3:
		{
			mat2x3* value = (mat2x3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2x3>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x4:
		{
			mat2x4* value = (mat2x4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat2x4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x2:
		{
			mat3x2* value = (mat3x2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3x2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x4:
		{
			mat3x4* value = (mat3x4*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat3x4>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x2:
		{
			mat4x2* value = (mat4x2*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4x2>(cellIndex, *value);
			return;
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x3:
		{
			mat4x3* value = (mat4x3*)(exposedStructField->getData());
			cpuBuffer->appendDataAtCell<mat4x3>(cellIndex, *value);
			return;
		}
		default:
		{
			cout << "ERROR: unexpected ResourceInternalType in appendExposedStructFieldDataToUniformBufferCell" << endl;
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::testUpdateResourceCPUValue(ExposedStructField& exposedStructField)
{
	switch (exposedStructField.getInternalType())
	{
		case ResourceInternalType::RIT_FLOAT:
		{
			float* value = (float*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_VEC2:
		{
			vec2* value = (vec2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_VEC3:
		{
			vec3* value = (vec3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_VEC4:
		{
			vec4* value = (vec4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE:
		{
			double* value = (double*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_VEC2:
		{
			dvec2* value = (dvec2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_VEC3:
		{
			dvec3* value = (dvec3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_VEC4:
		{
			dvec4* value = (dvec4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_INT:
		{
			int* value = (int*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_INT_VEC2:
		{
			ivec2* value = (ivec2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_INT_VEC3:
		{
			ivec3* value = (ivec3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_INT_VEC4:
		{
			ivec4* value = (ivec4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_UNSIGNED_INT:
		{
			uint* value = (uint*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC2:
		{
			uvec2* value = (uvec2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC3:
		{
			uvec3* value = (uvec3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC4:
		{
			uvec4* value = (uvec4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_BOOL:
		{
			bool* value = (bool*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_BOOL_VEC2:
		{
			bvec2* value = (bvec2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_BOOL_VEC3:
		{
			bvec3* value = (bvec3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_BOOL_VEC4:
		{
			bvec4* value = (bvec4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT2:
		case ResourceInternalType::RIT_FLOAT_MAT2x2:
		{
			mat2* value = (mat2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT3:
		case ResourceInternalType::RIT_FLOAT_MAT3x3:
		{
			mat3* value = (mat3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT4:
		case ResourceInternalType::RIT_FLOAT_MAT4x4:
		{
			mat4* value = (mat4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x3:
		{
			mat2x3* value = (mat2x3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x4:
		{
			mat2x4* value = (mat2x4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x2:
		{
			mat3x2* value = (mat3x2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x4:
		{
			mat3x4* value = (mat3x4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x2:
		{
			mat4x2* value = (mat4x2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x3:
		{
			mat4x3* value = (mat4x3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2:
		case ResourceInternalType::RIT_DOUBLE_MAT2x2:
		{
			mat2* value = (mat2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3:
		case ResourceInternalType::RIT_DOUBLE_MAT3x3:
		{
			mat3* value = (mat3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4:
		case ResourceInternalType::RIT_DOUBLE_MAT4x4:
		{
			mat4* value = (mat4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x3:
		{
			mat2x3* value = (mat2x3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x4:
		{
			mat2x4* value = (mat2x4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x2:
		{
			mat3x2* value = (mat3x2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x4:
		{
			mat3x4* value = (mat3x4*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x2:
		{
			mat4x2* value = (mat4x2*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x3:
		{
			mat4x3* value = (mat4x3*)(exposedStructField.getData());
			return setUniformCPUValue(exposedStructField.refStructFieldResource(), *value);
		}
		default:
		{
			cout << "ERROR: unexpected ResourceInternalType in testUpdateResourceCPUValue" << endl;
			return false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const float &value)
{
	float data = value;
	UniformFloat *uniformCasted = static_cast<UniformFloat*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if (data != uniformValue[0])
	{
		uniformCasted->setValue(&data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const vec2 &value)
{
	float data[2];
	data[0] = value[0];
	data[1] = value[1];
	UniformFloat2 *uniformCasted = static_cast<UniformFloat2*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const vec3 &value)
{
	float data[3];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	UniformFloat3 *uniformCasted = static_cast<UniformFloat3*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const vec4 &value)
{
	float data[4];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	data[3] = value[3];
	UniformFloat4 *uniformCasted = static_cast<UniformFloat4*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const double &value)
{
	double data = value;
	UniformDouble *uniformCasted = static_cast<UniformDouble*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if (data != uniformValue[0])
	{
		uniformCasted->setValue(&data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dvec2 &value)
{
	double data[2];
	data[0] = value[0];
	data[1] = value[1];
	UniformDouble2 *uniformCasted = static_cast<UniformDouble2*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dvec3 &value)
{
	double data[3];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	UniformDouble3 *uniformCasted = static_cast<UniformDouble3*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dvec4 &value)
{
	double data[4];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	data[3] = value[3];
	UniformDouble4 *uniformCasted = static_cast<UniformDouble4*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const int &value)
{
	int data = value;
	UniformInt *uniformCasted = static_cast<UniformInt*>(uniformBase);
	const int *uniformValue = uniformCasted->getValuePtr();

	if (data != uniformValue[0])
	{
		uniformCasted->setValue(&data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const ivec2 &value)
{
	int data[2];
	data[0] = value[0];
	data[1] = value[1];
	UniformInt2 *uniformCasted = static_cast<UniformInt2*>(uniformBase);
	const int *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const ivec3 &value)
{
	int data[3];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	UniformInt3 *uniformCasted = static_cast<UniformInt3*>(uniformBase);
	const int *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const ivec4 &value)
{
	int data[4];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	data[3] = value[3];
	UniformInt4 *uniformCasted = static_cast<UniformInt4*>(uniformBase);
	const int *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const uint &value)
{
	uint data = value;
	UniformUInt *uniformCasted = static_cast<UniformUInt*>(uniformBase);
	const uint *uniformValue = uniformCasted->getValuePtr();

	if (data != uniformValue[0])
	{
		uniformCasted->setValue(&data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const uvec2 &value)
{
	uint data[2];
	data[0] = value[0];
	data[1] = value[1];
	UniformUInt2 *uniformCasted = static_cast<UniformUInt2*>(uniformBase);
	const uint *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const uvec3 &value)
{
	uint data[3];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	UniformUInt3 *uniformCasted = static_cast<UniformUInt3*>(uniformBase);
	const uint *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const uvec4 &value)
{
	uint data[4];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	data[3] = value[3];
	UniformUInt4 *uniformCasted = static_cast<UniformUInt4*>(uniformBase);
	const uint *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const bool &value)
{
	bool data = value;
	UniformBool *uniformCasted = static_cast<UniformBool*>(uniformBase);
	const bool *uniformValue = uniformCasted->getValuePtr();

	if (data != uniformValue[0])
	{
		uniformCasted->setValue(&data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const bvec2 &value)
{
	bool data[2];
	data[0] = value[0];
	data[1] = value[1];
	UniformBool2 *uniformCasted = static_cast<UniformBool2*>(uniformBase);
	const bool *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const bvec3 &value)
{
	bool data[3];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	UniformBool3 *uniformCasted = static_cast<UniformBool3*>(uniformBase);
	const bool *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const bvec4 &value)
{
	bool data[4];
	data[0] = value[0];
	data[1] = value[1];
	data[2] = value[2];
	data[3] = value[3];
	UniformBool4 *uniformCasted = static_cast<UniformBool4*>(uniformBase);
	const bool *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat2 &value)
{
	float data[4];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	UniformMatF2 *uniformCasted = static_cast<UniformMatF2*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat3 &value)
{
	float data[9];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	data[6] = value[2][0];
	data[7] = value[2][1];
	data[8] = value[2][2];
	UniformMatF3 *uniformCasted = static_cast<UniformMatF3*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat4 &value)
{
	float data[16];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	data[8] = value[2][0];
	data[9] = value[2][1];
	data[10] = value[2][2];
	data[11] = value[2][3];
	data[12] = value[3][0];
	data[13] = value[3][1];
	data[14] = value[3][2];
	data[15] = value[3][3];
	UniformMatF4 *uniformCasted = static_cast<UniformMatF4*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]) ||
		(data[12] != uniformValue[12]) || (data[13] != uniformValue[13]) || (data[14] != uniformValue[14]) ||
		(data[15] != uniformValue[15]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat2x3 &value)
{
	float data[6];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	UniformMatF2x3 *uniformCasted = static_cast<UniformMatF2x3*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat2x4 &value)
{
	float data[8];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	UniformMatF2x4 *uniformCasted = static_cast<UniformMatF2x4*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat3x2 &value)
{
	float data[6];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	data[4] = value[2][0];
	data[5] = value[2][1];
	UniformMatF3x2 *uniformCasted = static_cast<UniformMatF3x2*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat3x4 &value)
{
	float data[12];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	data[8] = value[2][0];
	data[9] = value[2][1];
	data[10] = value[2][2];
	data[11] = value[2][3];
	UniformMatF3x4 *uniformCasted = static_cast<UniformMatF3x4*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat4x2 &value)
{
	float data[8];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	data[4] = value[2][0];
	data[5] = value[2][1];
	data[6] = value[3][0];
	data[7] = value[3][1];
	UniformMatF4x2 *uniformCasted = static_cast<UniformMatF4x2*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const mat4x3 &value)
{
	float data[12];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	data[6] = value[2][0];
	data[7] = value[2][1];
	data[8] = value[2][2];
	data[9] = value[3][0];
	data[10] = value[3][1];
	data[11] = value[3][2];
	UniformMatF4x3 *uniformCasted = static_cast<UniformMatF4x3*>(uniformBase);
	const float *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat2 &value)
{
	double data[4];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	UniformMatD2 *uniformCasted = static_cast<UniformMatD2*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) || (data[3] != uniformValue[3]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat3 &value)
{
	double data[9];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	data[6] = value[2][0];
	data[7] = value[2][1];
	data[8] = value[2][2];
	UniformMatD3 *uniformCasted = static_cast<UniformMatD3*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat4 &value)
{
	double data[16];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	data[8] = value[2][0];
	data[9] = value[2][1];
	data[10] = value[2][2];
	data[11] = value[2][3];
	data[12] = value[3][0];
	data[13] = value[3][1];
	data[14] = value[3][2];
	data[15] = value[3][3];
	UniformMatD4 *uniformCasted = static_cast<UniformMatD4*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]) ||
		(data[12] != uniformValue[12]) || (data[13] != uniformValue[13]) || (data[14] != uniformValue[14]) ||
		(data[15] != uniformValue[15]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat2x3 &value)
{
	double data[6];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	UniformMatD2x3 *uniformCasted = static_cast<UniformMatD2x3*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat2x4 &value)
{
	double data[8];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	UniformMatD2x4 *uniformCasted = static_cast<UniformMatD2x4*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat3x2 &value)
{
	double data[6];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	data[4] = value[2][0];
	data[5] = value[2][1];
	UniformMatD3x2 *uniformCasted = static_cast<UniformMatD3x2*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat3x4 &value)
{
	double data[12];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[0][3];
	data[4] = value[1][0];
	data[5] = value[1][1];
	data[6] = value[1][2];
	data[7] = value[1][3];
	data[8] = value[2][0];
	data[9] = value[2][1];
	data[10] = value[2][2];
	data[11] = value[2][3];
	UniformMatD3x4 *uniformCasted = static_cast<UniformMatD3x4*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat4x2 &value)
{
	double data[8];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[1][0];
	data[3] = value[1][1];
	data[4] = value[2][0];
	data[5] = value[2][1];
	data[6] = value[3][0];
	data[7] = value[3][1];
	UniformMatD4x2 *uniformCasted = static_cast<UniformMatD4x2*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderReflection::setUniformCPUValue(UniformBase *uniformBase, const dmat4x3 &value)
{
	double data[12];
	data[0] = value[0][0];
	data[1] = value[0][1];
	data[2] = value[0][2];
	data[3] = value[1][0];
	data[4] = value[1][1];
	data[5] = value[1][2];
	data[6] = value[2][0];
	data[7] = value[2][1];
	data[8] = value[2][2];
	data[9] = value[3][0];
	data[10] = value[3][1];
	data[11] = value[3][2];
	UniformMatD4x3 *uniformCasted = static_cast<UniformMatD4x3*>(uniformBase);
	const double *uniformValue = uniformCasted->getValuePtr();

	if ((data[0] != uniformValue[0]) || (data[1] != uniformValue[1]) || (data[2] != uniformValue[2]) ||
		(data[3] != uniformValue[3]) || (data[4] != uniformValue[4]) || (data[5] != uniformValue[5]) ||
		(data[6] != uniformValue[6]) || (data[7] != uniformValue[7]) || (data[8] != uniformValue[8]) ||
		(data[9] != uniformValue[9]) || (data[10] != uniformValue[10]) || (data[11] != uniformValue[11]))
	{
		uniformCasted->setValue(data);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

map<uint, ResourceInternalType> ShaderReflection::initResourceInternalTypeMap()
{
	uint hashedValue;
	uint counter = 0;
	map<uint, ResourceInternalType> mapResourceInternalType;

	for (const string& resource : arrayResourceKeywords)
	{
		hashedValue = uint(hash<string>()(resource.c_str()));
		mapResourceInternalType.insert(make_pair(hashedValue, static_cast<ResourceInternalType>(counter)));
		counter++;
	}

	return mapResourceInternalType;
}

/////////////////////////////////////////////////////////////////////////////////////////////

map<uint, ImageInternalFormat> ShaderReflection::initImageInternalType()
{
	uint hashedValue;
	uint counter = 0;
	map<uint, ImageInternalFormat> mapImageInternalType;

	for (const string& resource : arrayImageFormatQualifierKeywords)
	{
		hashedValue = uint(hash<string>()(resource.c_str()));
		mapImageInternalType.insert(make_pair(hashedValue, static_cast<ImageInternalFormat>(counter)));
		counter++;
	}

	return mapImageInternalType;
}

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBase* ShaderReflection::makeUniform(ResourceInternalType type, VkShaderStageFlagBits shaderStage, string&& name, string&& structName, string&& structType, ShaderStruct* shaderStruct)
{
	switch (type)
	{
		case ResourceInternalType::RIT_FLOAT:
		{
			return (UniformBase*)(new UniformFloat(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_VEC2:
		{
			return (UniformBase*)(new UniformFloat2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_VEC3:
		{
			return (UniformBase*)(new UniformFloat3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_VEC4:
		{
			return (UniformBase*)(new UniformFloat4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE:
		{
			return (UniformBase*)(new UniformDouble(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_VEC2:
		{
			return (UniformBase*)(new UniformDouble2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_VEC3:
		{
			return (UniformBase*)(new UniformDouble3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_VEC4:
		{
			return (UniformBase*)(new UniformDouble4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_INT:
		{
			return (UniformBase*)(new UniformInt(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_INT_VEC2:
		{
			return (UniformBase*)(new UniformInt2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_INT_VEC3:
		{
			return (UniformBase*)(new UniformInt3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_INT_VEC4:
		{
			return (UniformBase*)(new UniformInt4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_UNSIGNED_INT:
		{
			return (UniformBase*)(new UniformUInt(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC2:
		{
			return (UniformBase*)(new UniformUInt2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC3:
		{
			return (UniformBase*)(new UniformUInt3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_UNSIGNED_INT_VEC4:
		{
			return (UniformBase*)(new UniformUInt4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_BOOL:
		{
			return (UniformBase*)(new UniformBool(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_BOOL_VEC2:
		{
			return (UniformBase*)(new UniformBool2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_BOOL_VEC3:
		{
			return (UniformBase*)(new UniformBool3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_BOOL_VEC4:
		{
			return (UniformBase*)(new UniformBool4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT2:
		{
			return (UniformBase*)(new UniformMatF2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x3:
		{
			return (UniformBase*)(new UniformMatF2x3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT2x4:
		{
			return (UniformBase*)(new UniformMatF2x4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT3:
		{
			return (UniformBase*)(new UniformMatF3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x2:
		{
			return (UniformBase*)(new UniformMatF3x2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT3x4:
		{
			return (UniformBase*)(new UniformMatF3x4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT4:
		{
			return (UniformBase*)(new UniformMatF4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x2:
		{
			return (UniformBase*)(new UniformMatF4x2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_FLOAT_MAT4x3:
		{
			return (UniformBase*)(new UniformMatF4x3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2:
		{
			return (UniformBase*)(new UniformMatD2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x3:
		{
			return (UniformBase*)(new UniformMatD2x3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT2x4:
		{
			return (UniformBase*)(new UniformMatD2x4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3:
		{
			return (UniformBase*)(new UniformMatD3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x2:
		{
			return (UniformBase*)(new UniformMatD3x2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT3x4:
		{
			return (UniformBase*)(new UniformMatD3x4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4:
		{
			return (UniformBase*)(new UniformMatD4(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x2:
		{
			return (UniformBase*)(new UniformMatD4x2(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
		case ResourceInternalType::RIT_DOUBLE_MAT4x3:
		{
			return (UniformBase*)(new UniformMatD4x3(type, shaderStage, move(name), move(structName), move(structType), shaderStruct));
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkShaderStageFlagBits ShaderReflection::obtainShaderStage(spirv_cross::CompilerGLSL& compiler)
{
	const SPIREntryPoint& entryPoint = compiler.get_entry_point(string("main"));
	switch (entryPoint.model)
	{
		case ExecutionModelVertex:
		{
			return VK_SHADER_STAGE_VERTEX_BIT;
		}
		case ExecutionModelTessellationControl:
		{
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}
		case ExecutionModelTessellationEvaluation:
		{
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}
		case ExecutionModelGeometry:
		{
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		case ExecutionModelFragment:
		{
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		case ExecutionModelGLCompute:
		{
			return VK_SHADER_STAGE_COMPUTE_BIT;
		}
		default:
		{
			return VK_SHADER_STAGE_ALL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
