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

#ifndef _BUFFERVERIFICATIONHELPER_H_
#define _BUFFERVERIFICATIONHELPER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** This is a helper class to verify the information contained in the buffers generated
* during the voxelization and prefix sum post porcess of the scene */
class BufferVerificationHelper
{
public:

	/** Method to know if the voxel with hashed position given by indexHashed is empty or occupied
	* @param indexHashed                [in] hashed index of the 3D position to test
	* @param pVectorVoxelOccupiedBuffer [in] pointer to the buffer containing the occupied information at a bit level
	* @return rtrue if occupied, false otherwise */
	static bool isVoxelOccupied(uint indexHashed, uint* pVectorVoxelOccupiedBuffer);

	/** Output voxelOccupiedBuffer buffer
	* @return nothing */
	static void outputVoxelOccupiedBuffer();

	/** Verify voxelization generated fragment data and linked lists
	* @return nothing */
	static void verifyFragmentData();

	/** Method to know if the voxel at bit given as parameter in the uint representing a set
	* of 32 occupied voxel information is occupied or empty
	* @param value [in] value to test if it's occupied
	* @param bit   [in] bit to test if occupied
	* @return true if occupied, false otherwise
	* @return nothing */
	static bool getVoxelOccupied(uint value, uint bit);

	/** Method to debug the contents of m_voxelOccupiedBuffer buffer
	* @return nothing */
	static void verifyVoxelOccupiedBuffer();

	/** Will copy to CPU the buffer generated to store all the temporary accumulated values in m_prefixSumPlanarBuffer
	* and verify the sum is correct for each level */
	static void verifyPrefixSumData();

	/** Computes the hashed coordinates for the position given as parameter for the voxelization size given as parameter
	* @param texcoord  [in] texture coordinates
	* @param voxelSize [in] voxelization volume size
	* @return hashed value */
	static uint getHashedIndex(uvec3 texcoord, uint voxelSize);

	/** Computes the unhashed 3D coordinates for the position given with hashed value as parameter, for a uniform
	* voxelization volume given by voxelizationWidth
	* @param value             [in] value to unhash
	* @param voxelizationWidth [in] width of the voxelization volume the value parameter has been hashed
	* @return unhashed value for parameter value */
	static uvec3 unhashValue(uint value, uint voxelizationWidth);

	/** Count the number elements in voxelFirstIndexBuffer with value != max uint value, and compare the result with
	* the results of the prefix sum algorithm for all levels in the reduction phase of the algorithm
	* @return nothing */
	static void verifyVoxelFirstIndexBuffer();

	/** Verify the contents in voxelFirstIndexCompactedBuffer match the ones in voxelFirstIndexBuffer:
	* for each element in voxelFirstIndexBuffer with value != max uint value, the corresponding acculumated index
	* in voxelFirstIndexCompactedBuffer has to match the information
	* @return nothing */
	static void verifyVoxelFirstIndexAndVoxelFirstIndexCompacted();

	/** Write to file the information present in voxelHashedPositionCompactedBuffer unhashing the indices found
	* in this buffer
	* @return nothing */
	static void outputHashedPositionCompactedBufferInfo();

	/** Output the voxelHashedPositionCompactedBuffer buffer
	* @return nothing */
	static void outputIndirectionBuffer();

	/** Output the data from IndirectionIndexBuffer and IndirectionRankBuffer
	* @return nothing */
	static void outputIndirectionBufferData();

	/** Verify information at IndirectionIndexBuffer and IndirectionRankBuffer buffers
	* @return nothing */
	static void verifyIndirectionBuffers();

	/** Copy to CPU the buffer with the initial first index information given by m_voxelFirstIndexBuffer and
	* the compacted information from m_voxelFirstIndexCompactedBuffer, and verify the information has been properly compacted
	* by the parallel prefix sum algorithm
	* @return nothing */
	static void verifyVoxelizationProcessData();

	/** Debug method to verify the result of the summed area texture
	* @param initialTexture [in] texture wityh the initial data used by the GPU to generate the texture given by finalTexture
	* @param finalTexture   [in] summed-area texture result
	* @return nothing */
	static void verifySummedAreaResult(Texture* initialTexture, Texture* finalTexture);

	/** Compute the summed-area of the surface represented by the vecData parameter
	* @param vecData [in] vector representing the summed-area data
	* @param maxStep [in] max number of steps to compute the summed-area result, -1 means all steps
	* @return summed-area result */
	static vectorVectorFloat computeSummedArea(vectorVectorFloat& vecData, int maxStep);

	/** Verify the values in vecData0 and vecData1 are the same with a numerical precission error less than eps
	* @param vecData0 [in] one of the two vectors of data to compare
	* @param vecData1 [in] one of the two vectors of data to compare
	* @return true if values are equal (taking into account the eps precission value as maximum error threshold, false otherwise */
	static bool verifyAreEqual(const vectorVectorFloat& vecData0, const vectorVectorFloat& vecData1, float eps);

