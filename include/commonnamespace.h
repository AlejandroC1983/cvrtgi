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

#ifndef _COMMON_NAMESPACE_H_
#define _COMMON_NAMESPACE_H_

// GLOBAL INCLUDES
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <functional>
#include <utility>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <random>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>
#include "gli.hpp"
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/detail/type_half.hpp>

// PROJECT INCLUDES

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

namespace commonnamespace
{
	// std namespace
	using std::cout;
	using std::endl;
	using std::vector;
	using std::map;
	using std::string;
	using std::unordered_map;
	using std::unique_ptr;
	using std::hash;
	using std::move;
	using std::pair;
	using std::make_pair;
	using std::make_unique;
	using std::to_string;
	using std::ofstream;
	using std::setprecision;
	using std::max;
	using std::find;
	using std::ifstream;
	using std::remove;
	using std::ios;

	// glm namespace
	using glm::vec3;
	using glm::vec2;
	using glm::vec4;
	using glm::dvec3;
	using glm::dvec2;
	using glm::dvec4;
	using glm::ivec3;
	using glm::ivec2;
	using glm::ivec4;
	using glm::uvec3;
	using glm::uvec2;
	using glm::uvec4;
	using glm::bvec3;
	using glm::bvec2;
	using glm::bvec4;
	using glm::mat2;
	using glm::mat3;
	using glm::mat4;
	using glm::mat2x3;
	using glm::mat2x4;
	using glm::mat3x2;
	using glm::mat3x4;
	using glm::mat4x2;
	using glm::mat4x3;
	using glm::dmat2;
	using glm::dmat3;
	using glm::dmat4;
	using glm::dmat2x3;
	using glm::dmat2x4;
	using glm::dmat3x2;
	using glm::dmat3x4;
	using glm::dmat4x2;
	using glm::dmat4x3;
	using glm::pi;
	using glm::quat;
	using glm::normalize;
	using glm::inverse;
	using glm::cross;
	using glm::dot;
	using glm::inverseTranspose;
	using glm::l2Norm;
	using glm::radians;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _COMMON_NAMESPACE_H_
