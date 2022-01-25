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

#ifndef _SCENERASTERCOLORTEXTURETECHNIQUE_H_
#define _SCENERASTERCOLORTEXTURETECHNIQUE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class MaterialColorTexture;
class Texture;
class RenderPass;
class Framebuffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class SceneRasterColorTextureTechnique: public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(SceneRasterColorTextureTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	SceneRasterColorTextureTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~SceneRasterColorTextureTechnique();

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

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
	/** Slot for the keyboard signal when pressing the L key to switch between lighting and debug rasterization
	* @return nothing */
	void slotLKeyPressed();

	float                 m_elapsedTime;       //!< Elspased time since last frame
	bool                  m_addingTime;        //!< Logic flag value
	MaterialColorTexture* m_material;          //!< Material used in this technique
	Texture*              m_renderTargetColor; //!< Render target used for color
	Texture*              m_renderTargetDepth; //!< Render target used for depth
	RenderPass*           m_renderPass;        //!< Render pass used
	Framebuffer*          m_framebuffer;       //!< Framebuffer used
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SCENERASTERCOLORTEXTURETECHNIQUE_H_
