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
#ifndef _SHADERSTORAGEBUFFER_H_
#define _SHADERSTORAGEBUFFER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/shader/shaderstruct.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class ShaderStorageBuffer : public ShaderStruct
{
public:
	/** Default constructor
	* @return nothing */
	ShaderStorageBuffer();

	/** Parameter constructor
	* @param name              [in] shader storage buffer variable name (hasn't to do with declared variables in the shader, is this C++'s class instance name)
	* @param shaderStage       [in] enumerated with value the shader stage the shader storage buffer being built is used
	* @param bindingIndex      [in] binding index of this shader storage buffer buffer for the shader it's owned by
	* @param setIndex          [in] set index inside the binding index given by bindingIndex of this shader storage buffer buffer for the shader it's owned by
	* @param variableName      [in] name of the variable in the shader
	* @param structName        [in] name of the variable in the struct
	* @return nothing */
	ShaderStorageBuffer(string               &&name,
		                VkShaderStageFlagBits  shaderStage,
		                int                    bindingIndex,
		                int                    setIndex,
		                string               &&variableName,
		                string               &&structName);

	/** Default destructor
	* @return nothing */
	~ShaderStorageBuffer();
	
	GET_PTR(Buffer, m_buffer, Buffer)
	REF_PTR(Buffer, m_buffer, Buffer)
	SET_PTR(Buffer, m_buffer, Buffer)
	GET_SET(string, m_bufferName, BufferName)
	GET_SET(vectorString, m_vectorStructFieldName, VectorStructFieldName)
	GET_SET(vectorString, m_vectorStructFieldType, VectorStructFieldType)
	GET_SET(vector<ResourceInternalType>, m_vectorInternalType, VectorInternalType)
	GETCOPY_SET(VkDescriptorType, m_descriptorType, DescriptorType)
	
protected:
	Buffer*                      m_buffer;                //!< Buffer used in this shader storage buffer
	string                       m_bufferName;            //!< Name of the buffer resource assigned to m_buffer
	vectorString                 m_vectorStructFieldName; //!< String array with the name of each storage buffer field
	vectorString                 m_vectorStructFieldType; //!< String array with the type of each storage buffer field
	vector<ResourceInternalType> m_vectorInternalType;    //!< Contains the type of each storage buffer field (if in the possible values of ResourceInternalType)
	VkDescriptorType             m_descriptorType;        //!< Flags for the descriptor set built for this shader storage buffer
	VkDescriptorBufferInfo	     m_bufferInfo;            //!< Sahder storage buffer information struct
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SHADERSTORAGEBUFFER_H_
