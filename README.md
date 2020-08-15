# GlslCompiler
Provide a method to compiler and parse glsl shader both which could be used in vulkan
## Folders description
### _bin_
compiled binary files(include .lib and .dll) for **release | x64 | vs2017**.
### _src_
all the source code, if using the bin/*, only the header file is neccessary.
### _setting_
the project setting files in vs2017, make sure the vulkan and glslang installed.
### _dependency_
the header files have been used, all these come from glslang. 
## Use method
include GlslCompiler.h, add lib file into project, put dll file into the folder where executable file exist.
## Dependency
This project completely depend on glslang, and make sure vulkan is installed which contain glslang. If vulkan is usable, the VULKAN_SDK enviroment variable is avaliable.

Then get in vulkan folder. 
``` command lines
cd glslang
md build
cd build
cmake -G "Visual Studio 2017 Win64" .. -DCMAKE_INSTALL_PREFIX="install"
cmake --build . --config Release --target install
```
See detail in glslang' github or its README.md.