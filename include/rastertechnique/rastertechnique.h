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

#ifndef _RASTERTECHNIQUE_H_
#define _RASTERTECHNIQUE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/util/objectfactory.h"
#include "../../include/rastertechnique/rastertechniqueenum.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace rastertechniqueenum;

// DEFINES
#define USE_TIMESTAMP 1

/////////////////////////////////////////////////////////////////////////////////////////////

class RasterTechnique : public GenericResource
{
	friend class RasterTechniqueManager;
	friend class CoreManager;
	friend class GPUPipeline;
	DECLARE_FRIEND_REGISTERER(RasterTechnique)

protected:
	/** Parameter constructor
	* @param name     [in] technique's name
	* @param clasName [in] name of the class
	* @return nothing */
	RasterTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~RasterTechnique();

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Generate as many VkSemaphore as indicated by m_neededSemaphoreNumber
	* @return nothing */
	void generateSempahore();

	/** Called before rendering
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void prepare(float dt);

	/** Called for each technique before entering the while record and submit loop
	* @return nothing */
	void preRecordLoop();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial, or updated because new values where assigned
	* to exposed variables
	* @return nothing */
	virtual void updateMaterial();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	/** Record command buffer
	* @param currentImage      [in] current screen image framebuffer drawing to (in case it's needed)
	* @param commandBufferID   [in] Unique identifier of the command buffer returned as parameter
	* @param commandBufferType [in] Queue to submit the command buffer recorded in the call
	* @return command buffer the technique has recorded to */
	virtual VkCommandBuffer* record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType);

	/** Shut down
	* @return nothing */
	virtual void shutdown();

	GETCOPY_SET(bool, m_active, Active)
	GETCOPY_SET(bool, m_needsToRecord, NeedsToRecord)
	GETCOPY_SET(CommandRecordPolicy, m_recordPolicy, RecordPolicy)
	GET(vectorMaterialPtr, m_vectorMaterial, VectorMaterial)
	GET(vectorString, m_vectorMaterialName, VectorMaterialName)
	REF(vectorUint, m_vectorCommandBufferIndex, VectorCommandBufferIndex)
	REF(vectorBool, m_notifiedCommandBuffer, NotifiedCommandBuffer)
	GETCOPY(bool, m_executeCommand, ExecuteCommand)
	REF(vectorCommandBufferPtr, m_vectorCommand, VectorCommand)
	REF(vector<VkSemaphore>, m_vectorSemaphore, VectorSemaphore)
	GETCOPY(bool, m_isLastPipelineTechnique, IsLastPipelineTechnique)
	GETCOPY(RasterTechniqueType, m_rasterTechniqueType, RasterTechniqueType)
	GETCOPY(bool, m_computeHostSynchronize, ComputeHostSynchronize)
	GETCOPY(float, m_lastExecutionTime, LastExecutionTime)

