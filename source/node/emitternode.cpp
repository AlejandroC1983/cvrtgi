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
#include "../../include/node/emitternode.h"
#include "../../include/util/loopMacroDefines.h"
#include "../../include/scene/scene.h"
#include "../../include/util/mathutil.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

EmitterNode::EmitterNode(string&& name) : Node(move(name), move(string("EmitterNode")))
	, m_direction(vec3(0.0f))
	, m_step(0.0f)
	, m_pendingUntilOutOfScene(0)
	, m_initialTransform(mat4(1.0f))
	, m_voxelizationSize(uvec3(0))
	, m_sizeInTextureSpace(uvec2(0))
	, m_maxSizeTextureSpace(0)
	, m_stepMultiplier(0.0f)
	, m_contiguous0(vec3(0.0f))
	, m_contiguous1(vec3(0.0f))
	, m_contiguous2(vec3(0.0f))
	, m_contiguous3(vec3(0.0f))
	, m_transformedContinuous0(vec3(0.0f))
	, m_transformedContinuous1(vec3(0.0f))
	, m_transformedContinuous2(vec3(0.0f))
	, m_transformedContinuous3(vec3(0.0f))
	, m_initialTraslation(vec3(0.0f))
	, m_scaleRatio(0.0f)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

EmitterNode::EmitterNode(string&&          name,
				         const vectorUint& indices,
		                 const vectorVec3& vertices,
		                 const vectorVec2& texCoord,
		                 const vectorVec3& normals,
		                 const vectorVec3& tangents):
	Node(move(name), move(string("EmitterNode")), indices, vertices, texCoord, normals, tangents)
	, m_direction(vec3(0.0f))
	, m_step(0.0f)
	, m_pendingUntilOutOfScene(0)
	, m_initialTransform(mat4(1.0f))
	, m_voxelizationSize(uvec3(0))
	, m_sizeInTextureSpace(uvec2(0))
	, m_maxSizeTextureSpace(0)
	, m_stepMultiplier(0.0f)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

