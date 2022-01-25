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

#ifndef _MATHUTIL_H_
#define _MATHUTIL_H_

// GLOBAL INCLUDES
#include <assimp/scene.h>

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/geometry/triangle3d.h"
#include "../../include/geometry/bbox.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MathUtil
{
public:
	/** To build a quaternion which rotates fAngleStep rads around an A axis, see 
	* Mathematics for 3D Game Programming & Computer Graphics (Charles River Media, 2002) chapter 3.2
	* @param fAngleRad [in] angle to rotate vRotVec around vAxis
	* @param vAxis     [in] axis to rotate vRotVec fAngleRad radians
	* @param vRotVec   [in] vector to rotate around vAxis fAngleRad radians
	* @return rotated vector */
	static vec3 rotateVector(const float &fAngleRad, const vec3 &vAxis, const vec3 &vRotVec);

	/** Transforms the point vP by the matrix tMat
	* @param tMat [in]	   matrix used to transform vP
	* @param vP   [in / out] point to be transformed
	* @return nothing */
	static void transformPoint(const mat4 &tMat, vec3 &vP);

	/** Transforms the given vector (x,y,z) coordinates into OpenGL coordinates (-x,z,y), used for info from modelas loaded with
	* z up, x front and y right coordinate systems (like 3ds studio max)
	* @param vertices [in / out] vector to transform each position with the change (x,y,z) -> (-x,z,y)
	* @return nothing */
	static void transformXYZToMinusXZY(vectorVec3 &vecData);

	/** Transforms the given vec3 (x,y,z) coordinates into OpenGL coordinates (-x,z,y), used for info from modelas loaded with
	* z up, x front and y right coordinate systems (like 3ds studio max)
	* @param data [in / out] data to transform each position with the change (x,y,z) -> (-x,z,y)
	* @return nothing */
	static void transformXYZToMinusXZY(vec3 &data);

	/** Will compute the model space matrix and modify the vertices parameter to center the geometry in model space
	* @param vertices [inout] vector with the vertices forming the geometry
	* @return traslation to put the model in world coordinates as it was defined by the content of vertices before the call */
	static vec3 convertToModelSpace(vectorVec3& vertices);

	/** Using the indices in vectorIndex to index in vectorVertex and vectorNormal, builds a vector
	* of float elements with the final triangle information interleaved (V0 V1 V2 N0 V4 V5 V6 N1 ...)
	* @param vectorVertex [in] vector with the vertex information
	* @param vectorNormal [in] vector with the normal information
	* @param vectorIndex  [in] vector with the index information
	* @return geometry buffer */
	static vectorFloat buildGeometryBuffer(const vectorVec3& vectorVertex, const vectorVec3& vectorNormal, const vectorUint& vectorIndex);

	/** Convert a aiMatrix4x4 matrix in assimp to glm mat4
	* @param aiMatrix [in] matrix to convert
	* @return converted matrix */
	static mat4 assimpAIMatrix4x4ToGLM(const aiMatrix4x4* aiMatrix);

	/** Get the next power of two value for the parameter provided
	* @param value [in] value to compute the next power of two
	* @return next power of two value computed */
	static int getNextPowerOfTwo(uint value);

	/** Compute whether the value provided is a power of two or not
	* @param value [in] value to compute if it is a power of two
	* @return true in value is power of two, false otherwise */
	static bool isPowerOfTwo(uint value);
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATHUTIL_H_
