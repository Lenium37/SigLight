# RaspiToLight
[![pipeline status](https://projectbase.medien.hs-duesseldorf.de/cantes/19ss-raspitolight1/badges/master/pipeline.svg)](https://projectbase.medien.hs-duesseldorf.de/cantes/19ss-raspitolight1/commits/master)

## Building

### Linux & Mac OS
Building an executable for x86 Platform (NO crosscompiling for Raspberry, executable can run on your local PC).

First build the external libraries(fftw3, libsndf). E.g. you only need to do this once
```shell
python build_external_libraries.py
```

You can use the build_x86.sh build script or run the steps manually:
```shell
mkdir build
cd build
cmake ..
make
```
Your executable is located at: build/src/raspitolight

Optional: Speedup building by providing a number of core to use:
```shell
make -j <num of cores>
```
### Windows
Building an executable for x86 Platform (NO crosscompiling for Raspberry, executable can run on your local PC)
You need to install cmake, MinGW64 and Python, 7Zip to build the external libraries.

First build the external libraries(fftw3, libsndf). E.g. you only need to do this once
```shell
python build_external_libraries.py
```

Make sure to add Qt location to `CMAKE_PREFIX_PATH`

You can now use the build_x86.bat build script or run the steps manually:
```shell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j %NUMBER_OF_PROCESSORS%
```
Your executable is located at: build/src/raspitolight.exe

### Building the docs
```
doxygen doxygen_config
```
You need doxygen installed.
Mac Os:
```
brew install doxygen
```
Linux
```
sudo apt-get install doxygen
```


