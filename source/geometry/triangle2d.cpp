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
#include "../../include/geometry/triangle2d.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Triangle2D::Triangle2D():
	m_v0(vec2(.0f)),
	m_v1(vec2(.0f)),
	m_v2(vec2(.0f))
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

Triangle2D::Triangle2D(const vec2 &v0, const vec2 &v1, const vec2 &v2):
	m_v0(v0),
	m_v1(v1),
	m_v2(v2)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

vectorVec2 Triangle2D::getVertices()
{
	vectorVec2 vVerts;
	vVerts.clear();
	vVerts.push_back(m_v0);
	vVerts.push_back(m_v1);
	vVerts.push_back(m_v2);
	return vVerts;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Triangle2D::setVertices(const vec2 &v0, const vec2 &v1, const vec2 &v2)
{
	m_v0 = v0;
	m_v1 = v1;
	m_v2 = v2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
