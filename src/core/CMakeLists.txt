add_subdirectory(tinyxml2)
add_subdirectory(lib)
add_subdirectory(song_tag_utils)
add_subdirectory(mp3_to_wav)

if (${USE_PORT_AUDIO_SONG_PLAYER})
    message("adding PortAudio Song player lib")
    add_subdirectory(portaudio_song_player)
#
#    add_executable(portaudio-example
#            portaudio_example.cpp
#            )
#    target_link_libraries(portaudio-example CORE_LIB portaudio)

elseif (${USE_Q_MEDIA_SONG_PLAYER})
    message("adding QMedia Song player lib")
    add_subdirectory(qt_song_player)
endif ()


#set(Main_SRCS
#        main.cpp
#        )

#set(CORE_EXECUTABLE_NAME ${PROJECT_NAME}_core_test)

#add_executable(${CORE_EXECUTABLE_NAME}
#        ${Main_SRCS}
#        )

#add_executable(main_dmx_device
#        main_dmx_device.cpp
#        )

add_executable(analysis_only analysis_only.cpp analysis_only.cpp)
target_link_libraries(analysis_only CORE_LIB)

#add_executable(crash_demo crash_demo.cpp)

#target_link_libraries(crash_demo CORE_LIB)


#target_link_libraries(${CORE_EXECUTABLE_NAME} CORE_LIB)
#target_link_libraries(main_dmx_device CORE_LIB)

#add_executable(song_player song_player.cpp)
#target_link_libraries(song_player CORE_LIB)

#message("linking ${SONG_PLAYER_LIB}")
#target_link_libraries(song_player ${SONG_PLAYER_LIB})

#add_executable(lame_test lame_test.cpp)
#target_link_libraries(lame_test CORE_LIB)

