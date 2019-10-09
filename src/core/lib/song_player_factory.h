#include <memory>

//
// Created by Jan on 21.06.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_SONG_PLAYER_FACTORY_H_
#define RASPITOLIGHT_SRC_UI_SONG_PLAYER_FACTORY_H_

#include <memory>
#include <song_player.h>
#include <logger.h>

#ifdef USE_PORT_AUDIO_SONG_PLAYER
#include "port_audio_song_player.h"
#else
#include <q_media_song_player.h>
#endif

class SongPlayerFactory {
 public:
  static std::shared_ptr<SongPlayer> create_song_player(std::function<void(Song)> song_changed_callback,
                                                        std::function<void(SongPlayer::PlayerStatus)> status_changed_callback,
                                                        std::function<void(long long int)> position_changed_callback) {

#ifdef USE_PORT_AUDIO_SONG_PLAYER
    Logger::info("using PortAudioSongPlayer");
  return std::make_shared<PortAudioSongPlayer>(song_changed_callback, status_changed_callback, position_changed_callback);
#else
    Logger::info("using QMediaSongPlayer");
    return std::make_shared<QMediaSongPlayer>(song_changed_callback,
                                              status_changed_callback,
                                              position_changed_callback);
#endif

  };
};

#endif //RASPITOLIGHT_SRC_UI_SONG_PLAYER_FACTORY_H_
