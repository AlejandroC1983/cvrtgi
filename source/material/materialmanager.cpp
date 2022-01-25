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
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/material/materialcolortexture.h"
#include "../../include/material/materialscenevoxelization.h"
#include "../../include/material/materialbufferprefixsum.h"
#include "../../include/material/materiallighting.h"
#include "../../include/material/materialvoxelrasterinscenario.h"
#include "../../include/material/materialshadowmappingvoxel.h"
#include "../../include/material/materialclusterization.h"
#include "../../include/material/materialclusterizationprepare.h"
#include "../../include/material/materialclusterizationnewcenter.h"
#include "../../include/material/materialclusterizationinitvoxeldistance.h"
#include "../../include/material/materialclusterizationaddup.h"
#include "../../include/material/materialclusterizationinitaabb.h"
#include "../../include/material/materialclusterizationcomputeaabb.h"
#include "../../include/material/materialclusterizationbuildfinalbuffer.h"
#include "../../include/material/materialclusterizationcomputeneighbour.h"
#include "../../include/material/materialclusterizationmergeclusters.h"
#include "../../include/material/materialbuildvoxelshadowmapgeometry.h"
#include "../../include/material/materialresetclusterirradiancedata.h"
#include "../../include/material/materiallitcluster.h"
#include "../../include/material/materiallitclusterprocessresults.h"
#include "../../include/material/materiallightbouncevoxelirradiance.h"
#include "../../include/material/materialcameravisiblevoxel.h"
#include "../../include/material/materialdistanceshadowmapping.h"
#include "../../include/material/materiallightbouncevoxelgaussianfilter.h"
#include "../../include/material/materiallightbouncevoxelgaussianfiltersecond.h"
#include "../../include/material/materialantialiasing.h"
#include "../../include/material/materialclustervisibility.h"
#include "../../include/material/materialprefixsum.h"
#include "../../include/material/materialvoxelfacepenalty.h"
#include "../../include/material/materialcomputefrustumculling.h"
#include "../../include/material/materialindirectcolortexture.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/uniformbuffer/uniformbuffermanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

