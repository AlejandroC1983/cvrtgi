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

#ifndef _TRIANGLE2D_H_
#define _TRIANGLE2D_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/geometry/bbox.h"

// CLASS FORWARDING
class Node;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

// Class used to manage the mesh containment test
class Triangle2D
{
public:
	/** Default constructor
	* @return nothing */
	Triangle2D();

	/** Builds a new Triangle2D with v0, v1 and v2 as vertices
	* @param v0 [in] one of the three verties of the triangle
	* @param v1 [in] one of the three verties of the triangle
	* @param v2 [in] one of the three verties of the triangle
	* @return nothing */
	Triangle2D(const vec2 &v0, const vec2 &v1, const vec2 &v2);

	/** Getter of m_v0, m_v1 and m_v2
	* @return a vector with the three vertices of the triangle */
	vectorVec2 getVertices();

	/** Setter of m_v0, m_v1 and m_v2
	* @param v0 [in] one of the three verties of the triangle
	* @param v1 [in] one of the three verties of the triangle
	* @param v2 [in] one of the three verties of the triangle
	* @return nothing */
	void setVertices(const vec2 &v0, const vec2 &v1, const vec2 &v2);

	GET_SET(vec2, m_v0, Vert0)
	GET_SET(vec2, m_v1, Vert1)
	GET_SET(vec2, m_v2, Vert2)

protected:
	vec2 m_v0; //!< One of the three vertices of the triangle
	vec2 m_v1; //!< One of the three vertices of the triangle
	vec2 m_v2; //!< One of the three vertices of the triangle
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _TRIANGLE2D_H_
