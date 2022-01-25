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

#ifndef _MODEL_H_
#define _MODEL_H_

// GLOBAL INCLUDES
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/geometry/bbox.h"
#include "../../include/material/materialenum.h"

// CLASS FORWARDING
class Node;

// NAMESPACE
using namespace materialenum;

// DEFINES
const int minNumberTriangleToDecimate   = 300;
const float maxAABBDiagonalSizeDecimate = 30.0f;

/////////////////////////////////////////////////////////////////////////////////////////////

/** Simple model class to load models form the assimp library, each mesh is added to the scene as a node
*   the scene is node focused, being the model class a simple container at this moment, so the destruction of
*   scene nodes is carried out by the scene itself and not by each model. Information about node hierarchy is
*   pending. */

class Model
{
public:
	/** Default constructor
	* @return nothing */
	Model();

	/** Parameter constructor: loads a model given by the path parameter, adding each mesh in the model as
	* a single node to the scene
	* @param path                   [in] path to the scene or model to load
	* @param name                   [in] scene or model file to load
	* @param XYZToMinusXZY          [in] when loading models in a format like 3DS max, where the axis are different, the coordinates of the verices and vectors are switched to match OpenGL standard
	* @param makeMergedGeometryNode [in] if true, information to build a node with all the merged geometry of the model will be stored. This node can be retrieved by calling getCompactedGeometryNode
	* @return nothing */
	Model(const string &path, const string &name, bool XYZToMinusXZY, bool makeMergedGeometryNode);

	/** Destructor
	* @return nothing */
	~Model();

	/** Adds all the meshes present in m_vecMesh to the scene
	* @return nothing */
	void addMeshesToScene();

	/** If m_makeMergedGeometryNode is true, a node with the compacted geometry of the model will be generated and
	* returned as parameter
	* @param nodeName [in] name to assign to the node
	* @return node generated with the compacted goemetry of the model */
	Node* getCompactedGeometryNode(string&& nodeName);

	REF(vectorNodePtr, m_vecMesh, VecMesh)
	REF(string, m_path, Path)
	REF(string, m_name, Name)
	GET(BBox3D, m_aabb, AABB)

protected:
	/** Loads the model given by the path variable, returns true if everything went ok and false otherwise
	* param path [in] path to the model to load
	* @return true if everything went ok and false otherwise */
	bool loadModel(string path);

	/** Recursive function used to process all the nodes and therefore children of the model loaded with
	* the assimp library
	* @param node		  [in] assimp node to process
	* @param scene		  [in] assimp scene which contains the model that contains the node
	* @param accTransform [in] accumulated transform for the nodes and children meshes of the model
	* @return nothing */
	void processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4 accTransform);

	/** Generates a new node added to the scene from each mesh given by each node of the assimp loaded model
	* #param mesh      [in] assimp mesh to get data from and generate a new scene node
	* #param scene     [in] assimp scene which contains the model that contains the node
	* #param transform [in] transform info for this node
	* @return new node added to the scene from each mesh given by each node of the assimp loaded model */
	Node *processMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 &transform, const string& nodeName);

	/** Loops through all the nodes starting from the root node of the scene, and returns true if all of the meshes have tangent and
	* bitangent data, and false otherwise
	* @param scene [in] assimp scene
	* @return and returns true if all of the meshes have tangent and bitangent data, and false otherwise */
	bool hasTangentAndBitangent(const aiScene* scene);

	/** Loops through all the meshes starting from the node given as parameter, returns true if all of them have tangent and
	* bitangent data, and false otherwise
	* @param scene  [in] assimp scene
	* @param node   [in] current node
	* @param result [in] result to compare with after each mesh is tested
	* @return nothing */
	void hasTangentAndBitangent(const aiScene* scene, const aiNode* node, bool &result);

	/** Loops through all the nodes starting from the root node of the scene, and returns true if all of the meshes have normal
	* data, and false otherwise
	* @param scene [in] assimp scene
	* @return returns true if all of the meshes have normal data, and false otherwise */
	bool hasNormal(const aiScene* scene);

	/** Loops through all the meshes starting from the node given as parameter, returns true if all of them have normal
	* data, and false otherwise
	* @param scene  [in] assimp scene
	* @param node   [in] current node
	* @param result [in] result to compare with after each mesh is tested
	* @return nothing */
	void hasNormal(const aiScene* scene, const aiNode* node, bool &result);

	/** Loops through all the loaded material information from const aiScene* scene and makes new materials. The reflectance texture of
	* the material is given by the AI_MATKEY_TEXTURE_DIFFUSE(0) enum in aiMaterial::Get. The normal texture is assumed to exist, and
	* have the same name, but with the "_N" suffix. For instance, for "Name.png", the normal texture is assumed to be called "Name_N.png"
	* NOTE: since only .ktx files can be loaded, a small hck to replace the file extension is done (the texture file extensions are changed
	* to "ktx"), assuming the corresponding file also exists to be loaded.
	* NOTE: all textures loaded are assumed to be RGBA8 in .ktx format right now.
	* @param scene [in] assimp scene
	* @return nothing */
	void loadMaterials(const aiScene* scene);

	/** Helper method: Takes the texture file name recovered, textureName, and returns the corresponding reflectance and normal texture
	* names for loading
	* @param textureName [in] texture name to generate the two resulting ones
	* @param reflectance [in] final reflectance texture name
	* @param normal      [in] final normal texture name */
	static void getTextureNames(const string& textureName, string& reflectance, string& normal);

	/** Helper method: Returns the material texture type (opaque / alpha tested / alpha blended) using keywords present in the material texture,
	* pending to implement a proper way to do it.
	* @param textureName [in] texture name to generate the two resulting ones
	* @return The material texture type for the texture name provided */
	static MaterialSurfaceType getMaterialTextureType(const string& textureName);

	/** Helper method: Takes the texture file name recovered, textureName, and returns the corresponding reflectance and normal texture
	* names for loading
	* @param vectorIndex    [in] index information to add to the merged geometry node
	* @param vectorVertex   [in] vertex information to add to the merged geometry node
	* @param vectorTexCoord [in] texture cordinates information to add to the merged geometry node
	* @param vectorNormal   [in] normal information to add to the merged geometry node
	* @param vectorTangent  [in] tangent information to add to the merged geometry node
	* @param aabbMin        [in] minimum value of the AABB for the vertices in vectorVertex
	* @param aabbMax        [in] maximum value of the AABB for the vertices in vectorVertex
	* @param nodeName       [in] node name
	* @return nothing */
	void addMergedGeometryNodeInformation(const vectorUint& vectorIndex,
										  const vectorVec3& vectorVertex,
										  const vectorVec2& vectorTexCoord,
										  const vectorVec3& vectorNormal,
										  const vectorVec3& vectorTangent,
										  vec3 aabbMin,
										  vec3 aabbMax,
										  const string& nodeName);

	/** Look in the aiCamera array in the scene to look for a camera with name given as parameter. If found, then 
	* retrieve information from that camera (position, up, lookAt, zNear, zFar)
	* @param scene      [in] assimp scene to look for the camera
	* @param cameraName [in] camera name
	* @param position   [in] position
	* @param lookAt     [in] look at direction
	* @param up         [in] up direction
	* @param zNear      [in] clip plane near distance
	* @param zFar       [in] clip plane far distance
	* @return true if the camera was found, false otherwise */
	static bool getCameraInformation(const aiScene* scene, string&& cameraName, vec3& position, vec3& lookAt, vec3& up, float& zNear, float& zFar);

