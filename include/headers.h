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

#pragma once

// GLOBAL INCLUDES
#include <Windows.h>
#include "vulkan.h"
#include "../external/spirv-cross/spirv/GlslangToSpv.h"

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"

// CLASS FORWARDING
class Shader;
class Node;
class RasterTechnique;
class Texture;
class Sampler;
class Image;
class AtomicCounterUnit;
class UniformBase;
class ShaderStruct;
class RasterTechnique;
class ShaderStorageBuffer;
class Material;
class GenericResource;
class Framebuffer;
class Buffer;
class Camera;

// NAMESPACE
using namespace commonnamespace;

// DEFINES
typedef unsigned int			     uint;
typedef vector<char>		         vectorChar;
typedef vector<const char*>		     vectorCharPtr;
typedef vector<int>		             vectorInt;
typedef vector<void*>		         vectorVoidPtr;
typedef vector<uint>                 vectorUint;
typedef vector<uint32_t>             vectorUint32;
typedef vector<uint8_t>              vectorUint8;
typedef vector<uint32_t*>            vectorUint32Ptr;
typedef vector<size_t>               vectorSize_t;
typedef vector<float>                vectorFloat;
typedef vector<vector<float>>        vectorVectorFloat;
typedef vector<bool>                 vectorBool;
typedef vector<vec4>                 vectorVec4;
typedef vector<vector<vec4>>         vectorVectorVec4;
typedef vector<vec3>                 vectorVec3;
typedef vector<vector<vec3>>         vectorVectorVec3;
typedef vector<vec2>                 vectorVec2;
typedef vector<mat4>                 vectorMat4;
typedef vector<string>		         vectorString;
typedef vector<Node*>                vectorNodePtr;
typedef vector<Shader*>              vectorShaderPtr;
typedef vector<RasterTechnique*>     vectorRasterTechniquePtr;
typedef vector<vector<RasterTechnique*>> vectorVectorRasterTechniquePtr;
typedef map<string, uint>            mapStringUint;
typedef vector<Texture*>             vectorTexturePtr;
typedef map<string, Texture*>        mapStringTexturePtr;
typedef vector<UniformBase*>         vectorUniformBasePtr;
typedef vector<Sampler*>		     vectorSamplerPtr;
typedef vector<Image*>			     vectorImagePtr;
typedef vector<AtomicCounterUnit*>   vectorAtomicCounterUnitPtr;
typedef vector<ShaderStruct*>	     vectorShaderStructPtr;
typedef vector<ShaderStruct>	     vectorShaderStruct;
typedef vector<ShaderStorageBuffer*> vectorShaderStorageBufferPtr;
typedef vector<Material*>            vectorMaterialPtr;
typedef map<string, Node*>           mapStringNode;
typedef vector<GenericResource*>     vectorGenericResourcePtr;
typedef map<uint, VkCommandBuffer>   mapUintCommandBuffer;
typedef map<uint, uvec4>             mapUintUvec4;
typedef vector<VkCommandBuffer*>     vectorCommandBufferPtr;
typedef vector<Framebuffer*>         vectorFramebufferPtr;
typedef vector<Buffer*>              vectorBufferPtr;
typedef vector<uvec4>                vectorUvec4;
typedef vector<Camera*>              vectorCameraPtr;

#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define APP_NAME_STR_LEN 80
#else  // _WIN32
#define VK_USE_PLATFORM_XCB_KHR
#include <unistd.h>
#endif // _WIN32
#define GLM_FORCE_RADIANS

#define MOUSE_SPEED  0.001f
#define CAMERA_SPEED 0.5f
#define DEG_TO_RAD   pi<float>()/180.0f
#define RAD_TO_DEG   180.0f/pi<float>()

#define INSTANCE_FUNC_PTR(instance, entrypoint){											\
    m_fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(instance, "vk"#entrypoint); \
    if (m_fp##entrypoint == NULL) {															\
        cout << "Unable to locate the vkGetDeviceProcAddr: vk"#entrypoint;				\
        exit(-1);																			\
    }																						\
}

#define DEVICE_FUNC_PTR(dev, entrypoint){													\
    m_fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);		\
    if (m_fp##entrypoint == NULL) {															\
        cout << "Unable to locate the vkGetDeviceProcAddr: vk"#entrypoint;				\
        exit(-1);																			\
    }																						\
}

#define ZNEAR    0.1f // TODO: adapt per-scene
#define ZFAR  300.0f // TODO: adapt per-scene

/////////////////////////////////////////////////////////////////////////////////////////////

// TODO: MOVE FROM HERE
struct LayerProperties
{
	VkLayerProperties properties;
	vector<VkExtensionProperties> extensions;
};

/////////////////////////////////////////////////////////////////////////////////////////////

/** Used for the diferent types of geometry to represent in OpenGL (triangles, triangle strip, triangle fan for now) */
enum eGeometryType
{
	E_GT_TRIANGLES = 0, // A list of indexed triangles whci form a mesh
	E_GT_TRIANGLESTRIP, // A triangle strip mesh
	E_GT_TRIANGLEFAN,   // A triangle fan mesh
	E_GT_SIZE
};

/////////////////////////////////////////////////////////////////////////////////////////////

/** Used to classify a mesh, and know if it is a ligth volume or a 3D model, and if it is a light vol, which type is it */
enum eMeshType
{
	E_MT_RENDER_MODEL           = 1 << 0, // The mesh is not a light volume
	E_MT_INSTANCED_RENDER_MODEL = 1 << 1, // The mesh is for instance rendering
	E_MT_DEBUG_RENDER_MODEL     = 1 << 2, // The mesh is not a light volume and has debug purposes
	E_MT_EMITTER_MODEL          = 1 << 3, // The mesh is an emitter
	E_MT_LIGHTVOL_SPHERE        = 1 << 4, // The mesh is a light volume of type sphere
	E_MT_LIGHTVOL_CONE          = 1 << 5, // The mesh is a light volume of type cone
	E_MT_LIGHTVOL_TORUS         = 1 << 6, // The mesh is a light volume of type torus
	E_MT_DIRLIGHT_SQUARE        = 1 << 7, // The mesh is a square used for the directional light pass
	E_MT_RENDER_SHADOW          = 1 << 8, // The mesh is for shadow map rasterization
	E_MT_SIZE                   = 1 << 9  // Number of elements of type eMeshType
};

/////////////////////////////////////////////////////////////////////////////////////////////

inline eMeshType operator|(eMeshType a, eMeshType b)
{
	return static_cast<eMeshType>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

/////////////////////////////////////////////////////////////////////////////////////////////