protected:	
	/** Tests if the resource with name given by materialResourceName is used in this raster technique,
	* acting according with the notification type given by notificationType
	* @param materialResourceName [in] resource name in the notification
	* @param notificationType     [in] enum describing the type of notification
	* @return true if the raster technique was affected by the notification, and false otherwise */
	bool materialResourceNotification(string&& materialResourceName, ManagerNotificationType notificationType);

	/** Adds to m_mapIdCommandBuffer a new command buffer, returning its unique id and a pointer to the added variable
	* @param commandBufferId [in] command buffer generated unique id
	* return pointer to the command buffer variable added to m_mapIdCommandBuffer */
	VkCommandBuffer* addRecordedCommandBuffer(uint& commandBufferId);

	/** Will add to m_mapUintCommandBufferType a new pair, with the id representing a command buffer in m_mapIdCommandBuffer
	* and the type of queue that command buffer recorded to.
	* @param commandBufferId   [in] command buffer generated unique id
	* @param commandBufferType [in] type of queue used to record to
	* return true if the new pair was added successfully, false otherwise */
	bool addCommandBufferQueueType(uint commandBufferId, CommandBufferType commandBufferType);

	/** Destroy command buffers in m_mapIdCommandBuffer
	* @return nothing */
	void destroyCommandBuffers();

	/** Adds information about the execution time of a command buffer for this technique submitted to the
	* graphics / compute queue
	* @return nothing */
	void addExecutionTime();

	SET(uint, m_queryIndex0, QueryIndex0)
	SET(uint, m_queryIndex1, QueryIndex1)

	bool                     m_active;                   //!< True if the technique is active
	bool                     m_needsToRecord;            //!< True if the technique needs to record commands again
	bool                     m_executeCommand;           //!< True if the command recorded by this technique is to be executed, only valid if there are no other command arrays pending to execute in CoreManager::render(), being this an easy way to switch off / onn techniques execution without re-recording or generating a new array command
	CommandRecordPolicy      m_recordPolicy;             //!< Policy for this raster technique when recording commands
	vectorString             m_vectorUsedMaterial;       //!< Vector with the used materials in this technique (in order to benefit from the manager notifications, put all used material names here)
	vectorMaterialPtr        m_vectorMaterial;           //!< Vector with the materials being used in the raster technique, a ease instead of asking the material manager for pointers to elements with the names specified in m_vectorMaterialName. This vector has to have the same length as m_vectorMaterialName to map between them
	vectorString             m_vectorMaterialName;       //!< Vector with the material names being used in the raster technique
	vectorUint               m_vectorCommandBufferIndex; //!< Vector with indices identifying each unique command buffer recorded, pointers to all the command buffers this technique has recorded to, to be able to properly notify in the postQueueSubmit call
	vectorBool               m_notifiedCommandBuffer;    //!< Vector to know which elements in m_vectorCommandBuffer have been notified to the technique for the postQueueSubmit and postWholeSwapChainQueueSubmit calls
	mapUintCommandBuffer     m_mapIdCommandBuffer;       //!< Map containing command buffer unique ids as key, and the corresponding command buffer as mapped value
	mapUintCommandBufferType m_mapUintCommandBufferType; //!< Map containing unique ids as key (the same ones as in m_mapIdCommandBuffer), and an enum describing the type of queue the corresponding (to that id) command buffer recorded to (graphics / compute)
	float                    m_minExecutionTime;         //!< Minimum execution time of all queue submitted command buffers for this technique
	float                    m_maxExecutiontime;         //!< Maximum execution time of all queue submitted command buffers for this technique
	float                    m_lastExecutionTime;        //!< Execution time of last queue submitted command buffers for this technique
	float                    m_meanExecutionTime;        //!< Mean execution time of all queue submitted command buffers for this technique
	float                    m_accumulatedExecutionTime; //!< Accumulated value of all command buffer executions for this technique
	float                    m_numExecution;             //!< Number of times a command buffer for this technique has been submitted
	uint                     m_queryIndex0;              //!< One of the two indices of the queries used for performance measurement in the query pool
	uint                     m_queryIndex1;              //!< One of the two indices of the queries used for performance measurement in the query pool
	vectorCommandBufferPtr   m_vectorCommand;            //!< Vector with the command buffers recorded by this technique
	uint                     m_usedCommandBufferNumber;  //!< Number of command buffers this raster technique needs. For instance, the last technique in the pipeline will need to record as many command buffers as images are there in the swap chain. The value here is also the numer of elements added to m_vectorSemaphore
	uint                     m_neededSemaphoreNumber;    //!< Number of semaphore elements to generate in m_vectorSemaphore
	vector<VkSemaphore>      m_vectorSemaphore;          //!< Technique's semaphores for wait and signaling when submitting command buffers (more than one semaphore might be needed in case the technique submits more than one command buffer per swapchain image).
	bool                     m_isLastPipelineTechnique;  //!< True in case this raster technique is the last one in the pipeline, meaning it needs to record as many command buffers as the number of swapchain images
	RasterTechniqueType      m_rasterTechniqueType;      //!< Raster technique type, what queue type (compute or graphics) this technique will submit command buffers to
	bool                     m_computeHostSynchronize;   //!< In case the raster technique is of type RasterTechniqueType::RTT_COMPUTE, whether to wait for the command buffer send to the compute technique before continuing submitting more command buffers from the same / other techniques. This can be useful for techniques that iterate, sending several command buffers to the compute queue
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _RASTERTECHNIQUE_H_
