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
#include "../../include/geometry/bbox.h"
#include "../../include/node/node.h"
#include "../../include/util/loopMacroDefines.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

BBox3D::BBox3D():
	m_min(vec3(.0f)),
	m_max(vec3(.0f)),
	m_maxDistance(.0f),
	m_dirty(false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

BBox3D::BBox3D(const vec3 &vMin, const vec3 &vMax):
	m_min(vMin),
	m_max(vMax),
	m_dirty(false),
	m_center((m_min + m_max) * 0.5f)
{
	computeMaxDist();
}

/////////////////////////////////////////////////////////////////////////////////////////////

BBox3D::BBox3D(Node* pModel):
	m_dirty(false)
{
	computeFromModel(pModel);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BBox3D::insideBB(const vec3 &vP) const
{
	if (((vP.x >= m_min.x) && (vP.x <= m_max.x)) &&
		((vP.y >= m_min.y) && (vP.y <= m_max.y)) &&
		((vP.z >= m_min.z) && (vP.z <= m_max.z)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::computeFromModel(Node* pModel)
{
	m_min = vec3(.0f);
	m_max = vec3(.0f);

	if (!pModel)
	{
		cout << "WARNING: no model in BBox3D::computeFromModel(...)" << endl;
		return;
	}

	computeFromVertList(pModel->refVertices(), pModel);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::computeFromVertList(const vectorVec3 &vecData, Node* pModel)
{
	if ((pModel == nullptr) || (vecData.size() < 2))
	{
		if ((vecData.size() == 1))
		{
			m_min		  = vecData[0];
			m_max		  = vecData[0];
			m_maxDistance = .0f;
		}
		else
		{
			reset();
		}

		return;
	}

	// Transform first vertex and set the same value to m_min and m_max
	vec4 vFourD = pModel->getModelMat() * vec4(vecData[0], 1.0f);
	m_min		= vec3(vFourD.x, vFourD.y, vFourD.z);
	m_max		= vec3(vFourD.x, vFourD.y, vFourD.z);

	forI(vecData.size())
	{
		// Transform each vertex to the position of the model if one is supplied (if pModel == nullptr, then
		// the vertices must be is world space and not in model space)
		vFourD = pModel->getModelMat() * vec4(vecData[i], 1.0f);

		if (vFourD.x > m_max.x)
		{
			m_max.x = vFourD.x;
		}
		if (vFourD.y > m_max.y)
		{
			m_max.y = vFourD.y;
		}
		if (vFourD.z > m_max.z)
		{
			m_max.z = vFourD.z;
		}
		if (vFourD.x < m_min.x)
		{
			m_min.x = vFourD.x;
		}
		if (vFourD.y < m_min.y)
		{
			m_min.y = vFourD.y;
		}
		if (vFourD.z < m_min.z)
		{
			m_min.z = vFourD.z;
		}
	}
	computeMaxDist();
	computeCenter();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::computeMaxDist()
{
	m_maxDistance = l2Norm(m_max - m_min);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::computeCenter()
{
	m_center = (m_min + m_max) * 0.5f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::extend(const BBox3D &data)
{
	bool dirty = false;

	if (data.getMax().x > m_max.x)
	{
		m_max.x = data.getMax().x;
		dirty   = true;
	}

	if (data.getMax().y > m_max.y)
	{
		m_max.y = data.getMax().y;
		dirty = true;
	}

	if (data.getMax().z > m_max.z)
	{
		m_max.z = data.getMax().z;
		dirty   = true;
	}

	if (data.getMin().x < m_min.x)
	{
		m_min.x = data.getMin().x;
		dirty   = true;
	}

	if (data.getMin().y < m_min.y)
	{
		m_min.y = data.getMin().y;
		dirty   = true;
	}

	if (data.getMin().z < m_min.z)
	{
		m_min.z = data.getMin().z;
		dirty   = true;
	}

	if (dirty)
	{
		computeMaxDist();
		computeCenter();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::reset()
{
	m_min         = vec3(.0f);
	m_max         = vec3(.0f);
	m_maxDistance = .0f;
	m_center	  = vec3(.0f);
	m_dirty       = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BBox3D::getCenteredBoxMinMax(vec3& min, vec3& max)
{
	float offset;
	min           = m_min;
	max           = m_max;
	float maxSide = glm::max(max.x - min.x, glm::max(max.y - min.y, max.z - min.z));

	if ((max.y - min.y) < maxSide)
	{
		offset = (maxSide - (max.y - min.y)) * 0.5f;
		min.y -= offset;
		max.y += offset;
	}

	if ((max.x - min.x) < maxSide)
	{
		offset = (maxSide - (max.x - min.x)) * 0.5f;
		min.x -= offset;
		max.x += offset;
	}

	if ((max.z - min.z) < maxSide)
	{
		offset = (maxSide - (max.z - min.z)) * 0.5f;
		min.z -= offset;
		max.z += offset;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

BBox3D BBox3D::computeFromNodeVector(const vectorNodePtr& vectorNode)
{
	BBox3D aabb;

	forI(vectorNode.size())
	{
		if (!vectorNode[i]->refAffectSceneBB())
		{
			continue;
		}

		if (aabb.getMin() == aabb.getMax())
		{
			aabb = vectorNode[i]->getBBox(); // first bb
		}
		else
		{
			aabb.extend(vectorNode[i]->getBBox());
		}
	}

	return aabb;
}

/////////////////////////////////////////////////////////////////////////////////////////////
