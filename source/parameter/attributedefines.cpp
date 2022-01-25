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
#include "../../include/parameter/attributedefines.h"
#include "../../include/commonnamespace.h"

// NAMESPACE
using namespace commonnamespace;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

namespace attributedefines
{
	// Pipeline building parameters
	const char* g_frontFace                             = "frontFace";
	const char* g_cullMode                              = "cullMode";
	const char* g_depthClampEnable                      = "depthClampEnable";
	const char* g_depthBiasEnable                       = "depthBiasEnable";
	const char* g_depthBiasConstantFactor               = "depthBiasConstantFactor";
	const char* g_depthBiasClamp                        = "depthBiasClamp";
	const char* g_depthBiasSlopeFactor                  = "depthBiasSlopeFactor";
	const char* g_blendEnable                           = "blendEnable";
	const char* g_alphaColorBlendOp                     = "alphaColorBlendOp";
	const char* g_sourceAlphaColorBlendFactor           = "sourceAlphaColorBlendFactor";
	const char* g_destinationAlphaColorBlendFactor      = "destinationAlphaColorBlendFactor";
	const char* g_colorWriteMask                        = "colorWriteMask";
	const char* g_lineWidth                             = "lineWidth";
	const char* g_depthTestEnable                       = "depthTestEnable";
	const char* g_depthWriteEnable                      = "depthWriteEnable";
	const char* g_depthCompareOp                        = "depthCompareOp";
	const char* g_depthBoundsTestEnable                 = "depthBoundsTestEnable";
	const char* g_stencilTestEnable                     = "stencilTestEnable";
	const char* g_backFailOp                            = "backFailOp";
	const char* g_backPassOp                            = "backPassOp";
	const char* g_backCompareOp                         = "backCompareOp";
	const char* g_backCompareMask                       = "backCompareMask";
	const char* g_backReference                         = "backReference";
	const char* g_backDepthFailOp                       = "backDepthFailOp";
	const char* g_backWriteMask                         = "backWriteMask";
	const char* g_minDepthBounds                        = "minDepthBounds";
	const char* g_maxDepthBounds                        = "maxDepthBounds";
	const char* g_pipelineData                          = "pipelineData";

	// Pipeline building parameters
	const char* g_pipelineShaderStage                   = "pipelineShaderStage";
	const char* g_pipelineLayout                        = "pipelineLayout";

	const uint g_frontFaceHashed                        = uint(hash<string>()(g_frontFace));
	const uint g_cullModeHashed                         = uint(hash<string>()(g_cullMode));
	const uint g_depthClampEnableHashed                 = uint(hash<string>()(g_depthClampEnable));
	const uint g_depthBiasEnableHashed                  = uint(hash<string>()(g_depthBiasEnable));
	const uint g_depthBiasConstantFactorHashed          = uint(hash<string>()(g_depthBiasConstantFactor));
	const uint g_depthBiasClampHashed                   = uint(hash<string>()(g_depthBiasClamp));
	const uint g_depthBiasSlopeFactorHashed             = uint(hash<string>()(g_depthBiasSlopeFactor));
	const uint g_blendEnableHashed                      = uint(hash<string>()(g_blendEnable));
	const uint g_alphaColorBlendOpHashed                = uint(hash<string>()(g_alphaColorBlendOp));
	const uint g_sourceAlphaColorBlendFactorHashed      = uint(hash<string>()(g_sourceAlphaColorBlendFactor));
	const uint g_destinationAlphaColorBlendFactorHashed = uint(hash<string>()(g_destinationAlphaColorBlendFactor));
	const uint g_colorWriteMaskHashed                   = uint(hash<string>()(g_colorWriteMask));
	const uint g_lineWidthHashed                        = uint(hash<string>()(g_lineWidth));
	const uint g_depthTestEnableHashed                  = uint(hash<string>()(g_depthTestEnable));
	const uint g_depthWriteEnableHashed                 = uint(hash<string>()(g_depthWriteEnable));
	const uint g_depthCompareOpHashed                   = uint(hash<string>()(g_depthCompareOp));
	const uint g_depthBoundsTestEnableHashed            = uint(hash<string>()(g_depthBoundsTestEnable));
	const uint g_stencilTestEnableHashed                = uint(hash<string>()(g_stencilTestEnable));
	const uint g_backFailOpHashed                       = uint(hash<string>()(g_backFailOp));
	const uint g_backPassOpHashed                       = uint(hash<string>()(g_backPassOp));
	const uint g_backCompareOpHashed                    = uint(hash<string>()(g_backCompareOp));
	const uint g_backCompareMaskHashed                  = uint(hash<string>()(g_backCompareMask));
	const uint g_backReferenceHashed                    = uint(hash<string>()(g_backReference));
	const uint g_backDepthFailOpHashed                  = uint(hash<string>()(g_backDepthFailOp));
	const uint g_backWriteMaskHashed                    = uint(hash<string>()(g_backWriteMask));
	const uint g_minDepthBoundsHashed                   = uint(hash<string>()(g_minDepthBounds));
	const uint g_maxDepthBoundsHashed                   = uint(hash<string>()(g_maxDepthBounds));
	const uint g_pipelineDataHashed                     = uint(hash<string>()(g_pipelineData));

