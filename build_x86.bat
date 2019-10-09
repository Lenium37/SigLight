mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j %NUMBER_OF_PROCESSORS%
cd ..