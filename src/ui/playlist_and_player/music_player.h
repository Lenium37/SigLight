#ifndef RASPITOLIGHT_SRC_UI_PLAYLIST_AND_PLAYER_MUSIC_PLAYER_H_
#define RASPITOLIGHT_SRC_UI_PLAYLIST_AND_PLAYER_MUSIC_PLAYER_H_

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QVector>
#include <QStringList>
#include <QVectorIterator>
#include <QTime>
#include <song.h>
#include <song_player.h>
#include <song_player_factory.h>

#include "playlist.h"
#include "file_system_utils_qt.h"

class MusicPlayer : public QObject {
 Q_OBJECT

 public:
  MusicPlayer();

  void set_songs_directory_path(std::string _songs_directory_path);
  Song* add_to_playlist(QUrl url);
  QString get_formatted_time() const;
  QString format_time(qint64 time_milli_seconds) const;
  Playlist_item *get_playlist_media_at(int index);
  void next_song();
  void previous_song();
  void play_song();
  void pause_song();
  void stop_song();
  int playlist_index();
  void set_playlist_index(int index);
  bool is_media_playing();
  void set_media_to(int index);
  void save_playlist(std::string directory_to_save_playlist);
  void read_own_m_3_u_on_startup(std::string standard_directory_to_read_playlist);
  Song *get_current_song();
  int get_playlist_length();
  int volume();
  void set_volume(int volume);
  int playlist_index_for(Song *song);
  void delete_song_from_playlist(int index);
  int get_last_pls_index();
  double get_song_time();

 public slots:
  // Changing Position in the song has been shut down, because of time reasons
  // void slot_header_item_double_clicked(int x_pos);
  void slot_current_playlist_index_changed(int _current_playlist_index);
  void slot_media_inserted_into_pls_on_startup(int start_index, int end_index);
  void slot_title_or_artist_of_song_has_changed(int pls_index, bool is_title, std::string title_or_artist);
  void slot_lightshow_for_song_is_ready(Song *song);
  void slot_order_playlist_changed(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index);

 signals:
  void media_inserted_into_playlist(int start, int end);
  void media_inserted_into_playlist_on_startup(int start, int end);
  void title_or_artist_of_song_has_changed(int pls_index, bool is_title, std::string title_or_artist);
  void lightshow_for_song_is_ready(Song *song);
  void position_changed(qint64 position);

 private:
  Playlist *pls;
  int current_playlist_index;
  bool media_is_playing;
  std::string songs_directory_path;
  // QMediaPlayer *q_media_player;
  std::shared_ptr<SongPlayer> song_player;

  bool is_playlist(const QUrl &playlist_path);
  const std::string change_and_copy_file(const QUrl _file_path);

  void copy_wav(const QUrl &_file_url, const std::string &copied_file_url) const;
  void convert_to_wav(std::string from, std::string to);
};

#endif //RASPITOLIGHT_SRC_UI_PLAYLIST_AND_PLAYER_MUSIC_PLAYER_H_
