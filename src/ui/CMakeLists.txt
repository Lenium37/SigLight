# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
set(Qt5Widgets_DIR /Users/stevendrewers/Qt/5.12.4/clang_64/lib/cmake/Qt5Widgets)

find_package(Qt5Widgets CONFIG REQUIRED)

set(Main_SRCS
        main.cpp
        mainwindow.cpp
        fixturechoosingdialog.cpp
        createfixturedialog.cpp
        volume_button.cpp
        lightshow_generator.cpp
        file_system_utils_qt.cpp
        lightshow_player.cpp
        edit_fixture_dialog.cpp
        )

set(Main_headers
        mainwindow.h
        fixturechoosingdialog.h
        createfixturedialog.h
        volume_button.h
        lightshow_generator.h
        file_system_utils_qt.h
        lightshow_player.h
        edit_fixture_dialog.h
        )

set(Main_ui
        mainwindow.ui
        fixturechoosingdialog.ui
        createfixturedialog.ui
        edit_fixture_dialog.ui
        )

qt5_add_resources(RESOURCE_FILES resources/resources.qrc)

set(UI_EXECUTABLE_NAME ${PROJECT_NAME}_gui)

include(fixturemanager/CMakeLists.txt)
include(player_edit_view/CMakeLists.txt)
include(playlist_and_player/CMakeLists.txt)
include(playlist_view/CMakeLists.txt)


add_executable(${UI_EXECUTABLE_NAME}
        ${Main_SRCS}
        ${Main_headers}
        ${Main_ui}
        ${RESOURCE_FILES}
        )


install(TARGETS ${UI_EXECUTABLE_NAME} DESTINATION bin)


target_link_libraries(${UI_EXECUTABLE_NAME} Qt5::Widgets)
target_link_libraries(${UI_EXECUTABLE_NAME} CORE_LIB song_tag_utils mp3_to_wav)

if (NOT ${COMPILE_FOR_RASPBERRY})
    set(Qt5Multimedia_DIR /Users/stevendrewers/Qt/5.12.4/clang_64/lib/cmake/Qt5Multimedia)

    find_package(Qt5Multimedia CONFIG REQUIRED)
    target_link_libraries(${UI_EXECUTABLE_NAME} Qt5::Multimedia)
endif ()