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

#ifndef _SHADOWMAPPINGVOXELTECHNIQUE_H_
#define _SHADOWMAPPINGVOXELTECHNIQUE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class RenderPass;
class Framebuffer;
class MaterialShadowMappingVoxel;
class Camera;
class Texture;
class Buffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class ShadowMappingVoxelTechnique : public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(ShadowMappingVoxelTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ShadowMappingVoxelTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~ShadowMappingVoxelTechnique();

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called before rendering
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void prepare(float dt);

	/** Record command buffer
	* @param currentImage      [in] current screen image framebuffer drawing to (in case it's needed)
	* @param commandBufferID   [in] Unique identifier of the command buffer returned as parameter
	* @param commandBufferType [in] Queue to submit the command buffer recorded in the call
	* @return command buffer the technique has recorded to */
	virtual VkCommandBuffer* record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType);

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

protected:
	/** Slot to receive notification when the prefix sum process is completed
	* @return nothing */
	void slotPrefixSumCompleted();

	/** Slot to receive notification when the camera values (look at / position) are dirty
	* @return nothing */
	void slotCameraDirty();

	string                      m_shadowMappingTextureName;  //!< Name for the shadow mapping texture
	string                      m_offscreenDepthTextureName; //!< Name for the offscreen depth texture
	string                      m_renderPassName;            //!< Name for the render pass name
	string                      m_framebufferName;           //!< Name for the framebuffer name
	string                      m_cameraName;                //!< Name for the camera used
	RenderPass*                 m_renderPass;                //!< Render pass used for directional voxel shadow mapping technique
	Framebuffer*                m_framebuffer;               //!< Framebuffer used for directional voxel shadow mapping technique
	MaterialShadowMappingVoxel* m_material;                  //!< Material for directional voxel shadow mapping
	Camera*                     m_shadowMappingCamera;       //!< Voxel shadow mapping camera, is the same as the one used in the shadow mapping technique
	Texture*                    m_shadowMappingTexture;      //!< Voxel shadow mapping texture
	int                         m_shadowMapWidth;            //!< Voxel shadow map texture width
	int                         m_shadowMapHeight;           //!< Voxel shadow map texture height
	uint                        m_numUsedVertex;             //!< Number of used vertices for building the geometry used in the voxel shadow map pass
	bool                        m_prefixSumCompleted;        //!< Flag to know if the prefix sum step has completed
	bool                        m_newPassRequested;          //!< If true, new pass was requested to process the assigned buffer
	Texture*                    m_offscreenDepthTexture;     //!< Offscreen depth texture used together with the color attachment
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SHADOWMAPPINGVOXELTECHNIQUE_H_
