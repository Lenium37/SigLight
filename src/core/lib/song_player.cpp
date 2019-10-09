//
// Created by Jan on 20.06.2019.
//

#include "song_player.h"
Song SongPlayer::get_current_song() const {
  return current_song;
}

SongPlayer::PlayerStatus SongPlayer::get_player_status() const {
  return player_status;
}
SongPlayer::SongPlayer(std::function<void(Song)> song_changed_callback,
                       std::function<void(SongPlayer::PlayerStatus)> status_changed_callback,
                       std::function<void(long long int position)> position_changed_callback)
    : song_changed_callback(song_changed_callback),
      status_changed_callback(status_changed_callback),
      position_changed_callback(position_changed_callback) {}

void SongPlayer::set_player_status(SongPlayer::PlayerStatus playerStatus) {
  player_status = playerStatus;
  status_changed_callback(playerStatus);
}





