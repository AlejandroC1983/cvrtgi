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
#include "../../include/util/loopmacrodefines.h"
#include "../../include/util/factorytemplate.h"
#include "../../include/rastertechnique/rastertechniquefactory.h"
#include "../../include/rastertechnique/rastertechnique.h"
#include "../../include/rastertechnique/scenerastercolortexturetechnique.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/rastertechnique/scenelightingtechnique.h"
#include "../../include/rastertechnique/voxelrasterinscenariotechnique.h"
#include "../../include/rastertechnique/shadowmappingvoxeltechnique.h"
#include "../../include/rastertechnique/bufferprocesstechnique.h"
#include "../../include/rastertechnique/clusterizationpreparetechnique.h"
#include "../../include/rastertechnique/clusterizationtechnique.h"
#include "../../include/rastertechnique/clusterizationpreparetechnique.h"
#include "../../include/rastertechnique/clusterizationinitaabbtechnique.h"
#include "../../include/rastertechnique/clusterizationcomputeaabbtechnique.h"
#include "../../include/rastertechnique/clusterizationbuildfinalbuffertechnique.h"
#include "../../include/rastertechnique/clusterizationcomputeneighbourtechnique.h"
#include "../../include/rastertechnique/clusterizationmergeclustertechnique.h"
#include "../../include/rastertechnique/buildvoxelshadowmapgeometrytechnique.h"
#include "../../include/rastertechnique/litclustertechnique.h"
#include "../../include/rastertechnique/lightbouncevoxelirradiancetechnique.h"
#include "../../include/rastertechnique/cameravisiblevoxeltechnique.h"
#include "../../include/rastertechnique/distanceshadowmappingtechnique.h"
#include "../../include/rastertechnique/antialiasingtechnique.h"
#include "../../include/rastertechnique/clustervisibilitytechnique.h"
#include "../../include/rastertechnique/clustervisibleprefixsumtechnique.h"
#include "../../include/rastertechnique/computefrustumcullingtechnique.h"
#include "../../include/rastertechnique/sceneindirectdrawtechnique.h"
#include "../../include/rastertechnique/voxelfacepenaltytechnique.h"

// DEFINES

// NAMESPACE

// STATIC MEMBER INITIALIZATION
map<string, ObjectFactory<RasterTechnique>*> FactoryTemplate<RasterTechnique>::m_mapFactory = []() { map<string, ObjectFactory<RasterTechnique>*> temporal; return temporal; } ();
// C++ guarantees static variables inside a compilation unit (.cpp) are initialized in order of declaration, so registering the types
// below will find the static map already initialized
REGISTER_TYPE(RasterTechnique, RasterTechnique)
REGISTER_TYPE(RasterTechnique, SceneRasterColorTextureTechnique)
REGISTER_TYPE(RasterTechnique, SceneVoxelizationTechnique)
REGISTER_TYPE(RasterTechnique, BufferPrefixSumTechnique)
REGISTER_TYPE(RasterTechnique, SceneLightingTechnique)
REGISTER_TYPE(RasterTechnique, VoxelRasterInScenarioTechnique)
REGISTER_TYPE(RasterTechnique, ShadowMappingVoxelTechnique)
REGISTER_TYPE(RasterTechnique, BufferProcessTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationPrepareTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationInitAABBTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationComputeAABBTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationBuildFinalBufferTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationComputeNeighbourTechnique)
REGISTER_TYPE(RasterTechnique, ClusterizationMergeClusterTechnique)
REGISTER_TYPE(RasterTechnique, BuildVoxelShadowMapGeometryTechnique)
REGISTER_TYPE(RasterTechnique, LitClusterTechnique)
REGISTER_TYPE(RasterTechnique, LightBounceVoxelIrradianceTechnique)
REGISTER_TYPE(RasterTechnique, CameraVisibleVoxelTechnique)
REGISTER_TYPE(RasterTechnique, DistanceShadowMappingTechnique)
REGISTER_TYPE(RasterTechnique, AntialiasingTechnique)
REGISTER_TYPE(RasterTechnique, ClusterVisibilityTechnique)
REGISTER_TYPE(RasterTechnique, ClusterVisiblePrefixSumTechnique)
REGISTER_TYPE(RasterTechnique, VoxelFacePenaltyTechnique)
REGISTER_TYPE(RasterTechnique, ComputeFrustumCullingTechnique)
REGISTER_TYPE(RasterTechnique, SceneIndirectDrawTechnique)

/////////////////////////////////////////////////////////////////////////////////////////////
