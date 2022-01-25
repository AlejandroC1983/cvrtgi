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

#ifndef _ATTRIBUTEDEFINES_H_
#define _ATTRIBUTEDEFINES_H_

// GLOBAL INCLUDES
#include "../headers.h"
#include "../../include/commonnamespace.h"

// PROJECT INCLUDES

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** Namespace where all the attributes that can be used in the intitalization of any class that inherits from the
* GenericResource class are placed. A string and its corresponding hashed value should be the typical pairs of data. */

namespace attributedefines
{
	// Pipeline building parameters
	extern const char* g_frontFace;
	extern const char* g_cullMode;
	extern const char* g_depthClampEnable;
	extern const char* g_depthBiasEnable;
	extern const char* g_depthBiasConstantFactor;
	extern const char* g_depthBiasClamp;
	extern const char* g_depthBiasSlopeFactor;
	extern const char* g_blendEnable;
	extern const char* g_alphaColorBlendOp;
	extern const char* g_sourceAlphaColorBlendFactor;
	extern const char* g_destinationAlphaColorBlendFactor;
	extern const char* g_colorWriteMask;
	extern const char* g_lineWidth;
	extern const char* g_depthTestEnable;
	extern const char* g_depthWriteEnable;
	extern const char* g_depthCompareOp;
	extern const char* g_depthBoundsTestEnable;
	extern const char* g_stencilTestEnable;
	extern const char* g_backFailOp;
	extern const char* g_backPassOp;
	extern const char* g_backCompareOp;
	extern const char* g_backCompareMask;
	extern const char* g_backReference;
	extern const char* g_backDepthFailOp;
	extern const char* g_backWriteMask;
	extern const char* g_minDepthBounds;
	extern const char* g_maxDepthBounds;
	extern const char* g_pipelineData;

	// Pipeline building parameters
	extern const char* g_pipelineShaderStage;
	extern const char* g_pipelineLayout;

	// Pipeline building hashed parameters
	extern const uint g_frontFaceHashed;
	extern const uint g_cullModeHashed;
	extern const uint g_depthClampEnableHashed;
	extern const uint g_depthBiasEnableHashed;
	extern const uint g_depthBiasConstantFactorHashed;
	extern const uint g_depthBiasClampHashed;
	extern const uint g_depthBiasSlopeFactorHashed;
	extern const uint g_blendEnableHashed;
	extern const uint g_alphaColorBlendOpHashed;
	extern const uint g_sourceAlphaColorBlendFactorHashed;
	extern const uint g_destinationAlphaColorBlendFactorHashed;
	extern const uint g_colorWriteMaskHashed;
	extern const uint g_lineWidthHashed;
	extern const uint g_depthTestEnableHashed;
	extern const uint g_depthWriteEnableHashed;
	extern const uint g_depthCompareOpHashed;
	extern const uint g_depthBoundsTestEnableHashed;
	extern const uint g_stencilTestEnableHashed;
	extern const uint g_backFailOpHashed;
	extern const uint g_backPassOpHashed;
	extern const uint g_backCompareOpHashed;
	extern const uint g_backCompareMaskHashed;
	extern const uint g_backReferenceHashed;
	extern const uint g_backDepthFailOpHashed;
	extern const uint g_backWriteMaskHashed;
	extern const uint g_minDepthBoundsHashed;
	extern const uint g_maxDepthBoundsHashed;
	extern const uint g_pipelineDataHashed;
	extern const uint g_pipelineDataHashed;

	// Pipeline building hashed parameters
	extern const uint g_pipelineShaderStageHashed;
	extern const uint g_pipelineLayoutHashed;

	// Render pass building parameters
	extern const char* g_renderPassAttachmentFormat;
	extern const char* g_renderPassAttachmentSamplesPerPixel;
	extern const char* g_renderPassAttachmentFinalLayout;
	extern const char* g_renderPassAttachmentColorReference;
	extern const char* g_renderPassAttachmentDepthReference;
	extern const char* g_renderPassAttachmentPipelineBindPoint;

	// Render pass building hashed parameters
	extern const uint g_renderPassAttachmentFormatHashed;
	extern const uint g_renderPassAttachmentSamplesPerPixelHashed;
	extern const uint g_renderPassAttachmentFinalLayoutHashed;
	extern const uint g_renderPassAttachmentColorReferenceHashed;
	extern const uint g_renderPassAttachmentDepthReferenceHashed;
	extern const uint g_renderPassAttachmentPipelineBindPointHashed;

