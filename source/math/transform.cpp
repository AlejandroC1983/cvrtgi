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
#include "../../include/math/transform.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Transform::Transform():
	m_traslation(vec3(.0f)),
	m_scaling(vec3(1.0f)),
	m_rotation(quat()),
	m_mat(mat4()),
	m_dirty(false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

Transform::Transform(const vec3 traslation):
	m_traslation(traslation),
	m_scaling(vec3(1.0f)),
	m_rotation(quat()),
	m_mat(mat4()),
	m_dirty(false)
{
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

Transform::Transform(const vec3 traslation, const quat rotation):
	m_traslation(traslation),
	m_scaling(vec3(1.0f)),
	m_rotation(rotation),
	m_mat(mat4()),
	m_dirty(false)
{
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

Transform::Transform(const vec3 traslation, const quat rotation, const vec3 scaling):
	m_traslation(traslation),
	m_scaling(scaling),
	m_rotation(rotation),
	m_mat(mat4()),
	m_dirty(false)
{
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::updateMatrix()
{
	m_mat       = glm::mat4_cast(m_rotation);
	m_mat[3].x  = m_traslation.x;
	m_mat[3].y  = m_traslation.y;
	m_mat[3].z  = m_traslation.z;
	m_mat[0].x *= m_scaling.x;
	m_mat[1].y *= m_scaling.y;
	m_mat[2].z *= m_scaling.z;
	m_dirty     = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addRotationX(const float angle)
{
	m_rotation *= quat(vec3(angle, .0f, .0f));
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addRotationY(const float angle)
{
	m_rotation *= quat(vec3(.0f, angle, .0f));
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addRotationZ(const float angle)
{
	m_rotation *= quat(vec3(.0f, .0f, angle));
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addRotation(const vec3 angle)
{
	m_rotation *= quat(angle);
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addRotation(const quat q)
{
	m_rotation *= q;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::setRotation(const vec3 angle)
{
	m_rotation = quat(angle);
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::setRotation(const quat q)
{
	m_rotation = q;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addTraslation(const vec3 offset)
{
	m_traslation += offset;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::setTraslation(const vec3 translation)
{
	m_traslation = translation;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addScalingX(const float value)
{
	m_scaling.x += value;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addScalingY(const float value)
{
	m_scaling.y += value;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addScalingZ(const float value)
{
	m_scaling.z += value;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::addScaling(const vec3 value)
{
	m_scaling += value;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Transform::setScaling(const vec3 value)
{
	m_scaling = value;
	updateMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////
