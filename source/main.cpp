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
#include <chrono>

// PROJECT INCLUDES
#include "../include/headers.h"
#include "../include/core/coremanager.h"
#include "../include/scene/scene.h"
#include "../include/core/surface.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	s_pCoreManager = Singleton<CoreManager>::init();

	coreM->initialize();
	sceneM->init();
	gpuPipelineM->init();

	bool isWindowOpen = true;
	float elapsedTimeMiliseconds;
	float elapsedSinceApplicationStartMiliseconds;
	std::chrono::steady_clock::time_point timeInit = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point rasterTime0;
	std::chrono::steady_clock::time_point rasterTime1;

	while (isWindowOpen)
	{
		if (coreM->getReachedFirstRaster() && !coreM->getIsPrepared())
		{
			continue;
		}

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			rasterTime0 = std::chrono::high_resolution_clock::now();

			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (coreM->getEndApplicationMessage())
			{
				coreM->deInitialize();
				PostQuitMessage(0);
				return false;
			}

			sceneM->setDeltaTime(elapsedTimeMiliseconds);
			sceneM->setExecutionTime(elapsedSinceApplicationStartMiliseconds);
			inputM->updateInput(msg);
			inputM->updateKeyboard();
			sceneM->update();
			gpuPipelineM->update();
			coreM->prepare();
			RedrawWindow(coreM->getWindowPlatformHandle(), NULL, NULL, RDW_INTERNALPAINT);
			coreM->render();
			coreM->postRender();

			rasterTime1 = std::chrono::high_resolution_clock::now();
			elapsedTimeMiliseconds = float(std::chrono::duration_cast<std::chrono::milliseconds>(rasterTime1 - rasterTime0).count());
			elapsedSinceApplicationStartMiliseconds = float(std::chrono::duration_cast<std::chrono::milliseconds>(rasterTime1 - timeInit).count());
			//cout << "FPS = " << 1000.0f / elapsedTimeMiliseconds << endl;
		}
	}
	coreM->deInitialize();
}

/////////////////////////////////////////////////////////////////////////////////////////////
