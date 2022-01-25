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
#include "../../include/util/io.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

void* InputOutput::readFile(const char *filePath, size_t *fileSize)
{
	FILE *fp = fopen(filePath, "rb");

	if (!fp)
	{
		return NULL;
	}

	size_t retval;
	long int size;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void* spvShader = malloc(size+1); // Plus for NULL character '\0'
	memset(spvShader, 0, size+1);

	retval = fread(spvShader, size, 1, fp);
	assert(retval == 1);

	*fileSize = size;
	fclose(fp);
	return spvShader;
}

/////////////////////////////////////////////////////////////////////////////////////////////
