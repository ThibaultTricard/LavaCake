# Welcome to LavaCake!

Lavacake is a Open Source framework that aim to simplify the use of Vulkan in C++ for rappid prototyping.\
This framework provide multiple functions to help the manipulation of usual Vulkan structure\
such as queue, command buffer, render pass, shader module and many more.

LavaCake come with [GLFW](https://github.com/glfw/glfw) and [ImGui](https://github.com/ocornut/imgui) as submodule.\
This project was originally based onto the [Vulkan CookBook's examples](https://github.com/PacktPublishing/Vulkan-Cookbook).

LavaCake is primarily developed by Thibault Tricard for his research work.

# Disclamer

This Framework is made to help research and toy project, and may need to evolve in function of future evolution of Vulkan and of my needs.\
Although I will try not to break retrocompatibility between version of the framework, I canot fully guarantee it completely.

This project is a side project of mine and I don't plan on actively provide support for Lavacake.


Documentation is not very detailled at the moment, it's on the way.

# Building LavaCake

To build Lavacake you need to have a recent VulkanSDK installed.
before building make sure all submodule are up to date.

Then just use CMake

# Compatibility

## Windows
Lavacake was originaly developped with windows and is fully compatible with it.

## Linux
LavaCake should be compatible with Linux with it's 1.2 version that is currently under developpement.\
It is untested at the moment.

## MacOS
LavaCake is compatible with MacOS and Metal in it's 1.2 version through MoltenVK.

# Examples

Multiple exemples of how to use this library is availlable at https://github.com/ThibaultTricard/LavaCake-Examples

# Credits

The core of this library is based on the Vulkan Cookbook code.  

The code repository for [Vulkan Cookbook](https://www.packtpub.com/game-development/vulkan-cookbook?utm_source=github&utm_medium=repository&utm_campaign=9781786468154), published by [Packt](https://www.packtpub.com/).
 
