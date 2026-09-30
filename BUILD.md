# Build Guide (Win)

## Prerequisites 

### Manual
1. Pylon
2. Boost
3. Cmake
4. Visual Studio or VScode

`Use cmake to configure and Visual Studio / VScode to build`

### Conan
1. conan (e.g. `pip install conan`)
2. Pylon (conan does not have Pylon recipe so need to install it anyway)
3. Cmake

`cd vidre`

`conan profile detect -f`

`conan config install conan/`

`conan install . -u --build=missing -of=../ -pr=windows`

`cd ../build`

`cmake ../vidre/ -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake`

Next step is to build, you can open .sln for Visual Studio or build it via `cmake --build . `

Don't forget to match build_type at each step because libraries might be not found due to mismatch



# Distribute

`cpack --config CPackConfig.cmake -C Release`

### The target machine needs its own Pylon/Aptina libraries installed.