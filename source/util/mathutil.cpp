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
#include "../../include/util/mathutil.h"
#include "../../include/util/loopMacroDefines.h"
#include "../../include/math/transform.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 MathUtil::rotateVector(const float &fAngleRad, const vec3 &vAxis, const vec3 &vRotVec)
{
	float fSinAngle = sinf(fAngleRad / 2.0f);
	quat q          = quat(cosf(fAngleRad / 2.0f), vAxis.x * fSinAngle, vAxis.y * fSinAngle, vAxis.z * fSinAngle);
	quat qInv       = inverse(q);
	quat P          = quat(0.0f, vRotVec.x, vRotVec.y, vRotVec.z);
	P               = normalize(P);
	quat qPqInv     = q * P * qInv;
	return vec3(qPqInv.x, qPqInv.y, qPqInv.z);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MathUtil::transformPoint(const mat4 &tMat, vec3 &vP)
{
	vec4 vT = tMat * vec4(vP, 1.0f);
	vP		= vec3(vT.x, vT.y, vT.z);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MathUtil::transformXYZToMinusXZY(vectorVec3 &vecData)
{
	vec3 temp;
	forI(vecData.size())
	{
		temp = vecData[i];
		vecData[i] = vec3(-1.0f * temp.x, temp.z, temp.y);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MathUtil::transformXYZToMinusXZY(vec3 &data)
{
	data = vec3(-1.0f * data.x, data.z, data.y);
}

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 MathUtil::convertToModelSpace(vectorVec3& vertices)
{
	const uint maxIndex = uint(vertices.size());

	vec3 min = vec3(FLT_MAX);
	vec3 max = vec3(-1.0f * FLT_MAX);
	vec3 temp;

	forI(maxIndex)
	{
		temp  = vertices[i];
		max.x = glm::max(temp.x, max.x);
		max.y = glm::max(temp.y, max.y);
		max.z = glm::max(temp.z, max.z);
		min.x = glm::min(temp.x, min.x);
		min.y = glm::min(temp.y, min.y);
		min.z = glm::min(temp.z, min.z);
	}

	vec3 mid = (min + max) * 0.5f;

	forI(maxIndex)
	{
		vertices[i] -= mid;
	}

	return mid;
}

/////////////////////////////////////////////////////////////////////////////////////////////

vectorFloat MathUtil::buildGeometryBuffer(const vectorVec3& vectorVertex, const vectorVec3& vectorNormal, const vectorUint& vectorIndex)
{
	const uint maxIndex = uint(vectorIndex.size());

	vectorFloat result;
	result.resize((maxIndex / 3) * 12);

	float* pVector = result.data();
	uint counter   = 0;

	vec3 n0;
	vec3 n1;
	vec3 n2;
	vec3 normal;

	forIStep(3, maxIndex)
	{
		pVector[counter++] = vectorVertex[vectorIndex[i + 0]][0];
		pVector[counter++] = vectorVertex[vectorIndex[i + 0]][1];
		pVector[counter++] = vectorVertex[vectorIndex[i + 0]][2];
		pVector[counter++] = vectorVertex[vectorIndex[i + 1]][0];
		pVector[counter++] = vectorVertex[vectorIndex[i + 1]][1];
		pVector[counter++] = vectorVertex[vectorIndex[i + 1]][2];
		pVector[counter++] = vectorVertex[vectorIndex[i + 2]][0];
		pVector[counter++] = vectorVertex[vectorIndex[i + 2]][1];
		pVector[counter++] = vectorVertex[vectorIndex[i + 2]][2];
		n0                 = vectorNormal[vectorIndex[i + 0]];
		n1                 = vectorNormal[vectorIndex[i + 1]];
		n2                 = vectorNormal[vectorIndex[i + 2]];
		normal             = normalize(n0 + n1 + n2);
		pVector[counter++] = normal[0];
		pVector[counter++] = normal[1];
		pVector[counter++] = normal[2];
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

mat4 MathUtil::assimpAIMatrix4x4ToGLM(const aiMatrix4x4* aiMatrix)
{
	glm::mat4 to;

	to[0][0] = (float)aiMatrix->a1; to[0][1] = (float)aiMatrix->b1;  to[0][2] = (float)aiMatrix->c1; to[0][3] = (float)aiMatrix->d1;
	to[1][0] = (float)aiMatrix->a2; to[1][1] = (float)aiMatrix->b2;  to[1][2] = (float)aiMatrix->c2; to[1][3] = (float)aiMatrix->d2;
	to[2][0] = (float)aiMatrix->a3; to[2][1] = (float)aiMatrix->b3;  to[2][2] = (float)aiMatrix->c3; to[2][3] = (float)aiMatrix->d3;
	to[3][0] = (float)aiMatrix->a4; to[3][1] = (float)aiMatrix->b4;  to[3][2] = (float)aiMatrix->c4; to[3][3] = (float)aiMatrix->d4;

	return to;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int MathUtil::getNextPowerOfTwo(uint value)
{
	// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool MathUtil::isPowerOfTwo(uint value)
{
	return ((value & (value - 1)) == 0);

	//float logValue = glm::log2(float(value));
	//float decimal = logValue - floorf(logValue);
	//return ((logValue - decimal) > 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////
