//
// Created by Jan Honsbrok on 2019-06-21.
//

#include "port_audio_song_player.h"
#include "logger.h"
#include <unistd.h>
#include <iostream>

// todo find a better way to check and fail than this macro -> assertions
#define CHECK(x) { if(!(x)) { \
Logger::error("{}:{}: failure at: {}", __FILE__, __LINE__, #x); \
_exit(1); } }

template<typename T>
T fread_num(FILE *f) {
  T value;
  CHECK(fread(&value, sizeof(value), 1, f) == 1);
  return value; // no endian-swap for now... WAV is LE anyway...
}

std::string fread_str(FILE *f, size_t len) {
  std::string s(len, '\0');
  CHECK(fread(&s[0], 1, len, f) == len);
  return s;
}

void PortAudioSongPlayer::set_current_song(Song cur_song) {
  if (current_song_resources) {
    Pa_StopStream(current_song_resources->stream);
  }

  current_song = cur_song;
  if (!current_song.is_empty()) {
    read_song_information();
  }
  song_changed_callback(current_song);
}

void PortAudioSongPlayer::play() {
  Logger::info("song path: {}", current_song.get_file_path());
  if (!current_song.is_empty()) {
    open_stream();
    // todo set stream active
    set_player_status(PlayerStatus::PLAYING);
  } else {
    Logger::error("Can not play, we dont have a current song!");
  }

}

void PortAudioSongPlayer::read_song_information() {
  initialize_information_and_resources();
  open_wave_file();

  CHECK(fread_str((*current_song_resources).wavfile, 4) == "RIFF"); // TODO fail if not wave file

  uint32_t wavechunksize = fread_num<uint32_t>((*current_song_resources).wavfile); // todo: not needed anymore?
  CHECK(fread_str((*current_song_resources).wavfile, 4) == "WAVE");

  while (true) {
    std::string chunk_name = fread_str((*current_song_resources).wavfile, 4);
    uint32_t chunkLen = fread_num<uint32_t>((*current_song_resources).wavfile);
    current_song_information->chunk_len = chunkLen;
    if (chunk_name == CHUNK_NAME_FMT)
      process_fmt_chunk(chunkLen);
    else if (chunk_name == CHUNK_NAME_DATA) {
      log_data_chunk(chunkLen);
      break;
    } else {
      skip_chunk(chunkLen);
    }
  }
  current_song_resources->samples_start = ftell(current_song_resources->wavfile);
  current_song_resources->last_played = current_song_resources->samples_start;
}

void PortAudioSongPlayer::initialize_information_and_resources() {
  if (current_song_resources) {
    if (current_song_resources->stream) {
      Logger::info("Closing existing stream");
      Pa_CloseStream(current_song_resources->stream);
    }
  }
  current_song_information = nonstd::optional<SongInformation>(SongInformation());
  current_song_resources = nonstd::optional<SongResources>(SongResources());
}

void PortAudioSongPlayer::open_wave_file() {
  current_song_resources->wavfile = fopen(current_song.get_file_path().c_str(), "r");
}

void PortAudioSongPlayer::process_fmt_chunk(uint32_t chunk_len) {
  CHECK(chunk_len >= 16);
  uint16_t fmttag = fread_num<uint16_t>((*current_song_resources).wavfile); // 1: PCM (int). 3: IEEE float
  CHECK(fmttag == 1 || fmttag == 3);

  current_song_information->numChannels = fread_num<uint16_t>((*current_song_resources).wavfile);
  CHECK(current_song_information->numChannels > 0);
  Logger::info("{} channels", current_song_information->numChannels);

  current_song_information->sampleRate = fread_num<uint32_t>((*current_song_resources).wavfile);
  Logger::info("{} Hz", current_song_information->sampleRate);

  uint32_t byte_rate = fread_num<uint32_t>((*current_song_resources).wavfile);
  uint16_t block_align = fread_num<uint16_t>((*current_song_resources).wavfile);

  current_song_information->bitsPerSample = fread_num<uint16_t>((*current_song_resources).wavfile);
  current_song_information->bytesPerSample = current_song_information->bitsPerSample / 8;
  CHECK(byte_rate == current_song_information->sampleRate * current_song_information->numChannels
      * current_song_information->bytesPerSample);
  CHECK(block_align == current_song_information->numChannels * current_song_information->bytesPerSample);

  if (fmttag == 1 /*PCM*/) {
    switch (current_song_information->bitsPerSample) {
      case 8: current_song_information->sampleFormat = paInt8;
        break;
      case 16: current_song_information->sampleFormat = paInt16;
        break;
      case 32: current_song_information->sampleFormat = paInt32;
        break;
      default: CHECK(false);
    }
    Logger::info("PCM {}bit int", current_song_information->bitsPerSample);
  } else {
    CHECK(fmttag == 3 /* IEEE float */); // TODO remove magic number
    CHECK(current_song_information->bitsPerSample == 32);
    current_song_information->sampleFormat = paFloat32;
    Logger::info("32bit float");
  }
  if (chunk_len > 16) {
    uint16_t extended_size = fread_num<uint16_t>((*current_song_resources).wavfile);
    Logger::info("extended size: {}", extended_size);
    CHECK(chunk_len == 18 + extended_size);
    fseek((*current_song_resources).wavfile, extended_size, SEEK_CUR);
  }
}

