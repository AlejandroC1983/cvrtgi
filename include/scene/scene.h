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

#ifndef _SCENE_H_
#define _SCENE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/node/node.h"
#include "../../include/util/singleton.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING
class RasterTechnique;
class Camera;

// NAMESPACE
using namespace commonnamespace;

// DEFINES
#define sceneM s_pSceneSingleton->instance()

/** Struct used to store, for each scene element in vectorNodePtr, the position and bounding sphere radius */
struct InstanceData
{
	vec3 m_position; // Position of the scene elemenr
	float m_radius;  // Radius of the bounding sphere for the scene element
};

typedef vector<InstanceData> vectorInstanceData;

/////////////////////////////////////////////////////////////////////////////////////////////

class Scene: public Singleton<Scene>
{
public:
	/** Default constructor
	* @return nothing */
	Scene();

	/** Default destructor
	* @return nothing */
	~Scene();

	/** Initialization of the scene (here, the scene elements and materials needed are loaded)
	* @return nothing */
    bool init();

	/** Update all scene elements and camera
	* @return nothing */
    void update();

	/** Free all resources holded and managed by this class
	* @return nothing */
    void shutdown();

	/** Performs all the required computations when adding a model to the scene
	param node [in] node to add to the scene */
	void addModel(Node *node);

	/** Performs all the required computations when adding a light volume to the scene
	param light [in] light to add to the scene */
	void addLightVolume(Node *light);

	/** When a node is moved in the scene, the bb of the scene is updated
	* @return nothing */
	void updateBB();

	/** Returns a pointer to a scene element if any has the name
	* @param name [in] name of the node to search in m_mapStringNode
	* @return pointer to the found node or nulltpr otherwise */
	Node* refElementByName(string&& name);

	/** Returns a const pointer to a scene element if any has the name 
	* @param name [in] name of the node to search in m_mapStringNode 
	* @return const pointer to the found node or nulltpr otherwise */
	const Node* getElementByName(string&& name);

	/** Returns the index of the scene element with name given by the name parameter in the m_vModel vector
	* @param name [in] name of the node to search in m_mapStringNode
	* @return index of the found element or -1 otherwise */
	int getElementIndexByName(string&& name);

	/** Returns the index of the node given by the node parameter in the m_vModel vector
	* @param node [in] node to search in m_mapStringNode
	* @return index of the found node or -1 otherwise */
	int getElementIndex(Node* node);

	/** Takes the vector of pointers to Node instances vectorNode and sorts it by material hashed name value
	* @param vectorNode [inout] vector to sort
	* @return nothing */
	static void sortByMaterial(vectorNodePtr& vectorNode);

	/** Returns a vector with the scene elements with the flags given by meshType parameter
	* @param meshType [in] type of mesh, flags used to retieve elements
	* @return vector with the scene elements with the flags given by meshType parameter */
	vectorNodePtr getByMeshType(eMeshType meshType);

	/** Add cameraParameter to m_vectorCamera if not already added
	param cameraParameter [in] light to add to the scene if not already added
	* @ return true if added successfully, false otherwise */
	bool addCamera(Camera* cameraParameter);

	GET_SET(float, m_deltaTime, DeltaTime)
	REF_SET(BBox3D, m_aabb, Box)
	GET(vectorNodePtr, m_model, Model)
	REF(vectorNodePtr, m_model, Model)
	GET(vectorNodePtr, m_lightVolumes, LightVolumes)
	GET(string, m_sceneName, SceneName)
	GET(vectorString, m_transparentKeywords, TransparentKeywords)
	GET(vectorString, m_avoidDecimateKeywords, AvoidDecimateKeywords)
	GETCOPY_SET(float, m_executionTime, ExecutionTime)

protected:
	float		        m_deltaTime;             //!< Delta time, the time between the last frame rendered and this frame
	BBox3D		        m_aabb;                  //!< Scene bb, updated whenever an element is added in addModel
	vectorNodePtr       m_model;                 //!< Models in scene 
	vectorNodePtr       m_lightVolumes;          //!< Scene light volumes
	mapStringNode       m_mapStringNode;         //!< Map with the key mapped value pair <string, node pointer> to locate quicker nodes by name
	Camera*             m_sceneCamera;           //!< Scene camera
	static string       m_scenePath;             //!< Path to the scene to load
	static string       m_sceneName;             //!< Name of the scene file to load
	static vectorString m_transparentKeywords;   //!< Vector with strings to be used to identify transparent materials in the material name (pending a more ellaborated way to identify those materials from .obj scenes, which need a discard operation depending on alpha test value
	static vectorString m_avoidDecimateKeywords; //!< Vector with strings to be used to identify scene elements hat should not be decimated  or should have a quite high face count target (90% of the original or more)
	vectorCameraPtr     m_vectorCamera;          //!< Vector with all the scene cameras (emitters also have their cameras here)
	float               m_executionTime;         //!< Time the application has been running
};

static Scene *s_pSceneSingleton;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SCENE_H_
