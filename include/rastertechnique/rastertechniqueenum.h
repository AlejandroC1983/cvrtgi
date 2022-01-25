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

#ifndef _RASTER_TECHNIQUE_ENUM_H_
#define _RASTER_TECHNIQUE_ENUM_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES

// CLASS FORWARDING

// NAMESPACE
namespace rastertechniqueenum
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Policies for recording commands for each raster technique */
	enum class CommandRecordPolicy
	{
		CRP_SINGLE_TIME = 0,       //!< To record commands only one single time for the rester technique with this policy
		CRP_EVERY_SWAPCHAIN_IMAGE, //!< To record commands for every swap chainimage for the rester technique with this policy
		CRP_SIZE                   //!< Number of possible values
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Simple enumerated type to describe the type of queue the recorded command buffer should be submitted to */
	enum class CommandBufferType
	{
		CBT_GRAPHICS_QUEUE = 0, //!< To indicate that the recorded command buffer should be submitted to the graphics queue
		CBT_COMPUTE_QUEUE,      //!< To indicate that the recorded command buffer should be submitted to the compute queue
		CBT_SIZE                //!< Number of possible values
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Simple enumerated type to describe the type of queue a raster technique submits command buffers to */
	enum class RasterTechniqueType
	{
		RTT_GRAPHICS = 0, //!< Submit to the graphics queue
		RTT_COMPUTE,      //!< Submit to the compute queue
		RTT_SIZE          //!< Number of possible values
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
}

// DEFINES
#define NUMBER_OF_VIEWPORTS 1
#define NUMBER_OF_SCISSORS NUMBER_OF_VIEWPORTS
typedef map<uint, rastertechniqueenum::CommandBufferType> mapUintCommandBufferType;

#endif _RASTER_TECHNIQUE_ENUM_H_
