//
// Created by Jan on 20.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_SONG_PLAYER_H_
#define RASPITOLIGHT_SRC_CORE_LIB_SONG_PLAYER_H_

#include "song.h"
#include <vector>
#include <functional>

class SongPlayer {
 public:
  enum class PlayerStatus {
    PLAYING,
    PAUSED,
    STOPPED
  };
  SongPlayer(std::function<void(Song)> song_changed_callback,
             std::function<void(PlayerStatus)> status_changed_callback,
             std::function<void(long long int position)> position_changed_callback);

  Song get_current_song() const;
  /**
   * Sets the current song to the given song
   * song_changed_callback will be called
   * all resources allocated for the old current_song (if present) will be released
   * @param current_song
   */
  virtual void set_current_song(Song current_song) = 0;
  PlayerStatus get_player_status() const;

  /**
   * Plays the current song. Allocates needed resources
   */
  virtual void play() = 0;
  /**
   * Stops playing the song and resets playhead to position 0. Does not release any resources
   */
  virtual void stop() = 0;
  /**
   * Stops playing the song, playhead position remains. Does not release any resources
   */
  virtual void pause() = 0;

  virtual long long int current_song_duration() = 0;

  void set_player_status(PlayerStatus playerStatus);

  virtual void set_volume(int volume) = 0;

  virtual int get_volume() = 0;

  virtual void set_notify_interval(int interval_ms) = 0;

  virtual double get_song_time() = 0;

  virtual ~SongPlayer() = default;

 protected:
  Song current_song; // todo use an optional
  std::function<void(Song)> song_changed_callback;
  std::function<void(PlayerStatus)> status_changed_callback;
  std::function<void(int)> position_changed_callback;
  // todo position callback
  PlayerStatus player_status = PlayerStatus::STOPPED;

};

#endif //RASPITOLIGHT_SRC_CORE_LIB_SONG_PLAYER_H_
