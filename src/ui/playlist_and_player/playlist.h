#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <fstream>
#include <iostream>
#include <QUrl>
#include <QVector>
#include <QObject>
#include <QFileInfo>
#include <QStandardPaths>

#include "playlist_item.h"
#include <song.h>
#include "logger.h"
#include "file_system_utils.h"

class Playlist : public QObject
{
    Q_OBJECT
public:
    Playlist();

    void add_playlist_item(Song *song);
    void delete_song_from_playlist(int index);
    void move_songs_to(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index);
    Playlist_item* find_playlist_item_by_name(std::string file_name);
    int find_index_of_playlist_item_by_name(std::string file_name);
    void set_directory(std::string _directory_path);
    void set_m3u_file_name(std::string _m3u_file_name);
    std::string name_of_m3u_file();
    void delete_current_playlist();
    bool read_m3u_file(std::string file_to_read);
    bool write_m3u_file();
    Playlist_item* playlist_item_at(int index);
    Song* next_song();
    Song* previous_song();
    int playlist_length();
    Playlist_item* get_current_song();
    void set_index_current_song(int index_current_song);

public slots:
    void slot_lightshow_of_pls_item_ready(std::string song_name);
    void slot_title_or_artist_has_changed(int playlist_index, bool string_is_title, std::string title_or_artist);
    void slot_lightshow_for_song_is_ready(Song *song);

signals:
    void current_media_index_changed(int current_media_index);
    void current_rtl_m3u_read_and_inserted(int start, int end);

private:
    std::vector<Playlist_item *> playlist;
    std::string directory_path;
    std::string m3u_file_name;
    int index_current_song;

    inline bool file_exists(const std::string& path);


};

#endif // PLAYLIST_H
