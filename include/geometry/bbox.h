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
#ifndef _BBOX_H_
#define _BBOX_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING
class Node;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////
// Class used to manage the bounding boxes of the objects in the scene
class BBox3D
{
public:
	/** Default constructor
	* @return nothing */
	BBox3D();

	/** Parameter constructor
	* @param vMin [in] min value for the bb
	* @param vMax [in] max value for the bb
	* @return nothing */
	BBox3D(const vec3 &vMin, const vec3 &vMax);

	/** Parameter constructor
	* @param pModel [in] Node for which compute the bounding box
	* @return nothing */
	BBox3D(Node* pModel);

	/** Tests whether vP is inside the bb or not
	* @param vP [in] point to test if is inside the bb or not
	* @return nothing */
	bool insideBB(const vec3 &vP) const;

	/** Computes a bounding box for the model passed as parameter
	* @param pModel [in] Node for which compute the bounding box
	* @return nothing */
	void computeFromModel(Node* pModel);

	/** Computes the bounding box for the vertices passed as param
	* @param vecData [in] vertex data
	* @param pModel  [in] model for which compute the bounding box
	* @return nothing */
	void computeFromVertList(const vectorVec3 &vecData, Node* pModel);

	/** Extends this bb taking the data given as parameter, the resulting bb contains this bb and the one given by data
	* @param data [in] new bb to add volume to this one
	* @return nothing */
	void extend(const BBox3D &data);

	/** Resets the member variable values
	* @return nothing */
	void reset();

	/** Will consider a square box at the center of this box, and return the min and max of that
	* centered box as parameters
	* @param min [inout] centered box min value
	* @param max [inout] centered box max value
	* @return nothing */
	void getCenteredBoxMinMax(vec3& min, vec3& max);

	/** Computes the aabb box from the elements given by the vector of nodes argument
	* @param vectorNode [in] vector of elements to compute the aabb from
	* @return aabb of the elements present in vectorNode */
	static BBox3D computeFromNodeVector(const vectorNodePtr& vectorNode);

	GET_SET(vec3, m_min, Min)
	GET_SET(vec3, m_max, Max)
	GET_SET(float, m_maxDistance, MaxDist)
	GET_SET(bool, m_dirty,	Dirty)
	GET_SET(vec3, m_center, Center)

protected:
	/** Sets the value of m_maxDistance
	* @return nothing */
	void computeMaxDist();

	/** Computes m_center
	* @return nothing */
	void computeCenter();

	vec3  m_min;         //!< Min value for the bounding box
	vec3  m_max;         //!< Max value for the bounding box
	float m_maxDistance; //!< Distance between m_vMin and m_max
	bool  m_dirty;		 //!< True of the bb must be recalculated for some reason (for example, the scene bb when a node changes its position)
	vec3  m_center;	     //!< Box center
};
/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BBOX_H_
