//
// Created by Jan Honsbrok on 2019-06-21.
//

#ifndef RASPITOLIGHT_PORTAUDIOSONGPLAYER_H
#define RASPITOLIGHT_PORTAUDIOSONGPLAYER_H

static const char *const CHUNK_NAME_FMT = "fmt ";
static const char *const CHUNK_NAME_DATA = "data";
#include "song_player.h"
#include <portaudio.h>
#include "nonstd/optional.hpp"
// todo fix naming
struct SongInformation {
  int numChannels;
  int sampleRate;
  uint32_t chunk_len;
  PaSampleFormat sampleFormat;
  int bytesPerSample, bitsPerSample;
  long long int current_song_duration;
};

struct SongResources {
  PaStream *stream = nullptr;
  FILE *wavfile = nullptr; // todo better use a C++ stream here?
  size_t last_played = 0;
  size_t samples_start = 0;
};

class PortAudioSongPlayer : public SongPlayer {
 public:
  PortAudioSongPlayer(std::function<void(Song)> song_changed_callback,
                      std::function<void(SongPlayer::PlayerStatus)> status_changed_callback,
                      std::function<void(long long int position)> position_changed_callback) : SongPlayer(
      song_changed_callback,
      status_changed_callback,
      position_changed_callback) {
  };
  void set_current_song(Song current_song) override;

  void play() override;
  void stop() override;
  void pause() override;
  long long int current_song_duration() override;

  void set_volume(int volume) override;
  int get_volume() override;
  void set_notify_interval(int interval_ms) override;

  static void stream_finished_callback(void *user_data);
  static int pa_stream_callback(
      const void *input, void *output,
      unsigned long frame_count,
      const PaStreamCallbackTimeInfo *time_info,
      PaStreamCallbackFlags status_flags,
      void *user_data);
  double get_song_time() override;
 private:
  nonstd::optional<SongInformation> current_song_information;
  nonstd::optional<SongResources> current_song_resources;
  void read_song_information();
  void open_stream();
  void release_resources();

  void process_fmt_chunk(uint32_t chunk_len);
  void log_data_chunk(uint32_t chunk_len) const;
  void skip_chunk(uint32_t chunk_len) const;
  void initialize_information_and_resources();
  void open_wave_file();
  int volume = 100;
  float volumeMultiplier = 1;
  bool port_audio_initialized = false;
  double start_time = 0;
  void create_song_start_time();
};

#endif //RASPITOLIGHT_PORTAUDIOSONGPLAYER_H
