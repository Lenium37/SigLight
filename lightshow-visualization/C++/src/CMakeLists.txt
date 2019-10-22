# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
# Find the QXml library
find_package(Qt5Xml REQUIRED)
find_package(Qt5Multimedia CONFIG REQUIRED)

add_subdirectory(tinyxml2)

set(main_headers
        lightshow_window.h
        )

set(main_srcs
        main.cpp
        lightshow_window.cpp
        )


qt5_add_resources(RESOURCE_FILES resources/resources.qrc)

add_executable(${PROJECT_NAME}
        ${main_srcs}
        ${main_headers}
        #${main_ui}
        ${RESOURCE_FILES}
        )


install(TARGETS ${PROJECT_NAME} DESTINATION bin)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Xml)
target_link_libraries(${PROJECT_NAME} Qt5::Multimedia)
target_link_libraries(${PROJECT_NAME} tinyxml2)