void PortAudioSongPlayer::log_data_chunk(uint32_t chunk_len) const {
  CHECK(current_song_information->sampleRate != 0);
  CHECK(current_song_information->numChannels > 0);
  CHECK(current_song_information->bytesPerSample > 0);
  Logger::info("len: {} secs",
               double(chunk_len) / current_song_information->sampleRate / current_song_information->numChannels
                   / current_song_information->bytesPerSample);
  Logger::info("len: {} millisecs",
               double(chunk_len) / current_song_information->sampleRate / current_song_information->numChannels
                   / current_song_information->bytesPerSample * 1000);
}

void PortAudioSongPlayer::skip_chunk(uint32_t chunk_len) const {
  CHECK(fseek(current_song_resources->wavfile, chunk_len, SEEK_CUR) == 0);
}

struct CallbackData {
 public:
  CallbackData(SongInformation *song_information, SongResources *song_resources)
      : song_information(song_information), song_resources(song_resources) {}
  SongInformation *song_information;
  SongResources *song_resources;
  PortAudioSongPlayer *port_audio_song_player;
};

int PortAudioSongPlayer::pa_stream_callback(
    const void *input, void *output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo *time_info,
    PaStreamCallbackFlags status_flags,
    void *user_data) {
  CallbackData *callback_data = (CallbackData *) user_data;
  SongInformation *song_information = callback_data->song_information;
  SongResources *song_resources = callback_data->song_resources;

  size_t
      num_read = fread(output,
                       song_information->bytesPerSample * song_information->numChannels,
                       frame_count,
                       song_resources->wavfile);
  unsigned long read_frame_count = frame_count;
  read_frame_count -= num_read;
  size_t last_played_pos = song_resources->last_played;
  song_resources->last_played = ftell(song_resources->wavfile);

  if (read_frame_count > 0) {
    Logger::debug("reachend end of file");
    memset(output, 0, read_frame_count * song_information->numChannels * song_information->bytesPerSample);
    callback_data->port_audio_song_player->position_changed_callback(
        double(song_information->chunk_len) / song_information->sampleRate / song_information->numChannels
            / song_information->bytesPerSample * 1000);
    return paComplete;
  } else {
    int16_t *data = (int16_t *) output;
    for (unsigned int i = 0; i < frame_count * song_information->numChannels; i++) {
      data[i] = data[i] * callback_data->port_audio_song_player->volumeMultiplier;
    }
  }

  //Logger::info("frame count: {}", num_read);
  callback_data->port_audio_song_player->position_changed_callback(
      (song_resources->last_played - song_resources->samples_start) / song_information->numChannels
          / song_information->bytesPerSample / song_information->sampleRate * 1000);

  return paContinue;
}

void PortAudioSongPlayer::stream_finished_callback(void *user_data) {
  CallbackData *callback_data = (CallbackData *) user_data;
  Logger::debug("stream_finished_callback");
  callback_data->port_audio_song_player->set_player_status(PlayerStatus::STOPPED);
  delete callback_data;
}

