//
// Created by Johannes on 05.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHTSHOW_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHTSHOW_H_

//#include <unistd.h>
#include "lightshow/lightshow_fixture.h"
#include <vector>
#include <dmx_device.h>
#include "lightshow/box_fir.h"
#include <logger.h>
#include <ctime>
#include <iostream>
#include <chrono>
#include <unistd.h>

struct color_change {
  float timestamp;
  std::string color;
};

class Lightshow {
 public:
  Lightshow();
  ~Lightshow();

  std::string get_sound_src();
  std::vector<LightshowFixture> get_fixtures();
  std::vector<LightshowFixture> &get_fixtures_reference();

  void set_sound_src(std::string sound_src);
  void add_fixture(LightshowFixture fixture);
  void add_multiple_fixtures(std::initializer_list<LightshowFixture> a_args);
  void empty_fixtures();
  void set_length(int length);
  int get_length();
  int get_channel_count();
  void set_channel_count(int channel_count);

  std::vector<std::vector<std::uint8_t>> read_channel_values();

  void prepare_analysis_for_song(char *song_path, bool need_bass, bool need_mid, bool need_high, bool need_onsets, int user_bpm, float onset_value, int onset_bass_lower_frequency, int onset_bass_upper_frequency, int onset_bass_threshold, int onset_snare_lower_frequency, int onset_snare_upper_frequency, int onset_snare_threshold, std::vector<float> custom_segments);

  std::vector<LightshowFixture> get_fixtures_bass();
  std::vector<LightshowFixture> get_fixtures_middle();
  std::vector<LightshowFixture> get_fixtures_high();
  std::vector<LightshowFixture> get_fixtures_ambient();

  void add_fixture_bass(LightshowFixture fixture);
  void add_fixture_middle(LightshowFixture fixture);
  void add_fixture_high(LightshowFixture fixture);
  void add_fixture_ambient(LightshowFixture fixture);

  void add_value_change_bass(time_value_int tvi);
  void add_value_change_mid(time_value_int tvi);
  void add_value_change_high(time_value_int tvi);

  std::vector<time_value_int> get_value_changes_bass();
  std::vector<time_value_int> get_value_changes_middle();
  std::vector<time_value_int> get_value_changes_high();

  std::vector<time_value_int> get_combined_values_changes(std::initializer_list<std::vector<time_value_int>> a_args);

  std::vector<time_value_int> clear_outlier_values(std::vector<time_value_int> vector, int time_interval);

  void set_resolution(const int resolution_);
  int get_resolution();

  int get_wait_time_standard();
  int get_wait_time_special();

  std::vector<time_value_float> get_timestamps_segment_changes();
  std::vector<double> get_all_beats();
  std::vector<float> get_specific_beats(std::string beat_type, float start = 0, float end = 0);
  std::vector<float> get_onset_timestamps();
  std::vector<float> get_onset_bass_timestamps();
  std::vector<float> get_onset_snare_timestamps();
  std::vector<float> get_onset_timestamps_in_segment(float start, float end);
  int get_bpm();
  void set_bpm(int _bpm);

  float get_onset_value();
  void set_onset_value(float _onset_value);

  int get_onset_bass_lower_frequency();
  void set_onset_bass_lower_frequency(int _onset_bass_lower_frequency);
  int get_onset_bass_upper_frequency();
  void set_onset_bass_upper_frequency(int _onset_bass_upper_frequency);
  int get_onset_bass_threshold();
  void set_onset_bass_threshold(int _onset_bass_threshold);
  int get_onset_snare_lower_frequency();
  void set_onset_snare_lower_frequency(int _onset_snare_lower_frequency);
  int get_onset_snare_upper_frequency();
  void set_onset_snare_upper_frequency(int _onset_snare_upper_frequency);
  int get_onset_snare_threshold();
  void set_onset_snare_threshold(int _onset_snare_threshold);

  std::vector<float> get_custom_segments();
  void set_custom_segments(std::vector<float> _custom_segments);

  double get_seed();

 private:
  std::string sound_src;
  std::vector<LightshowFixture> fixtures;
  std::vector<LightshowFixture> fixtures_bass;
  std::vector<LightshowFixture> fixtures_middle;
  std::vector<LightshowFixture> fixtures_high;
  std::vector<LightshowFixture> fixtures_ambient;
  int length;
  int channel_count;
  void update_channel_count();
  Analysis analysis;
  std::vector<time_value_int> value_changes_bass;
  std::vector<time_value_int> value_changes_middle;
  std::vector<time_value_int> value_changes_high;
  std::vector<time_value_float> timestamps_segment_changes;
  std::vector<float> onset_timestamps;
  std::vector<float> onset_bass_timestamps;
  std::vector<float> onset_snare_timestamps;
  std::vector<float> custom_segments;

  int resolution = 40; // 40 = DMX update every 25ms.    20 = DMX update every 50ms.    10 = DMX update every 100ms.
  float fade_duration = 1; // fade of color changes takes 1s

  int wait_time_standard = 25000;
  int wait_time_special = 20800;
  int bpm = 0;
  std::vector<float> timestamps_of_all_beats;
  std::vector<time_value_float> timestamps_colorchanges_ambient;
  std::vector<double> all_beats;
  std::vector<float> beats_1_2_3_4;
  std::vector<float> beats_2_4;
  std::vector<float> beats_1_3;
  std::vector<float> beats_1;
  std::vector<float> beats_2;
  std::vector<float> beats_3;
  std::vector<float> beats_4;
  std::vector<float> beats_1_every_other_bar;

  void generalize_values(std::vector<time_value_int> &v,
                         std::uint8_t lower_border,
                         std::uint8_t upper_border,
                         std::uint8_t value);
  void get_bpm_and_beats(bool &finished, int user_bpm, std::vector<float> _custom_segments);
  double first_beat = 0;
  float onset_value = 9;
  int onset_bass_lower_frequency = 30;
  int onset_bass_upper_frequency = 65;
  int onset_bass_threshold = 160;
  int onset_snare_lower_frequency = 150;
  int onset_snare_upper_frequency = 300;
  int onset_snare_threshold = 175;

  void prepare_beat_timestamps();

  double seed;

};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHTSHOW_H_
