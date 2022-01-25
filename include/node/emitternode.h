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

#ifndef _EMITTERNODE_H_
#define _EMITTERNODE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/node/node.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class EmitterNode : public Node
{
public:
	/** Parameter constructor
	* @param name [in] name of the node
	* @return nothing */
	EmitterNode(string&& name);

	/** Parameter constructor to fil all the per-vertex data of the node
	* @param name     [in] name of the node
	* @param indices  [in] indices of the vertices conforming this vector
	* @param vertices [in] vertex positions
	* @param texCoord [in] texture coordinates
	* @param normals  [in] normal data
	* @param tangents [in] tangent data
	* @return nothing */
	EmitterNode(string&&          name,
				const vectorUint& indices,
		        const vectorVec3& vertices,
		        const vectorVec2& texCoord,
		        const vectorVec3& normals,
		        const vectorVec3& tangents);

	/** Default destructor
	* @return nothing */
	virtual ~EmitterNode();

	/** Initialize al the properties of this node: position, rotation, material and other parameter
	* @param vPosition [in] node position
	* @param rotation  [in] node rotation
	* @param scale     [in] node scale
	* @param material  [in] material to use for this node for scene (main purpose) rasterization
	* @param eMeshType [in] node mesh type
	* @return true if initialization went ok and false otherwise */
	bool initialize(const vec3 &vPosition, const quat &rotation, const vec3 &scale, Material* material, eMeshType eMeshType);

	/** Method to initialize the emitter information about the step to advance according to the
	* scene voxelization volume size
	* @param stepMultiplier   [in] factor to multiply the step by
	* @param voxelizationSize [in] voxelization size
	* @parma growAngle        [in] angle used to scale the emitter planar geometry when growing
	* @return nothing */
	void setupEmitter(float stepMultiplier, uvec3 voxelizationSize, float growAngle);

	/** Method to compute the size of the current plane in world coordinates to the corresponding
	* size in texels in the voxelizaed scene
	* @return nothing */
	void computeCurrentSizeInTextureSpace();

	/** Generates the offseted vertex for vertex v1 in the triangle defined by v0, v1 and v2,
	* displaced in the m_direction direction and step of m_step with an outwards angle in radians of m_growAngle
	* @param sign [in]    sign (to know if the object is moving forward and therefore growing, or backwards and therefore shrinking)
	* @param v0   [inout] one of the three vertices of the triangle to perform the computation
	* @param v1   [inout] one of the three vertices of the triangle to perform the computation, this one is the one displaced in the m_direction
	* @param v2   [inout] one of the three vertices of the triangle to perform the computation
	* @return generated offseted vertex */
	vec3 generateOffsetedVertex(float sign, vec3& v0, vec3& v1, vec3& v2);

	/** Generates the consecutive vertices corresponding to the ones that form the plane, and have been
	* displaced in the m_direction direction and step of m_step with an outwards angle in radians of m_growAngle
	* @param sign [in]    sign (to know if the object is moving forward and therefore growing, or backwards and therefore shrinking)
	* @param v0   [inout] one of the four consecutive vertices of the resulting plane
	* @param v1   [inout] one of the four consecutive vertices of the resulting plane
	* @param v2   [inout] one of the four consecutive vertices of the resulting plane
	* @param v3   [inout] one of the four consecutive vertices of the resulting plane
	* @return size of the new generated geometry */
	float generateOffsetedGeometry(float sign, vec3& v0, vec3& v1, vec3& v2, vec3& v3);

	/** Will translate the emitter in the m_direction direction an offset of m_step,
	* while making the geometry grow with and outwards angle of m_growAngle
	* @param sign [in] offset sign
	* @return nothing */
	void advanceEmitter(float sign);

	/** Compute the consecutive vertices forming the plane decribed by the vertices given as parameter in vertices
	* @param indices      [in]    vector with the geometry indices
	* @param vertices     [in]    vector with the geometry vertices
	* @param consecutive0 [inout] one of the four consecutive vertices of the plane
	* @param consecutive1 [inout] one of the four consecutive vertices of the plane
	* @param consecutive2 [inout] one of the four consecutive vertices of the plane
	* @param consecutive3 [inout] one of the four consecutive vertices of the plane
	* @return nothing */
	static void inferParametersFromVertexData(const vectorVec3& vertices,
		                                                  vec3& consecutive0,
	                                                      vec3& consecutive1,
	                                                      vec3& consecutive2,
	                                                      vec3& consecutive3);

	/** Will fill the vertices and indices array with informaiton from consecutive vertices v0, v1, v2 and v3
	* to generate a two-sided plane
	* @param v0       [in]    one of the four consecutive plane vertices
	* @param v1       [in]    one of the four consecutive plane vertices
	* @param v2       [in]    one of the four consecutive plane vertices
	* @param v3       [in]    one of the four consecutive plane vertices
	* @param vertices [inout] resulting geometry vertices
	* @param indices  [inout] resulting geometry indices
	* @return nothing */
	static void buildTwoSidedPlane(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vectorVec3& vertices, vectorUint& indices);

	/** Returns the world coordinates of the center of the plane
	* @return world coordinates of the center of the plane */
	vec3 getWorldPlaneCenter();

	/** Update the value of m_scaleRatio
	* @return nothing */
	void computeScaleRatio();

	/** Computes the vertices in world space of the emitter in the next advance step performed
	* @param sign [inout] sign to advence the vertices (1.0 forward, -1.0f backwards)
	* @param v0   [inout] one of the four vertices of the emitter in world space if the emitter is advanced
	* @param v1   [inout] one of the four vertices of the emitter in world space if the emitter is advanced
	* @param v2   [inout] one of the four vertices of the emitter in world space if the emitter is advanced
	* @param v3   [inout] one of the four vertices of the emitter in world space if the emitter is advanced
	* @return nothing */
	void getNextStepVertices(float sign, vec3& v0, vec3& v1, vec3& v2, vec3& v3);

	/** Computes the center of the emitter plane in the next advance step performed (in world space coordinates)
	* @param sign [inout] sign to advence the vertices (1.0 forward, -1.0f backwards)
	* @return center of the emitter plane in the next advance step performed */
	vec3 getNextWorldPlaneCenter(float sign);

	GETCOPY(vec3, m_direction, Direction)
	GETCOPY(float, m_step, Step)
	SET(vec3, m_contiguous0, Contiguous0)
	SET(vec3, m_contiguous1, Contiguous1)
	SET(vec3, m_contiguous2, Contiguous2)
	SET(vec3, m_contiguous3, Contiguous3)
	SET(vec3, m_transformedContinuous0, TransformedContinuous0)
	SET(vec3, m_transformedContinuous1, TransformedContinuous1)
	SET(vec3, m_transformedContinuous2, TransformedContinuous2)
	SET(vec3, m_transformedContinuous3, TransformedContinuous3)
	GETCOPY(vec3, m_transformedContinuous0, TransformedContinuous0)
	GETCOPY(vec3, m_transformedContinuous1, TransformedContinuous1)
	GETCOPY(vec3, m_transformedContinuous2, TransformedContinuous2)
	GETCOPY(vec3, m_transformedContinuous3, TransformedContinuous3)
	GET_SET(vec3, m_initialTraslation, InitialTraslation)
	GET_SET(vectorFloat, m_bufferData, BufferData)

protected:
	vec3        m_direction;              //!< Movement direction of the emitter plane for the simulation (forward vector)
	float       m_step;                   //!< Step of offset for the forward direction
	float       m_growAngle;              //!< Angle used to scale the emitter planar geometry when growing
	uint        m_pendingUntilOutOfScene; //!< Number of steps before the plane is out if the scene voxelization volume
	mat4        m_initialTransform;       //!< Node initial transform
	uvec3       m_voxelizationSize;       //!< Voxelization size for the current scene
	uvec2       m_sizeInTextureSpace;     //!< Size of the current emitter geometry in world coordinates in texture space
	uint        m_maxSizeTextureSpace;    //!< Simply max(m_sizeInTextureSpace.x, m_sizeInTextureSpace.y)
	float       m_stepMultiplier;         //!< Ratio between max scene aabb dimension size and voxelization size
	vec3        m_contiguous0;            //!< One of the four consecutive vertices defining the plane
	vec3        m_contiguous1;            //!< One of the four consecutive vertices defining the plane
	vec3        m_contiguous2;            //!< One of the four consecutive vertices defining the plane
	vec3        m_contiguous3;            //!< One of the four consecutive vertices defining the plane
	vec3        m_initialTraslation;      //!< Initial traslation after setting emiterr's geoemtry to model space
	vec3        m_transformedContinuous0; //!< One of the four consecutive vertices defining the plane in world space (updated every call to advanceEmitter)
	vec3        m_transformedContinuous1; //!< One of the four consecutive vertices defining the plane in world space (updated every call to advanceEmitter)
	vec3        m_transformedContinuous2; //!< One of the four consecutive vertices defining the plane in world space (updated every call to advanceEmitter)
	vec3        m_transformedContinuous3; //!< One of the four consecutive vertices defining the plane in world space (updated every call to advanceEmitter)
	float       m_scaleRatio;             //!< The scale ratio at which this emitter plan grows when advancing
	vectorFloat m_bufferData;             //!< Buffer generated during model loading with the contents to be transferred to the emitter buffer
	vectorUint  m_indicesCopy;            //!< Copy of the cector of indices of this node that will be sent to GPU, needed a copy for some postprocessing
	vectorVec3  m_verticesCopy;           //!< Copy of the cector of vertices of this node that will be sent to GPU, needed a copy for some postprocessing
};
/////////////////////////////////////////////////////////////////////////////////////////////

#endif _EMITTERNODE_H_
