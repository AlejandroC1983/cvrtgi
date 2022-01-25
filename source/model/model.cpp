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
#include <cctype>
#include <experimental/filesystem>

// PROJECT INCLUDES
#include "../../include/model/model.h"
#include "../../include/node/node.h"
#include "../../include/node/emitternode.h"
#include "../../include/scene/scene.h"
#include "../../include/util/mathutil.h"
#include "../../include/util/loopMacroDefines.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/texture/texture.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/material/materialcolortexture.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
int Model::m_modelCounter = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

Model::Model():
	  m_path("")
	, m_name("")
	, m_XYZToMinusXZY(false)
	, m_makeMergedGeometryNode(false)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Model::Model(const string &path, const string &name, bool XYZToMinusXZY, bool makeMergedGeometryNode):
	  m_path(path)
	, m_name(name)
	, m_XYZToMinusXZY(XYZToMinusXZY)
	, m_makeMergedGeometryNode(makeMergedGeometryNode)
{
	bool result = loadModel(path + name);

	if (!result)
	{
		cout << "ERROR loading model with path " << path << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Model::~Model()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Model::loadModel(string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
		return false;
	}

	if (!hasNormal(scene))
	{
		scene = import.ApplyPostProcessing(aiProcess_GenNormals);
	}

	if (!hasTangentAndBitangent(scene))
	{
		scene = import.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	}

	scene = import.ApplyPostProcessing(aiProcess_ImproveCacheLocality);

	loadMaterials(scene);

	if (scene->HasLights())
	{
		uint numLight = scene->mNumLights;

		forI(numLight)
		{
 			m_vectorEmitterName.push_back(string(scene->mLights[i]->mName.C_Str()));
		}
	}

	if (scene->HasCameras())
	{
		uint numCamera = scene->mNumCameras;
		forI(numCamera)
		{
			m_vectorCameraName.push_back(string(scene->mCameras[i]->mName.C_Str()));
		}
	}

	aiMatrix4x4 transform;
	processNode(scene->mRootNode, scene, transform);
	m_aabb = BBox3D::computeFromNodeVector(m_vecMesh);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4 accTransform)
{
	cout << "Node name is: " << node->mName.C_Str() << endl;

	aiMatrix4x4 transform = node->mTransformation * accTransform;

	vectorString::iterator it = find(m_vectorEmitterName.begin(), m_vectorEmitterName.end(), string(node->mName.C_Str()));

	if (it != m_vectorEmitterName.end())
	{
		mat4 nodeMatrix = MathUtil::assimpAIMatrix4x4ToGLM(&transform);

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(nodeMatrix, scale, rotation, translation, skew, perspective);

		const mat4 inverted      = inverse(nodeMatrix);
		const vec3 lookAt        = normalize(glm::vec3(inverted[2]));
		vec4 lookAtTransformed4D = nodeMatrix * vec4(lookAt, 0.0f);
		vec3 lookAtTransformed   = vec3(lookAtTransformed4D.x, lookAtTransformed4D.y, lookAtTransformed4D.z);

		Camera* sceneCamera = cameraM->buildCamera(move(string(node->mName.C_Str())),
			CameraType::CT_FIRST_PERSON,
			//translation, // overwritten by value below
			vec3(-6.53219557f, 16.7277241f, -3.21760464f), // Render for comparison with Cyril Crassin, final emitter position used
			lookAtTransformed,
			vec3(0.0f, 1.0f, 0.0f),
			ZNEAR,
			ZFAR,
			glm::pi<float>() * 0.25f);

		//sceneCamera->setIsAnimated(true);

		sceneM->addCamera(sceneCamera);
	}

	it = find(m_vectorCameraName.begin(), m_vectorCameraName.end(), string(node->mName.C_Str()));

	if (it != m_vectorCameraName.end())
	{
		float zFar;
		float zNear;

		vec3 up;
		vec3 position;
		vec3 lookAt;
		
		bool result = getCameraInformation(scene, move(string(node->mName.C_Str())), position, lookAt, up, zNear, zFar);

		if(result)
		{
			mat4 nodeMatrix          = MathUtil::assimpAIMatrix4x4ToGLM(&transform);
			mat4 inverted            = inverse(nodeMatrix);
			vec3 lookAt              = normalize(glm::vec3(inverted[2]));
			vec4 lookAtTransformed4D = nodeMatrix * vec4(lookAt, 0.0f);
			vec3 lookAtTransformed   = vec3(lookAtTransformed4D.x, lookAtTransformed4D.y, lookAtTransformed4D.z);
			vec4 upTransformed4D     = nodeMatrix * vec4(up, 0.0f);
			vec3 upTransformed       = vec3(upTransformed4D.x, upTransformed4D.y, upTransformed4D.z);

			Camera* sceneCamera = cameraM->buildCamera(move(string(node->mName.C_Str())),
													   CameraType::CT_FIRST_PERSON,
													   // position, // overwritten by line below
													   vec3(-9.16604, 5.5527, -2.12906), // Render for comparison with max FPS with VCT
													   lookAtTransformed,
													   upTransformed,
													   zNear,
													   zFar,
													   glm::pi<float>() * 0.25f); // Assimp import from .fbx won't load correct fov value https://github.com/assimp/assimp/issues/245
			
			sceneM->addCamera(sceneCamera);
		}
	}

	// Process all the node's meshes (if any)
	aiMesh* mesh = nullptr;
	forI(node->mNumMeshes)
	{
		mesh = scene->mMeshes[node->mMeshes[i]];
		if (mesh->HasTextureCoords(0)) // Avoid those meshes without texture coordinates
		{
			Node* temp = processMesh(mesh, scene, transform, string(node->mName.C_Str()));
			if (temp != nullptr)
			{
				m_vecMesh.push_back(temp);
			}
		}
		else
		{
			cout << "ERROR: mesh " << mesh->mName.C_Str() << " has no texture coordinates" << endl;
		}
	}

	// Then do the same for each of its children
	forI(node->mNumChildren)
	{
		processNode(node->mChildren[i], scene, transform);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Model::hasTangentAndBitangent(const aiScene* scene)
{
	bool result = true;
	hasTangentAndBitangent(scene, scene->mRootNode, result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::hasTangentAndBitangent(const aiScene* scene, const aiNode* node, bool &result)
{
	// Loop through each mesh of this node and test if the node has tangent and bitangent data
	aiMesh* mesh = nullptr;
	forI(node->mNumMeshes)
	{
		mesh = scene->mMeshes[node->mMeshes[i]];
		if (!mesh->HasTangentsAndBitangents())
		{
			result = false;
			return;
		}
	}

	// If all mesh have tangent and bitangent data, try the children
	for (uint i = 0; i < node->mNumChildren && result; ++i)
	{
		hasTangentAndBitangent(scene, node->mChildren[i], result);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Model::hasNormal(const aiScene* scene)
{
	bool result = true;
	hasNormal(scene, scene->mRootNode, result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::hasNormal(const aiScene* scene, const aiNode* node, bool &result)
{
	// Loop through each mesh of this node and test if the node has normal data
	aiMesh* mesh = nullptr;
	forI(node->mNumMeshes)
	{
		mesh = scene->mMeshes[node->mMeshes[i]];
		if (!mesh->HasNormals())
		{
			result = false;
			return;
		}
	}

	// If all mesh have tangent and bitangent data, try the children
	for (uint i = 0; i < node->mNumChildren && result; ++i)
	{
		hasNormal(scene, node->mChildren[i], result);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::loadMaterials(const aiScene* scene)
{
	aiMaterial* material;
	aiString materialName;
	aiString reflectanceTextureName;

	int numReflectance;
	string materialString;
	size_t found;
	string reflectanceTextureFinalName;
	string normalTextureFinalName;
	Texture* reflectance;
	Texture* normal;

	forI(scene->mNumMaterials)
	{
		material = scene->mMaterials[i];
		material->Get(AI_MATKEY_NAME, materialName);

		materialString = materialName.C_Str();
		std::transform(materialString.begin(), materialString.end(), materialString.begin(), ::tolower);
		found = materialString.find(string("emitter"));

		if (found != string::npos)
		{
			materialM->buildMaterial(move(string("MaterialPlainColor")), move(string(materialName.C_Str())), nullptr);
		}
		else
		{
			numReflectance                            = material->GetTextureCount(aiTextureType_DIFFUSE);
			bool overwriteReflectanceTextureFinalName = true;

			if (numReflectance == 0)
			{
				cout << "WARNING: no reflectance texture information for material " << materialName.C_Str() << " at Model::loadMaterials, trying to find reflectance and normal .ktx textures from material names" << endl;

				// Try to find reflectance and normal textures based on the material's name
				reflectanceTextureFinalName   = materialString;
				string reflectanceTexturePath = m_path + reflectanceTextureFinalName + ".ktx";
				string normalTexturePath      = m_path + reflectanceTextureFinalName + "_N.ktx";

				if (!std::experimental::filesystem::exists(reflectanceTexturePath))
				{
					cout << "ERROR: unable to find reflectance texture file based on material name for material " << materialName.C_Str() << " at Model::loadMaterials" << endl;
					continue;
				}

				if (!std::experimental::filesystem::exists(normalTexturePath))
				{
					cout << "ERROR: unable to find normal texture file based on material name for material " << materialName.C_Str() << " at Model::loadMaterials" << endl;
					continue;
				}

				overwriteReflectanceTextureFinalName = false;
			}

			if (overwriteReflectanceTextureFinalName)
			{
				material->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), reflectanceTextureName);
				reflectanceTextureFinalName = reflectanceTextureName.C_Str();
			}

			if (reflectanceTextureFinalName == "")
			{
				cout << "ERROR: no reflectance texture name for material " << materialName.C_Str() << " at Model::loadMaterials" << endl;
			}

			assert(reflectanceTextureFinalName != "");

			getTextureNames(reflectanceTextureFinalName, reflectanceTextureFinalName, normalTextureFinalName);

			MaterialSurfaceType surfaceType = getMaterialTextureType(reflectanceTextureFinalName);

			reflectance = textureM->build2DTextureFromFile(
				move(string(reflectanceTextureFinalName)),
				move(m_path + reflectanceTextureFinalName),
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
				VK_FORMAT_R8G8B8A8_UNORM);

			normal = textureM->build2DTextureFromFile(
				move(string(normalTextureFinalName)),
				move(m_path + normalTextureFinalName),
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
				VK_FORMAT_R8G8B8A8_UNORM);

			MultiTypeUnorderedMap *attributeMaterial = new MultiTypeUnorderedMap();
			attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_reflectanceTextureResourceName), string(reflectance->getName())));
			attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_normalTextureResourceName), string(normal->getName())));

			if (surfaceType != MaterialSurfaceType::MST_OPAQUE)
			{
				attributeMaterial->newElement<AttributeData<MaterialSurfaceType>*>(new AttributeData<MaterialSurfaceType>(string(g_materialSurfaceType), move(surfaceType)));
			}

			materialM->buildMaterial(move(string("MaterialColorTexture")), move(string(materialName.C_Str())), attributeMaterial);
			materialM->buildMaterial(move(string("MaterialIndirectColorTexture")), move(string(materialName.C_Str()) + "Instanced"), attributeMaterial);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::getTextureNames(const string& textureName, string& reflectance, string& normal)
{
	if (textureName.find("_BaseColor") != string::npos)
	{
		int indexLastUnderScore = int(textureName.find_last_of('_'));
		string newTextureName = textureName.substr(0, indexLastUnderScore);
		reflectance = "/Textures/" + newTextureName + ".ktx";
		normal = "/Textures/" + newTextureName + "_N.ktx";
	}
	else
	{
		string temp = textureName.substr(0, textureName.find_last_of('.'));
		reflectance = temp + ".ktx";
		normal = temp + "_N.ktx";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

MaterialSurfaceType Model::getMaterialTextureType(const string& textureName)
{
	const vectorString& transparentKeywords = sceneM->getTransparentKeywords();

	forI(transparentKeywords.size())
	{
		auto it = std::search(textureName.begin(), textureName.end(), transparentKeywords[i].begin(), transparentKeywords[i].end(), [](char char1, char char2) { return std::toupper(char1) == std::toupper(char2); });
		if (it != textureName.end())
		{
			cout << "TEXTURE " << textureName << " will be alpha tested " << endl;
			return MaterialSurfaceType::MST_ALPHATESTED;
		}
	}

	return MaterialSurfaceType::MST_OPAQUE;
}

#include "Simplify.h"

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::addMergedGeometryNodeInformation(const vectorUint& vectorIndex,
											 const vectorVec3& vectorVertex,
											 const vectorVec2& vectorTexCoord,
											 const vectorVec3& vectorNormal,
											 const vectorVec3& vectorTangent,
											 vec3 aabbMin,
											 vec3 aabbMax,
											 const string& nodeName)
{
	bool addedToMergedGeometry = false;
	const vectorString& refVectorAvoidDecimateKeywords = sceneM->getAvoidDecimateKeywords();
	bool avoidDecimate = false;
	forI(refVectorAvoidDecimateKeywords.size())
	{
		if (nodeName.find(refVectorAvoidDecimateKeywords[i]) != string::npos)
		{
			avoidDecimate = true;
			break;
		}
	}

	float sizeOfAABB = distance(aabbMin, aabbMax);

	if (!avoidDecimate && (vectorVertex.size() > minNumberTriangleToDecimate) && (sizeOfAABB < maxAABBDiagonalSizeDecimate))
	{
		int simplifyTargetCount = static_cast<int>(vectorVertex.size()) / 9;

		// The bigger the object, the less aggressive the decimation
		if((sizeOfAABB >= 15.0f) && (sizeOfAABB < 20.0f))
		{
			simplifyTargetCount = static_cast<int>(vectorVertex.size()) / 9;
		}
		else if ((sizeOfAABB >= 20.0f) && (sizeOfAABB < 25.0f))
		{
			simplifyTargetCount = static_cast<int>(vectorVertex.size()) / 9;
		}
		else if ((sizeOfAABB >= 25.0f) && (sizeOfAABB < 30.0f))
		{
			simplifyTargetCount = static_cast<int>(vectorVertex.size()) / 9;
		}

		prepareDecimationData(vectorIndex, vectorVertex);
		Simplify::simplify_mesh(simplifyTargetCount);

		if ((Simplify::triangles.size() < (vectorIndex.size() / 3)) && (Simplify::triangles.size() > 3))
		{
			uint finalVertexSize   = static_cast<uint>(Simplify::vertices.size());
			uint finalTriangleSize = static_cast<int>(Simplify::triangles.size()) * 3;

			vectorVec3 vectorVertexDecimated(finalVertexSize);
			forI(finalVertexSize)
			{
				vectorVertexDecimated[i] = vec3(Simplify::vertices[i].p.x, Simplify::vertices[i].p.y, Simplify::vertices[i].p.z);
			}

			vectorUint vectorIndexDecimated(finalTriangleSize);
			forI(finalTriangleSize / 3)
			{
				vectorIndexDecimated[3 * i + 0] = Simplify::triangles[i].v[0];
				vectorIndexDecimated[3 * i + 1] = Simplify::triangles[i].v[1];
				vectorIndexDecimated[3 * i + 2] = Simplify::triangles[i].v[2];
			}

			vectorVec2 vectorTexCoordDecimated = vectorTexCoord;
			vectorVec3 vectorNormalDecimated   = vectorNormal;
			vectorVec3 vectorTangentDecimated  = vectorTangent;

			vectorTexCoordDecimated.resize(finalVertexSize);
			vectorNormalDecimated.resize(finalVertexSize);
			vectorTangentDecimated.resize(finalVertexSize);

			addMergedGeometry(vectorIndexDecimated, vectorVertexDecimated, vectorTexCoordDecimated, vectorNormalDecimated, vectorTangentDecimated);

			addedToMergedGeometry = true;
		}
	}

	if (!addedToMergedGeometry)
	{
		addMergedGeometry(vectorIndex, vectorVertex, vectorTexCoord, vectorNormal, vectorTangent);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Model::getCameraInformation(const aiScene* scene, string&& cameraName, vec3& position, vec3& lookAt, vec3& up, float& zNear, float& zFar)
{
	aiCamera* camera;
	string cameraNameTemp;

	uint numCamera = scene->mNumCameras;

	forI(numCamera)
	{
		camera         = scene->mCameras[i];
		cameraNameTemp = string(camera->mName.C_Str());

		if (cameraNameTemp == cameraName)
		{
			position = vec3(camera->mPosition.x, camera->mPosition.y, camera->mPosition.z);
			lookAt   = vec3(camera->mLookAt.x, camera->mLookAt.y, camera->mLookAt.z);
			up       = vec3(camera->mUp.x, camera->mUp.y, camera->mUp.z);
			zNear    = camera->mClipPlaneNear;
			zFar     = camera->mClipPlaneFar;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Node *Model::processMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 &transform, const string& nodeName)
{
	vectorUint indices;
	vectorVec3 vertices;
	vectorVec2 texCoord;
	vectorVec3 normals;
	vectorVec3 tangents;

	vec3 vector3;
	vec2 vector2;

	forI(mesh->mNumVertices)
	{
		vector3 = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertices.push_back(vector3);

		if (mesh->HasTextureCoords(0)) // only first texture coordinate channel covered now
		{
			vector2 = vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			texCoord.push_back(vector2);
		}

		if (mesh->HasNormals())
		{
			vector3 = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			normals.push_back(vector3);
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vector3 = vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			tangents.push_back(vector3);
		}
	}

	// Process indices
	aiFace face;
	forI(mesh->mNumFaces)
	{
		face = mesh->mFaces[i];
		forJ(face.mNumIndices)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// If the model comes from a file with coordinate system as 3DS max ("z" up, "x" front and "y" right), then coordinates must be
	// switched (as well as traslation, scale and rotations)
	if (m_XYZToMinusXZY)
	{
		//MathUtil::transformXYZToMinusXZY(vertices);
		//MathUtil::transformXYZToMinusXZY(normals);
		//MathUtil::transformXYZToMinusXZY(tangents);
	}

	// Transform vertices
	mat4 mat = MathUtil::assimpAIMatrix4x4ToGLM(&transform);

	vec3 aabbMin = vec3( FLT_MAX,  FLT_MAX,  FLT_MAX);
	vec3 aabbMax = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	forI(vertices.size())
	{
		vec3 v3D    = vertices[i];
		vec4 v4D    = mat * vec4(v3D, 1.0f);
		vertices[i] = vec3(v4D.x, v4D.y, v4D.z);

		aabbMax.x = glm::max(aabbMax.x, vertices[i].x);
		aabbMax.y = glm::max(aabbMax.y, vertices[i].y);
		aabbMax.z = glm::max(aabbMax.z, vertices[i].z);
		
		aabbMin.x = glm::min(aabbMin.x, vertices[i].x);
		aabbMin.y = glm::min(aabbMin.y, vertices[i].y);
		aabbMin.z = glm::min(aabbMin.z, vertices[i].z);
	}

	string name = to_string(m_modelCounter);
	m_modelCounter++;

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	aiString materialName;
	material->Get(AI_MATKEY_NAME, materialName);
	Material* materialInstance = materialM->getElement(move(string(materialName.C_Str())));
	Material* materialInstanceInstanced = materialM->getElement(move(string(materialName.C_Str()) + "Instanced"));

	if (materialInstance == nullptr)
	{
		cout << "ERROR: material " << materialName.C_Str() << " not found for model " << name << ", using DefaultMaterial" << endl;
		materialInstance = materialM->getElement(move(string("DefaultMaterial")));
	}

	if (materialInstanceInstanced == nullptr)
	{
		cout << "ERROR: material " << materialName.C_Str() << "Instanced not found for model " << name << ", using DefaultMaterialInstanced" << endl;
		materialInstanceInstanced = materialM->getElement(move(string("DefaultMaterialInstanced")));
	}

	Node *sceneNode;
	bool isEmitter = materialInstance->getIsEmitter();
	if (isEmitter)
	{
		vec3 v0;
		vec3 v1;
		vec3 v2;
		vec3 v3;
		EmitterNode::inferParametersFromVertexData(vertices, v0, v1, v2, v3);
		vectorVec3 vectorTemp;
		vectorTemp.resize(4);
		vectorTemp[0]          = v0;
		vectorTemp[1]          = v1;
		vectorTemp[2]          = v2;
		vectorTemp[3]          = v3;
		vectorFloat vectorData = MathUtil::buildGeometryBuffer(vectorTemp, normals, indices);
		vectorTemp             = vertices;
		MathUtil::convertToModelSpace(vectorTemp);
		EmitterNode::buildTwoSidedPlane(v0, v1, v2, v3, vertices, indices);
		vec3 traslation        = MathUtil::convertToModelSpace(vertices);
		sceneNode              = new EmitterNode(move(name), indices, vertices, texCoord, normals, tangents);
		EmitterNode* casted    = static_cast<EmitterNode*>(sceneNode);
		casted->setBufferData(vectorData);
		casted->setInitialTraslation(traslation);
		casted->setContiguous0(v0);
		casted->setContiguous1(v1);
		casted->setContiguous2(v2);
		casted->setContiguous3(v3);
		casted->setTransformedContinuous0(v0);
		casted->setTransformedContinuous1(v1);
		casted->setTransformedContinuous2(v2);
		casted->setTransformedContinuous3(v3);
	}
	else
	{
		sceneNode = new Node(move(name), move(string("Node")), indices, vertices, texCoord, normals, tangents);
	}

	if (m_makeMergedGeometryNode)
	{
		addMergedGeometryNodeInformation(indices, vertices, texCoord, normals, tangents, aabbMin, aabbMax, nodeName);
	}

	// NOTE: Remove the call to computeBB() if the results are the same as in aabbMin and aabbMax
	sceneNode->initialize(vec3(0.0f), quat(vec3(.0f)), vec3(1.0f), materialInstance, materialInstanceInstanced, isEmitter ? E_MT_EMITTER_MODEL : E_MT_RENDER_MODEL);

	return sceneNode;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::addMeshesToScene()
{
	forIT(m_vecMesh)
	{
		sceneM->addModel(*it);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Node* Model::getCompactedGeometryNode(string&& nodeName)
{
	Node* mergedGeometryNode = nullptr;
	if (m_makeMergedGeometryNode)
	{
		for(int i = 0; i < m_mergedIndex.size(); ++i)
		{
			vec3 temp = m_mergedVertex[m_mergedIndex[i]];
		}
		mergedGeometryNode = new Node(move(nodeName), move(string("Node")), m_mergedIndex, m_mergedVertex, m_mergedTexCoord, m_mergedNormal, m_mergedTangent);
		mergedGeometryNode->initialize(vec3(0.0f), quat(vec3(0.0f)), vec3(1.0f), materialM->getElement(move(string("DefaultMaterial"))), materialM->getElement(move(string("DefaultMaterialInstanced"))), E_MT_RENDER_SHADOW);
	}

	return mergedGeometryNode;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::prepareDecimationData(const vectorUint& vectorIndex, const vectorVec3& vectorVertex)
{
	Simplify::triangles.clear();
	Simplify::vertices.clear();
	Simplify::refs.clear();
	Simplify::mtllib = "";
	Simplify::materials.clear();

	uint numIndex  = static_cast<uint>(vectorIndex.size());
	uint numVertex = static_cast<uint>(vectorVertex.size());

	Simplify::triangles.resize(numIndex / 3);
	Simplify::vertices.resize(numVertex);

	Simplify::Vertex v;
	Simplify::Triangle t;

	forI(vectorVertex.size())
	{
		v.p.x                 = vectorVertex[i].x;
		v.p.y                 = vectorVertex[i].y;
		v.p.z                 = vectorVertex[i].z;
		Simplify::vertices[i] = v;
	}

	forI(numIndex / 3)
	{
		t.v[0]                 = vectorIndex[3 * i + 0];
		t.v[1]                 = vectorIndex[3 * i + 1];
		t.v[2]                 = vectorIndex[3 * i + 2];
		t.material             = -1;
		Simplify::triangles[i] = t;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Model::addMergedGeometry(const vectorUint& vectorIndex,
						      const vectorVec3& vectorVertex,
						      const vectorVec2& vectorTexCoord,
						      const vectorVec3& vectorNormal,
						      const vectorVec3& vectorTangent)
{
	uint offset     = uint(m_mergedVertex.size());
	m_mergedVertex.insert(  m_mergedVertex.end(),   vectorVertex.begin(),   vectorVertex.end());
	m_mergedNormal.insert(  m_mergedNormal.end(),   vectorNormal.begin(),   vectorNormal.end());
	m_mergedTexCoord.insert(m_mergedTexCoord.end(), vectorTexCoord.begin(), vectorTexCoord.end());
	m_mergedTangent.insert( m_mergedTangent.end(),  vectorTangent.begin(),  vectorTangent.end());
	uint startIndex = uint(m_mergedIndex.size());
	m_mergedIndex.insert(m_mergedIndex.end(), vectorIndex.begin(), vectorIndex.end());
	uint endIndex   = uint(m_mergedIndex.size());

	if (startIndex > 0)
	{
		forIFrom(startIndex, endIndex)
		{
			m_mergedIndex[i] += offset;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////