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

#ifndef _TRIANGLE3D_H_
#define _TRIANGLE3D_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/geometry/bbox.h"

// CLASS FORWARDING
class Node;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

// Class used to manage the mesh containment test
class Triangle3D
{
public:
	/** Default constructor
	* @return nothing */
	Triangle3D();

	/** Builds a new CTriangle3D with v0, v1 and v2 as vertices
	* @param v0 [in] one of the three verties of the triangle
	* @param v1 [in] one of the three verties of the triangle
	* @param v2 [in] one of the three verties of the triangle
	* @return nothing */
	Triangle3D(const vec3 &v0, const vec3 &v1, const vec3 &v2);

	/** Computes the bounding box of the triangle
	* @return the bounding box of the triangle */
	BBox3D getBBox();

	/** Getter of m_v0, m_v1 and m_v2
	* @return a vector with the three vertices of the triangle */
	vectorVec3 getVertices();

	/** Setter of m_v0, m_v1 and m_v2
	* @param v0 [in] one of the three verties of the triangle
	* @param v1 [in] one of the three verties of the triangle
	* @param v2 [in] one of the three verties of the triangle
	* @return nothing */
	void setVertices(const vec3 &v0, const vec3 &v1, const vec3 &v2);

	GET_SET(vec3, m_v0, Vert0)
	GET_SET(vec3, m_v1, Vert1)
	GET_SET(vec3, m_v2, Vert2)

protected:
	vec3 m_v0; //!< One of the three vertices of the triangle
	vec3 m_v1; //!< One of the three vertices of the triangle
	vec3 m_v2; //!< One of the three vertices of the triangle
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _TRIANGLE3D_H_
