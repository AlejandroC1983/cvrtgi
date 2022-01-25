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

#ifndef _SCENELIGHTINGTECHNIQUE_H_
#define _SCENELIGHTINGTECHNIQUE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class Texture;
class LitClusterTechnique;
class RenderPass;
class Framebuffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class SceneLightingTechnique: public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(SceneLightingTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	SceneLightingTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~SceneLightingTechnique();

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
	/** Slot for the keyboard signal when pressing the L key to switch between lighting and debug rasterization
	* @return nothing */
	void slotLKeyPressed();

	/** For each instance of MaterialColorTexture, an instance of MaterialLighting is generated, with the suffix
	* "_Lighting", so it can be recovered later when rasterizing the scene elements. The method is called when the scene
	* has been already loaded and the material instances of MaterialColorTexture for the scene elements generated
	* @return nothing */
	void generateLightingMaterials();

	/** Slot for the keyboard signal when pressing the 1 key to remove 100 units from the MaterialLighting::m_irradianceMultiplier value
	* for each MaterialLighting in m_vectorMaterial
	* @return nothing */
	void slot1KeyPressed();

	/** Slot for the keyboard signal when pressing the 2 key to add 100 units from the MaterialLighting::m_irradianceMultiplier value
	* for each MaterialLighting in m_vectorMaterial
	* @return nothing */
	void slot2KeyPressed();

	/** Slot for the keyboard signal when pressing the 7 key to remove 100 units from the MaterialLighting::m_directIrradianceMultiplier value
	* for each MaterialLighting in m_vectorMaterial
	* @return nothing */
	void slot7KeyPressed();

	/** Slot for the keyboard signal when pressing the 8 key to add 100 units from the MaterialLighting::m_directIrradianceMultiplier value
	* for each MaterialLighting in m_vectorMaterial
	* @return nothing */
	void slot8KeyPressed();

	static string        m_lightingMaterialSuffix;          //!< Suffix added to the material names used in the voxelization step
	LitClusterTechnique* m_litClusterTechnique;             //!< Pointer to the lit cluster technique
	Texture*             m_renderTargetColor;               //!< Render target used for color
	Texture*             m_renderTargetDepth;               //!< Render target used for depth
	RenderPass*          m_renderPass;                      //!< Render pass used
	Framebuffer*         m_framebuffer;                     //!< Framebuffer used
	Buffer*              m_indirectCommandBufferMainCamera; //!< Pointer to the indirect command buffer for the main camera
	vectorNodePtr        m_arrayNode;                       //!< Vector with pointers to the scene nodes with flag eMeshType E_MT_RENDER_MODEL
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SCENELIGHTINGTECHNIQUE_H_