	// Pipeline building hashed parameters
	const uint g_pipelineShaderStageHashed              = uint(hash<string>()(g_pipelineShaderStage));
	const uint g_pipelineLayoutHashed                   = uint(hash<string>()(g_pipelineLayout));

	// Render pass building parameters
	const char* g_renderPassAttachmentFormat            = "attachmentFormat";
	const char* g_renderPassAttachmentSamplesPerPixel   = "attachmentSamplesPerPixel";
	const char* g_renderPassAttachmentFinalLayout       = "attachmentFinalLayout";
	const char* g_renderPassAttachmentColorReference    = "attachmentColorReference";
	const char* g_renderPassAttachmentDepthReference    = "attachmentDepthReference";
	const char* g_renderPassAttachmentPipelineBindPoint = "attachmentPipelineBindPoint";

	// Render pass building hashed parameters
	const uint g_renderPassAttachmentFormatHashed            = uint(hash<string>()(g_renderPassAttachmentFormat));
	const uint g_renderPassAttachmentSamplesPerPixelHashed   = uint(hash<string>()(g_renderPassAttachmentSamplesPerPixel));
	const uint g_renderPassAttachmentFinalLayoutHashed       = uint(hash<string>()(g_renderPassAttachmentFinalLayout));
	const uint g_renderPassAttachmentColorReferenceHashed    = uint(hash<string>()(g_renderPassAttachmentColorReference));
	const uint g_renderPassAttachmentDepthReferenceHashed    = uint(hash<string>()(g_renderPassAttachmentDepthReference));
	const uint g_renderPassAttachmentPipelineBindPointHashed = uint(hash<string>()(g_renderPassAttachmentPipelineBindPoint));

	// Manager template names
	const char* g_textureManager         = "textureManager";
	const char* g_bufferManager          = "bufferManager";
	const char* g_shaderManager          = "shaderManager";
	const char* g_framebufferManager     = "framebufferManager";
	const char* g_inputSingleton         = "inputSingleton";
	const char* g_sceneSingleton         = "sceneSingleton";
	const char* g_pipelineManager        = "pipelineManager";
	const char* g_uniformBufferManager   = "uniformBufferManager";
	const char* g_materialManager        = "materialManager";
	const char* g_rasterTechniqueManager = "rasterTechniqueManager";
	const char* g_renderPassManager      = "renderPassManager";
	const char* g_cameraManager          = "cameraManager";

	// Reflectance and normal material texture resource names
	const char* g_reflectanceTextureResourceName   = "reflectanceTextureResource";
	const char* g_normalTextureResourceName        = "normalTextureResource";

	// Reflectance and normal material texture resource hashed names
	const uint g_reflectanceTextureResourceNameHashed   = uint(hash<string>()(g_reflectanceTextureResourceName));
	const uint g_normalTextureResourceNameHashed        = uint(hash<string>()(g_normalTextureResourceName));

	// Lit cluster compute shader source code chunk
	const char* g_litClusterCodeChunk = "litClusterCodeChunk";

	// Lit cluster compute shader source code chunk hashed name
	const uint g_litClusterCodeChunkHashed = uint(hash<string>()(g_litClusterCodeChunk));

	// Lit cluster process results compute shader source code chunk
	const char* g_litClusterProcessResultsCodeChunk = "litClusterProcessResultsCodeChunk";

	// Lit cluster process results compute shader source code chunk hashed name
	const uint g_litClusterProcessResultsCodeChunkHashed = uint(hash<string>()(g_litClusterProcessResultsCodeChunk));

	// Light bounce voxel irradiance field compute shader source code chunk
	const char* g_lightBounceVoxelIrradianceCodeChunk = "lightBounceVoxelIrradianceCodeChunk";

	// Light bounce voxel irradiance field compute shader source code chunk hashed name
	const uint g_lightBounceVoxelIrradianceCodeChunkHashed = uint(hash<string>()(g_lightBounceVoxelIrradianceCodeChunk));

