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

#ifndef _VOXELRASTERINSCENARIOTECHNIQUE_H_
#define _VOXELRASTERINSCENARIOTECHNIQUE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class MaterialVoxelRasterInScenario;
class Buffer;
class Texture;
class RenderPass;
class Framebuffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class VoxelRasterInScenarioTechnique : public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(VoxelRasterInScenarioTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	VoxelRasterInScenarioTechnique(string &&name, string&& className);

	/** Default destructor
	* @return nothing */
	virtual ~VoxelRasterInScenarioTechnique();

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

	/** Slot for the keyboard signal when pressing the R key to switch between voxel reflectance data and 
	* debug rasterization
	* @return nothing */
	void slotRKeyPressed();

	/** Slot for the keyboard signal when pressing the N key to switch between voxel normal data and
	* debug rasterization
	* @return nothing */
	void slotNKeyPressed();

	/** Slot for the keyboard signal when pressing the V key to switch between voxel lit data and
	* debug rasterization
	* @return nothing */
	void slotVKeyPressed();

	/** Slot for the keyboard signal when pressing the L key to switch between lighting and debug rasterization
	* @return nothing */
	void slotLKeyPressed();

	/** Slot for the keyboard signal when pressing the M key to switch between lighting and debug rasterization for
	* the voxel associated mean curvature H
	* @return nothing */
	void slotHKeyPressed();

	/** Slot for the keyboard signal when pressing the M key to switch between lighting and debug rasterization for
	* the voxel associated mean normal direction
	* @return nothing */
	void slotMKeyPressed();

	/** Slot for the keyboard signal when pressing the C key to switch between lighting and debug rasterization for
	* the cluster owner of the voxel in which the fragment is
	* @return nothing */
	void slotCKeyPressed();

	/** Slot for the keyboard signal when pressing the K key to switch between lighting and debug rasterization for
	* the lit cluster
	* @return nothing */
	void slotKKeyPressed();

	/** Slot for the keyboard signal when pressing the T key to switch between lighting and debug rasterization for
	* the lit cluster
	* @return nothing */
	void slotTKeyPressed();

	/** Slot for the keyboard signal when pressing the Y key to switch between lighting and fragment density
	* @return nothing */
	void slotYKeyPressed();

	MaterialVoxelRasterInScenario* m_material;                         //!< Voxel raster in scenario material
	uint                           m_numOccupiedVoxel;                 //!< Number of occupied voxels after voxelization process
	Texture*                       m_voxelShadowMappingTexture;        //!< Voxel shadow mapping texture
	Buffer*                        m_voxelrasterinscenariodebugbuffer; //!< Pointer to debug buffer
	Texture*                       m_renderTargetColor;                //!< Render target used for color
	Texture*                       m_renderTargetDepth;                //!< Render target used for depth
	RenderPass*                    m_renderPass;                       //!< Render pass used
	Framebuffer*                   m_framebuffer;                      //!< Framebuffer used
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _VOXELRASTERINSCENARIOTECHNIQUE_H_