protected:
	/** Prepare the vertex and index data for the decimation operation
	* @param vectorIndex  [in] vector with the index data
	* @param vectorVertex [in] vector with the vertex data
	* @return nothing */
	void prepareDecimationData(const vectorUint& vectorIndex, const vectorVec3& vectorVertex);

	/** Helper method: Takes the texture file name recovered, textureName, and returns the corresponding reflectance and normal texture
	* names for loading
	* @param vectorIndex    [in] index information to add to the merged geometry node
	* @param vectorVertex   [in] vertex information to add to the merged geometry node
	* @param vectorTexCoord [in] texture cordinates information to add to the merged geometry node
	* @param vectorNormal   [in] normal information to add to the merged geometry node
	* @param vectorTangent  [in] tangent information to add to the merged geometry node
	* @return nothing */
	void addMergedGeometry(const vectorUint& vectorIndex, 
						   const vectorVec3& vectorVertex,
						   const vectorVec2& vectorTexCoord,
						   const vectorVec3& vectorNormal,
						   const vectorVec3& vectorTangent);

	vectorNodePtr m_vecMesh;                //!< vector with the meshes of this model
	string		  m_path;                   //!< path to the scene or model
	string		  m_name;                   //!< file name of the scene or model
	bool		  m_XYZToMinusXZY;          //!< When loading models in a format like 3DS max, where the axis are different, the coordinates of the verices and vectors are switched to match OpenGL standard
	BBox3D        m_aabb;                   //!< Model aabb
	bool          m_makeMergedGeometryNode; //!< if true, a node with all the geometry of all nodes of the model will be built
	vectorUint    m_mergedIndex;            //!< temp vector used to store information for the merged geometry node (if m_makeMergedGeometryNode is true)
	vectorVec3    m_mergedVertex;           //!< temp vector used to store information for the merged geometry node (if m_makeMergedGeometryNode is true)
	vectorVec2    m_mergedTexCoord;         //!< temp vector used to store information for the merged geometry node (if m_makeMergedGeometryNode is true)
	vectorVec3    m_mergedNormal;           //!< temp vector used to store information for the merged geometry node (if m_makeMergedGeometryNode is true)
	vectorVec3    m_mergedTangent;          //!< temp vector used to store information for the merged geometry node (if m_makeMergedGeometryNode is true)
	vectorString  m_vectorEmitterName;      //!< Temp vector used to store emitter names (emitter information is not available in the aiLight structure, storing the name of all emitters to obtain its transform data when processing scene nodes)
	vectorString  m_vectorCameraName;       //!< Temp vector used to store camera names (camera information is not available in the aiCamera structure, storing the name of all cameras to obtain its transform data when processing scene nodes)
	static int    m_modelCounter;           //!< Simple counter to name added models to the scene
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MODEL_H_
