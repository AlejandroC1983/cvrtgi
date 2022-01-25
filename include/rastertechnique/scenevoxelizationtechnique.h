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

#ifndef _SCENEVOXELIZATIONTECHNIQUE_H_
#define _SCENEVOXELIZATIONTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class MaterialSceneVoxelization;
class RenderPass;
class Framebuffer;
class Texture;
class Buffer;

// NAMESPACE

// DEFINES
typedef Nano::Signal<void()> SignalVoxelizationComplete;
const uint maxValue = 4294967295;

/////////////////////////////////////////////////////////////////////////////////////////////

// Per fragment struct
struct PerFragmentData
{
	vec4 position;        // Fragment position in xyz fields, scene element indes in the w field
	vec4 normal;          // Fragment normal in xyz fields, emitted fragment counter in the w field
	uvec4 compressedData; // Fragment reflectance in x field, fragment accumulated irradiance in the y field
};

// Per voxel struct
// NOTE: using vec3 instead of vec4 seems to give some problems, maybe because of the memory layout
//       (a part of the buffer would not be available at the last set of indices)
struct PerVoxelAxisIrradiance
{
	vec4 axisIrradiance[6]; //!< The irradiance of each axis is set in the array following the order -x, +x, -y, +y, -z, +z
};

/** Enum to control the different steps of the voxelization technique */
enum class VoxelizationStep
{
	VS_INIT = 0,
	VS_FIRST_CB_RECORDED,
	VS_FIRST_CB_SUBMITTED,
	VS_FIRST_CB_ACTION,
	VS_SECOND_CB_RECORDED,
	VS_SECOND_CB_SUBMITTED,
	VS_SECOND_CB_ACTION,
	VS_FINISH
};

/////////////////////////////////////////////////////////////////////////////////////////////

class SceneVoxelizationTechnique: public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(SceneVoxelizationTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	SceneVoxelizationTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~SceneVoxelizationTechnique();

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called before rendering
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void prepare(float dt);

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

	/** Called after submitting commands in the command queue, here decisions * to re-record each technique are taken
	* @param commandBufferID [in] command buffer unique id, to let the technique know which command buffer was successfully submitted
	* @return nothing */
	//virtual void postQueueSubmit(uint commandBufferID);

	GETCOPY(int, m_voxelizedSceneWidth, VoxelizedSceneWidth)
	GETCOPY(int, m_voxelizedSceneHeight, VoxelizedSceneHeight)
	GETCOPY(int, m_voxelizedSceneDepth, VoxelizedSceneDepth)
	GET(mat4, m_projection, Projection)
	GET(mat4, m_viewX, ViewX)
	GET(mat4, m_viewY, ViewY)
	GET(mat4, m_viewZ, ViewZ)
	GETCOPY(uint, m_fragmentCounter, FragmentCounter)
	REF(SignalVoxelizationComplete, m_voxelizationComplete, VoxelizationComplete)
	GETCOPY(uint, m_fragmentOccupiedCounter, FragmentOccupiedCounter)
		
protected:

	/** For each instance of MaterialColorTexture, an instance of MaterialSceneVoxelization is generated, with the suffix
	* "_Voxelization", so it can be recovered later when rasterizing the scene elements. The method is called when the scene
	* has been already loaded and the material instances of MaterialColorTexture for the scene elements generated
	* @return nothing */
	void generateVoxelizationMaterials();

	/** Updates for every EmitterNode in the scene the information about forward step distance (dependent on scene aabb and
	* voxelization volume size defined by m_voxelizedSceneWidth, m_voxelizedSceneHeight and m_voxelizedSceneDepth
	* @return nothing */
	void updateEmitterVoxelizationInfo();

	/** Builds the m_emitterBuffer emitter buffer
	* @return nothing */
	void buildEmitterBuffer();

	int                        m_voxelizedSceneWidth;           //!< One of the dimensions of the 3D texture for the scene voxelization
	int                        m_voxelizedSceneHeight;          //!< One of the dimensions of the 3D texture for the scene voxelization
	int                        m_voxelizedSceneDepth;           //!< One of the dimensions of the 3D texture for the scene voxelization
	vec3                       m_pixelDiagonal;                 //!< Pixel diagonal size
	RenderPass*                m_renderPass;                    //!< Render pass used for voxelization
	Framebuffer*               m_framebuffer;                   //!< Framebuffer used for voxelization
	Texture*                   m_voxelizationTexture;           //!< 3D texture with the voxelized scene
	mat4                       m_projection;                    //!< Orthographic projection matrix used for voxelization
	mat4                       m_viewX;                         //!< x axis view matrix used for voxelization
	mat4                       m_viewY;                         //!< y axis view matrix used for voxelization
	mat4                       m_viewZ;                         //!< z axis view matrix used for voxelization
	bool                       m_bufferResizingFlag;            //!< Flag to know if the buffer resizing to store fragment information has been done
	bool                       m_firstPassFragmentCountDone;    //!< Flag to know if the first pass, were the number of generated fragments are atomically counted, has been done
	bool                       m_secondPassFragmentStoreDone;   //!< Flag to know if the second pass, were the fragment information is sotred has been done
	uint                       m_fragmentCounter;               //!< Emitted fragments counter
	uint                       m_fragmentOccupiedCounter;       //!< Number of non-empty cells in the 3D voxelization volume
	Buffer*                    m_voxelOccupiedBuffer;           //!< Shader storage buffer to know whether a voxel position is occupied or is not
	Buffer*                    m_voxelFirstIndexBuffer;         //!< Shader storage buffer to know the index of the first fragment generated for the voxelization 3D volume of coordinates hashed
	Buffer*                    m_fragmentCounterBuffer;         //!< Shader storage buffer atomic counter (emitted fragments counter in index 0, and number of non-empty cells in the 3D voxelization volume in index 1)
	Buffer*                    m_fragmentOccupiedCounterBuffer; //!< Shader storage buffer to count the number of occupied 3D volume voxelization positions
	Buffer*                    m_fragmentDataBuffer;            //!< Shader storage buffer with the per-fragment data generated during the voxelization
	Buffer*                    m_fragmentIrradianceBuffer;      //!< Shader storage buffer with the per-fragment irradiance for all scene emitters
	Buffer*                    m_nextFragmentIndexBuffer;       //!< Shader storage buffer with the next fragment index in the case several fragments fall in the same 3D voxelization volume
	Buffer*                    m_emitterBuffer;                 //!< Buffer with the geometry of all emitters in the scene (which are supposed to be planar). For now, only one emitter is used.
	float                      m_storeInformation;              //!< Flag to control when to store information in the ssbo m_fragmentDataBuffer and m_fragmentCoordinateAndIndexBuffer
	SignalVoxelizationComplete m_voxelizationComplete;          //!< Signal to notify when the voxelization step is complete
	uvec3                      m_voxelizationSize;              //!< Size of the voxelization volume in each dimension
	static string              m_voxelizationMaterialSuffix;    //!< Suffix added to the mateiral names used in the voxelization step
	float                      m_stepMultiplier;                //!< Ratio between max scene aabb dimension size and voxelization size
	VoxelizationStep           m_currentStep;                   //!< Enum to control the steps taken in this voxelization technique
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SCENEVOXELIZATIONTECHNIQUE_H_
