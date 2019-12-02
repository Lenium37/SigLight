//
// Created by Jan on 20.06.2019.
//

#include <logger.h>
#include "q_media_song_player.h"

QMediaSongPlayer::QMediaSongPlayer(std::function<void(Song)> song_changed_callback,
                                   std::function<void(SongPlayer::PlayerStatus)> status_changed_callback,
                                   std::function<void(long long int position)> position_changed_callback) : SongPlayer(
    song_changed_callback,
    status_changed_callback,
    position_changed_callback) {
  q_media_player = new QMediaPlayer();
  connect(q_media_player, &QMediaPlayer::positionChanged, this, [this](long long int pos) {
    this->position_changed_callback(pos);
  });
  this->set_volume(50);
}
void QMediaSongPlayer::set_current_song(Song cur_song) {
  current_song = cur_song;
  song_changed_callback(current_song);
  q_media_player->setMedia(QUrl::fromLocalFile(QString::fromStdString(current_song.get_file_path())));
}
void QMediaSongPlayer::play() {
  q_media_player->play();
  player_status = PlayerStatus::PLAYING;
}
void QMediaSongPlayer::stop() {
  q_media_player->stop();
  player_status = PlayerStatus::STOPPED;
}
void QMediaSongPlayer::pause() {
  q_media_player->pause();
  player_status = PlayerStatus::PAUSED;
}
long long int QMediaSongPlayer::current_song_duration() {
  return q_media_player->duration();
}
void QMediaSongPlayer::set_volume(int volume) {
  q_media_player->setVolume(volume);
}
int QMediaSongPlayer::get_volume() {
  return q_media_player->volume();
}
void QMediaSongPlayer::set_notify_interval(int interval_ms) {
  q_media_player->setNotifyInterval(interval_ms);
  Logger::debug("set QMediaPlayer NotifyInterval");
}
double QMediaSongPlayer::get_song_time() {
  Logger::critical("QMediaSongPlayer::get_song_time NOT IMPLEMENTED YET!!!");
  return 0;
}