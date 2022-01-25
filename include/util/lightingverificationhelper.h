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

#ifndef _LIGHTINGVERIFICATIONHELPER_H_
#define _LIGHTINGVERIFICATIONHELPER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** This is a helper class to verify the lighting information of the texture space expanding planes */
class LightingVerificationHelper
{
public:
	/** Will save to path given by parameter, with file name given by parameter, the information present in the
	* vectorData parameter for plotting. For scaling purposes, the information in each element of vectorData
	* will be multiplied by the multiplier parameter
	* @param vectorData [in] vector with the texture information
	* @param multiplier [in] value that will multiply each element in vectorData
	* @param minValue   [in] min value range
	* @param maxValue   [in] max value range
	* @param zRangeMin  [in] z range min value
	* @param zRangeMax  [in] z range max value
	* @param filePath   [in] path for the outputted file
	* @param fileName   [in] name for the outputted file
	* @return nothing */
	static void saveDataForPlot(const vectorVectorFloat& vectorData,
		                        float                    multiplier,
								float                    minValue,
								float                    maxValue,
								float                    zRangeMin,
								float                    zRangeMax,
		                        const string&            filePath,
		                        const string&            fileName);

	/** Computes the minimum and maximum values of the texture information present in vectorData
	* @param vectorData [in]    vector with the texture information
	* @param min        [inout] minimum value in vectorData
	* @param max        [inout] maximum value in vectorData
	*@return nothing */
	static void getMinAndMaxValues(const vectorVectorFloat& vectorData, float& min, float& max);

	static vectorVectorFloat m_vectorFirstPass;         //!< Vector with the first lighting texture information to compare
	static vectorVectorFloat m_vectorSecondPass;        //!< Vector with the second lighting texture information to compare
	static bool              m_firstPassDataSet;        //!< True if the information of the first pass to compare (m_vectorFirstPass) has been set
	static bool              m_secondPassDataSet;       //!< True if the information of the second pass to compare (m_vectorSecondPass) has been set
	static uint              m_outputTexturePlotWidth;  //!< If outputting data to generate plots, width of the texture to generate
	static uint              m_outputTexturePlotHeight; //!< If outputting data to generate plots, height of the texture to generate
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _LIGHTINGVERIFICATIONHELPER_H_
