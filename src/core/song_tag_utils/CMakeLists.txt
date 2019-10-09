add_library(song_tag_utils STATIC
        song_info_utils.h
        song_info_utils.cpp)


target_include_directories(song_tag_utils PUBLIC ../../../external_libraries/taglib-1.11.1/install/include)
target_link_directories(song_tag_utils PUBLIC ../../../external_libraries/taglib-1.11.1/install/lib)
target_link_libraries(song_tag_utils tag)

target_include_directories(song_tag_utils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../../../externals/ghc)

# Link against zlib
if(${UNIX})
    target_link_libraries(song_tag_utils z)
endif()

add_executable(song_info song_info.cpp)
target_link_libraries(song_info song_tag_utils)

add_executable(song_info_copy song_info_copy.cpp)
target_link_libraries(song_info_copy song_tag_utils)