	// Manager template names
	extern const char* g_textureManager;
	extern const char* g_bufferManager;
	extern const char* g_shaderManager;
	extern const char* g_framebufferManager;
	extern const char* g_inputSingleton;
	extern const char* g_sceneSingleton;
	extern const char* g_pipelineManager;
	extern const char* g_uniformBufferManager;
	extern const char* g_materialManager;
	extern const char* g_rasterTechniqueManager;
	extern const char* g_renderPassManager;
	extern const char* g_cameraManager;

	// Reflectance and normal material texture resource names
	extern const char* g_reflectanceTextureResourceName;
	extern const char* g_normalTextureResourceName;

	// Reflectance and normal material texture resource hashed names
	extern const uint g_reflectanceTextureResourceNameHashed;
	extern const uint g_normalTextureResourceNameHashed;

	// Lit cluster compute shader source code chunk
	extern const char* g_litClusterCodeChunk;

	// Lit cluster compute shader source code chunk hashed name
	extern const uint g_litClusterCodeChunkHashed;

	// Lit cluster process results compute shader source code chunk
	extern const char* g_litClusterProcessResultsCodeChunk;

	// Lit cluster process results compute shader source code chunk hashed name
	extern 	const uint g_litClusterProcessResultsCodeChunkHashed;

	// Light bounce voxel irradiance field compute shader source code chunk
	extern const char* g_lightBounceVoxelIrradianceCodeChunk;

	// Light bounce voxel irradiance field compute shader source code chunk hashed name
	extern const uint g_lightBounceVoxelIrradianceCodeChunkHashed;

	// Light bounce voxel Gaussian filter compute shader source code chunk
	extern const char* g_lightBounceVoxelGaussianFilterCodeChunk;

	// Light bounce voxel Gaussian filter compute shader source code chunk hashed name
	extern const uint g_lightBounceVoxelGaussianFilterCodeChunkHashed;

	// Reset clusterization irradiance data compute shader source code chunk
	extern const char* g_clusterizationClearIrradianceDataCodeChunk;

	// Reset clusterization irradiance data compute shader source code chunk hashed name
	extern const uint g_clusterizationClearIrradianceDataCodeChunkHashed;

	// Build voxel shadow map geometry compute shader source code chunk
	extern const char* g_buildVoxelShadowMapGeometryCodeChunk;

	// Build voxel shadow map geometry compute shader source code chunk hashed name
	extern const uint g_buildVoxelShadowMapGeometryCodeChunkHashed;

	// Voxel clusterization compute shader source code chunk
	extern const char* g_voxelClusterizationCodeChunk;

	// Voxel clusterization compute shader source code chunk hashed name
	extern const uint g_voxelClusterizationCodeChunkHashed;

	// Voxel clusterization new center compute shader source code chunk
	extern const char* g_voxelClusterizationNewCenterCodeChunk;

	// Voxel clusterization new center compute shader source code chunk hashed name
	extern const uint g_voxelClusterizationNewCenterCodeChunkHashed;

	// Voxel clusterization voxel distance initialization compute shader source code chunk
	extern const char* g_voxelClusterizationInitVoxelDistanceCodeChunk;

	// Voxel clusterization voxel distance initialization compute shadoer code chunk hashed name
	extern const uint g_voxelClusterizationInitVoxelDistanceCodeChunkHashed;

	// Voxel clusterization voxel add up compute shader source code chunk
	extern const char* g_voxelClusterizationAddUpCodeChunk;

	// Voxel clusterization voxel add up compute shadoer code chunk hashed name
	extern const uint g_voxelClusterizationAddUpCodeChunkHashed;

	// Voxel clusterization voxel compute AABB compute shader source code chunk
	extern const char* g_voxelClusterizationComputeAABBCodeChunk;

	// Voxel clusterization voxel compute AABB shader code chunk hashed name
	extern const uint g_voxelClusterizationComputeAABBCodeChunkHashed;

	// Voxel clusterization build final buffer compute shader source code chunk
	extern const char* g_voxelClusterizationBuildFinalBufferCodeChunk;

	// Voxel clusterization build final buffer compute AABB shader code chunk hashed name
	extern const uint g_voxelClusterizationBuildFinalBufferCodeChunkHashed;

	// Voxel clusterization compute neighbour compute shader source code chunk
	extern const char* g_voxelClusterizationComputeNeighbourCodeChunk;

	// Voxel clusterization compute neighbour compute shader code chunk hashed name
	extern const uint g_voxelClusterizationComputeNeighbourCodeChunkHashed;

