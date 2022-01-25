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

#ifndef _DISTANCESHADOWMAPPINGTECHNIQUE_H_
#define _DISTANCESHADOWMAPPINGTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class RenderPass;
class Framebuffer;
class MaterialDistanceShadowMapping;
class Camera;
class Texture;

// NAMESPACE

// DEFINES
typedef Nano::Signal<void()> SignalDistanceShadowMapCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class DistanceShadowMappingTechnique : public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(DistanceShadowMappingTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	DistanceShadowMappingTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~DistanceShadowMappingTechnique();

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

	/** Slot to receive notification when the camera values (look at / position) are dirty
	* @return nothing */
	void slotCameraDirty();

	GET_PTR(Camera, m_camera, Camera)
	GETCOPY(float, m_emitterRadiance, EmitterRadiance)

protected:
	RenderPass*                    m_renderPass;                    //!< Render pass used for directional voxel shadow mapping technique
	Framebuffer*                   m_framebuffer;                   //!< Framebuffer used for directional voxel shadow mapping technique
	MaterialDistanceShadowMapping* m_material;                      //!< Material for the distance shadow mapping technique
	Camera*                        m_camera;                        //!< Camera used by the technique
	Texture*                       m_distanceShadowMappingTexture;  //!< Distance shadow mapping texture name
	int                            m_shadowMapWidth;                //!< Voxel shadow map texture width
	int                            m_shadowMapHeight;               //!< Voxel shadow map texture height
	Texture*                       m_offscreenDistanceDepthTexture; //!< Offscreen distance depth texture used together with the color attachment
	float                          m_emitterRadiance;               //!< Radiance of the emitter this shadow map represents
	bool                           m_useCompactedGeometry;          //!< flag to use GPU frustum culling geometry or the lower resolution compaced scene node for the distance shadow mapping
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _DISTANCESHADOWMAPPINGTECHNIQUE_H_
