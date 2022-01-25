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
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/rastertechnique/clusterizationinitaabbtechnique.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION
uint BufferVerificationHelper::m_accumulatedReductionLevelBase = 0;
uint BufferVerificationHelper::m_accumulatedReductionLevel0    = 0;
uint BufferVerificationHelper::m_accumulatedReductionLevel1    = 0;
uint BufferVerificationHelper::m_accumulatedReductionLevel2    = 0;
uint BufferVerificationHelper::m_accumulatedReductionLevel3    = 0;
bool BufferVerificationHelper::m_outputAllInformationConsole   = true;
bool BufferVerificationHelper::m_outputAllInformationFile      = true;

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferVerificationHelper::isVoxelOccupied(uint indexHashed, uint* pVectorVoxelOccupiedBuffer)
{
	float integerPart;
	float indexDecimalFloat = float(indexHashed) / 32.0f;
	float fractional        = glm::modf(indexDecimalFloat, integerPart);
	uint index              = uint(integerPart);
	uint bit                = uint(fractional * 32.0);
	uint value              = pVectorVoxelOccupiedBuffer[index];
	bool result             = ((value & 1 << bit) > 0);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::outputVoxelOccupiedBuffer()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Voxel occupied buffer content" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Voxel occupied buffer content" << endl;
	}

	vectorUint8 vectorVoxelOccupied;
	Buffer* voxelOccupiedBuffer = bufferM->getElement(move(string("voxelOccupiedBuffer")));
	voxelOccupiedBuffer->getContentCopy(vectorVoxelOccupied);
	uint numVoxelOccupied       = uint(voxelOccupiedBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelOccupied        = (uint*)(vectorVoxelOccupied.data());
	uint numFlagElements        = 0;

	forI(numVoxelOccupied)
	{
		uint vectorData = pVoxelOccupied[i];

		if (vectorData > 0)
		{
			if (m_outputAllInformationConsole)
			{
				cout << "Occupied data for index " << i << endl;
			}
			if (m_outputAllInformationFile)
			{
				outFile << "Occupied data for index " << i << endl;
			}
			forJ(32)
			{
				uint value    = 1 << j;
				uint bitIndex = i * 32 + j;
				bool result   = ((vectorData & value) > 0);

				if (m_outputAllInformationConsole)
				{
					cout << "\t bit number " << j << " has occupation flag " << result << endl;
				}
				if (m_outputAllInformationFile)
				{
					outFile << "\t bit number " << j << " has occupation flag " << result << endl;
				}

				if (result)
				{
					numFlagElements++;
				}
			}
		}
	}

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint firstIndexOccupiedElement = technique->getFirstIndexOccupiedElement();

	cout << ">>>>> Number of voxel occupied from occupied voxel is " << numFlagElements << endl;
	cout << ">>>>> According to prefix sum, the number of occupied voxels is " << firstIndexOccupiedElement << endl;
	if (m_outputAllInformationFile)
	{
		outFile << ">>>>> Number of voxel occupied from occupied voxel is " << numFlagElements << endl;
		outFile << ">>>>> According to prefix sum, the number of occupied voxels is " << firstIndexOccupiedElement << endl;
	}

	assert(numFlagElements == firstIndexOccupiedElement);

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyFragmentData()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	// Get atomic counter value in the second pass
	uint temp;
	Buffer* fragmentCounterBuffer = bufferM->getElement(move(string("fragmentCounterBuffer")));
	fragmentCounterBuffer->getContent((void*)(&temp));

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Fragment data" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Fragment data" << endl;
	}

	cout << "Value of fragmentCounterBuffer is " << temp << endl;
	if (m_outputAllInformationFile)
	{
		outFile << "Value of fragmentCounterBuffer is " << temp << endl;
	}

	// Information from the voxel first index buffer
	vectorUint8 vectorVoxelFirstIndex;
	Buffer* voxelFirstIndexBuffer = bufferM->getElement(move(string("voxelFirstIndexBuffer")));
	voxelFirstIndexBuffer->getContentCopy(vectorVoxelFirstIndex);
	uint numVoxelFirstIndex       = uint(voxelFirstIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelFirstIndex        = (uint*)(vectorVoxelFirstIndex.data());

	// Information from the fragment index buffer
	vectorUint8 vectorNextFragmentIndex;
	Buffer* nextFragmentIndexBuffer = bufferM->getElement(move(string("nextFragmentIndexBuffer")));
	nextFragmentIndexBuffer->getContentCopy(vectorNextFragmentIndex);
	uint numNextFragmentIndex       = uint(nextFragmentIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pNextFragmentIndex        = (uint*)(vectorNextFragmentIndex.data());

	// Information from the voxel occupied buffer
	vectorUint8 vectorVoxelOccupied;
	Buffer* voxelOccupiedBuffer = bufferM->getElement(move(string("voxelOccupiedBuffer")));
	voxelOccupiedBuffer->getContentCopy(vectorVoxelOccupied);
	uint numVoxelOccupied       = uint(voxelOccupiedBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelOccupied        = (uint*)(vectorVoxelOccupied.data());

	vector<uint> vectorAlreadySearched;
	uint numFragmentProcessed         = 0;
	uint numFragmentsAboveOnePerVoxel = 0;
	uint numFirstElementProcessed     = 0;

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint voxelizationWidth = technique->getVoxelizationWidth();

	// Verify the linked lists have been properly built:
	// + For each hashed voxel index i in pHashedFragmentIndex, retrieve at the hashed index the index of the
	//   first element of the linked list of fragments generated at that 3D position of the voxelization volume
	// + Follow the values of all fragments generated at position given by hashed value, starting from
	//   pNextFragmentIndex[j], when uint32 max -1 value is found, that is the last fragment
	/*forI(numHashedFragmentIndex)
	{
		uint hashedPosition = 0; // PENDING: RETRIEVE EACH FRAGMENT 3D TEXTURE COORDINATES FROM THE WORLD POSITION
		bool isOcuppied     = isVoxelOccupied(hashedPosition, pVoxelOccupied);

		if (!isOcuppied)
		{
			if (m_outputAllInformationConsole)
			{
				cout << "ERROR: non-flagged bit in occupied buffer" << endl;
			}
			if (m_outputAllInformationFile)
			{
				outFile << "ERROR: non-flagged bit in occupied buffer" << endl;
			}
		}

		assert(isOcuppied == true);

		if (find(vectorAlreadySearched.begin(), vectorAlreadySearched.end(), hashedPosition) == vectorAlreadySearched.end())
		{
			uvec3 unhashed = unhashValue(hashedPosition, voxelizationWidth);
			if (m_outputAllInformationConsole)
			{
				cout << "Analizing data for first fragment processed " << numFirstElementProcessed << " with hashed position " << hashedPosition << ", in 3D = (" << unhashed.x << "," << unhashed.y << "," << unhashed.z << "), isOcuppied=" << isOcuppied << endl;
			}
			if (m_outputAllInformationFile)
			{
				outFile << "Analizing data for first fragment processed " << numFirstElementProcessed << " with hashed position " << hashedPosition << ", in 3D = (" << unhashed.x << "," << unhashed.y << "," << unhashed.z << "), isOcuppied=" << isOcuppied << endl;
			}

			numFirstElementProcessed++;

			vectorAlreadySearched.push_back(hashedPosition);
			uint firstIndex  = pVoxelFirstIndex[hashedPosition];
			int currentIndex = int(pNextFragmentIndex[firstIndex]);
			uint counter = 0;

			numFragmentProcessed++;
			if (m_outputAllInformationConsole)
			{
				cout << "\t Linked list of fragments for this position:" << endl;
				cout << "\t\t Index number " << counter << " is " << int(firstIndex) << endl;
			}
			if (m_outputAllInformationFile)
			{
				outFile << "\t Linked list of fragments for this position:" << endl;
				outFile << "\t\t Index number " << counter << " is " << int(firstIndex) << endl;
			}

			while (currentIndex != -1)
			{
				counter++;
				if (m_outputAllInformationConsole)
				{
					cout << "\t\t Index number " << counter << " is " << currentIndex << endl;
				}
				if (m_outputAllInformationFile)
				{
					outFile << "\t\t Index number " << counter << " is " << currentIndex << endl;
				}
				currentIndex = int(pNextFragmentIndex[currentIndex]);
				numFragmentProcessed++;
				numFragmentsAboveOnePerVoxel++;
			}
		}
	}

	cout << ">>>>> Number of processed fragments is " << numFragmentProcessed << endl;
	cout << ">>>>> Number of fragments above the first in filled voxels is " << numFragmentsAboveOnePerVoxel << endl;
	if (m_outputAllInformationFile)
	{
		outFile << ">>>>> Number of processed fragments is " << numFragmentProcessed << endl;
		outFile << ">>>>> Number of fragments above the first in filled voxels is " << numFragmentsAboveOnePerVoxel << endl;
	}

	if (numFragmentProcessed != numHashedFragmentIndex)
	{
		cout << "ERROR: not matching the number of processed fragment" << endl;
		if (m_outputAllInformationFile)
		{
			outFile << "ERROR: not matching the number of processed fragment" << endl;
		}
	}

	assert((numFragmentProcessed == numHashedFragmentIndex));

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		outFile << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}*/
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferVerificationHelper::getVoxelOccupied(uint value, uint bit)
{
	uint bitShift = (1 << bit);
	bool result = ((value & (1 << bit)) > 0);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyVoxelOccupiedBuffer()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::trunc);
	}

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Fragment data" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Fragment data" << endl;
	}

	vectorUint8 vectorVoxelOccupied;
	Buffer* voxelOccupiedBuffer = bufferM->getElement(move(string("voxelOccupiedBuffer")));
	voxelOccupiedBuffer->getContentCopy(vectorVoxelOccupied);
	uint numVoxelOccupied       = uint(voxelOccupiedBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelOccupied        = (uint*)(vectorVoxelOccupied.data());

	vectorUint8 vectorVoxelFirstIndex;
	Buffer* voxelFirstIndexBuffer = bufferM->getElement(move(string("voxelFirstIndexBuffer")));
	voxelFirstIndexBuffer->getContentCopy(vectorVoxelFirstIndex);
	uint numVoxelFirstIndex       = uint(voxelFirstIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelFirstIndex        = (uint*)(vectorVoxelFirstIndex.data());

	bool anyError                       = false;
	uint numShouldBeOccupiedAndAreEmpty = 0;

	forI(numVoxelFirstIndex)
	{
		uint hashedValue = pVoxelFirstIndex[i];

		if (hashedValue != 4294967295)
		{
			float integerPart;
			float indexDecimalFloat = float(i) / 32.0f;
			float fractional        = glm::modf(indexDecimalFloat, integerPart);
			uint index              = uint(integerPart);
			uint bit                = uint(fractional * 32.0);
			bool result             = getVoxelOccupied(pVoxelOccupied[index], bit);

			if (!result)
			{
				if (m_outputAllInformationConsole)
				{
					cout << "ERROR: occupied flag not set at index " << i << " in BufferVerificationHelper::verifyVoxelOccupiedBuffer" << endl;
				}
				if (m_outputAllInformationFile)
				{
					outFile << "ERROR: occupied flag not set at index " << i << " in BufferVerificationHelper::verifyVoxelOccupiedBuffer" << endl;
				}
				numShouldBeOccupiedAndAreEmpty++;
				anyError = true;
			}
		}
	}

	cout << ">>>>> The number of elements that are empty and should be occupied is " << numShouldBeOccupiedAndAreEmpty << endl;
	if (m_outputAllInformationFile)
	{
		outFile << ">>>>> The number of elements that are empty and should be occupied is " << numShouldBeOccupiedAndAreEmpty << endl;
	}
	assert(anyError == false);

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyPrefixSumData()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	Buffer* prefixSumPlanarBuffer = bufferM->getElement(move(string("prefixSumPlanarBuffer")));
	vectorUint8 vectorPrefixSumPlanarBuffer;
	prefixSumPlanarBuffer->getContentCopy(vectorPrefixSumPlanarBuffer);
	uint numPrefixSumPlanar       = uint(prefixSumPlanarBuffer->getDataSize()) / sizeof(uint);
	uint* pPrefixSumPlanar        = (uint*)(vectorPrefixSumPlanarBuffer.data());

	m_accumulatedReductionLevelBase = 0;
	m_accumulatedReductionLevel0    = 0;
	m_accumulatedReductionLevel1    = 0;
	m_accumulatedReductionLevel2    = 0;
	m_accumulatedReductionLevel3    = 0;

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	vectorUint vectorPrefixSumNumElement = technique->getVectorPrefixSumNumElement();

	forI(vectorPrefixSumNumElement[0])
	{
		m_accumulatedReductionLevelBase += pPrefixSumPlanar[i];
	}

	forI(vectorPrefixSumNumElement[1])
	{
		m_accumulatedReductionLevel0 += pPrefixSumPlanar[i + vectorPrefixSumNumElement[0]];
	}

	forI(vectorPrefixSumNumElement[2])
	{
		m_accumulatedReductionLevel1 += pPrefixSumPlanar[i + vectorPrefixSumNumElement[0] + vectorPrefixSumNumElement[1]];
	}

	forI(vectorPrefixSumNumElement[3])
	{
		m_accumulatedReductionLevel2 += pPrefixSumPlanar[i + vectorPrefixSumNumElement[0] + vectorPrefixSumNumElement[1] + vectorPrefixSumNumElement[2]];
	}

	forI(vectorPrefixSumNumElement[4])
	{
		m_accumulatedReductionLevel3 += pPrefixSumPlanar[i + vectorPrefixSumNumElement[0] + vectorPrefixSumNumElement[1] + vectorPrefixSumNumElement[2] + vectorPrefixSumNumElement[3]];
	}

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Prefix sum data" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Prefix sum data" << endl;
	}

	cout << ">>>>> Accumulated per-level verification value for accumulatedValueLevelBase is " << m_accumulatedReductionLevelBase << endl;
	cout << ">>>>> Accumulated per-level verification value for accumulatedValueLevel0 is " << m_accumulatedReductionLevel0 << endl;
	cout << ">>>>> Accumulated per-level verification value for accumulatedValueLevel1 is " << m_accumulatedReductionLevel1 << endl;
	cout << ">>>>> Accumulated per-level verification value for accumulatedValueLevel2 is " << m_accumulatedReductionLevel2 << endl;
	cout << ">>>>> Accumulated per-level verification value for accumulatedValueLevel3 is " << m_accumulatedReductionLevel3 << endl;
	if (m_outputAllInformationFile)
	{
		outFile << ">>>>> Accumulated per-level verification value for accumulatedValueLevelBase is " << m_accumulatedReductionLevelBase << endl;
		outFile << ">>>>> Accumulated per-level verification value for accumulatedValueLevel0 is " << m_accumulatedReductionLevel0 << endl;
		outFile << ">>>>> Accumulated per-level verification value for accumulatedValueLevel1 is " << m_accumulatedReductionLevel1 << endl;
		outFile << ">>>>> Accumulated per-level verification value for accumulatedValueLevel2 is " << m_accumulatedReductionLevel2 << endl;
		outFile << ">>>>> Accumulated per-level verification value for accumulatedValueLevel3 is " << m_accumulatedReductionLevel3 << endl;
	}

	vectorUint vectorAccumulated =
	{
		m_accumulatedReductionLevelBase,
		m_accumulatedReductionLevel0,
		m_accumulatedReductionLevel1,
		m_accumulatedReductionLevel2,
		m_accumulatedReductionLevel3
	};

	sort(vectorAccumulated.begin(), vectorAccumulated.end());
	uint maxIndex = uint(vectorAccumulated.size());
	uint initialIndex = 0;

	forI(maxIndex)
	{
		if (vectorAccumulated[i] > 0)
		{
			initialIndex = i;
			break;
		}
	}

	bool anyValueDifferent = false;
	forI(maxIndex - initialIndex - 1)
	{
		if (vectorAccumulated[i + initialIndex] != vectorAccumulated[i + initialIndex + 1])
		{
			anyValueDifferent = true;
			break;
		}
	}

	assert(anyValueDifferent == false);

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint BufferVerificationHelper::getHashedIndex(uvec3 texcoord, uint voxelSize)
{
	return texcoord.x * voxelSize * voxelSize + texcoord.y * voxelSize + texcoord.z;
}

/////////////////////////////////////////////////////////////////////////////////////////////

uvec3 BufferVerificationHelper::unhashValue(uint value, uint voxelizationWidth)
{
	float sizeFloat = float(voxelizationWidth);
	float number    = float(value) / sizeFloat;

	uvec3 result;
	float integerPart;
	float fractional = glm::modf(number, integerPart);
	result.z         = uint(fractional * sizeFloat);
	number          /= sizeFloat;
	fractional       = glm::modf(number, integerPart);
	result.y         = uint(fractional * sizeFloat);
	result.x         = uint(integerPart);

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyVoxelFirstIndexBuffer()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	vectorUint8 vectorFirstIndex;
	Buffer* voxelFirstIndexBuffer = bufferM->getElement(move(string("voxelFirstIndexBuffer")));
	voxelFirstIndexBuffer->getContentCopy(vectorFirstIndex);
	uint numFirstIndex            = uint(voxelFirstIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pFirstIndex             = (uint*)(vectorFirstIndex.data());

	uint tempValueFirstIndex;
	uint numNonNull = 0;

	forI(numFirstIndex)
	{
		tempValueFirstIndex = pFirstIndex[i];
		if (tempValueFirstIndex != maxValue)
		{
			numNonNull++;
		}
	}

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Voxel First Index Buffer verification" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Voxel First Index Buffer verification" << endl;
	}

	cout << ">>>>> Number of non null elements in voxelFirstIndexBuffer is " << numNonNull << endl;
	if (m_outputAllInformationFile)
	{
		outFile << ">>>>> Number of non null elements in voxelFirstIndexBuffer is " << numNonNull << endl;
	}

	bool anyValueDifferent = false;
	if (((m_accumulatedReductionLevelBase > 0) && (m_accumulatedReductionLevelBase != numNonNull)) ||
		((m_accumulatedReductionLevel0    > 0) && (m_accumulatedReductionLevel0 != numNonNull)) ||
		((m_accumulatedReductionLevel1    > 0) && (m_accumulatedReductionLevel1 != numNonNull)) ||
		((m_accumulatedReductionLevel2    > 0) && (m_accumulatedReductionLevel2 != numNonNull)) ||
		((m_accumulatedReductionLevel3    > 0) && (m_accumulatedReductionLevel3 != numNonNull)))
	{
		anyValueDifferent = true;
	}

	assert(anyValueDifferent == false);

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyVoxelFirstIndexAndVoxelFirstIndexCompacted()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	vectorUint8 vectorFirstIndex;
	Buffer* voxelFirstIndexBuffer = bufferM->getElement(move(string("voxelFirstIndexBuffer")));
	voxelFirstIndexBuffer->getContentCopy(vectorFirstIndex);
	uint numFirstIndex            = uint(voxelFirstIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pFirstIndex             = (uint*)(vectorFirstIndex.data());

	vectorUint8 vecorCompacted;
	Buffer* voxelFirstIndexCompactedBuffer = bufferM->getElement(move(string("voxelFirstIndexCompactedBuffer")));
	voxelFirstIndexCompactedBuffer->getContentCopy(vecorCompacted);
	uint numCompacted                      = uint(voxelFirstIndexCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pCompacted                       = (uint*)(vecorCompacted.data());

	uint counter       = 0;
	uint numNonNull    = 0;
	bool anyIndexError = false;

	uint tempCompacted;
	uint tempValueFirstIndex;

	forI(numFirstIndex)
	{
		tempValueFirstIndex = pFirstIndex[i];
		if (tempValueFirstIndex != maxValue)
		{
			tempCompacted = pCompacted[counter];
			if (tempValueFirstIndex != tempCompacted)
			{
				anyIndexError = true;
			}
			counter++;
		}
	}

	if (anyIndexError)
	{
		cout << "ERROR: not compacted buffer value match at BufferPrefixSumTechnique::verifyVoxelFirstIndexAndVoxelFirstIndexCompacted" << endl;
		if (m_outputAllInformationFile)
		{
			outFile << "ERROR: not compacted buffer value match at BufferPrefixSumTechnique::verifyVoxelFirstIndexAndVoxelFirstIndexCompacted" << endl;
		}
	}

	assert(anyIndexError == false);

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::outputHashedPositionCompactedBufferInfo()
{
	// Unhash the values in voxelHashedPositionCompactedBuffer
	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint voxelizationWidth              = technique->getVoxelizationWidth();

	vectorUint8 vecorHashedCompacted;
	Buffer* voxelHashedPositionCompactedBuffer = bufferM->getElement(move(string("voxelHashedPositionCompactedBuffer")));
	voxelHashedPositionCompactedBuffer->getContentCopy(vecorHashedCompacted);
	uint numHashedCompacted                    = uint(voxelHashedPositionCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pHashedCompacted                     = (uint*)(vecorHashedCompacted.data());

	uint hashed;
	uvec3 unhashed;

	ofstream outFile;
	outFile.open("verify.txt", ofstream::app);

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Hashed position compacted buffer info" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Hashed position compacted buffer info" << endl;
	}

	forI(numHashedCompacted)
	{
		hashed   = pHashedCompacted[i];
		unhashed = unhashValue(hashed, voxelizationWidth);

		if (m_outputAllInformationConsole)
		{
			cout << "Position " << i << " of buffer voxelHashedPositionCompacted has unhashed values (" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
		if (m_outputAllInformationFile)
		{
			outFile << "Position " << i << " of buffer voxelHashedPositionCompacted has unhashed values (" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
	}

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::outputIndirectionBuffer()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Indirection Buffer output" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Indirection Buffer output" << endl;
	}

	vectorUint8 vectorFirstIndexCompacted;
	Buffer* voxelFirstIndexCompactedBuffer = bufferM->getElement(move(string("voxelFirstIndexCompactedBuffer")));
	voxelFirstIndexCompactedBuffer->getContentCopy(vectorFirstIndexCompacted);
	uint numFirstIndexCompacted            = uint(voxelFirstIndexCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pFirstIndexCompacted             = (uint*)(vectorFirstIndexCompacted.data());

	vectorUint8 vectorHashedPositionCompactedBuffer;
	Buffer* voxelHashedPositionCompactedBuffer = bufferM->getElement(move(string("voxelHashedPositionCompactedBuffer")));
	voxelHashedPositionCompactedBuffer->getContentCopy(vectorHashedPositionCompactedBuffer);
	uint numHashedPositionCompactedBuffer      = uint(voxelHashedPositionCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pHashedPositionCompactedBuffer       = (uint*)(vectorHashedPositionCompactedBuffer.data());

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint voxelizationWidth              = technique->getVoxelizationWidth();

	forI(numHashedPositionCompactedBuffer)
	{
		uvec3 unhashed = unhashValue(pHashedPositionCompactedBuffer[i], voxelizationWidth);
		if (m_outputAllInformationConsole)
		{
			cout << "First index compacted at index " << i << " is pFirstIndexCompacted[i]=" << pFirstIndexCompacted[i] << " and hashed value pHashedPositionCompactedBuffer[i]=" << pHashedPositionCompactedBuffer[i] << " being a hashed position=(" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
		if (m_outputAllInformationFile)
		{
			outFile << "First index compacted at index " << i << " is pFirstIndexCompacted[i]=" << pFirstIndexCompacted[i] << " and hashed value pHashedPositionCompactedBuffer[i]=" << pHashedPositionCompactedBuffer[i] << " being a hashed position=(" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
	}

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::outputIndirectionBufferData()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	vectorUint8 vectorIndirectionBuffer;
	Buffer* IndirectionIndexBuffer = bufferM->getElement(move(string("IndirectionIndexBuffer")));
	IndirectionIndexBuffer->getContentCopy(vectorIndirectionBuffer);
	uint numIndirectionBuffer      = uint(IndirectionIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pIndirectionBuffer       = (uint*)(vectorIndirectionBuffer.data());

	// Retrieve data from indirection buffer for rank data
	vectorUint8 vectorIndirectionRankBuffer;
	Buffer* IndirectionRankBuffer = bufferM->getElement(move(string("IndirectionRankBuffer")));
	IndirectionRankBuffer->getContentCopy(vectorIndirectionRankBuffer);
	uint numIndirectionRankBuffer = uint(IndirectionRankBuffer->getDataSize()) / sizeof(uint);
	uint* pIndirectionRankBuffer  = (uint*)(vectorIndirectionRankBuffer.data());

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint voxelizationWidth              = technique->getVoxelizationWidth();

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Indirection buffer data" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Indirection buffer data" << endl;
	}

	forI(numIndirectionBuffer)
	{
		float integerPart;
		float sizeFloat  = float(voxelizationWidth);
		float number     = float(i) / sizeFloat;
		float fractional = glm::modf(number, integerPart);

		uvec3 unhashed;
		unhashed.x = uint(integerPart);
		unhashed.y = uint(fractional * sizeFloat);
		if (m_outputAllInformationConsole)
		{
			cout << "index " << i << " at indirection buffer, pIndirectionBuffer[i]=" << pIndirectionBuffer[i] << ", pIndirectionRankBuffer[i]=" << pIndirectionRankBuffer[i] << " being index " << i << " the hashed position=(" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
		if (m_outputAllInformationFile)
		{
			outFile << "index " << i << " at indirection buffer, pIndirectionBuffer[i]=" << pIndirectionBuffer[i] << ", pIndirectionRankBuffer[i]=" << pIndirectionRankBuffer[i] << " being index " << i << " the hashed position=(" << unhashed.x << "," << unhashed.y << "," << unhashed.z << ")" << endl;
		}
	}

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyIndirectionBuffers()
{
	ofstream outFile;
	if (m_outputAllInformationFile)
	{
		outFile.open("verify.txt", ofstream::app);
	}

	// Retrieve data from indirection buffer for index data
	vectorUint8 vectorIndirectionBuffer;
	Buffer* IndirectionIndexBuffer = bufferM->getElement(move(string("IndirectionIndexBuffer")));
	IndirectionIndexBuffer->getContentCopy(vectorIndirectionBuffer);
	uint numIndirectionBuffer      = uint(IndirectionIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pIndirectionBuffer       = (uint*)(vectorIndirectionBuffer.data());

	// Retrieve data from indirection buffer for rank data
	vectorUint8 vectorIndirectionRankBuffer;
	Buffer* IndirectionRankBuffer = bufferM->getElement(move(string("IndirectionRankBuffer")));
	IndirectionRankBuffer->getContentCopy(vectorIndirectionRankBuffer);
	uint numIndirectionRankBuffer = uint(IndirectionRankBuffer->getDataSize()) / sizeof(uint);
	uint* pIndirectionRankBuffer  = (uint*)(vectorIndirectionRankBuffer.data());

	// Retrieve data from first index buffer already compacted
	vectorUint8 vectorFirstIndexCompacted;
	Buffer* voxelFirstIndexCompactedBuffer = bufferM->getElement(move(string("voxelFirstIndexCompactedBuffer")));
	voxelFirstIndexCompactedBuffer->getContentCopy(vectorFirstIndexCompacted);
	uint numFirstIndexCompacted            = uint(voxelFirstIndexCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pFirstIndexCompacted             = (uint*)(vectorFirstIndexCompacted.data());

	// Retrieve data from hashed buffer compacted
	vectorUint8 vectorHashedPositionCompactedBuffer;
	Buffer* voxelHashedPositionCompactedBuffer = bufferM->getElement(move(string("voxelHashedPositionCompactedBuffer")));
	voxelHashedPositionCompactedBuffer->getContentCopy(vectorHashedPositionCompactedBuffer);
	uint numHashedPositionCompactedBuffer      = uint(voxelHashedPositionCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pHashedPositionCompactedBuffer       = (uint*)(vectorHashedPositionCompactedBuffer.data());

	// Retrieve data from next fragment index buffer
	vectorUint8 vectorNextFragmentIndex;
	Buffer* nextFragmentIndexBuffer = bufferM->getElement(move(string("nextFragmentIndexBuffer")));
	nextFragmentIndexBuffer->getContentCopy(vectorNextFragmentIndex);
	uint numNextFragmentIndex       = uint(nextFragmentIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pNextFragmentIndex        = (uint*)(vectorNextFragmentIndex.data());

	BufferPrefixSumTechnique* technique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	uint voxelizationWidth              = technique->getVoxelizationWidth();

	vectorUint vectorFirstIndexCompactedUsed;

	uint numCompactedBufferAnaylized = 0;
	uint numIndirectionAnalyzed      = 0;
	uint numNextFragmentAnalyzed     = 0;

	if (m_outputAllInformationConsole)
	{
		cout << endl;
		cout << "*************************************" << endl;
		cout << "Indirection Buffers verification" << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << endl;
		outFile << "*************************************" << endl;
		outFile << "Indirection Buffers verification" << endl;
	}

	forI(numIndirectionBuffer)
	{
		uint temp = pIndirectionBuffer[i];
		uint rank = pIndirectionRankBuffer[i];

		if (temp != maxValue)
		{
			numIndirectionAnalyzed++;

			if (m_outputAllInformationConsole)
			{
				cout << "In indirection buffer, at index " << i << " there is an entry with value " << temp << " and rank " << rank << endl;
			}
			if (m_outputAllInformationFile)
			{
				outFile << "In indirection buffer, at index " << i << " there is an entry with value " << temp << " and rank " << rank << endl;
			}

			forJ(rank)
			{
				if (m_outputAllInformationConsole)
				{
					cout << "\t Analyzing now the element number " << j << " in the rank" << endl;
				}
				if (m_outputAllInformationFile)
				{
					outFile << "\t Analyzing now the element number " << j << " in the rank" << endl;
				}
				uint firstIndex     = pFirstIndexCompacted[temp + j];
				uint hashedPosition = pHashedPositionCompactedBuffer[temp + j];
				uvec3 unhashed      = unhashValue(hashedPosition, voxelizationWidth);

				int currentIndex = int(pNextFragmentIndex[firstIndex]);
				numNextFragmentAnalyzed++;

				uint counter = 0;
				if (m_outputAllInformationConsole)
				{
					cout << "\t Emitted fragments for hashed position " << hashedPosition << " (" << unhashed.x << ", " << unhashed.y << ", " << unhashed.z << "):" << endl;
					cout << "\t\t Fragment number " << counter << " at index " << currentIndex << endl;
				}
				if (m_outputAllInformationFile)
				{
					outFile << "\t Emitted fragments for hashed position " << hashedPosition << " (" << unhashed.x << ", " << unhashed.y << ", " << unhashed.z << "):" << endl;
					outFile << "\t\t Fragment number " << counter << " at index " << currentIndex << endl;
				}

				assert(find(vectorFirstIndexCompactedUsed.begin(), vectorFirstIndexCompactedUsed.end(), hashedPosition) == vectorFirstIndexCompactedUsed.end());
				vectorFirstIndexCompactedUsed.push_back(hashedPosition);

				while (currentIndex != -1)
				{
					counter++;
					currentIndex = int(pNextFragmentIndex[currentIndex]);
					if (m_outputAllInformationConsole)
					{
						cout << "\t\t Fragment number " << counter << " at index " << currentIndex << endl;
					}
					if (m_outputAllInformationFile)
					{
						outFile << "\t\t Fragment number " << counter << " at index " << currentIndex << endl;
					}
					numNextFragmentAnalyzed++;
				}

				numCompactedBufferAnaylized++;
			}
		}
	}

	uint firstIndexOccupiedElement = technique->getFirstIndexOccupiedElement();
	cout << "The number of elements analyzed for the pFirstIndexCompacted buffer is " << numCompactedBufferAnaylized << ". The number of elements of the pFirstIndexCompacted buffer is " << firstIndexOccupiedElement << endl;
	cout << "The number of non-null entries in indirection buffer is " << numIndirectionAnalyzed << endl;
	cout << "The number of elements analyzed in pNextFragmentIndex is " << numNextFragmentAnalyzed << endl;
	if (m_outputAllInformationFile)
	{
		outFile << "The number of elements analyzed for the pFirstIndexCompacted buffer is " << numCompactedBufferAnaylized << ". The number of elements of the pFirstIndexCompacted buffer is " << firstIndexOccupiedElement << endl;
		outFile << "The number of non-null entries in indirection buffer is " << numIndirectionAnalyzed << endl;
		outFile << "The number of elements analyzed in pNextFragmentIndex is " << numNextFragmentAnalyzed << endl;
	}

	SceneVoxelizationTechnique* techniqueVoxelization = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	uint fragmentCounter                              = techniqueVoxelization->getFragmentCounter();

	assert(fragmentCounter           == numNextFragmentAnalyzed);
	assert(firstIndexOccupiedElement == numCompactedBufferAnaylized);

	if (m_outputAllInformationConsole)
	{
		cout << "*************************************" << endl;
		cout << endl;
	}
	if (m_outputAllInformationFile)
	{
		outFile << "*************************************" << endl;
		outFile << endl;
	}

	if (m_outputAllInformationFile)
	{
		outFile.close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyVoxelizationProcessData()
{
	verifyFragmentData();

	if (m_outputAllInformationConsole)
	{
		outputVoxelOccupiedBuffer();
	}

	verifyVoxelFirstIndexBuffer();
	verifyVoxelFirstIndexAndVoxelFirstIndexCompacted();

	static bool doStoreInFile = true;
	if (doStoreInFile)
	{
		outputHashedPositionCompactedBufferInfo();
	}

	if (m_outputAllInformationConsole)
	{
		outputIndirectionBuffer();
		outputIndirectionBufferData();
	}

	verifyIndirectionBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifySummedAreaResult(Texture* initialTexture, Texture* finalTexture)
{
	vectorVectorFloat vectorInitial;
	vectorVectorFloat vectorResult;
	vectorVectorVec4 temp;
	VulkanStructInitializer::getTextureData(initialTexture, VK_FORMAT_R32_SFLOAT, temp, vectorInitial);
	VulkanStructInitializer::getTextureData(finalTexture,   VK_FORMAT_R32_SFLOAT, temp, vectorResult);

	vectorVectorFloat vectorInitialSummedArea = computeSummedArea(vectorInitial, -1);
	bool result                               = verifyAreEqual(vectorResult, vectorInitialSummedArea, 0.001f);

	assert(result == true);
}

/////////////////////////////////////////////////////////////////////////////////////////////

vectorVectorFloat BufferVerificationHelper::computeSummedArea(vectorVectorFloat& vecData, int maxStep)
{
	// For now, square areas are assumed
	uint size = uint(vecData.size());

	// Offset data is the same for rows and columns step since square areas are assumed
	vectorUint vectorOffset;
	uint perDimensionStep = uint(ceil(log2(float(size))));
	vectorOffset.resize(perDimensionStep);

	forI(perDimensionStep)
	{
		vectorOffset[i]   = uint(pow(2.0, float(i)));
	}

	vectorVectorFloat vecSource      = vecData;
	vectorVectorFloat vecDestination = vecData;

	uint offset;
	uint row;
	uint column;
	float valueSource;
	float valueDestination;

	uint finalMaxStep = (maxStep == -1) ? perDimensionStep : glm::min(uint(maxStep), perDimensionStep);

	// Rows first
	forI(finalMaxStep)
	{
		offset = vectorOffset[i];
		forJ(size - offset)
		{
			row = j;
			forK(size)
			{
				column                               = k;
				valueSource                          = vecSource     [row         ][column];
				valueDestination                     = vecDestination[row + offset][column];
				vecDestination[row + offset][column] = valueSource + valueDestination;
			}
		}

		vecSource = vecDestination;
	}

	// Columns
	forI(finalMaxStep)
	{
		offset = vectorOffset[i];
		forJ(size - offset)
		{
			column = j;
			forK(size)
			{
				row                                  = k;
				valueSource                          = vecSource     [row][column         ];
				valueDestination                     = vecDestination[row][column + offset];
				vecDestination[row][column + offset] = valueSource + valueDestination;
			}
		}

		vecSource = vecDestination;
	}

	return vecDestination;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferVerificationHelper::verifyAreEqual(const vectorVectorFloat& vecData0, const vectorVectorFloat& vecData1, float eps)
{
	uint sizeData0 = uint(vecData0.size());
	uint sizeData1 = uint(vecData1.size());

	if (sizeData0 != sizeData1)
	{
		cout << "ERROR: different size parameters in SummedAreaTextureTechnique::verifyAreEqual" << endl;
		return false;
	}

	forI(sizeData0)
	{
		if (vecData0[i].size() != vecData1[i].size())
		{
			cout << "ERROR: different size parameters in SummedAreaTextureTechnique::verifyAreEqual" << endl;
			return false;
		}
	}

	uint numDifferent = 0;
	uint numElement;
	forI(sizeData0)
	{
		numElement = uint(vecData0[i].size());

		forJ(numElement)
		{
			if (abs(vecData0[i][j] - vecData1[i][j]) > eps)
			{
				float data0 = vecData0[i][j];
				float data1 = vecData1[i][j];
				float dif = abs(data0 - data1);
				numDifferent++;
			}
		}
	}

	if (numDifferent > 0)
	{
		cout << "ERROR: wrong values in summed-area texture results for threshold " << eps << ", the number of elements with error above the threshold is " << numDifferent << endl;
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferVerificationHelper::verifyAddUpTextureValue(Texture* texture, float value, float threshold)
{
	vectorVectorFloat vectorData;
	vectorVectorVec4 temp;
	VulkanStructInitializer::getTextureData(texture, VK_FORMAT_R32_SFLOAT, temp, vectorData);

	const uint maxWidth = texture->getWidth();
	const uint maxHeigt = texture->getHeight();

	double accumulated = 0.0;
	forI(maxWidth)
	{
		forJ(maxHeigt)
		{
			accumulated += double(vectorData[i][j]);

			if (double(vectorData[i][j]) < 0.0)
			{
				int a = 0;
			}
		}
	}

	if (abs(accumulated - double(value)) < double(threshold))
	{
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

vec3 BufferVerificationHelper::voxelSpaceToWorld(uvec3 coordinates, vec3 voxelSize, vec3 sceneExtent, vec3 sceneMin)
{
	vec3 result = vec3(float(coordinates.x), float(coordinates.y), float(coordinates.z));
	result     /= vec3(voxelSize.x, voxelSize.y, voxelSize.z);
	result     *= sceneExtent;
	result     += sceneMin;
	result     += ((vec3(sceneExtent) / vec3(voxelSize.x, voxelSize.y, voxelSize.z)) * 0.5f);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

uvec3 BufferVerificationHelper::worldToVoxelSpace(vec3 coordinates, vec3 voxelSize, vec3 sceneExtent, vec3 sceneMin)
{
	vec3 result = coordinates;
	result     -= sceneMin;
	result     /= sceneExtent;
	result     *= vec3(voxelSize);

	return uvec3(uint(result.x), uint(result.y), uint(result.z));
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferVerificationHelper::intervalIntersection(int minX0, int maxX0, int minX1, int maxX1)
{
	if ((((minX0 >= minX1) && (minX0 <= maxX1)) || ((maxX0 >= minX1) && (maxX0 <= maxX1))) ||
		(((minX1 >= minX0) && (minX1 <= maxX0)) || ((maxX1 >= minX0) && (maxX1 <= maxX0))))
	{
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyClusterFinalDataBuffer()
{
	vectorUint8 vectorVoxelHashedPositionCompactedBuffer;
	Buffer* voxelHashedPositionCompactedBuffer = bufferM->getElement(move(string("voxelHashedPositionCompactedBuffer")));
	voxelHashedPositionCompactedBuffer->getContentCopy(vectorVoxelHashedPositionCompactedBuffer);
	uint numVoxelHashedPositionCompactedBuffer = uint(voxelHashedPositionCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pVoxelHashedPositionCompactedBuffer  = (uint*)(vectorVoxelHashedPositionCompactedBuffer.data());

	vectorUint8 vectorVoxelClusterOwnerIndexBuffer;
	Buffer* voxelClusterOwnerIndexBuffer = bufferM->getElement(move(string("voxelClusterOwnerIndexBuffer")));
	voxelClusterOwnerIndexBuffer->getContentCopy(vectorVoxelClusterOwnerIndexBuffer);
	uint numVoxelClusterOwnerIndexBuffer = uint(voxelClusterOwnerIndexBuffer->getDataSize()) / sizeof(uint);
	int* pVoxelClusterOwnerIndexBuffer  = (int*)(vectorVoxelClusterOwnerIndexBuffer.data());

	vectorUint8 vectorClusterizationFinalBuffer;
	Buffer* clusterizationFinalBuffer        = bufferM->getElement(move(string("clusterizationFinalBuffer")));
	clusterizationFinalBuffer->getContentCopy(vectorClusterizationFinalBuffer);
	uint numClusterizationFinalBuffer        = uint(clusterizationFinalBuffer->getDataSize()) / sizeof(ClusterData);
	ClusterData* pClusterizationFinalBuffer  = (ClusterData*)(vectorClusterizationFinalBuffer.data());

	// First, verify the cluster owner data in voxelClusterOwnerIndexBuffer is correct regarding the information in clusterizationFinalBuffer

	uint i;
	vectorUint vectorNumVoxelPerCluster;
	vectorNumVoxelPerCluster.resize(numClusterizationFinalBuffer);
	memset(vectorNumVoxelPerCluster.data(), 0, vectorNumVoxelPerCluster.size() * size_t(sizeof(uint)));

	for (i = 0; i < numVoxelClusterOwnerIndexBuffer; ++i)
	{
		if (pVoxelClusterOwnerIndexBuffer[i] != -1)
		{
			vectorNumVoxelPerCluster[pVoxelClusterOwnerIndexBuffer[i]]++;
		}
	}

	uint numNeighbour;
	ClusterData* temp;
	uint j;

	ivec4 minAABB;
	ivec4 maxAABB;
	int counterVoxelInside;
	ivec3 coords;

	for (i = 0; i < numClusterizationFinalBuffer; ++i)
	{
		temp = &pClusterizationFinalBuffer[i];

		//cout << "Cluster " << i << endl;
		//cout << "\t Index=" << temp->minAABB.w << endl;
		//cout << "\t Num neighbour=" << temp->maxAABB.w << endl;
		//cout << "\t Num occupied voxels=" << temp->centerAABB.w << endl;
		//cout << "\t m=(" << temp->minAABB.x << "," << temp->minAABB.y << "," << temp->minAABB.z << ")" << endl;
		//cout << "\t M=(" << temp->maxAABB.x << "," << temp->maxAABB.y << "," << temp->maxAABB.z << ")" << endl;

		if (vectorNumVoxelPerCluster[temp->minAABB.w] != temp->centerAABB.w)
		{
			cout << "ERROR: cluster " << i << " has " << vectorNumVoxelPerCluster[temp->minAABB.w] << " occupied voxels when it should have " << temp->centerAABB.w << endl;
		}

		minAABB            = temp->minAABB;
		maxAABB            = temp->maxAABB;
		counterVoxelInside = 0;

		for (j = 0; j < numVoxelHashedPositionCompactedBuffer; ++j)
		{
			coords = ivec3(unhashValue(pVoxelHashedPositionCompactedBuffer[j], 32));

			if ((coords.x >= minAABB.x) && (coords.x <= maxAABB.x) &&
				(coords.y >= minAABB.y) && (coords.y <= maxAABB.y) &&
				(coords.z >= minAABB.z) && (coords.z <= maxAABB.z) &&
				(pVoxelClusterOwnerIndexBuffer[j] == temp->minAABB.w))
			{
				counterVoxelInside++;
			}
		}

		if (counterVoxelInside != temp->centerAABB.w)
		{
			//cout << "ERROR: cluster " << i << " has " << counterVoxelInside << " voxels inside when it should have " << temp->centerAABB.w << endl;
		}

		numNeighbour = uint(temp->maxAABB.w);

		cout << "\t Neighbour ";
		for (j = 0; j < numNeighbour; ++j)
		{
			cout << ", " << temp->arrayNeighbourIndex[j] << " ";
		}

		cout << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::countNumberVoxelWithNoClusterOwner()
{
	vectorUint8 vectorVoxelClusterOwnerIndexBuffer;
	Buffer* voxelClusterOwnerIndexBuffer = bufferM->getElement(move(string("voxelClusterOwnerIndexBuffer")));
	voxelClusterOwnerIndexBuffer->getContentCopy(vectorVoxelClusterOwnerIndexBuffer);
	uint numVoxelClusterOwnerIndexBuffer = uint(voxelClusterOwnerIndexBuffer->getDataSize()) / sizeof(uint);
	int* pVoxelClusterOwnerIndexBuffer   = (int*)(vectorVoxelClusterOwnerIndexBuffer.data());

	int numberVoxelNoClusterOwner = 0;
	for (uint i = 0; i < numVoxelClusterOwnerIndexBuffer; ++i)
	{
		if (pVoxelClusterOwnerIndexBuffer[i] == -1)
		{
			numberVoxelNoClusterOwner++;
		}
	}

	cout << "There are " << numVoxelClusterOwnerIndexBuffer << " occupied voxels and " << numberVoxelNoClusterOwner << " voxels with no cluster owner, which is a " << float(numberVoxelNoClusterOwner) / float(numVoxelClusterOwnerIndexBuffer) << " percent of the total " << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::findMergeCandidateCluster(int maxNumberVoxel)
{
	vectorUint8 vectorClusterizationFinalBuffer;
	Buffer* clusterizationFinalBuffer        = bufferM->getElement(move(string("clusterizationFinalBuffer")));
	clusterizationFinalBuffer->getContentCopy(vectorClusterizationFinalBuffer);
	uint numClusterizationFinalBuffer        = uint(clusterizationFinalBuffer->getDataSize()) / sizeof(ClusterData);
	ClusterData* pClusterizationFinalBuffer  = (ClusterData*)(vectorClusterizationFinalBuffer.data());

	vectorUint8 vectorVoxelClusterOwnerIndexBuffer;
	Buffer* voxelClusterOwnerIndexBuffer = bufferM->getElement(move(string("voxelClusterOwnerIndexBuffer")));
	voxelClusterOwnerIndexBuffer->getContentCopy(vectorVoxelClusterOwnerIndexBuffer);
	uint numVoxelClusterOwnerIndexBuffer = uint(voxelClusterOwnerIndexBuffer->getDataSize()) / sizeof(uint);
	int* pVoxelClusterOwnerIndexBuffer   = (int*)(vectorVoxelClusterOwnerIndexBuffer.data());

	uint i;

	vectorUint vectorNumVoxelPerCluster;
	vectorNumVoxelPerCluster.resize(numClusterizationFinalBuffer);
	memset(vectorNumVoxelPerCluster.data(), 0, vectorNumVoxelPerCluster.size() * size_t(sizeof(uint)));
	for (i = 0; i < numVoxelClusterOwnerIndexBuffer; ++i)
	{
		if (pVoxelClusterOwnerIndexBuffer[i] != -1)
		{
			vectorNumVoxelPerCluster[pVoxelClusterOwnerIndexBuffer[i]]++;
		}
	}

	uint j;
	uint numNeighbour;
	int numOccupied;
	int indexNeighbour;
	ClusterData* temp;
	ClusterData* neighbour;
	vec4 mainDirection;
	vec4 mainDirectionNeighbour;
	uint numElementInCluster;
	ivec4 minAABB;
	ivec4 maxAABB;

	for (i = 0; i < numClusterizationFinalBuffer; ++i)
	{
		temp         = &pClusterizationFinalBuffer[i];
		numOccupied  = temp->centerAABB.w;

		if (numOccupied > maxNumberVoxel)
		{
			continue;
		}

		mainDirection = temp->mainDirection;
		numNeighbour  = uint(temp->maxAABB.w);
		minAABB       = temp->minAABB;
		maxAABB       = temp->maxAABB;

		for (j = 0; j < numNeighbour; ++j)
		{
			indexNeighbour         = temp->arrayNeighbourIndex[j];
			neighbour              = &pClusterizationFinalBuffer[indexNeighbour];
			mainDirectionNeighbour = neighbour->mainDirection;

			if(dot(vec3(mainDirection.x, mainDirection.y, mainDirection.z),
			       vec3(mainDirectionNeighbour.x, mainDirectionNeighbour.y, mainDirectionNeighbour.z)) < 0.867f)
			{
				continue;
			}

			if (neighbour->centerAABB.w > maxNumberVoxel)
			{
				numElementInCluster = vectorNumVoxelPerCluster[indexNeighbour];

				if (numElementInCluster != neighbour->centerAABB.w)
				{
					cout << "ERROR: neighbour " << j << " of cluster " << i << " has " << neighbour->centerAABB.w << " clusters and should have " << numElementInCluster << endl;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyLitClusterInformation()
{
	vectorUint8 vectorLitTestClusterBuffer;
	Buffer* litTestClusterBuffer        = bufferM->getElement(move(string("litTestClusterBuffer")));
	litTestClusterBuffer->getContentCopy(vectorLitTestClusterBuffer);
	uint numLitTestClusterBuffer        = uint(litTestClusterBuffer->getDataSize()) / sizeof(uint);
	uint* pLitTestClusterBuffer         = (uint*)(vectorLitTestClusterBuffer.data());

	uint numLitClusterProcessed     = 0;
	Buffer* litClusterCounterBuffer = bufferM->getElement(move(string("litClusterCounterBuffer")));
	litClusterCounterBuffer->getContent((void*)(&numLitClusterProcessed));

	vectorUint8 vectorLitVisibleIndexCluster;
	Buffer* litVisibleIndexCluster = bufferM->getElement(move(string("litVisibleIndexClusterBuffer")));
	litVisibleIndexCluster->getContentCopy(vectorLitVisibleIndexCluster);
	uint numLitVisibleIndexCluster = uint(litVisibleIndexCluster->getDataSize()) / sizeof(uint);
	uint* pLitVisibleIndexCluster = (uint*)(vectorLitVisibleIndexCluster.data());

	vectorUint vectorLitTestNotNullIndices;
	vectorUint vectorLitVisibleNotNullIndices;

	forI(numLitTestClusterBuffer)
	{
		if (pLitTestClusterBuffer[i] != 0)
		{
			vectorLitTestNotNullIndices.push_back(i);
		}
	}

	forI(numLitClusterProcessed)
	{
		vectorLitVisibleNotNullIndices.push_back(pLitVisibleIndexCluster[i]);
	}

	vectorUint vectorLitVisibleNotNullIndicesCopy = vectorLitVisibleNotNullIndices;

	sort(vectorLitVisibleNotNullIndices.begin(), vectorLitVisibleNotNullIndices.end());

	if (vectorLitTestNotNullIndices.size() != vectorLitVisibleNotNullIndices.size())
	{
		assert(1);
	}

	forI(vectorLitTestNotNullIndices.size())
	{
		if (vectorLitTestNotNullIndices[i] != vectorLitVisibleNotNullIndices[i])
		{
			assert(1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyClusterVisibilityCompactedData()
{
	// TODO: Copy from the prefix sum buffer the offset to start each of the sets of visible cluster in the compacted buffer

	vectorUint8 vectorClusterVisibilityBuffer;
	Buffer* clusterVisibilityBuffer = bufferM->getElement(move(string("clusterVisibilityBuffer")));
	clusterVisibilityBuffer->getContentCopy(vectorClusterVisibilityBuffer);
	uint numclusterVisibilityBuffer = uint(clusterVisibilityBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityBuffer = (uint*)(vectorClusterVisibilityBuffer.data());

	vectorUint vectorManuallyCompacted;
	for (uint i = 0; i < numclusterVisibilityBuffer; ++i)
	{
		if (pClusterVisibilityBuffer[i] != maxValue)
		{
			vectorManuallyCompacted.push_back(pClusterVisibilityBuffer[i]);
		}
	}

	vectorUint8 vectorClusterVisibilityCompacted;
	Buffer* clusterVisibilityCompactedBuffer = bufferM->getElement(move(string("clusterVisibilityCompactedBuffer")));
	clusterVisibilityCompactedBuffer->getContentCopy(vectorClusterVisibilityCompacted);
	uint numClusterVisibilityCompacted = uint(clusterVisibilityCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityCompacted = (uint*)(vectorClusterVisibilityCompacted.data());

	if (numClusterVisibilityCompacted != uint(vectorManuallyCompacted.size()))
	{
		cout << " ERROR: The amount of compacted visible clusters and the amount of manually verified compacted visible clusters do not match" << endl;
		assert(1);
	}

	for (uint i = 0; i < numClusterVisibilityCompacted; ++i)
	{
		if (pClusterVisibilityCompacted[i] != vectorManuallyCompacted[i])
		{
			cout << " ERROR: The values of the compacted and manually verified compacted visible cluster do not match" << endl;
			assert(1);
		}
	}

	cout << "The values of the visible clusters are verified successfully" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyClusterVisibilityIndices()
{
	vectorUint8 vectorClusterVisibilityDebugBuffer;
	Buffer* clusterVisibilityDebugBuffer = bufferM->getElement(move(string("clusterVisibilityDebugBuffer")));
	clusterVisibilityDebugBuffer->getContentCopy(vectorClusterVisibilityDebugBuffer);
	uint numClusterVisibilityDebugBuffer = uint(clusterVisibilityDebugBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityDebugBuffer = (uint*)(vectorClusterVisibilityDebugBuffer.data());

	uint localWorkGroupIndex = 0;
	uint numberThread = numClusterVisibilityDebugBuffer / 5;
	bool bufferVerificationFailed = false;

	for (uint i = 0; i < numberThread; ++i)
	{
		uint idxBuffer                     = pClusterVisibilityDebugBuffer[5 * i + 0];
		uint startIndexBuffer              = pClusterVisibilityDebugBuffer[5 * i + 1];
		uint voxelIndexBuffer              = pClusterVisibilityDebugBuffer[5 * i + 2];
		uint faceIndexBuffer               = pClusterVisibilityDebugBuffer[5 * i + 3];
		uint g_sharedNumVisibleVoxelBuffer = pClusterVisibilityDebugBuffer[5 * i + 4];

		float integerPart;
		float voxelIndexFloat = float(idxBuffer) / (128.0f * 6.0f);
		float fractionalPart = glm::modf(voxelIndexFloat, integerPart);
		int voxelIndex = int(integerPart);
		voxelIndexFloat = float(localWorkGroupIndex) / 6.0f;
		fractionalPart = glm::modf(voxelIndexFloat, integerPart);
		int faceIndex = int(round(fractionalPart * 6.0));
		int startIndex = (voxelIndex * int(128) * 6) + faceIndex * int(128);

		if ((startIndexBuffer != startIndex) || (voxelIndexBuffer != voxelIndex) || (faceIndexBuffer != faceIndex))
		{
			bufferVerificationFailed = true;
		}

		localWorkGroupIndex++;
		localWorkGroupIndex = localWorkGroupIndex % 128;
	}

	if (bufferVerificationFailed)
	{
		cout << "ERROR: buffer verification failed at BufferVerificationHelper::verifyClusterVisibilityIndices()" << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferVerificationHelper::verifyClusterVisibilityFirstIndexBuffer()
{
	vectorUint8 vectorClusterVisibilityFirstIndexBuffer;
	Buffer* clusterVisibilityFirstIndexBuffer = bufferM->getElement(move(string("clusterVisibilityFirstIndexBuffer")));
	clusterVisibilityFirstIndexBuffer->getContentCopy(vectorClusterVisibilityFirstIndexBuffer);
	uint numClusterVisibilityFirstIndexBuffer = uint(clusterVisibilityFirstIndexBuffer->getDataSize()) / sizeof(uint);
	uint* pclusterVisibilityFirstIndexBuffer = (uint*)(vectorClusterVisibilityFirstIndexBuffer.data());

	vectorUint8 vectorClusterVisibilityCompactedBuffer;
	Buffer* clusterVisibilityCompactedBuffer = bufferM->getElement(move(string("clusterVisibilityCompactedBuffer")));
	clusterVisibilityCompactedBuffer->getContentCopy(vectorClusterVisibilityCompactedBuffer);
	uint numClusterVisibilityCompactedBuffer = uint(clusterVisibilityCompactedBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityCompactedBuffer = (uint*)(vectorClusterVisibilityCompactedBuffer.data());

	vectorUint8 vectorClusterVisibilityBuffer;
	Buffer* clusterVisibilityBuffer = bufferM->getElement(move(string("clusterVisibilityBuffer")));
	clusterVisibilityBuffer->getContentCopy(vectorClusterVisibilityBuffer);
	uint numclusterVisibilityBuffer = uint(clusterVisibilityBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityBuffer = (uint*)(vectorClusterVisibilityBuffer.data());

	vectorUint8 vectorClusterVisibilityNumberBuffer;
	Buffer* clusterVisibilityNumberBuffer = bufferM->getElement(move(string("clusterVisibilityNumberBuffer")));
	clusterVisibilityNumberBuffer->getContentCopy(vectorClusterVisibilityNumberBuffer);
	uint numClusterVisibilityNumberBuffer = uint(clusterVisibilityNumberBuffer->getDataSize()) / sizeof(uint);
	uint* pClusterVisibilityNumberBuffer = (uint*)(vectorClusterVisibilityNumberBuffer.data());

	// Now do a verification of clusterVisibilityFirstIndexBuffer
	for (uint i = 0; i < numClusterVisibilityNumberBuffer; ++i)
	{
		uint numVisible = pClusterVisibilityNumberBuffer[i];

		if (numVisible > 0)
		{
			uint firstIndex = pclusterVisibilityFirstIndexBuffer[i];
			vectorUint vectorFromCompacted;
			for (uint j = firstIndex; j < firstIndex + numVisible; ++j)
			{
				vectorFromCompacted.push_back(pClusterVisibilityCompactedBuffer[j]);
			}

			vectorUint vectorFromNotCompacted;
			for (uint j = i * 128; j < i * 128 + 128; ++j)
			{
				if (pClusterVisibilityBuffer[j] != 4294967295)
				{
					vectorFromNotCompacted.push_back(pClusterVisibilityBuffer[j]);
				}
			}

			if (vectorFromCompacted.size() != vectorFromNotCompacted.size())
			{
				cout << "ERROR: There is not the same size in compacted and not compacted vectors for index" << i << endl;
				cout << "Start index for vector not compacted is " << i * 128 << ", end index " << i * 128 + 128 << endl;
				cout << "Start index for vector compacted is " << firstIndex << ", end index is " << firstIndex + numVisible << endl;
				assert(1);
			}

			for (uint j = 0; j < vectorFromCompacted.size(); ++j)
			{
				if (vectorFromCompacted[j] != vectorFromNotCompacted[j])
				{
					cout << "ERROR: There is not the same value in compacted and not compacted for index" << i << endl;
					cout << "Start index for vector not compacted is " << i * 128 << ", end index " << i * 128 + 128 << endl;
					cout << "Start index for vector compacted is " << firstIndex << ", end index is " << firstIndex + numVisible << endl;
					assert(1);
				}
			}
		}
	}

	cout << "The verification of clusterVisibilityFirstIndexBuffer finisehd with no errors detected" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////
