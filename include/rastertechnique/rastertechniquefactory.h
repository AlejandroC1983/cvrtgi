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

#ifndef _RASTERTECHNIQUEFACTORY_H_
#define _RASTERTECHNIQUEFACTORY_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/factorytemplate.h"
#include "../../include/util/managertemplate.h"

// CLASS FORWARDING
class RasterTechnique;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class RasterTechniqueFactory: public FactoryTemplate<RasterTechnique>
{

};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _RASTERTECHNIQUEFACTORY_H_
