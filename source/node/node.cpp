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
#include "../../include/node/node.h"
#include "../../include/scene/scene.h"
#include "../../include/util/loopmacrodefines.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION
float Node::s_instanceCounter = 0.0f;

/////////////////////////////////////////////////////////////////////////////////////////////

Node::Node(string&& name, string&& className): GenericResource(move(name), move(className), GenericResourceType::GRT_NODE),
	m_material(nullptr),
	m_materialInstanced(nullptr),
	m_geomType(E_GT_TRIANGLES),
	m_modelMat(mat4(.0f)),
	m_meshType(E_MT_SIZE),
	m_followingPath(false),
	m_startIndex(0),
	m_endIndex(0),
	m_indexOffset(0),
	m_indexSize(0),
	m_generateNormal(true),
	m_generateTangent(true),
	m_affectSceneBB(true),
	m_instanceCounter(s_instanceCounter)
{
	s_instanceCounter += 1.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Node::Node(string&&          name,
		   string&&          className,
	       const vectorUint &indices,
		   const vectorVec3 &vertices,
		   const vectorVec2 &texCoord,
		   const vectorVec3 &normals,
		   const vectorVec3 &tangents) : GenericResource(move(name), move(className), GenericResourceType::GRT_NODE),
	m_material(nullptr),
	m_materialInstanced(nullptr),
	m_geomType(E_GT_TRIANGLES),
	m_modelMat(mat4(.0f)),
	m_meshType(E_MT_SIZE),
	m_followingPath(false),
	m_startIndex(0),
	m_endIndex(0),
	m_indexOffset(0),
	m_indexSize(0),
	m_generateNormal(true),
	m_generateTangent(true),
	m_affectSceneBB(true),
	m_instanceCounter(s_instanceCounter)
{
	s_instanceCounter += 1.0f;

	m_indices  = indices;
	m_vertices = vertices;
	m_normals  = normals;
	m_texCoord = texCoord;
	m_tangents = tangents;

	if (m_normals.size() == m_vertices.size())
	{
		m_generateNormal = false;
	}

	if (m_tangents.size() == m_vertices.size())
	{
		m_generateTangent = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Node::~Node()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Node::initialize(const vec3& vPosition, const quat& rotation, const vec3& scale, Material* material, Material* materialInstanced, eMeshType eMeshType)
{
	m_material	        = material;
	m_materialInstanced = materialInstanced;
	m_meshType          = eMeshType;
	m_transform         = Transform(vPosition, rotation, scale);
	m_affectSceneBB     = (eMeshType == eMeshType::E_MT_RENDER_MODEL);

	generateGeometry(); //Build up the array of vertices

	if (m_generateNormal)
	{
		generateNormals();
	}

	if (m_generateTangent)
	{
		generateTangents();
	}

	compressNormals();
	compressTangents();

	computeBB();

	buildPerVertexInfo();

	bindData();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::generateNormals()
{
	// For each set of three index, take the corresponding vertices v0, v1 and v2, and generate normals for the three by
	// making vectors v0v1 and v0v2, and the cross product between them, giving an orthonormal vector to the triangle v0-v1-v2
	// As each vertex can be used in multiple triangles, each time a normal is calculated for a particular vertex, it is added
	// to the previously obtained normals for that vertex. At the end, the resulting vector is divided by the amount of normal
	// vectors and renormalized, obtaining an averaged normal

	vectorVec3 faceNormals; //Temporary array to store the face normals
	vectorInt shareCount;

    m_normals.resize(m_vertices.size()); //We want a normal for each vertex
    shareCount.resize(m_vertices.size());

	forI(shareCount.size())
	{
		shareCount[i] = 0;
	}

    uint numTriangles = uint(m_indices.size()) / 3;
    faceNormals.resize(numTriangles); //One normal per triangle

	forI(numTriangles)
	{
        vec3 v0	    = m_vertices.at(m_indices.at(i*3));
        vec3 v1	    = m_vertices.at(m_indices.at((i*3)+1));
        vec3 v2	    = m_vertices.at(m_indices.at((i*3)+2));
        vec3 vec0   = v1 - v0;
		vec3 vec1   = v2 - v0;
        vec3 normal = faceNormals.at(i);

		normal = cross(vec0, vec1);
		normal = normalize(normal);

		forJ(3)
		{
            int index		  = m_indices.at((i*3)+j);
			m_normals.at(index) += normal;
            shareCount.at(index)++;
        }
    }

	forI(m_vertices.size())
	{
		m_normals.at(i).x = m_normals.at(i).x / shareCount.at(i);
        m_normals.at(i).y = m_normals.at(i).y / shareCount.at(i);
        m_normals.at(i).z = m_normals.at(i).z / shareCount.at(i);
		m_normals.at(i) = normalize(m_normals.at(i));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::generateTangents()
{
	vectorVec3 tan1;
	tan1.resize(m_vertices.size() * 2);
	vectorVec3 tan2;
	tan2.resize(m_vertices.size() * 2);
	forIT(tan1)
	{
		it->x = 0.0f;
		it->y = 0.0f;
		it->z = 0.0f;
	}
	forIT(tan2)
	{
		it->x = 0.0f;
		it->y = 0.0f;
		it->z = 0.0f;
	}

	uint numTriangles = uint(m_indices.size()) / 3;

	forI(numTriangles)
	{
		vec3 v1 = m_vertices.at(m_indices.at(i*3));
        vec3 v2 = m_vertices.at(m_indices.at((i*3)+1));
        vec3 v3 = m_vertices.at(m_indices.at((i*3)+2));
				 
		vec2 w1 = m_texCoord.at(m_indices.at(i*3));
        vec2 w2 = m_texCoord.at(m_indices.at((i*3)+1));
        vec2 w3 = m_texCoord.at(m_indices.at((i*3)+2));

		float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

		float r    = 1.0F / (s1 * t2 - s2 * t1);
        vec3 sdir = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        vec3 tdir = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		// If there are bad tex coords, do not add to the tan1 and tan2 vectors
		bool b1 = (isnan(r) || isnan(sdir.x) || isnan(sdir.y) || isnan(sdir.z) || isnan(tdir.x) || isnan(tdir.y) || isnan(tdir.z));
		if (!b1)
		{
			tan1.at(m_indices.at(i * 3))     += sdir;
			tan1.at(m_indices.at(i * 3 + 1)) += sdir;
			tan1.at(m_indices.at(i * 3 + 2)) += sdir;

			tan2.at(m_indices.at(i * 3))     += tdir;
			tan2.at(m_indices.at(i * 3 + 1)) += tdir;
			tan2.at(m_indices.at(i * 3 + 2)) += tdir;
		}
	}

	m_tangents.clear();
	m_tangents.resize(m_vertices.size());

	forA(m_vertices.size())
	{
		vec3 n     = m_normals.at(a);
		vec3 t     = tan1.at(a);
		vec3 vTemp = t;
		// Gram-Schmidt orthogonalize: only if dot product between n and t is != 0, otherwise (0,0,0) vector will be generated
		if (dot(n, t) != 0.0f)
		{
			vTemp = (t - n) * dot(n, t);
		}
		vTemp = normalize(vTemp);
		m_tangents.at(a) = vTemp;
    }
    
	tan1.clear();
	tan2.clear();

	// taken from http://www.terathon.com/code/tangent.html for generating correctly the tangent "T" vector
	// (must be aligned with the increasing "u" direction of the texture coordinates, as the v direction corresponds
	// to the B or binormal vector)
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::compressTangents()
{
	forIT(m_tangents)
	{
		it->x = 0.5f * (it->x + 1.0f);
		it->y = 0.5f * (it->y + 1.0f);
		it->z = 0.5f * (it->z + 1.0f);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::compressNormals()
{
	forIT(m_normals)
	{
		it->x = 0.5f * (it->x + 1.0f);
		it->y = 0.5f * (it->y + 1.0f);
		it->z = 0.5f * (it->z + 1.0f);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::bindData()
{
	// Once the per vertex data has been uploaded, the texture coordinate, normal and tangent data per vertex can be
	// deleted as it won't be used again. The indices and vertices arrays can be used to obtain information such as
	// the point in polyhedron test
	m_texCoord.clear();
	m_normals.clear();
	m_tangents.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::prepare(const float &fDt)
{
	// update bb if the model matrix changed
	// TODO: this should be done obtaining the matrix in object space and modifying it insted of all the geometry
	if (m_transform.getDirty())
	{
		m_aabb.computeFromVertList(m_vertices, this);
		m_transform.setDirty(false);
		if (m_affectSceneBB)
		{
			sceneM->refBox().setDirty(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::generateGeometry()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Node::testPointInside(const vec3 &vP)
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::computeBB()
{
	// NOTE: Remove if results are the same as in Model::processMesh
	m_aabb.computeFromModel(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::setBBox(BBox3D* pBBox)
{
	if (!pBBox)
	{
		return;
	}

	m_aabb.setMin(pBBox->getMin());
	m_aabb.setMax(pBBox->getMax());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Node::buildPerVertexInfo()
{
	forI(m_vertices.size())
	{
		m_vertexData.push_back(m_vertices.at(i).x);
		m_vertexData.push_back(m_vertices.at(i).y);
		m_vertexData.push_back(m_vertices.at(i).z);
		m_vertexData.push_back(m_texCoord.at(i).x);
		m_vertexData.push_back(m_texCoord.at(i).y);
		m_vertexData.push_back(m_instanceCounter); // The instance counter info is in the z component of the UV input attribute
		m_vertexData.push_back(m_normals.at(i).x);
		m_vertexData.push_back(m_normals.at(i).y);
		m_vertexData.push_back(m_normals.at(i).z);
		m_vertexData.push_back(m_tangents.at(i).x);
		m_vertexData.push_back(m_tangents.at(i).y);
		m_vertexData.push_back(m_tangents.at(i).z);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////