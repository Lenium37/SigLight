# SigLight
SigLight is used to automatically generate Lightshows from .mp3 or .wav files. You can add DMX fixtures, select their current DMX channel and tell SigLight what each fixture should represent. For example a fixture can visualize certain frequency areas (Bass/Mid/High), blink on onsets or just act as an ambient light and change colors, whenever there is a new segment in the song. Auto-Types take it to the next level by automatically selecting the fitting type for each segment - depending on your input if a certain fixture should be more active in the foreground or rather a background light. The whole analysis and generation of the lightshow is then done automatically.

It should run on Windows, Linux and Mac. Even on a Raspberry Pi 4.

## Demos
Some videos we took of generated Lightshows:

 - [Foo Fighters - The Pretender (second half)](https://www.youtube.com/watch?v=HQ3oEwyQBOY)
 - [Rammstein - Pussy](https://www.youtube.com/watch?v=BE-hXpBuePE)
 - [Red Hot Chili Peppers - Californication](https://www.youtube.com/watch?v=vu3IhwIEHVQ)

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

First build the external libraries(fftw3, libsndf and others). E.g. you only need to do this once
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
Your executable is located at: build/src/SigLight.exe

