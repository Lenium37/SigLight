project(SigLight) # accessable with ${PROJECT_NAME}
cmake_minimum_required(VERSION 2.8.11)

option(FORCE_COMPILE_FOR_RASPBERRY "Forces for example usage of portaudio" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
# todo do we need this?
set(FFTW_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/fftw3/install)

# Activate C++ 11 features
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if (COMPILER_SUPPORTS_CXX11)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -pthread")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
    endif ()
elseif (COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
else ()
    # MSVC, On by default (if available)
endif ()


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CONAN_FOUND ${UNIX})

if (${CONAN_FOUND})
    message("conan is avaible")

    message("calling conan..")
    message("adding bincrafters remote")

    execute_process(COMMAND
            conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
            )
    message("install dependencies")
    execute_process(
            COMMAND conan install .. --build missing
            RESULT_VARIABLE CONAN_RESULT
    )

    # If conan is being used, configure CMake to use conan for dependencies.
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
    add_definitions(-DBOOST_STACK_TRACE_AVAIBLE="")
    add_definitions(-DBOOST_STACKTRACE_USE_ADDR2LINE="")
    set(BOOST_STACK_TRACE_AVAIBLE TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else ()
    message("Conan not found. Skipping.")
    set(BOOST_STACK_TRACE_AVAIBLE FALSE)
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL armv7l)
    set(COMPILE_FOR_RASPBERRY TRUE)
    message("compiling for Raspberry Pi")
else ()
    set(COMPILE_FOR_RASPBERRY FALSE)
    message("compiling NOT for Raspberry Pi")
endif ()

if (${FORCE_COMPILE_FOR_RASPBERRY})
    message("Forcing compile for Raspberry")
    set(COMPILE_FOR_RASPBERRY TRUE)
endif ()

if (${COMPILE_FOR_RASPBERRY})
    message("PortAudio will be used")

    add_definitions(-DUSE_PORT_AUDIO_SONG_PLAYER="")
    set(USE_PORT_AUDIO_SONG_PLAYER TRUE)
    set(USE_Q_MEDIA_SONG_PLAYER FALSE)
    set(SONG_PLAYER_LIB portaudio_song_player)
else ()
    message("QMediaSongPlayer will be used")

    add_definitions(-DUSE_Q_MEDIA_SONG_PLAYER="")
    set(USE_PORT_AUDIO_SONG_PLAYER FALSE)
    set(USE_Q_MEDIA_SONG_PLAYER TRUE)
    set(SONG_PLAYER_LIB qt_song_player)
endif ()


set(CMAKE_PREFIX_PATH "C:\\Qt\\5.12.2\\mingw73_64")
add_subdirectory(src)

