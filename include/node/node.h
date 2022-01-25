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

#ifndef _NODE_H_
#define _NODE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/geometry/bbox.h"
#include "../../include/math/transform.h"
#include "../../include/material/material.h"

// CLASS FORWARDING
class Material;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Node : public GenericResource
{
public:
	/** Parameter constructor
	* @param name      [in] name of the node
	* @param className [in] name of the class (for children classes)
	* @return nothing */
	Node(string&& name, string&& className);

	/** Parameter constructor to fil all the per-vertex data of the node
	* @param name      [in] name of the node
	* @param className [in] name of the class (for children classes)
	* @param indices   [in] indices of the vertices conforming this vector
	* @param vertices  [in] vertex positions
	* @param texCoord  [in] texture coordinates
	* @param normals   [in] normal data
	* @param tangents  [in] tangent data
	* @return nothing */
	Node(string&& name, string&& className, const vectorUint &indices, const vectorVec3 &vertices, const vectorVec2 &texCoord, const vectorVec3 &normals, const vectorVec3 &tangents);

	/** Default destructor
	* @return nothing */
	virtual ~Node();

	/** Initialize al the properties of this node: position, rotation, material and other parameter
	* @param vPosition         [in] node position
	* @param rotation          [in] node rotation
	* @param scale             [in] node scale
	* @param material          [in] material to use for this node for scene (main purpose) rasterization
	* @param materialInstanced [in] material to use for this node for instanced rasterization
	* @param eMeshType         [in] node mesh type
	* @return true if initialization went ok and false otherwise */
	virtual bool initialize(const vec3& vPosition, const quat& rotation, const vec3& scale, Material* material, Material* materialInstanced, eMeshType eMeshType);

	/** Called before rasterizing the node, all the code that needs to be updated is executed here
	* @param fDt [in] elapsed time from last call
	* @return nothing */
	virtual void  prepare(const float &fDt);

	/** Generates the geometry of this node (in the case of an analytical geometry node, this method is used to compute the node's geometry)
	* @return nothing */
	virtual void  generateGeometry();

	/** Generates node's normals
	* @return nothing */
	void generateNormals();

	/** Generates node's tangents
	* @return nothing */
	void generateTangents();

	/** Compress node's tangents
	* @return nothing */
	void compressTangents();

	/** Compress node's normals
	* @return nothing */
	void compressNormals();

	/** Sends to GPU the node's per vertex data
	* @return nothing */
	void bindData();

	/** Tests whether vP is indide the model or not
	* param vP [in] point to test if it is inside the model geometry or not
	* @return nothing */
	virtual bool testPointInside(const vec3 &vP);

	/** Computes the bounding box of the model
	* @return nothing */
	virtual void computeBB();

	/** Fills m_vertexData with all the info for this model taken from m_vertices, m_normals, m_texCoord and m_tangents
	* building an array of structures for best performance when doing draw calls
	* @return nothing */
	void buildPerVertexInfo();

	REF(vectorUint, m_indices, Indices)
	REF(vectorVec3, m_vertices, Vertices)
	REF(vectorVec3, m_normals, Normals)
	REF(vectorVec2, m_texCoord, TexCoord)
	REF(vectorVec3, m_tangents, Tangents)
	REF(vectorFloat, m_vertexData, VertexData)
	GET(BBox3D, m_aabb, BBox)
	GET_SET(eGeometryType, m_geomType, GeomType)
	GET_SET(eMeshType, m_meshType, MeshType)
	GET_SET(bool, m_followingPath, FollowingPath)
	GET_SET(uint, m_startIndex, StartIndex)
	GET_SET(uint, m_endIndex, EndIndex)
	GET_SET(uint, m_indexOffset, IndexOffset)
	GET_SET(uint, m_indexSize, IndexSize)
	REF_SET(Transform, m_transform, Transform)
	REF_SET(bool, m_affectSceneBB, AffectSceneBB)
	GET_PTR(Material, m_material, Material)
	REF_PTR(Material, m_material, Material)
	GET_PTR(Material, m_materialInstanced, MaterialInstanced)
	REF_PTR(Material, m_materialInstanced, MaterialInstanced)

	/** Getter of m_transform.getMat()
	* @return m_transform.getMat() */
    mat4 getModelMat() const;

	/** Setter of m_aabb
	* @return nothing */
	void setBBox(BBox3D* pBBox);

	static bool comparison(const Node* node1, const Node* node2)
	{
		// Scene nodes should have at least the default material
		assert(node1->m_material != nullptr);
		assert(node2->m_material != nullptr);

		return (node1->m_material->getHashedName() < node2->m_material->getHashedName());
	}

protected:
	vectorUint    m_indices;           //!< vector of indices of this node that will be sent to GPU
	vectorVec3    m_vertices;          //!< vector of vertices of this node that will be sent to GPU
	vectorVec2    m_texCoord;          //!< vector of texture coordinates of this node that will be sent to GPU
	vectorVec3    m_normals;           //!< vector of normals of this node that will be sent to GPU
	vectorVec3    m_tangents;          //!< vector of tangents of this node that will be sent to GPU
	vectorFloat	  m_vertexData;        //!< All the previous per - vertex information stored as an array of structures with all the info for each vertex
	BBox3D		  m_aabb;              //!< Boundig box of the mesh
	eGeometryType m_geomType;          //!< Geometry type of the mesh (triangles, triangle strip, triangle fan for now)
	mat4		  m_modelMat;          //!< Model matrix for the mesh (if the model is static, it only has to be computed once, but if the model has traslation / rotation / scaling changes, each time it changes)
	eMeshType	  m_meshType;          //!< To clasify each type of mesh: light volume / no light volume, torus light volume, etc
	bool		  m_followingPath;     //!< True if th model follows a path given
	uint		  m_startIndex;        //!< Start index in the scene index buffer for render this model
	uint		  m_endIndex;          //!< End index in the scene index buffer for render this model
	uint		  m_indexOffset;       //!< Offset inside the scene index buffer where all the meshes geometry is loaded, to find the indices corresponding to this mesh
	uint		  m_indexSize;         //!< Size of the index list for this mesh
	bool		  m_generateNormal;    //!< If true, then normals will be generated from the given geometry
	bool		  m_generateTangent;   //!< If true, then tangents will be generated from the given geometry
	Transform     m_transform;         //!< Node's transform
	bool		  m_affectSceneBB;     //!< If true, this node will be taken into account for the scene bb calculations whenever it is added to the scene or modified its bb (translated, rotated or scaled), true by default
	Material*     m_material;          //!< Material to use for this node for scene (main purpose) rasterization
	Material*     m_materialInstanced; //!< Material to use for this node for instanced rasterization
	float         m_instanceCounter;   //!< Current value of s_instanceCounter in Node constructor before its increased by one
	static float  s_instanceCounter;   //!< Static variable to know the number of nodes instanced and have per-element vertex information in shaders, float value is currnently used since per-vertex data is currently composed of 32-bit float values
};

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE METHODS

/////////////////////////////////////////////////////////////////////////////////////////////

inline mat4 Node::getModelMat() const
{
	return m_transform.getMat();
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _NODE_H_
