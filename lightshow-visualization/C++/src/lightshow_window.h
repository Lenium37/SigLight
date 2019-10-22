//
// Created by Johannes on 22.10.2019.
//

#ifndef LIGHTSHOW_VISUALIZER_SRC_LIGHTSHOW_WINDOW_H_
#define LIGHTSHOW_VISUALIZER_SRC_LIGHTSHOW_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>

class LightshowWindow : public QMainWindow {
  Q_OBJECT

 public:
  LightshowWindow();
  ~LightshowWindow();

  int init();
  void update_fixtures(qint64 timestamp);

 private:
  QLabel* fix_bass;
  QLabel* fix_mid;
  QLabel* fix_high;
  int channel_index_bass;
  int channel_index_mid;
  int channel_index_high;
  std::vector<std::vector<std::uint8_t>> all_channel_values;
};

#endif //LIGHTSHOW_VISUALIZER_SRC_LIGHTSHOW_WINDOW_H_
