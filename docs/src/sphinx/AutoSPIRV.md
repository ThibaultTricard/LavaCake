# AutoSPIRV

AutoSPIRV is a set Cmake macro that allows to register your GLSL shader 
and automatically compile them to SPIRV before build time.
Shaders will be compiled only if it has been modified since the last compilation.

This set contain three macros: 

- clearShader(): reset the register of shader to compile.
- addShader(glslShader sprivShader spirvVersion): register a GLSL shader, the desired path to spirv shader and the verion of spirv to use.

  exemple : 
    addShader(path/to/shader.frag other/path/to/shader.frag.spv spv1.3)

- AutoSPIRV(TARGET): add rules to compile all currently registerd shader before building of the target.