	/** Will verify the add-up of all values present in the texture given as parameter equal to the value parameter
	* @param texture   [in] texture to verify
	* @param value     [in] value to compare with the add-up of all values in the texture parameter
	* @param threshold [in] error threshold
	* @return true if add-up value and float are equal with a maximum error given by threshold, false otherwise */
	static bool verifyAddUpTextureValue(Texture* texture, float value, float threshold);

	/** Returns the world coordinates of the center of the voxel of texture coordinates given by coordinates in a
	* voxelization of size voxelSize
	* @param coordinates [in] integer voxel texture coordinates
	* @param voxelSize   [in] size of the voxelization texture (in float)
	* @param sceneExtent [in] scene extent
	* @param sceneMin    [in] scene aabb min value
	* @return voxel world coordinates from coordinates parameter */
	static vec3 voxelSpaceToWorld(uvec3 coordinates, vec3 voxelSize, vec3 sceneExtent, vec3 sceneMin);

	/** Returns the voxel coordinates of the world coordinates giv3en as parameter
	* @param coordinates [in] world space coordinates
	* @param voxelSize   [in] size of the voxelization texture (in float)
	* @param sceneExtent [in] scene extent
	* @param sceneMin    [in] scene aabb min value
	* @return texture space coordinates of the world coordinates given as parameter */
	static uvec3 worldToVoxelSpace(vec3 coordinates, vec3 voxelSize, vec3 sceneExtent, vec3 sceneMin);

	/** Utility method to know if the intervals (minX0, maxX0) and (minX1, maxX1) do intersect
	* @param minX0 [in] minimum value for the first interval
	* @param maxX0 [in] maximum value for the first interval
	* @param minX1 [in] minimum value for the second interval
	* @param maxX1 [in] maximum value for the second interval
	* @return true if there's an intersection, false otherwise */
	static bool BufferVerificationHelper::intervalIntersection(int minX0, int maxX0, int minX1, int maxX1);

	/** Verify the cluster information in the final cluster data buffer, clusterizationFinalBuffer, comparing it with the data
	* in voxelClusterOwnerIndexBuffer and voxelHashedPositionCompactedBuffer
	* @return nothing */
	static void verifyClusterFinalDataBuffer();

	/** Count in voxelClusterOwnerIndexBuffer those voxels that have no cluster owner
	* @return nothing */
	static void countNumberVoxelWithNoClusterOwner();

	/** Find in clusterizationFinalBuffer those voxels that can be merged into bigger ones
	* @param maxVoxelCluster [in] maximum number of voxels a cluster has to be considered as a cluster that can be merged into one of its neighbours
	* @return nothing */
	static void findMergeCandidateCluster(int maxNumberVoxel);

	/** Take the information from litTestClusterBuffer set in LitClusterTechnique and the information from
	* litClusterCounterBuffer and litVisibleClusterBuffer set in LitClusterProcessResultsTechnique and verify
	* the number and indices of lit cluster are the same
	* @return nothing */
	static void verifyLitClusterInformation();

	/** Copy to CPU the buffer with the initial cluster visibility information given by clusterVisibilityBuffer and
	* the compacted information from clusterVisibilityCompactedBuffer, and verify the information has been properly compacted
	* by the parallel prefix sum algorithm
	* @return nothing */
	static void verifyClusterVisibilityCompactedData();

	/** Copy to CPU the buffer with the amount of visible clusters from each voxel face, clusterVisibilityDebugBuffer
	* to verify the indices used for mapping are properly computed
	* @return nothing */
	static void verifyClusterVisibilityIndices();

	/** Verify the clusterVisibilityFirstIndexBuffer which has the compacted data of the starting index of all
	* visible clusters from each voxel face.
	* @return nothing */
	static void verifyClusterVisibilityFirstIndexBuffer();

	static uint m_accumulatedReductionLevelBase; //!< Debug variable to know the accumulated value of non null elements at base level of the algorithm during the reduction step
	static uint m_accumulatedReductionLevel0;    //!< Debug variable to know the accumulated value of non null elements at level 0 of the algorithm during the reduction step
	static uint m_accumulatedReductionLevel1;    //!< Debug variable to know the accumulated value of non null elements at level 1 of the algorithm during the reduction step
	static uint m_accumulatedReductionLevel2;    //!< Debug variable to know the accumulated value of non null elements at level 2 of the algorithm during the reduction step
	static uint m_accumulatedReductionLevel3;    //!< Debug variable to know the accumulated value of non null elements at level 3 of the algorithm during the reduction step
	static bool m_outputAllInformationConsole;   //!< If true, all detailed information of all tests will be written to console
	static bool m_outputAllInformationFile;      //!< If true, all detailed information of all tests will be written to file
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BUFFERVERIFICATIONHELPER_H_