EmitterNode::~EmitterNode()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool EmitterNode::initialize(const vec3 &vPosition, const quat &rotation, const vec3 &scale, Material* material, eMeshType eMeshType)
{
	Node::initialize(vPosition, rotation, scale, material, nullptr, eMeshType);

	// Emitters are assumed to be planes (for now). Take the first triangle and obtain the normal direction
	if (m_indices.size() < 3)
	{
		cout << "ERROR: not enough geometry indices for emitter " << m_name << endl;
		return false;
	}

	vec3 v0            = m_vertices[m_indices[0]];
	vec3 v1            = m_vertices[m_indices[1]];
	vec3 v2            = m_vertices[m_indices[2]];
	vec3 v0v1          = normalize(v1 - v0);
	vec3 v0v2          = normalize(v2 - v0);
	m_direction        = -1.0f * normalize(cross(v0v1, v0v2));
	m_affectSceneBB    = false;
	m_initialTransform = m_transform.getMat();
	m_indicesCopy      = m_indices;
	m_verticesCopy     = m_vertices;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::setupEmitter(float stepMultiplier, uvec3 voxelizationSize, float growAngle)
{
	m_stepMultiplier   = stepMultiplier;
	m_voxelizationSize = voxelizationSize;
	m_growAngle        = growAngle;

	// Obtain the step by just intersectiong a m_direction segment from origin with an unit cube
	vec3 start = vec3(0.0f);

	const uint maxIndex = 6;
	vectorVec3 vectorPlaneOrigin;
	vectorPlaneOrigin.resize(maxIndex);
	vectorPlaneOrigin[0] = vec3( 1.0f,  0.0f,  0.0f);
	vectorPlaneOrigin[1] = vec3( 0.0f,  1.0f,  0.0f);
	vectorPlaneOrigin[2] = vec3( 0.0f,  0.0f,  1.0f);
	vectorPlaneOrigin[3] = vec3(-1.0f,  0.0f,  0.0f);
	vectorPlaneOrigin[4] = vec3( 0.0f, -1.0f,  0.0f);
	vectorPlaneOrigin[5] = vec3( 0.0f,  0.0f, -1.0f);

	m_step = FLT_MAX;
	float intersectionDistance;
	forI(maxIndex)
	{
		if (glm::intersectRayPlane(start, m_direction, vectorPlaneOrigin[i], -1.0f * vectorPlaneOrigin[i], intersectionDistance) && !isinf(intersectionDistance))
		{
			m_step = glm::min(m_step, abs(intersectionDistance));
		}
	}

	if (m_step <= 0.0f)
	{
		cout << "ERROR: no intersection computed in EmitterNode::setupEmitter for node with name " << m_name << endl;
	}

	assert(m_step > 0.0f);

	// Now redimension the step according to voxelization resolution
	m_step *= m_stepMultiplier;

	// Compute the value of m_pendingUntilOutOfScene
	BBox3D& box = sceneM->refBox();
	vec3 min = box.getMin();
	vec3 max = box.getMax();

	vectorVec3 vectorPlaneAABB;
	vectorPlaneAABB.resize(maxIndex);
	vectorPlaneAABB[0] = min;
	vectorPlaneAABB[1] = min;
	vectorPlaneAABB[2] = min;
	vectorPlaneAABB[3] = max;
	vectorPlaneAABB[4] = max;
	vectorPlaneAABB[5] = max;
	
	vec3 v3D;
	vec4 v4D;
	float finalDistance;
	vec3 vertexMin;
	uint initialIndex;

	float maxTotalDistance = -1.0f * FLT_MAX;
	const uint maxVertex   = uint(m_verticesCopy.size());
	mat4 modelMatrix       = m_transform.getMat();
	bool anyVertexInside   = false;
	
	forI(maxVertex)
	{
		v3D = m_verticesCopy[i];
		v4D = modelMatrix * vec4(v3D, 1.0f);
		v3D = vec3(v4D.x, v4D.y, v4D.z); // transformed 3D position

		if (!box.insideBB(v3D))
		{
			continue;
		}

		// Test only with min or max (depending on what point is facing m_direction)
		vertexMin            = normalize(min - v3D);
		initialIndex         = (dot(m_direction, vertexMin) > 0.0f) ? 0 : 3;
		anyVertexInside      = true;
		finalDistance        = FLT_MAX;
		intersectionDistance = FLT_MAX;

		forJ(3)
		{
			if (glm::intersectRayPlane(v3D, m_direction, vectorPlaneAABB[j + initialIndex], vectorPlaneOrigin[j + initialIndex], intersectionDistance) && !isinf(intersectionDistance))
			{
				finalDistance = glm::min(finalDistance, abs(intersectionDistance));
			}
		}

		maxTotalDistance = glm::max(maxTotalDistance, finalDistance);
	}

	if (!anyVertexInside)
	{
		cout << "ERROR: no vertex of the emitter " << m_name << " is inside the scene bounding box in EmitterNode::setupEmitter" << endl;
	}

	assert(anyVertexInside == true);

	if (maxTotalDistance == FLT_MAX)
	{
		cout << "ERROR: no proper diatance of the emitter " << m_name << " computed in EmitterNode::setupEmitter" << endl;
	}

	assert(maxTotalDistance != (-1.0f * FLT_MAX));

	m_pendingUntilOutOfScene = uint(ceil(maxTotalDistance / m_step)) + 1;

	computeCurrentSizeInTextureSpace();
	//inferEmitterVertices();

	computeScaleRatio();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::computeCurrentSizeInTextureSpace()
{
	// First, compute width and height of the emitter plane (the emitter is assumed to be formed by two triangles,
	// meta-information would be needed otherwise, or more computations to infer neighboring vertices)

	vec3 v0               = m_verticesCopy[m_indicesCopy[0]];
	vec3 v1               = m_verticesCopy[m_indicesCopy[1]];
	vec3 v2               = m_verticesCopy[m_indicesCopy[2]];
	mat4 modelMatrix      = m_transform.getMat();
	vec4 v04D             = modelMatrix * vec4(v0, 1.0f);
	vec4 v14D             = modelMatrix * vec4(v1, 1.0f);
	vec4 v24D             = modelMatrix * vec4(v2, 1.0f);
	v0                    = vec3(v04D);
	v1                    = vec3(v14D);
	v2                    = vec3(v24D);
	float d0              = glm::distance(v1, v0);
	float d1              = glm::distance(v1, v2);
	m_sizeInTextureSpace  = uvec2(uint(d0 / m_stepMultiplier) + 1, uint(d1 / m_stepMultiplier) + 1);
	m_maxSizeTextureSpace = glm::max(m_sizeInTextureSpace.x, m_sizeInTextureSpace.y);
}

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 EmitterNode::generateOffsetedVertex(float sign, vec3& v0, vec3& v1, vec3& v2)
{
	vec3 direction0      = glm::normalize(v1 - v2);
	vec3 direction1      = glm::normalize(v1 - v0);
	vec3 addedDirections = glm::normalize(direction0 + direction1);
	vec3 orthogonal      = normalize(cross(m_direction, addedDirections ));
	vec3 candidate0      = MathUtil::rotateVector(m_growAngle, orthogonal, m_direction);
	vec3 candidate1      = MathUtil::rotateVector(-1.0f * m_growAngle, orthogonal, m_direction);
	float testOffset     = glm::distance(v1, v0) / 10.0f;
	vec3 testPoint0      = v1 + candidate0 * testOffset;
	vec3 testPoint1      = v1 + candidate1 * testOffset;
	float distanceAddUp0 = glm::distance(testPoint0, v0) + glm::distance(testPoint0, v1) + glm::distance(testPoint0, v2);
	float distanceAddUp1 = glm::distance(testPoint1, v0) + glm::distance(testPoint1, v1) + glm::distance(testPoint1, v2);

	vec3* finaDirection = (distanceAddUp0 > distanceAddUp1) ? &candidate0 : &candidate1;
	return v1 + (*finaDirection) * m_step * sign;
}

/////////////////////////////////////////////////////////////////////////////////////////////

float EmitterNode::generateOffsetedGeometry(float sign, vec3& v0, vec3& v1, vec3& v2, vec3& v3)
{
	vec3 newV0 = generateOffsetedVertex(sign, v3, v0, v1);
	vec3 newV1 = generateOffsetedVertex(sign, v0, v1, v2);
	vec3 newV2 = generateOffsetedVertex(sign, v1, v2, v3);
	vec3 newV3 = generateOffsetedVertex(sign, v0, v3, v2);

	float area = glm::length(newV1 - newV0) * glm::length(newV3 - newV0);
	v0         = newV0;
	v1         = newV1;
	v2         = newV2;
	v3         = newV3;

	return area;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::advanceEmitter(float sign)
{
	m_transform.addTraslation(sign * m_direction * m_step);
	vec3 traslation = m_transform.getTraslation();
	m_transform.setTraslation(vec3(0.0f));
	m_transform.addScaling(sign * vec3(m_scaleRatio));
	m_transform.setTraslation(traslation);

	getNextStepVertices(sign, 
						m_transformedContinuous0,
						m_transformedContinuous1,
						m_transformedContinuous2,
						m_transformedContinuous3);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::inferParametersFromVertexData(const vectorVec3& vertices,
	                                                        vec3& consecutive0,
	                                                        vec3& consecutive1,
	                                                        vec3& consecutive2,
	                                                        vec3& consecutive3)
{
	// The expected geometry is a two triangle plane
	assert(vertices.size() == 4);

	vec3 v0 = vertices[0];
	vec3 v1 = vertices[1];
	vec3 v2 = vertices[2];
	vec3 v3 = vertices[3];

	vec3 v0v1 = glm::normalize(v1 - v0);
	vec3 v0v2 = glm::normalize(v2 - v0);

	if (dot(v0v1, v0v2) > glm::epsilon<float>())
	{
		vec3 v0v3 = glm::normalize(v3 - v0);

		if (dot(v0v1, v0v3) > glm::epsilon<float>())
		{
			consecutive0 = v3;
			consecutive1 = v0;
			consecutive2 = v2;
			consecutive3 = v1;
		}
		else
		{
			consecutive0 = v1;
			consecutive1 = v0;
			consecutive2 = v3;
			consecutive3 = v2;
		}
	}
	else
	{
		consecutive0 = v1;
		consecutive1 = v0;
		consecutive2 = v2;
		consecutive3 = v3;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::buildTwoSidedPlane(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vectorVec3& vertices, vectorUint& indices)
{
	vertices.clear();
	indices.clear();

	vertices.resize(4);
	vertices[0] = v0;
	vertices[1] = v1;
	vertices[2] = v2;
	vertices[3] = v3;

	indices.clear();
	indices.resize(12);
	indices[0]  = 0;
	indices[1]  = 1;
	indices[2]  = 2;
	indices[3]  = 2;
	indices[4]  = 1;
	indices[5]  = 0;
	indices[6]  = 0;
	indices[7]  = 3;
	indices[8]  = 2;
	indices[9]  = 2;
	indices[10] = 3;
	indices[11] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 EmitterNode::getWorldPlaneCenter()
{
	return (m_transformedContinuous0 + 
		    m_transformedContinuous1 + 
		    m_transformedContinuous2 + 
		    m_transformedContinuous3) / 4.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::computeScaleRatio()
{
	vec3 newV0    = m_contiguous0;
	vec3 newV1    = m_contiguous1;
	vec3 newV2    = m_contiguous2;
	vec3 newV3    = m_contiguous3;
	float newSize = generateOffsetedGeometry(1.0f, newV0, newV1, newV2, newV3);
	float oldSize = glm::length(m_contiguous1 - m_contiguous0) * glm::length(m_contiguous3 - m_contiguous0);
	m_scaleRatio  = newSize / oldSize;
	m_scaleRatio -= 1.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EmitterNode::getNextStepVertices(float sign, vec3& v0, vec3& v1, vec3& v2, vec3& v3)
{
	Transform transform = m_transform;
	transform.addTraslation(sign * m_direction * m_step);
	vec3 traslation = transform.getTraslation();
	transform.setTraslation(vec3(0.0f));
	transform.addScaling(sign * vec3(m_scaleRatio));
	transform.setTraslation(traslation);

	vec4 contiguous0Local = vec4(m_contiguous0 - m_initialTraslation, 1.0f);
	vec4 contiguous1Local = vec4(m_contiguous1 - m_initialTraslation, 1.0f);
	vec4 contiguous2Local = vec4(m_contiguous2 - m_initialTraslation, 1.0f);
	vec4 contiguous3Local = vec4(m_contiguous3 - m_initialTraslation, 1.0f);
	vec4 temp0            = transform.getMat() * contiguous0Local;
	vec4 temp1            = transform.getMat() * contiguous1Local;
	vec4 temp2            = transform.getMat() * contiguous2Local;
	vec4 temp3            = transform.getMat() * contiguous3Local;
	v0                    = vec3(temp0.x, temp0.y, temp0.z);
	v1                    = vec3(temp1.x, temp1.y, temp1.z);
	v2                    = vec3(temp2.x, temp2.y, temp2.z);
	v3                    = vec3(temp3.x, temp3.y, temp3.z);
}

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 EmitterNode::getNextWorldPlaneCenter(float sign)
{
	vec3 v0;
	vec3 v1;
	vec3 v2;
	vec3 v3;
	getNextStepVertices(2.0f * sign, v0, v1, v2, v3);

	return (v0 + v1 + v2 + v3) / 4.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////
