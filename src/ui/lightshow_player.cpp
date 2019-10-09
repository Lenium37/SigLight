//
// Created by Johannes on 08.08.2019.
//

#include "lightshow_player.h"

LightshowPlayer::LightshowPlayer(DmxDevice &device) : device(device) {

}

LightshowPlayer::~LightshowPlayer() {

}

void LightshowPlayer::send_new_dmx_data(qint64 position) {
  //Logger::debug("sending new dmx data, position: {}", position);

  if(position/25 == 0)
    this->device.write_data(all_channel_values.front(), this->channel_count); // first frame
  else if(position/25 + 1 < all_channel_values.size())
    this->device.write_data(all_channel_values[position/25 + 1], this->channel_count); // mid song
  else
    this->device.write_data(all_channel_values.back(), this->channel_count); // last frame
}

void LightshowPlayer::play_lightshow(Lightshow *lightshow,
                                     DmxDevice &device,
                                     bool &lightshow_playing,
                                     bool &lightshow_paused,
                                     MusicPlayer *player) {

  Logger::info("playing lightshow for song: {}", lightshow->get_sound_src());

  // prepare DMX values and start DMX device
  std::vector<std::uint8_t> channels(lightshow->get_channel_count());

  //device.start_device();
  device.start_daemon_thread();
  //std::vector<std::vector<std::uint8_t>> all_channel_values = lightshow->read_channel_values();
  this->all_channel_values = lightshow->read_channel_values();
  this->channel_count = lightshow->get_channel_count();

  Logger::debug("Is Player nullptr: {}", player == nullptr);
  player->play_song();

#ifdef USE_PORT_AUDIO_SONG_PLAYER
  /**
   * This timing code is used on Pi as it cannot use QMediaPlayer and due to that cannot rely on the positionChanged-signal.
   * This manual timing is definitely less accurate!
   */
  for (int time = 0; time < all_channel_values.size(); time++) {
    device.write_data(all_channel_values[time], lightshow->get_channel_count());
    //Logger::trace("current song time: {}", player->get_song_time());
    if(player->get_song_time() > 0) {
        time = (player->get_song_time() * 1000) / 25;
        //Logger::trace("Set DMX frame to {}", time);
    }
    if (!lightshow_playing) {
      Logger::debug("stopped copying DMX-Arrays");
      break;
    }
    if (time % 10 == 0) {
      usleep(lightshow->get_wait_time_special()); // vier mal pro Sekunde weniger sleepen.
    } else usleep(lightshow->get_wait_time_standard());
    while (lightshow_paused) {
      if (!lightshow_playing) {
        Logger::debug("stopped copying DMX-Arrays while paused");
        break;
      }
      usleep(500);
    }
  }
  Logger::debug("lightshow finished");
  lightshow_playing = false;
  usleep(50000);
#endif
}

void LightshowPlayer::set_lightshow(Lightshow *_lightshow) {
  this->current_lightshow = _lightshow;
  Logger::debug("Set Lightshow of LightshowPlayer to {}", _lightshow->get_sound_src());
}
