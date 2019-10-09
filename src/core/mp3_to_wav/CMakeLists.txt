add_library(mp3_to_wav STATIC mp_3_to_wav_converter.h mp_3_to_wav_converter.cpp)
target_link_libraries(mp3_to_wav song_tag_utils)
target_include_directories(mp3_to_wav PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mp3_to_wav_exec mp3_to_wav_exec.cpp)
target_link_libraries(mp3_to_wav_exec mp3_to_wav)