	// Voxel clusterization merge clusters compute shader source code chunk
	extern const char* g_voxelClusterizationMergeClustersCodeChunk;

	// Voxel clusterization merge clusters compute shader code chunk hashed name
	extern const uint g_voxelClusterizationMergeClustersCodeChunkHashed;

	// Voxel clusterization preparation compute shader source code chunk
	extern const char* g_voxelClusterizationPrepareCodeChunk;

	// Voxel clusterization preparation compute shader source code chunk hashed name
	extern const uint g_voxelClusterizationPrepareCodeChunkHashed;

	// Cluster data initialization compute shader source code chunk
	extern const char* g_clusterizationInitAABBCodeChunk;

	// Cluster data initialization compute shader source code chunk hashed name
	extern const uint g_clusterizationInitAABBCodeChunkHashed;

	// Camera visible voxel compute shader source code chunk
	extern const char* g_cameraVisibleVoxelCodeChunk;

	// Camera visible voxel compute shader source code chunk hashed
	extern const uint g_cameraVisibleVoxelCodeChunkHashed;

	// Distance shadow mapping distance texture name compute shader source code chunk
	extern const char* g_distanceShadowMapDistanceTextureCodeChunk;

	// Distance shadow mapping distance texture name compute shader source code chunk hashed
	extern const uint g_distanceShadowMapDistanceTextureCodeChunkHashed;

	// Distance shadow mapping offscreen texture name compute shader source code chunk
	extern const char* g_distanceShadowMapOffscreenTextureCodeChunk;

	// Distance shadow mapping offscreen texture name compute shader source code chunk hashed
	extern const uint g_distanceShadowMapOffscreenTextureCodeChunkHashed;

	// Distance shadow mapping material name compute shader source code chunk
	extern const char* g_distanceShadowMapMaterialNameCodeChunk;

	// Distance shadow mapping material name compute shader source code chunk hashed
	extern const uint g_distanceShadowMapMaterialNameCodeChunkHashed;

	// Distance shadow mapping framebuffer name compute shader source code chunk
	extern const char* g_distanceShadowMapFramebufferNameCodeChunk;

	// Distance shadow mapping framebuffer name compute shader source code chunk hashed
	extern const uint g_distanceShadowMapFramebufferNameCodeChunkHashed;

	// Distance shadow mapping camera name compute shader source code chunk
	extern const char* g_distanceShadowMapCameraNameCodeChunk;

	// Distance shadow mapping camera name compute shader source code chunk hashed
	extern const uint g_distanceShadowMapCameraNameCodeChunkHashed;

	// Distance shadow mapping use compacted geometry compute shader source code chunk
	extern const char* g_distanceShadowMapUseCompactedGeometryCodeChunk;

	// Distance shadow mapping use compacted geometry compute shader source code chunk hashed
	extern const uint g_distanceShadowMapUseCompactedGeometryCodeChunkHashed;

	// Cluster visibility compute shader source code chunk
	extern const char* g_clusterVisibilityCodeChunk;

	// Cluster visibility compute shader source code chunk hashed name
	extern const uint g_clusterVisibilityCodeChunkHashed;

	// Voxel face visible filtering compute shader source code chunk
	extern const char* g_voxelFacePenaltyCodeChunk;

	// Voxel face visible filtering compute shader source code chunk hashed name
	extern const uint g_voxelFacePenaltyCodeChunkHashed;

	// Distance shadow mapping indirect command buffer name code chunk
	extern const char* g_indirectCommandBufferCodeChunk;

	// Distance shadow mapping indirect command buffer name code chunk hashed
	extern const uint g_indirectCommandBufferCodeChunkHashed;

	// Compute frustum culling compute shader source code chunk
	extern const char* g_computeFrustumCullingCodeChunk;

	// Compute frustum culling compute shader source code chunk hashed name
	extern const uint g_computeFrustumCullingCodeChunkHashed;

	// Material surface type
	extern const char* g_materialSurfaceType;

	// Material surface type hashed name
	extern const uint g_materialSurfaceTypeChunkHashed;

	// Material distance shadw map source code chunk
	extern const char* g_distanceShadowMapUseInstancedRendering;

	// Material distance shadw map source code chunk hashed
	extern const uint g_distanceShadowMapUseInstancedRenderingHashed;

	// Material distance shadw map source code chunk
	extern const char* g_distanceShadowMapUseInstancedRendering;

	// Material distance shadw map source code chunk hashed
	extern const uint g_distanceShadowMapUseInstancedRenderingHashed;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _ATTRIBUTEDEFINES_H_
