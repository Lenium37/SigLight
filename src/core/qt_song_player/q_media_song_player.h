//
// Created by Jan on 20.06.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_Q_MEDIA_SONG_PLAYER_H_
#define RASPITOLIGHT_SRC_UI_Q_MEDIA_SONG_PLAYER_H_

#include <QtMultimedia/QMediaPlayer>
#include "song_player.h"
#include <QObject>

class QMediaSongPlayer : private QObject, public SongPlayer {
 Q_OBJECT
 public:
  QMediaSongPlayer(std::function<void(Song)> song_changed_callback,
                   std::function<void(SongPlayer::PlayerStatus)> status_changed_callback,
                   std::function<void(long long int position)> position_changed_callback);

  void set_current_song(Song cur_song) override;
  void play() override;
  void pause() override;
  void stop() override;
  void set_volume(int volume) override;
  int get_volume() override;
  void set_notify_interval(int interval_ms) override;
  long long int current_song_duration() override;
  double get_song_time() override;
 private:
  QMediaPlayer *q_media_player = nullptr;
};

#endif //RASPITOLIGHT_SRC_UI_Q_MEDIA_SONG_PLAYER_H_
