execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions( -DGIT_BRANCH_NAME="${GIT_BRANCH_NAME}" )
message("Building on Git branch ${GIT_BRANCH_NAME}")

execute_process(
        COMMAND  git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions( -DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}" )
message("Building commit with hash ${GIT_COMMIT_HASH}")

add_definitions(-DUSE_FFTW)

set(CORE_LIB_SRCS
        logger.h
        logger.cpp
        dmx_device.h
        dmx_device.cpp
        dmx_device_k_8062.h
        dmx_device_k_8062.cpp
        file_system_utils.h
        song.cpp
        song.h
        song_player.h
        song_player.cpp
        #port_audio_song_player.cpp
        #port_audio_song_player.h
        stacktrace_registration.h
        stacktrace_registration.cpp
        version.h
        version.cpp
        dmx_device_eurolite_pro.h
        dmx_device_eurolite_pro.cpp

        ../../../external_libraries/gist/source/Gist/src/gist.h
        ../../../external_libraries/gist/source/Gist/src/gist.cpp
        ../../../external_libraries/gist/source/Gist/src/core/CoreTimeDomainFeatures.h
        ../../../external_libraries/gist/source/Gist/src/core/CoreTimeDomainFeatures.cpp
        ../../../external_libraries/gist/source/Gist/src/core/CoreFrequencyDomainFeatures.h
        ../../../external_libraries/gist/source/Gist/src/core/CoreFrequencyDomainFeatures.cpp
        ../../../external_libraries/gist/source/Gist/src/fft/AccelerateFFT.h
        ../../../external_libraries/gist/source/Gist/src/fft/AccelerateFFT.cpp
        ../../../external_libraries/gist/source/Gist/src/fft/WindowFunctions.h
        ../../../external_libraries/gist/source/Gist/src/fft/WindowFunctions.cpp
        ../../../external_libraries/gist/source/Gist/src/mfcc/MFCC.h
        ../../../external_libraries/gist/source/Gist/src/mfcc/MFCC.cpp
        ../../../external_libraries/gist/source/Gist/src/onset-detection-functions/OnsetDetectionFunction.h
        ../../../external_libraries/gist/source/Gist/src/onset-detection-functions/OnsetDetectionFunction.cpp
        ../../../external_libraries/gist/source/Gist/src/pitch/Yin.h
        ../../../external_libraries/gist/source/Gist/src/pitch/Yin.cpp




        )

include(lightshow/CMakeLists.txt)

set(CORE_LIBRARY_NAME CORE_LIB)

add_library(${CORE_LIBRARY_NAME} STATIC ${CORE_LIB_SRCS})

# Specify here the include directories exported by this library
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../../../externals/spdlog
        ../../../externals/fmt
        ../../../externals/ghc
        ../../../externals/optional-lite
        )

set(LIB_FOLDER lib)
if (EXISTS ../../../external_libraries/libsndfile/install/lib64)
    set(LIB_FOLDER lib64)
endif ()

# make libsndfile avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libsndfile/install/include)
target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libsndfile/install/${LIB_FOLDER})
target_link_libraries(${CORE_LIBRARY_NAME} sndfile)

# make fftw3 headers avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/fftw3/install/include)
target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/fftw3/install/${LIB_FOLDER})
target_link_libraries(${CORE_LIBRARY_NAME} fftw3)

# make libusb headers avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libusb/install/include)
target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libusb/install/lib)
target_link_libraries(${CORE_LIBRARY_NAME} usb-1.0)

# make mingw-thread headers avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/mingw_thread/install/include)
# NOTE mingw-thread is header only, so no linking is required

target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/fftw3/install/lib)
target_link_libraries(${CORE_LIBRARY_NAME} tinyxml2)

message("linking ${SONG_PLAYER_LIB}")
target_link_libraries(${CORE_LIBRARY_NAME} ${SONG_PLAYER_LIB})

if (${CONAN_FOUND})
    target_link_libraries(CORE_LIB ${CONAN_LIBS})
endif ()

if (BOOST_STACK_TRACE_AVAIBLE)
    target_link_libraries(CORE_LIB dl)
endif ()

# make lame avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libmp3lame-CMAKE/source/libmp3lame-CMAKE/include)
target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/libmp3lame-CMAKE/install/lib)
target_link_libraries(${CORE_LIBRARY_NAME} mp3lame)

# make taglib avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/taglib-1.11.1/install/include)
target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/taglib-1.11.1/install/lib)
target_link_libraries(${CORE_LIBRARY_NAME} tag)

# make gist avaible
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/gist/source/Gist/src)

#make essentia and dependencies avaible

# THIS WORKED SOMEHOW
#target_include_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/essentia)
#target_link_directories(${CORE_LIBRARY_NAME} PUBLIC ../../../external_libraries/essentia/build/src/)
#target_link_libraries(${CORE_LIBRARY_NAME} essentia)

#target_include_directories(${CORE_LIBRARY_NAME} PUBLIC /usr/local/Cellar/eigen/3.3.7/include/eigen3)


# THIS DIDN'T WORK AT ALL
#add_library(essentia STATIC IMPORTED)
#set_target_properties(essentia PROPERTIES
        #IMPORTED_LOCATION ../../../external_libraries/essentia/build/src/libessentia.a)


