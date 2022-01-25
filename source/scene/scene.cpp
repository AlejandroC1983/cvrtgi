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

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/scene/scene.h"
#include "../../include/util/containerutilities.h"
#include "../../include/util/singleton.h"
#include "../../include/model/model.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/core/coremanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
string Scene::m_scenePath = "../data/scenes/sponza/";
string Scene::m_sceneName = "sponza_curtain_midle.fbx";
vectorString Scene::m_transparentKeywords = { "vasePlant", "chain", "thorn" };
vectorString Scene::m_avoidDecimateKeywords = { };

/////////////////////////////////////////////////////////////////////////////////////////////

Scene::Scene() :
	m_deltaTime(.0f)
	, m_sceneCamera(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Scene::~Scene()
{
	// Delete scene models
	deleteVectorInstances(m_model);
	m_model.clear();

	// Delete models of type light volume
	deleteVectorInstances(m_lightVolumes);
	m_lightVolumes.clear();

	// Delete cameras
	deleteVectorInstances(m_vectorCamera);
	m_vectorCamera.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Scene::init()
{
	////////////////////////////////////////////////////////////////////////////////////

	// Sponza with 256 voxelization resolution
	// Capture for comparison with Cyril Crassin, emitter at vec3(-6.53219557f, 16.7277241f, -3.21760464f)
	//gpuPipelineM->addRasterFlag(move(string("EMITTER_RADIANCE")), 10000);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_X")), 30);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Y")), 70);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Z")), 80);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_X")), 234);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Y")), 174);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Z")), 170);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_TEST_VOXEL_TO_LIGHT_DIRECTION")), 1);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_ADD_BOUNDARIES")), 1);
	//gpuPipelineM->addRasterFlag(move(string("AVOID_VOXEL_FACE_PENALTY")), 1);
	//gpuPipelineM->addRasterFlag(move(string("SCENE_VOXELIZATION_RESOLUTION")), 256);
	//gpuPipelineM->addRasterFlag(move(string("IRRADIANCE_MULTIPLIER")), 4200); // This value is divided by 100000 in the shader
	//gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_VOXEL_TO_VOXEL_ADDED")), 27); // This value is divided by 10 in the shader
	//gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_CLUSTER_TO_VOXEL_ADDED")), 4800); // This value is divided by 10 in the shader
	//gpuPipelineM->addRasterFlag(move(string("DIRECT_IRRADIANCE_MULTIPLIER")), 1400); // This value is divided by 10 in the shader

	// Same scene config, 128 voxelization resolution
	gpuPipelineM->addRasterFlag(move(string("EMITTER_RADIANCE")), 10000);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_X")), 15);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Y")), 35);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Z")), 40);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_X")), 117);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Y")), 87);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Z")), 85);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_TEST_VOXEL_TO_LIGHT_DIRECTION")), 1);
	gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_ADD_BOUNDARIES")), 1);
	gpuPipelineM->addRasterFlag(move(string("AVOID_VOXEL_FACE_PENALTY")), 1);
	gpuPipelineM->addRasterFlag(move(string("SCENE_VOXELIZATION_RESOLUTION")), 128);
	gpuPipelineM->addRasterFlag(move(string("IRRADIANCE_MULTIPLIER")), 17300); // This value is divided by 100000 in the shader
	gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_VOXEL_TO_VOXEL_ADDED")), 30); // This value is divided by 10 in the shader
	gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_CLUSTER_TO_VOXEL_ADDED")), 148600); // This value is divided by 10 in the shader
	gpuPipelineM->addRasterFlag(move(string("DIRECT_IRRADIANCE_MULTIPLIER")), 1500); // This value is divided by 10 in the shader

	// Same scene config, 64 voxelization resolution
	//gpuPipelineM->addRasterFlag(move(string("EMITTER_RADIANCE")), 10000);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_X")), 7);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Y")), 16);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MIN_COORDINATE_Z")), 20);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_X")), 58);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Y")), 43);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_MAX_COORDINATE_Z")), 42);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_TEST_VOXEL_TO_LIGHT_DIRECTION")), 1);
	//gpuPipelineM->addRasterFlag(move(string("LIT_VOXEL_ADD_BOUNDARIES")), 1);
	//gpuPipelineM->addRasterFlag(move(string("AVOID_VOXEL_FACE_PENALTY")), 1);
	//gpuPipelineM->addRasterFlag(move(string("SCENE_VOXELIZATION_RESOLUTION")), 64);
	//gpuPipelineM->addRasterFlag(move(string("IRRADIANCE_MULTIPLIER")), 72000); // This value is divided by 100000 in the shader
	//gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_VOXEL_TO_VOXEL_ADDED")), 575); // This value is divided by 10 in the shader
	//gpuPipelineM->addRasterFlag(move(string("FORM_FACTOR_CLUSTER_TO_VOXEL_ADDED")), 9600); // This value is divided by 10 in the shader
	//gpuPipelineM->addRasterFlag(move(string("DIRECT_IRRADIANCE_MULTIPLIER")), 1500); // This value is divided by 10 in the shader

	////////////////////////////////////////////////////////////////////////////////////

	// Scene raster settings
	gpuPipelineM->addRasterFlag(move(string("CLUSTER_VISIBILITY_USE_SHADOW_MAP")), 0);

	// TODO: Initialize somewhere else and add the real defined values of the variables
	shaderM->addGlobalHeaderSourceCode(move(string("#version 450\n\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("/////////////////////////////////////////////////////////////\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("// GLOBAL DEFINES\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define USE_CLUSTER_APPROACH\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define NUM_ELEMENT_GATHERING_ARRAY 128\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define NUM_ELEMENT_PER_OCTANT 10\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define NUM_SPHERE_SAMPLE 1024\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define NUM_SPHERICAL_COEFFICIENTS 9\n")));
	if (gpuPipelineM->getRasterFlagValue(move(string("CLUSTER_VISIBILITY_USE_SHADOW_MAP"))) == 1)
	{
		shaderM->addGlobalHeaderSourceCode(move(string("#define CLUSTER_VISIBILITY_USE_SHADOW_MAP 1\n")));
	}
	if (gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_TEST_VOXEL_TO_LIGHT_DIRECTION"))) == 1)
	{
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_TEST_VOXEL_TO_LIGHT_DIRECTION 1\n")));
	}
	if (gpuPipelineM->getRasterFlagValue(move(string("AVOID_VOXEL_FACE_PENALTY"))) == 1)
	{
		shaderM->addGlobalHeaderSourceCode(move(string("#define AVOID_VOXEL_FACE_PENALTY 1\n")));
	}
	if (gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_ADD_BOUNDARIES"))) == 1)
	{
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MIN_COORDINATE_X " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MIN_COORDINATE_X")))) + "\n")));
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MIN_COORDINATE_Y " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MIN_COORDINATE_Y")))) + "\n")));
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MIN_COORDINATE_Z " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MIN_COORDINATE_Z")))) + "\n")));
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MAX_COORDINATE_X " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MAX_COORDINATE_X")))) + "\n")));
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MAX_COORDINATE_Y " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MAX_COORDINATE_Y")))) + "\n")));
		shaderM->addGlobalHeaderSourceCode(move(string("#define LIT_VOXEL_MAX_COORDINATE_Z " + to_string(gpuPipelineM->getRasterFlagValue(move(string("LIT_VOXEL_MAX_COORDINATE_Z")))) + "\n")));
	}

	// TODO: Adapt this value to the different voxelization resolutions
	shaderM->addGlobalHeaderSourceCode(move(string("#define FIND_HASHED_POSITION_NUM_ITERATION 8\n")));

	shaderM->addGlobalHeaderSourceCode(move(string("#define FORM_FACTOR_VOXEL_TO_VOXEL_ADDED " + to_string(gpuPipelineM->getRasterFlagValue(move(string("FORM_FACTOR_VOXEL_TO_VOXEL_ADDED")))) + "\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define FORM_FACTOR_CLUSTER_TO_VOXEL_ADDED " + to_string(gpuPipelineM->getRasterFlagValue(move(string("FORM_FACTOR_CLUSTER_TO_VOXEL_ADDED")))) + "\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define IRRADIANCE_MULTIPLIER " + to_string(gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_MULTIPLIER")))) + "\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define DIRECT_IRRADIANCE_MULTIPLIER " + to_string(gpuPipelineM->getRasterFlagValue(move(string("DIRECT_IRRADIANCE_MULTIPLIER")))) + "\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("#define IRRADIANCE_FIELD_GRADIENT_OFFSET 0.1\n")));
	shaderM->addGlobalHeaderSourceCode(move(string("/////////////////////////////////////////////////////////////\n\n")));

	gpuPipelineM->preSceneLoadResources();

	Model* model = new Model(m_scenePath, m_sceneName, true, true);
	model->addMeshesToScene();
	Node* mergedGeometry = model->getCompactedGeometryNode(move(string("sceneCompactedGeometry")));

	if (mergedGeometry != nullptr)
	{
		addModel(mergedGeometry);
	}

	delete model;

	// Force update of aabb to have the data ready for the instance of SceneVoxelizationTechnique
	update();

	m_sceneCamera = cameraM->getElement(move(string("maincamera")));

	cameraM->setAsMainCamera(m_sceneCamera);
	cameraM->loadCameraRecordingData();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::update()
{
	// TODO: Update the scene following a target amount of times per sencod, right now is dependent on the framerate
	forIT(m_model)
	{
		(*it)->prepare(m_deltaTime);
	}

	forIT(m_lightVolumes)
	{
		(*it)->prepare(m_deltaTime);
	}

	if (m_aabb.getDirty())
	{
		updateBB();
		m_aabb.setDirty(false);
	}

	//cameraM->updateMovementState(m_deltaTime, 0.5f);
	
	cameraM->updateCameraAnimation(m_deltaTime);
	cameraM->updateMovementState(m_deltaTime, 0.05f);

	
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::shutdown()
{
	this->~Scene();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::addModel(Node* node)
{
	bool result = addIfNoPresent(move(string(node->getName())), node, m_mapStringNode);

	assert(result);

	if (!result)
	{
		cout << "ERROR: in Scene::addModel, there's another node with the same name." << endl;
		return;
	}

	m_model.push_back(node);

	if (!node->refAffectSceneBB())
	{
		return;
	}

	if (m_aabb.getMin() == m_aabb.getMax())
	{
		m_aabb = node->getBBox(); // first bb
	}
	else
	{
		m_aabb.extend(node->getBBox());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::updateBB()
{
	m_aabb = BBox3D::computeFromNodeVector(m_model);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Node* Scene::refElementByName(string&& name)
{
	return getByKey(move(name), m_mapStringNode);
}

/////////////////////////////////////////////////////////////////////////////////////////////

const Node* Scene::getElementByName(string&& name)
{
	return getByKey(move(name), m_mapStringNode);
}

/////////////////////////////////////////////////////////////////////////////////////////////

int Scene::getElementIndexByName(string&& name)
{
	return findIndexByNameMethodPtr(m_model, move(name));
}

/////////////////////////////////////////////////////////////////////////////////////////////

int Scene::getElementIndex(Node* node)
{
	return findElementIndex(m_model, node);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::sortByMaterial(vectorNodePtr& vectorNode)
{
	sort(vectorNode.begin(), vectorNode.end(), Node::comparison);
}

/////////////////////////////////////////////////////////////////////////////////////////////

vectorNodePtr Scene::getByMeshType(eMeshType meshType)
{
	vectorNodePtr vectorResult;
	const uint maxIndex = uint(m_model.size());

	forI(maxIndex)
	{
		if (m_model[i]->getMeshType() & meshType)
		{
			vectorResult.push_back(m_model[i]);
		}
	}

	return vectorResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Scene::addCamera(Camera* cameraParameter)
{
	return addIfNoPresent(cameraParameter, m_vectorCamera);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Scene::addLightVolume(Node* light)
{
	m_lightVolumes.push_back(light);
}

/////////////////////////////////////////////////////////////////////////////////////////////
