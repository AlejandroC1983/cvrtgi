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

#ifndef _SHADERMANAGER_H_
#define _SHADERMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"
#include "../headers.h"
#include "../../include/material/materialenum.h"

// CLASS FORWARDING
class Shader;

// NAMESPACE
using namespace materialenum;

// DEFINES
#define shaderM s_pShaderManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class ShaderManager: public ManagerTemplate<Shader>, public Singleton<ShaderManager>
{
	friend class CoreManager;

public:
	/** Constructor
	* @return nothing */
	ShaderManager();

	/** Destructor
	* @return nothing */
	virtual ~ShaderManager();

	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName     [in] name of the new instance (the m_sName member variable)
	* @param vertexShaderText [in] vertex shader source code
	* @param surfaceType      [in] material surface type to add flags for this shader
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShaderV(string&& instanceName, const char *vertexShaderText, MaterialSurfaceType surfaceType);

	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName       [in] name of the new instance (the m_sName member variable)
	* @param vertexShaderText   [in] vertex shader source code
	* @param geometryShaderText [in] geometry shader source code
	* @param surfaceType        [in] material surface type to add flags for this shader
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShaderVG(string&& instanceName, const char *vertexShaderText, const char *geometryShaderText, MaterialSurfaceType surfaceType);

	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName       [in] name of the new instance (the m_sName member variable)
	* @param vertexShaderText   [in] vertex shader source code
	* @param fragmentShaderText [in] fragment shader source code
	* @param surfaceType        [in] material surface type to add flags for this shader
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShaderVF(string&& instanceName, const char *vertexShaderText, const char *fragmentShaderText, MaterialSurfaceType surfaceType);

	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName       [in] name of the new instance (the m_sName member variable)
	* @param vertexShaderText   [in] vertex shader source code
	* @param geometryShaderText [in] geometry shader source code
	* @param fragmentShaderText [in] fragment shader source code
	* @param surfaceType        [in] material surface type to add flags for this shader
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShaderVGF(string&& instanceName, const char *vertexShaderText, const char *geometryShaderText, const char *fragmentShaderText, MaterialSurfaceType surfaceType);

	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName      [in] name of the new instance (the m_sName member variable)
	* @param computeShaderText [in] compute shader source code
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShaderC(string&& instanceName, const char *computeShaderText, MaterialSurfaceType surfaceType);

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();

	/** Retrieves the max size of the push constants buffers (hardware dependent)
	* @return nothing */
	void obtainMaxPushConstantsSize();

	/** Adds to m_globalHeaderSourceCode the source code contained in code
	* @return current value of m_nextInstanceSuffix */
	void addGlobalHeaderSourceCode(string&& code);

	/** Returns m_nextInstanceSuffix and increments the value
	* @return current value of m_nextInstanceSuffix */
	static uint getNextInstanceSuffix();

	GET(int, m_maxPushConstantsSize, MaxPushConstantsSize)

protected:
	/** Builds a new shader, a pointer to the shader is returned, nullptr is returned if any errors while building it
	* @param instanceName [in] name of the new instance (the m_sName member variable)
	* @param arrayShader  [in] array with the source code of each one of the possible shader stages considered (index 0 is vertex shader source code, 1 geometry shader, 2 fragment and 3 compute shader)
	* @param surfaceType  [in] material surface type to add flags for this shader
	* @return a pointer to the built shader, nullptr otherwise */
	Shader* buildShader(string&& instanceName, vectorCharPtr arrayShader, MaterialSurfaceType surfaceType);

	/** Convert GLSL shader to SPIR-V shader
	* @param shaderType [in]    type of shader to convert (vertex, geometry, fragment, ...)
	* @param pShader    [in]    pointer to the shader source code
	* @param spirv      [inout] vector with the resulting conversion if everything went fine
	* @return true if conversion was ok, false otherwise */
	bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *pShader, vector<unsigned int> &spirv);

	/** Converts the shader stage given value into a spirv-cross one
	* @param shaderType [in] shader stage to convert to its spirv-cross equivalent
	* @return enum with the converted value of shaderType into spirv-cross equivalent */
	EShLanguage getLanguage(const VkShaderStageFlagBits shaderType);

	/** Builds some supposed GPU resources, this information should be requested and fulfilled
	* on the fly from the actual device
	* @param resources [inout] resulting supposed GPU resources in this data struct
	* @return nothing */
	void initializeResources(TBuiltInResource &resources);

	/** Builds the shader stages taking an array of char* to the source code of each shader,
	* returning the shader stages and also in arraySPVShaderStage the SPV representation of
	* the corresponding shader
	* @param arrayShader            [in]    vector of const char* to each of the possible source code shaders (index 0 is vertex shader, 1 geometry, 2 fragment and 3 compute)
	* @param shaderHeaderSourceCode [in]    per-shader source code to be added to the each of the shader stages being built
	* @param arraySPVShaderStage    [inout] vector with the SPV representation of each one of the given shader source code in arrayShader
	* @return built shader stages */
	vector<VkPipelineShaderStageCreateInfo> buildShaderStages(const vectorCharPtr& arrayShader, const string& shaderHeaderSourceCode, vector<vector<uint>>& arraySPVShaderStage); // Set this one as protected

	// TODO: use crtp to avoid this virtual method call
	/** Assigns the corresponding slots to listen to signals affecting the resources
	* managed by this manager */
	virtual void assignSlots();

	// TODO: use crtp to avoid this virtual method call
	/** Slot for managing added, elements signal
	* @param managerName      [in] name of the manager performing the notification
	* @param elementName      [in] name of the element added
	* @param notificationType [in] enum describing the type of notification
	* @return nothing */
	virtual void slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType);

	/** For debug purposes, the source code given as parameter will be outputted throught console with line numbers 
	* @param sourceCode [in] source code string to output
	* @return nothing */
	void outputSourceWithLineNumber(const string& sourceCode);

	int         m_maxPushConstantsSize;   //!< Max push constant hardware-dependent value
	string      m_globalHeaderSourceCode; //!< This string contains the source code that will be added at the top of all shaders and can be used as global code
	static uint m_nextInstanceSuffix;     //!< Helper variable to add suffix to new Shader instances and avoid generating two shaders with the same name
};

static ShaderManager* s_pShaderManager;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SHADERMANAGER_H_