	// Light bounce voxel Gaussian filter compute shader source code chunk
	const char* g_lightBounceVoxelGaussianFilterCodeChunk = "lightBounceVoxelGaussianFilterCodeChunk";

	// Light bounce voxel Gaussian filter compute shader source code chunk hashed name
	const uint g_lightBounceVoxelGaussianFilterCodeChunkHashed = uint(hash<string>()(g_lightBounceVoxelGaussianFilterCodeChunk));

	// Reset clusterization irradiance data compute shader source code chunk
	const char* g_clusterizationClearIrradianceDataCodeChunk = "clusterizationClearIrradianceDataCodeChunk";

	// Reset clusterization irradiance data compute shader source code chunk hashed name
	const uint g_clusterizationClearIrradianceDataCodeChunkHashed = uint(hash<string>()(g_clusterizationClearIrradianceDataCodeChunk));

	// Build voxel shadow map geometry compute shader source code chunk
	const char* g_buildVoxelShadowMapGeometryCodeChunk = "buildVoxelShadowMapGeometryCodeChunk";

	// Build voxel shadow map geometry compute shader source code chunk hashed name
	const uint g_buildVoxelShadowMapGeometryCodeChunkHashed = uint(hash<string>()(g_buildVoxelShadowMapGeometryCodeChunk));

	// Voxel clusterization compute shader source code chunk
	const char* g_voxelClusterizationCodeChunk = "voxelClusterizationCodeChunk";

