//
// Created by Johannes on 08.08.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_PLAYER_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_PLAYER_H_

#include <dmx_device.h>
//#include <dmx_device_k_8062.h>
#include <playlist_and_player/music_player.h>
#include "lightshow/lightshow.h"
#include <unistd.h>


class LightshowPlayer: public QObject {
 Q_OBJECT


 public:
  LightshowPlayer(DmxDevice &device);
  ~LightshowPlayer();

  void play_lightshow(Lightshow *lightshow, DmxDevice &device, bool &lightshow_playing, bool &lightshow_paused, MusicPlayer *player);
  void set_lightshow(Lightshow *_lightshow);

 public slots:
  void send_new_dmx_data(qint64 position);

 private:
  Lightshow *current_lightshow;
  std::vector<std::vector<std::uint8_t>> all_channel_values;
  DmxDevice &device;
  int channel_count;
};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_PLAYER_H_
