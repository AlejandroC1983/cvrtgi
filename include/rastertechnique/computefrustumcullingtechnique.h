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

#ifndef _COMPUTEFRUSTUMCULLINGTECHNIQUE_H_
#define _COMPUTEFRUSTUMCULLINGTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"
#include "../../include/scene/scene.h"

// CLASS FORWARDING
class Buffer;
class MaterialComputeFrustumCulling;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalComputeFrustumCullingCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ComputeFrustumCullingTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ComputeFrustumCullingTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ComputeFrustumCullingTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called in record method after start recording command buffer, to allow any image / memory barriers
	* needed for the resources being used
	* @param commandBuffer [in] command buffer to record to
	* @return nothing */
	virtual void recordBarriers(VkCommandBuffer* commandBuffer);

	/** Called before rendering
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void prepare(float dt);

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalComputeFrustumCullingCompletion, m_signalComputeFrustumCullingCompletion, SignalComputeFrustumCullingCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalComputeFrustumCullingCompletion m_signalComputeFrustumCullingCompletion;    //!< Signal for completion of the technique
	Buffer*                               m_instanceDataBuffer;                       //!< Buffer with the information from Scene::m_vectorInstanceData
	Buffer*                               m_frustumDebugBuffer;                       //!< Buffer for debug purposes
	Buffer*                               m_frustumElementCounterMainCameraBuffer;    //!< Buffer with the amount of elements that passed the frustum test for the main camera
	Buffer*                               m_frustumElementCounterEmitterCameraBuffer; //!< Buffer with the amount of elements that passed the frustum test for the emitter camera
	Buffer*                               m_indirectCommandBufferMainCamera;          //!< Buffer with the indirect commands to be used in conjunction with the ComputeFrustumCullingTechnique technique for the main camera
	Buffer*                               m_indirectCommandBufferEmitterCamera;       //!< Buffer with the indirect commands to be used in conjunction with the ComputeFrustumCullingTechnique technique for the emitter camera
	vector<VkDrawIndexedIndirectCommand>  m_arrayIndirectCommand;                     //!< Vector with the indirect commands to cull not visible scene elements
	uint                                  m_frustumElementMainCameraCounter;          //!< Helper variable where to store the value of frustumElementCounterMainCameraBuffer
	uint                                  m_frustumElementEmitterCameraCounter;       //!< Helper variable where to store the value of frustumElementCounterEmitterCameraBuffer
	MaterialComputeFrustumCulling*        m_materialComputeFrustumCulling;            //!< Pointer to the compute frustum culling material
	vectorNodePtr                         m_arrayNode;                                //!< Vector with pointers to the scene nodes with flag eMeshType E_MT_RENDER_MODEL
	vectorInstanceData                    m_vectorInstanceData;                       //!< Vector with the scene elements position and bounding sphere radius
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _COMPUTEFRUSTUMCULLINGTECHNIQUE_H_
