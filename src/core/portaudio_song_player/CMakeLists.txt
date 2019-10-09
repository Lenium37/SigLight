add_library(portaudio_song_player STATIC
        port_audio_song_player.h
        port_audio_song_player.cpp)

target_include_directories(portaudio_song_player PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

# make portaudio headers avaible
target_include_directories(portaudio_song_player PUBLIC ../../../external_libraries/portaudio/install/include)
target_link_directories(portaudio_song_player PUBLIC ../../../external_libraries/portaudio/install/lib)

target_link_libraries(portaudio_song_player portaudio)
target_link_libraries(portaudio_song_player CORE_LIB)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL armv7l)
    target_link_libraries(portaudio_song_player asound)
endif ()

if (APPLE)
    # on Mac OS we have to link against these system libraries
    target_link_libraries(portaudio_song_player "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio")
endif ()