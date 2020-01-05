# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtMultimedia library
set(Qt5Multimedia_DIR /Users/stevendrewers/Qt/5.12.4/clang_64/lib/cmake/Qt5Multimedia)
find_package(Qt5Multimedia CONFIG REQUIRED)

add_library(qt_song_player STATIC
        q_media_song_player.h
        q_media_song_player.cpp)

target_include_directories(qt_song_player PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(qt_song_player Qt5::Multimedia)
target_link_libraries(qt_song_player CORE_LIB)