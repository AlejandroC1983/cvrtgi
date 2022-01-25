Clustered voxel real-time global illumination
========

![](banner.gif)

This project contains a full implementation of clustered voxel real-time global illumination, including all the graphics framework written in C++ and Vulkan, and all the shaders needed for the technique.

Build for Windows
------------

*Note: Unfortunately CMakeListst.txt does not fully automate the build process, with assimp and spirv-cross libraries precompiled for Visual Studio 2017. The spirv-cross code has been modified to allow better shader reflection funtionality, differing from the code in the original repository. Automatizing the build of those libraries to fully automate and build for any Windows platform is pending work.*

*Note: All dependencies needed for this project are included in the external folder, included Vulkan libraries. Vulkan version used is 1.1.101.0.*

* Clone / download the repository.
* Run CMakeListst.txt and select Visual Studio 2017 x64 bits as generator for the project.
* Open and compile the solution in Debug / Release modes (Release recommended).
* Copy external/assimp/dll/Debug/x64/assimpd.dll and external/assimp/dll/Release/x64/assimp.dll to build/Debug and build/Release respectively.

Controls
-------
* Arrows to move the camera.
* S to transition between the different cameras in the scene.
* L to enable lighting.
* V to switch to voxel view.
* R When in voxel view, to show reflectance.
* N When in voxel view, to show normal direction.
* M When in voxel view, to show mean normal direction.

How to run
-------

* Run the project from the Visual Studio Solution with Ctrl+F5.
* Press S 3 times to reach the emitter camera (the scene has 4 cameras, being the first one the main camera, and the third one the emitter camera), orient and move it to lit the scene in the way you prefer.
* Press L to enable lighting.
* Press S again to cycle and go to the main camera (lighting is not updated for secondary cameras).

License
-------

The project is licensed under the Apache 2.0 license.
