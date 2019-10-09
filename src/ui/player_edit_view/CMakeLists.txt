list(APPEND Main_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/change_lightshow_dialog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/header_mult_tracks.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lightshow_fixture_properties.cpp
        ${CMAKE_CURRENT_LIST_DIR}/multiple_tracks.cpp
        ${CMAKE_CURRENT_LIST_DIR}/music_slider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/player_edit_view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/standard_lightshow.cpp
	${CMAKE_CURRENT_LIST_DIR}/song_label.cpp
     )
list(APPEND Main_headers
        ${CMAKE_CURRENT_LIST_DIR}/change_lightshow_dialog.h
        ${CMAKE_CURRENT_LIST_DIR}/header_mult_tracks.h
	${CMAKE_CURRENT_LIST_DIR}/lightshow_fixture_properties.h
        ${CMAKE_CURRENT_LIST_DIR}/multiple_tracks.h
        ${CMAKE_CURRENT_LIST_DIR}/music_slider.h
        ${CMAKE_CURRENT_LIST_DIR}/player_edit_view.h
        ${CMAKE_CURRENT_LIST_DIR}/standard_lightshow.h
        ${CMAKE_CURRENT_LIST_DIR}/song_label.h
    )
list(APPEND Main_ui
        ${CMAKE_CURRENT_LIST_DIR}/change_lightshow_dialog.ui
)
