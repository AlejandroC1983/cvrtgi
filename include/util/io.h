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

#ifndef _IO_H_
#define _IO_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class InputOutput
{
public:
	/** Read file given path as parameter
	* @param filePath [in] Path of the file to read
	* @param fileSize [in] Size of the file read
	* @return pointer to file content */
	static void* readFile(const char *filePath, size_t *fileSize);
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _IO_H_
