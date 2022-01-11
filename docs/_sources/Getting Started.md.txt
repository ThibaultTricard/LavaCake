# Getting Started
## Setting up your project 

The first step to use Lavacake is to dowload by cloning it\
in your project folder and initialising it's submodules.

```
git clone https://github.com/ThibaultTricard/LavaCake.git
cd LavaCake
git checkout dev-1.3
git pull
git submodule update --init --recursive
```

## CMake

The next step is to include LavaCake in your CMakeLists.txt\
A typical CMakeLists.txt for a LavaCake project look like this.

```
cmake_minimum_required (VERSION 3.0)
project( "MyLavaCakeProject" )


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Relative/Path/to/LavaCake)

add_executable(MyLavaCakeProject main.cpp)
target_link_libraries(MyLavaCakeProject LavaCake)
```

If you are using LavaCake you are probably using some shader,\
If you are coding them in glsl you might want to keep their\
spirv version up to date.\
To do so you can use the addShader and AutoSPIRV macro that come
with the framework.

```
cmake_minimum_required (VERSION 3.0)
project( "MyLavaCakeProject" )


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Relative/Path/to/LavaCake)

addShader(path/to/vertshader.vert path/to/vertshader.vert.spv)
addShader(path/to/fragshader.frag path/to/fragshader.frag.spv)

add_executable(MyLavaCakeProject main.cpp)
AutoSPIRV(MyLavaCakeProject)
target_link_libraries(MyLavaCakeProject LavaCake)
```

For more detail on these macro [here](https://github.com/ThibaultTricard/LavaCake/wiki/AutoSPIRV)