	// Voxel clusterization compute shader source code chunk hashed name
	const uint g_voxelClusterizationCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationCodeChunk));

	// Voxel clusterization new center compute shader source code chunk
	const char* g_voxelClusterizationNewCenterCodeChunk = "voxelClusterizationNewCenterCodeChunk";

	// Voxel clusterization new center compute shader source code chunk hashed name
	const uint g_voxelClusterizationNewCenterCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationNewCenterCodeChunk));

	// Voxel clusterization voxel distance initialization compute shader source code chunk
	const char* g_voxelClusterizationInitVoxelDistanceCodeChunk = "voxelClusterizationInitVoxelDistanceCodeChunk";

	// Voxel clusterization voxel distance initialization compute shadoer code chunk hashed name
	const uint g_voxelClusterizationInitVoxelDistanceCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationInitVoxelDistanceCodeChunk));

	// Voxel clusterization voxel add up compute shader source code chunk
	const char* g_voxelClusterizationAddUpCodeChunk = "voxelClusterizationAddUpCodeChunk";

	// Voxel clusterization voxel add up compute shadoer code chunk hashed name
	const uint g_voxelClusterizationAddUpCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationAddUpCodeChunk));

	// Voxel clusterization voxel compute AABB compute shader source code chunk
	const char* g_voxelClusterizationComputeAABBCodeChunk = "voxelClusterizationComputeAABBCodeChunk";

	// Voxel clusterization voxel compute AABB shader code chunk hashed name
	const uint g_voxelClusterizationComputeAABBCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationComputeAABBCodeChunk));

	// Voxel clusterization build final buffer compute shader source code chunk
	const char* g_voxelClusterizationBuildFinalBufferCodeChunk = "voxelClusterizationBuildFinalBufferCodeChunk";

	// Voxel clusterization build final buffer compute AABB shader code chunk hashed name
	const uint g_voxelClusterizationBuildFinalBufferCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationBuildFinalBufferCodeChunk));

	// Voxel clusterization compute neighbour compute shader source code chunk
	const char* g_voxelClusterizationComputeNeighbourCodeChunk = "voxelClusterizationComputeNeighbourCodeChunk";

	// Voxel clusterization compute neighbour compute shader code chunk hashed name
	const uint g_voxelClusterizationComputeNeighbourCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationComputeNeighbourCodeChunk));

	// Voxel clusterization merge clusters compute shader source code chunk
	const char* g_voxelClusterizationMergeClustersCodeChunk = "voxelClusterizationMergeClustersCodeChunk";

	// Voxel clusterization merge clusters compute shader code chunk hashed name
	const uint g_voxelClusterizationMergeClustersCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationMergeClustersCodeChunk));

	// Voxel clusterization preparation compute shader source code chunk
	const char* g_voxelClusterizationPrepareCodeChunk = "voxelClusterizationPrepareCodeChunk";

	// Voxel clusterization preparation compute shader source code chunk hashed name
	const uint g_voxelClusterizationPrepareCodeChunkHashed = uint(hash<string>()(g_voxelClusterizationPrepareCodeChunk));

	// Camera visible voxel compute shader source code chunk
	const char* g_cameraVisibleVoxelCodeChunk = "cameraVisibleVoxelCodeChunk";

	// Camera visible voxel compute shader source code chunk hashed
	const uint g_cameraVisibleVoxelCodeChunkHashed = uint(hash<string>()(g_cameraVisibleVoxelCodeChunk));

	// Distance shadow mapping distance texture name compute shader source code chunk
	const char* g_distanceShadowMapDistanceTextureCodeChunk = "distanceShadowMapDistanceTextureCodeChunk";

	// Distance shadow mapping distance texture name compute shader source code chunk hashed
	const uint g_distanceShadowMapDistanceTextureCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapDistanceTextureCodeChunk));

	// Distance shadow mapping offscreen texture name compute shader source code chunk
	const char* g_distanceShadowMapOffscreenTextureCodeChunk = "distanceShadowMapOffscreenTextureCodeChunk";

	// Distance shadow mapping offscreen texture name compute shader source code chunk hashed
	const uint g_distanceShadowMapOffscreenTextureCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapOffscreenTextureCodeChunk));

	// Distance shadow mapping material name compute shader source code chunk
	const char* g_distanceShadowMapMaterialNameCodeChunk = "distanceShadowMapMaterialNameCodeChunk";

	// Distance shadow mapping material name compute shader source code chunk hashed
	const uint g_distanceShadowMapMaterialNameCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapMaterialNameCodeChunk));

	// Distance shadow mapping framebuffer name compute shader source code chunk
	const char* g_distanceShadowMapFramebufferNameCodeChunk = "distanceShadowMapFramebufferNameCodeChunk";

	// Distance shadow mapping framebuffer name compute shader source code chunk hashed
	const uint g_distanceShadowMapFramebufferNameCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapFramebufferNameCodeChunk));

	// Distance shadow mapping camera name compute shader source code chunk
	const char* g_distanceShadowMapCameraNameCodeChunk = "distanceShadowMapCameraNameCodeChunk";

	// Distance shadow mapping camera name compute shader source code chunk hashed
	const uint g_distanceShadowMapCameraNameCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapCameraNameCodeChunk));

	// Distance shadow mapping use compacted geometry compute shader source code chunk
	const char* g_distanceShadowMapUseCompactedGeometryCodeChunk = "distanceShadowMapUseCompactedGeometryCodeChunk";

	// Distance shadow mapping use compacted geometry compute shader source code chunk hashed
	const uint g_distanceShadowMapUseCompactedGeometryCodeChunkHashed = uint(hash<string>()(g_distanceShadowMapUseCompactedGeometryCodeChunk));

	// Cluster visibility compute shader source code chunk
	const char* g_clusterVisibilityCodeChunk = "clusterVisibilityCodeChunk";

	// Cluster visibility compute shader source code chunk hashed name
	const uint g_clusterVisibilityCodeChunkHashed = uint(hash<string>()(g_clusterVisibilityCodeChunk));

	// Voxel face visible filtering compute shader source code chunk
	const char* g_voxelFacePenaltyCodeChunk = "voxelFacePenaltyCodeChunk";

	// Voxel face visible filtering compute shader source code chunk hashed name
	const uint g_voxelFacePenaltyCodeChunkHashed = uint(hash<string>()(g_voxelFacePenaltyCodeChunk));

	// Distance shadow mapping indirect command buffer name code chunk
	const char* g_indirectCommandBufferCodeChunk = "indirectCommandBufferCodeChunk";

	// Distance shadow mapping indirect command buffer name code chunk hashed
	const uint g_indirectCommandBufferCodeChunkHashed = uint(hash<string>()(g_indirectCommandBufferCodeChunk));

	// Compute frustum culling compute shader source code chunk
	const char* g_computeFrustumCullingCodeChunk = "computeFrustumCullingCodeChunk";

	// Compute frustum culling compute shader source code chunk hashed name
	const uint g_computeFrustumCullingCodeChunkHashed = uint(hash<string>()(g_computeFrustumCullingCodeChunk));

	// Material surface type
	const char* g_materialSurfaceType = "materialSurfaceType";

	// Material surface type hashed name
	const uint g_materialSurfaceTypeChunkHashed = uint(hash<string>()(g_materialSurfaceType));

	// Material distance shadw map source code chunk
	const char* g_distanceShadowMapUseInstancedRendering = "distanceShadowMapUseInstancedrendering";

	// Material distance shadw map source code chunk hashed
	const uint g_distanceShadowMapUseInstancedRenderingHashed = uint(hash<string>()(g_distanceShadowMapUseInstancedRendering));
}

/////////////////////////////////////////////////////////////////////////////////////////////
