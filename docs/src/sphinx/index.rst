Welcome to LavaCake!
####################

Lavacake is an Open Source framework that aims to simplify the use of Vulkan in C++ for rapid prototyping.
This framework provides multiple functions to help manipulate the usual Vulkan structures such as queue, command buffer, render pass, shader module, and many more.

LavaCake comes with `GLFW <https://github.com/glfw/glfw>`_. and `ImGui <https://github.com/ocornut/imgui>`_. as submodule. 
This project was originally based on the `Vulkan CookBook's examples <https://github.com/PacktPublishing/Vulkan-Cookbook>`_. .

LavaCake is primarily developed by Thibault Tricard for his research work.


Disclamer
#########

This Framework is made to help research and toy project, and may need to evolve in function of future evolution of Vulkan and of my needs.
Although I will try not to break retrocompatibility between version of the framework, I canot fully guarantee it completely.

This project is a side project of mine and I don't plan on actively provide support for Lavacake.


Building LavaCake
#################
To build Lavacake, you need to have a recent VulkanSDK installed.
Before building, make sure all submodule are correctly updated.

Then use CMake

Compatibility
#############

Windows
*******
Lavacake was originally developed with windows and is fully compatible with it.

Linux
*****
LavaCake is compatible with Linux with its 1.3 version currently under development.\

MacOS
*****
LavaCake is compatible with macOS and Metal in its 1.3 version through MoltenVK.
Of course, limitations may exist due to the interface between Vulkan and Metal

Examples
########
Multiple examples of how to use this library are available at https://github.com/ThibaultTricard/LavaCake-Examples/tree/dev-1.3

Credits
#######
Although it has evolved, this library is based on the Vulkan Cookbook code.  

The code repository for `Vulkan Cookbook <https://www.packtpub.com/game-development/vulkan-cookbook?utm_source=github&utm_medium=repository&utm_campaign=9781786468154>`_., published by `Packt <https://www.packtpub.com/>`_..

.. toctree::
   :maxdepth: 2

   Getting Started
   HelloWorld
   HeadlessCompute
   AutoSPIRV
   Framework









