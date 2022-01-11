# AutoSPIRV

AutoSPIRV is a set Cmake macro that allows to register your GLSL shader 
and automatically compile them to SPIRV before build time.
Shaders will be compiled only if it has been modified since the last compilation.

This set contain three macro : 

- clearShader() : reset the register of shader to compile.
- addShader(glslShader sprivShader) : register a GLSL shader and the desired path to spirv shader.

  exemple : 
    addShader(path/to/shader.frag other/path/to/shader.frag.spv)

- AutoSPIRV(TARGET) : add rules to compile all currently registerd shader before building of the target.