MaterialManager::MaterialManager()
{
	m_managerName = g_materialManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

MaterialManager::~MaterialManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Material* MaterialManager::buildMaterial(string&& className, string&& instanceName, MultiTypeUnorderedMap* attributeData)
{
	if (existsElement(move(instanceName)))
	{
		return getElement(move(instanceName));
	}

	// TODO: improve the automatization of this process...
	Material* material = nullptr;
	if (className == "MaterialColorTexture")
	{
		material = new MaterialColorTexture(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialColorTexture* materialCasted = static_cast<MaterialColorTexture*>(material);

			if (attributeData->elementExists(g_reflectanceTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_reflectanceTextureResourceNameHashed);
				materialCasted->setReflectanceTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_normalTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_normalTextureResourceNameHashed);
				materialCasted->setNormalTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_materialSurfaceTypeChunkHashed))
			{
				AttributeData<MaterialSurfaceType>* attribute = attributeData->getElement<AttributeData<MaterialSurfaceType>*>(g_materialSurfaceTypeChunkHashed);
				materialCasted->setMaterialSurfaceType(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialSceneVoxelization")
	{
		material = new MaterialSceneVoxelization(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialSceneVoxelization* materialCasted = static_cast<MaterialSceneVoxelization*>(material);

			if (attributeData->elementExists(g_reflectanceTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_reflectanceTextureResourceNameHashed);
				materialCasted->setReflectanceTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_normalTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_normalTextureResourceNameHashed);
				materialCasted->setNormalTextureName(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialBufferPrefixSum")
	{
		material = new MaterialBufferPrefixSum(move(string(instanceName)));
	}
	else if (className == "MaterialLighting")
	{
		material = new MaterialLighting(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLighting* materialCasted = static_cast<MaterialLighting*>(material);

			if (attributeData->elementExists(g_reflectanceTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_reflectanceTextureResourceNameHashed);
				materialCasted->setReflectanceTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_normalTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_normalTextureResourceNameHashed);
				materialCasted->setNormalTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_materialSurfaceTypeChunkHashed))
			{
				AttributeData<MaterialSurfaceType>* attribute = attributeData->getElement<AttributeData<MaterialSurfaceType>*>(g_materialSurfaceTypeChunkHashed);
				materialCasted->setMaterialSurfaceType(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialVoxelRasterInScenario")
	{
		material = new MaterialVoxelRasterInScenario(move(string(instanceName)));
	}
	else if (className == "MaterialShadowMappingVoxel")
	{
		material = new MaterialShadowMappingVoxel(move(string(instanceName)));
	}
	else if (className == "MaterialClusterization")
	{
		material = new MaterialClusterization(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterization* materialCasted = static_cast<MaterialClusterization*>(material);

			if (attributeData->elementExists(g_voxelClusterizationCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationPrepare")
	{
		material = new MaterialClusterizationPrepare(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationPrepare* materialCasted = static_cast<MaterialClusterizationPrepare*>(material);

			if (attributeData->elementExists(g_voxelClusterizationPrepareCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationPrepareCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationNewCenter")
	{
		material = new MaterialClusterizationNewCenter(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationNewCenter* materialCasted = static_cast<MaterialClusterizationNewCenter*>(material);

			if (attributeData->elementExists(g_voxelClusterizationNewCenterCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationNewCenterCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationInitVoxelDistance")
	{
		material = new MaterialClusterizationInitVoxelDistance(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationInitVoxelDistance* materialCasted = static_cast<MaterialClusterizationInitVoxelDistance*>(material);

			if (attributeData->elementExists(g_voxelClusterizationInitVoxelDistanceCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationInitVoxelDistanceCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationAddUp")
	{
		material = new MaterialClusterizationAddUp(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationAddUp* materialCasted = static_cast<MaterialClusterizationAddUp*>(material);

			if (attributeData->elementExists(g_voxelClusterizationAddUpCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationAddUpCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationInitAABB")
	{
		material = new MaterialClusterizationInitAABB(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationInitAABB* materialCasted = static_cast<MaterialClusterizationInitAABB*>(material);

			if (attributeData->elementExists(g_voxelClusterizationAddUpCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationAddUpCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationComputeAABB")
	{
		material = new MaterialClusterizationComputeAABB(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationComputeAABB* materialCasted = static_cast<MaterialClusterizationComputeAABB*>(material);

			if (attributeData->elementExists(g_voxelClusterizationComputeAABBCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationComputeAABBCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationBuildFinalBuffer")
	{
		material = new MaterialClusterizationBuildFinalBuffer(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationBuildFinalBuffer* materialCasted = static_cast<MaterialClusterizationBuildFinalBuffer*>(material);

			if (attributeData->elementExists(g_voxelClusterizationBuildFinalBufferCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationBuildFinalBufferCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationComputeNeighbour")
	{
		material = new MaterialClusterizationComputeNeighbour(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationComputeNeighbour* materialCasted = static_cast<MaterialClusterizationComputeNeighbour*>(material);

			if (attributeData->elementExists(g_voxelClusterizationComputeNeighbourCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationComputeNeighbourCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialClusterizationMergeClusters")
	{
		material = new MaterialClusterizationMergeClusters(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterizationMergeClusters* materialCasted = static_cast<MaterialClusterizationMergeClusters*>(material);

			if (attributeData->elementExists(g_voxelClusterizationMergeClustersCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelClusterizationMergeClustersCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialBuildVoxelShadowMapGeometry")
	{
		material = new MaterialBuildVoxelShadowMapGeometry(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialBuildVoxelShadowMapGeometry* materialCasted = static_cast<MaterialBuildVoxelShadowMapGeometry*>(material);

			if (attributeData->elementExists(g_buildVoxelShadowMapGeometryCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_buildVoxelShadowMapGeometryCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialResetClusterIrradianceData")
	{
		material = new MaterialResetClusterIrradianceData(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialResetClusterIrradianceData* materialCasted = static_cast<MaterialResetClusterIrradianceData*>(material);

			if (attributeData->elementExists(g_clusterizationClearIrradianceDataCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_clusterizationClearIrradianceDataCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialLitCluster")
	{
		material = new MaterialLitCluster(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLitCluster* materialCasted = static_cast<MaterialLitCluster*>(material);

			if (attributeData->elementExists(g_litClusterCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_litClusterCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialLitClusterProcessResults")
	{
		material = new MaterialLitClusterProcessResults(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLitClusterProcessResults* materialCasted = static_cast<MaterialLitClusterProcessResults*>(material);

			if (attributeData->elementExists(g_litClusterProcessResultsCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_litClusterProcessResultsCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialLightBounceVoxelIrradiance")
	{
		material = new MaterialLightBounceVoxelIrradiance(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLightBounceVoxelIrradiance* materialCasted = static_cast<MaterialLightBounceVoxelIrradiance*>(material);

			if (attributeData->elementExists(g_lightBounceVoxelIrradianceCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_lightBounceVoxelIrradianceCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialCameraVisibleVoxel")
	{
		material = new MaterialCameraVisibleVoxel(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialCameraVisibleVoxel* materialCasted = static_cast<MaterialCameraVisibleVoxel*>(material);

			if (attributeData->elementExists(g_cameraVisibleVoxelCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_cameraVisibleVoxelCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialDistanceShadowMapping")
	{
		material = new MaterialDistanceShadowMapping(move(string(instanceName)));

		if (attributeData->elementExists(g_distanceShadowMapUseInstancedRenderingHashed))
		{
			MaterialDistanceShadowMapping* materialCasted = static_cast<MaterialDistanceShadowMapping*>(material);

			AttributeData<bool>* attribute = attributeData->getElement<AttributeData<bool>*>(g_distanceShadowMapUseInstancedRenderingHashed);
			materialCasted->setUseInstancedRendering(attribute->m_data);
		}
	}
	else if (className == "MaterialLightBounceVoxelGaussianFilter")
	{
		material = new MaterialLightBounceVoxelGaussianFilter(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLightBounceVoxelGaussianFilter* materialCasted = static_cast<MaterialLightBounceVoxelGaussianFilter*>(material);

			if (attributeData->elementExists(g_lightBounceVoxelGaussianFilterCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_lightBounceVoxelGaussianFilterCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialLightBounceVoxelGaussianFilterSecond")
	{
		material = new MaterialLightBounceVoxelGaussianFilterSecond(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialLightBounceVoxelGaussianFilterSecond* materialCasted = static_cast<MaterialLightBounceVoxelGaussianFilterSecond*>(material);

			if (attributeData->elementExists(g_lightBounceVoxelGaussianFilterCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_lightBounceVoxelGaussianFilterCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialAntialiasing")
	{
		material = new MaterialAntialiasing(move(string(instanceName)));
	}
	else if (className == "MaterialClusterVisibility")
	{
		material = new MaterialClusterVisibility(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterVisibility* materialCasted = static_cast<MaterialClusterVisibility*>(material);

			if (attributeData->elementExists(g_clusterVisibilityCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_clusterVisibilityCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialPrefixSum")
	{
		material = new MaterialPrefixSum(move(string(instanceName)));
	}
	else if (className == "MaterialComputeFrustumCulling")
	{
		material = new MaterialComputeFrustumCulling(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialClusterVisibility* materialCasted = static_cast<MaterialClusterVisibility*>(material);

			if (attributeData->elementExists(g_computeFrustumCullingCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_computeFrustumCullingCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialIndirectColorTexture")
	{
		material = new MaterialIndirectColorTexture(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialIndirectColorTexture* materialCasted = static_cast<MaterialIndirectColorTexture*>(material);

			if (attributeData->elementExists(g_reflectanceTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_reflectanceTextureResourceNameHashed);
				materialCasted->setReflectanceTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_normalTextureResourceNameHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_normalTextureResourceNameHashed);
				materialCasted->setNormalTextureName(attribute->m_data);
			}

			if (attributeData->elementExists(g_materialSurfaceTypeChunkHashed))
			{
				AttributeData<MaterialSurfaceType>* attribute = attributeData->getElement<AttributeData<MaterialSurfaceType>*>(g_materialSurfaceTypeChunkHashed);
				materialCasted->setMaterialSurfaceType(attribute->m_data);
			}
		}
	}
	else if (className == "MaterialVoxelFacePenalty")
	{
		material = new MaterialVoxelFacePenalty(move(string(instanceName)));

		if (attributeData != nullptr)
		{
			MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(material);

			if (attributeData->elementExists(g_voxelFacePenaltyCodeChunkHashed))
			{
				AttributeData<string>* attribute = attributeData->getElement<AttributeData<string>*>(g_voxelFacePenaltyCodeChunkHashed);
				materialCasted->setComputeShaderThreadMapping(attribute->m_data);
			}
		}
	}
	
	addElement(move(string(instanceName)), material);
	material->m_name = move(instanceName);
	material->m_ready = true;

	material->init();

	return material;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::assignSlots()
{
	shaderM->refElementSignal().connect<MaterialManager, &MaterialManager::slotElement>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType)
{
	if (!gpuPipelineM->getPipelineInitialized())
	{
		return;
	}

	vector<Material*> vectorMaterial;

	if (managerName == g_shaderManager)
	{
		Material* material;
		map<string, Material*>::iterator it = m_mapElement.begin();
		for (it; it != m_mapElement.end(); ++it)
		{
			material = it->second;
			if (material->shaderResourceNotification(move(string(elementName)), notificationType))
			{
				vectorMaterial.push_back(material);
			}
		}
	}

	if (gpuPipelineM->getPipelineInitialized())
	{
		forIT(vectorMaterial)
		{
			emitSignalElement(move(string((*it)->getName())), notificationType);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::buildMaterialUniformBuffer()
{
	int maxSize = -1;
	int counter = 0;
	forIT(m_mapElement)
	{
		if ((*it).second->getExposedStructFieldSize() > maxSize)
		{
			maxSize = (*it).second->getExposedStructFieldSize();
		}

		(*it).second->setMaterialUniformBufferIndex(counter);
		counter++;
	}

	m_materialUniformData         = uniformBufferM->buildUniformBuffer(move(string("materialUniformBuffer")), maxSize, int(m_mapElement.size()));
	m_materialUBDynamicAllignment = uint(m_materialUniformData->getDynamicAllignment());

	forIT(m_mapElement)
	{
		updateGPUBufferMaterialData((*it).second);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::updateGPUBufferMaterialData(Material* materialToUpdate)
{
	void *data;
	uint32_t materialOffset = uint32_t(materialToUpdate->getMaterialUniformBufferIndex() * m_materialUBDynamicAllignment);
	VkResult result         = vkMapMemory(coreM->getLogicalDevice(), m_materialUniformData->refBufferInstance()->getMemory(), materialOffset, m_materialUBDynamicAllignment, 0, &data);

	assert(result == VK_SUCCESS);

	uint8_t* cpuBufferSourceData = static_cast<uint8_t*>(m_materialUniformData->refCPUBuffer().refUBHostMemory());
	cpuBufferSourceData         += materialOffset;
	memcpy(data, cpuBufferSourceData, m_materialUBDynamicAllignment);

	vkUnmapMemory(coreM->getLogicalDevice(), m_materialUniformData->refBufferInstance()->getMemory());
}

/////////////////////////////////////////////////////////////////////////////////////////////