void PortAudioSongPlayer::open_stream() {
  Logger::info("Opening portaudio stream");

  if (!port_audio_initialized) {
    Logger::debug("initialize portaudio");
    CHECK(Pa_Initialize() == paNoError);
    port_audio_initialized = true;
  } else {
    Logger::debug("portaudio already initialized");
  }

  PaStreamParameters output_parameters;

  output_parameters.device = Pa_GetDefaultOutputDevice();
  CHECK(output_parameters.device != paNoDevice);

  output_parameters.channelCount = current_song_information->numChannels;
  output_parameters.sampleFormat = current_song_information->sampleFormat;
  output_parameters.hostApiSpecificStreamInfo = nullptr;
  output_parameters.suggestedLatency = Pa_GetDeviceInfo(output_parameters.device)->defaultHighOutputLatency;
  Logger::debug("Pa_OpenStream\n");

  CallbackData *data = new CallbackData(&current_song_information.value(), &current_song_resources.value());
  data->port_audio_song_player = this;

  Logger::info("Seeking {}", current_song_resources->last_played);
  fseek(current_song_resources->wavfile, current_song_resources->last_played, SEEK_SET);
  Logger::info("found {}", ftell(current_song_resources->wavfile));

  PaError ret = Pa_OpenStream(
      &current_song_resources->stream,
      nullptr, // no input
      &output_parameters,
      current_song_information->sampleRate,
      paFramesPerBufferUnspecified, // framesPerBuffer
      0, // flags
      &PortAudioSongPlayer::pa_stream_callback,
      data //void *userData
  );
  printf("Pa_OpenStream: %d\n", ret);
  if (ret != paNoError) {
    fprintf(stderr, "Pa_OpenStream failed: (err %i) %s\n", ret, Pa_GetErrorText(ret));
    if (current_song_resources->stream)
      Pa_CloseStream(current_song_resources->stream);
  }

  PaError err =
      Pa_SetStreamFinishedCallback(current_song_resources->stream, &PortAudioSongPlayer::stream_finished_callback);
  Logger::debug("Pa_SetStreamFinishedCallback return: {}", Pa_GetErrorText(err));

  CHECK(Pa_StartStream(current_song_resources->stream) == paNoError);

  create_song_start_time();
}
void PortAudioSongPlayer::create_song_start_time() {
  start_time = Pa_GetStreamTime(current_song_resources->stream);

  for (unsigned int i = 0; i < 1000000; i++) {
    start_time = Pa_GetStreamTime(current_song_resources->stream);
    if (start_time != 0) {
      Logger::debug("Song PaTime start time: {}", start_time);
      return;
    }
  }
  start_time = 0;
}

void PortAudioSongPlayer::stop() {
  // todo stop stream
  auto ret = Pa_StopStream(current_song_resources->stream);
  if (ret != paNoError) {
    fprintf(stderr, "Pa_StopStream failed: (err %i) %s\n", ret, Pa_GetErrorText(ret));
  }
  Pa_CloseStream(current_song_resources->stream);
  current_song_resources->stream = nullptr;
  current_song_resources->last_played = current_song_resources->samples_start;
  // set last played position to 0
}

void PortAudioSongPlayer::pause() {
  // todo stop stream
  auto ret = Pa_StopStream(current_song_resources->stream);
  if (ret != paNoError) {
    fprintf(stderr, "Pa_StopStream failed: (err %i) %s\n", ret, Pa_GetErrorText(ret));
  }
  Pa_CloseStream(current_song_resources->stream);
  current_song_resources->stream = nullptr;
}

void PortAudioSongPlayer::release_resources() {

}

long long int PortAudioSongPlayer::current_song_duration() {
  return double(current_song_information->chunk_len) / current_song_information->sampleRate
      / current_song_information->numChannels
      / current_song_information->bytesPerSample * 1000;
}
void PortAudioSongPlayer::set_volume(int vol) {
  volume = vol;
  volumeMultiplier = static_cast<float>(volume) / 100.0f;
}
int PortAudioSongPlayer::get_volume() {
  return volume;
}

void PortAudioSongPlayer::set_notify_interval(int interval_ms) {
  Logger::debug("using PortAudio, not setting any NotifyInterval");
}
double PortAudioSongPlayer::get_song_time() {
  if (current_song_resources) {
    return Pa_GetStreamTime(current_song_resources->stream) - start_time;
  }
  return 0;
}
