//
// Created by Jan on 06.06.2019.
//

#include <random>
#include <math.h>
#include "lightshow_generator.h"

#define PI 3.14159265


LightshowGenerator::LightshowGenerator() {

}

std::shared_ptr<Lightshow> LightshowGenerator::generate(int resolution, Song *song, std::shared_ptr<Lightshow> lightshow, int user_bpm) {

  bool need_bass = false;
  bool need_mid = false;
  bool need_high = false;
  bool need_onsets = false;
  //int user_bpm = 0;

  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    std::string fix_type = lightshow->get_fixtures()[i].get_type();
    std::string timestamp_type = lightshow->get_fixtures()[i].get_timestamps_type();
    if(fix_type == "bass" || fix_type == "color_change_beats_action")
      need_bass = true;

    if(fix_type == "mid" || fix_type == "color_change_beats_action")
      need_mid = true;

    if(fix_type == "high")
      need_high = true;

    if(timestamp_type == "Onsets" || timestamp_type == "onsets") {
      need_onsets = true;
    }
  }

  lightshow->set_resolution(resolution);
  lightshow->set_sound_src(song->get_file_path());
  lightshow->prepare_analysis_for_song((char*)song->get_file_path().c_str(), need_bass, need_mid, need_high, need_onsets, user_bpm);

  int fixtures_in_group_one_after_another = 0;
  int fixtures_in_group_one_after_another_back_and_forth = 0;
  int fixtures_in_group_one_after_another_back_and_forth_blink = 0;
  int fixtures_in_group_one_after_another_blink = 0;
  int fixtures_in_group_two_after_another = 0;
  int fixtures_in_group_two_after_another_blink = 0;
  int fixtures_in_group_alternate_odd_even = 0;
  int fixtures_in_group_alternate_odd_even_blink = 0;
  int fixtures_in_group_random_flashes = 0;


  //for (LightshowFixture fix: lightshow->get_fixtures_reference()) {
  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    LightshowFixture &fix = lightshow->get_fixtures_reference()[i];
    std::string fix_type = fix.get_type();
    if (fix_type == "group_one_after_another" && fix.get_position_in_group() > fixtures_in_group_one_after_another)
      fixtures_in_group_one_after_another = fix.get_position_in_group();
    else if(fix_type == "group_one_after_another_back_and_forth" && fix.get_position_in_group() > fixtures_in_group_one_after_another_back_and_forth)
      fixtures_in_group_one_after_another_back_and_forth = fix.get_position_in_group();
    else if(fix_type == "group_one_after_another_back_and_forth_blink" && fix.get_position_in_group() > fixtures_in_group_one_after_another_back_and_forth_blink)
      fixtures_in_group_one_after_another_back_and_forth_blink = fix.get_position_in_group();
    else if (fix_type == "group_one_after_another_blink"
        && fix.get_position_in_group() > fixtures_in_group_one_after_another_blink)
      fixtures_in_group_one_after_another_blink = fix.get_position_in_group();
    else if (fix_type == "group_two_after_another" && fix.get_position_in_group() > fixtures_in_group_two_after_another)
      fixtures_in_group_two_after_another = fix.get_position_in_group();
    else if (fix_type == "group_two_after_another_blink"
        && fix.get_position_in_group() > fixtures_in_group_two_after_another_blink)
      fixtures_in_group_two_after_another_blink = fix.get_position_in_group();
    else if (fix_type == "group_alternate_odd_even"
        && fix.get_position_in_group() > fixtures_in_group_alternate_odd_even)
      fixtures_in_group_alternate_odd_even = fix.get_position_in_group();
    else if (fix_type == "group_alternate_odd_even_blink"
        && fix.get_position_in_group() > fixtures_in_group_alternate_odd_even_blink)
      fixtures_in_group_alternate_odd_even_blink = fix.get_position_in_group();
    else if (fix_type == "group_random_flashes" && fix.get_position_in_group() > fixtures_in_group_random_flashes)
      fixtures_in_group_random_flashes = fix.get_position_in_group();
  }

  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    LightshowFixture & fix = lightshow->get_fixtures_reference()[i];
    std::string fix_type = fix.get_type();
    std::transform(fix_type.begin(), fix_type.end(), fix_type.begin(), ::tolower);

    std::string timestamps_type = fix.get_timestamps_type();
    std::transform(timestamps_type.begin(), timestamps_type.end(), timestamps_type.begin(), ::tolower);
    std::cout << timestamps_type << std::endl;

    std::vector<float> timestamps;
    if(timestamps_type == "onsets")
      timestamps = lightshow->get_onset_timestamps();
    else if(timestamps_type.find("beats") != string::npos) { // contains beats
      std::vector<double> timestamps_double = lightshow->get_all_beats();
      if(timestamps_type == "beats 1/2/3/4" || timestamps_type == "beats 1/2/3/4 action") {
        for(int i = 0; i < timestamps_double.size(); i++)
          timestamps.push_back((float)timestamps_double[i] / 44100);
      } else if(timestamps_type == "beats 2/4" || timestamps_type == "beats 2/4 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if((i + 1) % 2 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 1/3" || timestamps_type == "beats 1/3 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if((i + 1) % 2 == 1)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 1" || timestamps_type == "beats 1 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if(i % 4 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 2" || timestamps_type == "beats 2 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if((i - 1) % 4 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 3" || timestamps_type == "beats 3 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if((i - 2) % 4 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 4" || timestamps_type == "beats 4 action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if((i - 3) % 4 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      } else if(timestamps_type == "beats 1 every other bar" || timestamps_type == "beats 1 every other bar action") {
        for(int i = 0; i < timestamps_double.size(); i++) {
          if(i % 8 == 0)
            timestamps.push_back((float)timestamps_double[i] / 44100);
        }
      }

      if(timestamps_type.find("action") != string::npos) {
        std::vector<time_value_int> bass_values = lightshow->get_value_changes_bass();
        std::vector<time_value_int> mid_values = lightshow->get_value_changes_middle();
        std::vector<float> timestamps_action;
        for (int i = 0; i < timestamps.size(); i++) {
          for (time_value_int &tvi_bass: bass_values) {
            if (tvi_bass.time >= (float) timestamps[i] - 0.02 && tvi_bass.time <= (float) timestamps[i] + 0.02) {
              if (tvi_bass.value > 75)
                timestamps_action.push_back(timestamps[i]);
              break;
            }
          }
          for (time_value_int tvi_mid: mid_values) {
            if (tvi_mid.time >= (float) timestamps[i] - 0.02 && tvi_mid.time <= (float) timestamps[i] + 0.02) {
              if (tvi_mid.value > 175)
                timestamps_action.push_back(timestamps[i]);
              break;
            }
          }
        }
        //timestamps.clear();
        timestamps = timestamps_action;
      }
    }

    if(fix.has_shutter) {
      std::vector<time_value_int> v;
      v.push_back({0.0, 255});
      v.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(),
                   0});
      fix.add_value_changes_to_channel(v, fix.get_channel_shutter());
    }

    if(fix.has_pan && fix.has_tilt) {
      int frequency = lightshow->get_resolution();
      std::vector<time_value_int> tilt_steps;
      std::vector<time_value_int> pan_steps;
      std::vector<time_value_int> vc_tilt;
      std::vector<time_value_int> vc_pan;
      std::vector<time_value_int> vc_shutter;
      std::vector<time_value_int> vc_zoom;
      std::vector<time_value_int> vc_focus;

      float time_step = 0.025;
      int pan_center = 127;
      int tilt_center = 127;

      if(fix.get_modifier_pan() && fix.get_degrees_per_pan())
        pan_center = pan_center + (fix.get_modifier_pan() / fix.get_degrees_per_pan());
      if(fix.get_modifier_tilt() && fix.get_degrees_per_tilt())
        tilt_center = tilt_center + (fix.get_modifier_tilt() / fix.get_degrees_per_tilt());

      std::cout << "pan_center: " << pan_center << std::endl;
      std::cout << "tilt_center: " << tilt_center << std::endl;

      float amplitude_tilt = 30;
      float amplitude_pan = 30;

      float time_of_one_beat = (float) 60 / (float) lightshow->get_bpm();
      float time_of_two_beats = 2.0f * time_of_one_beat;
      float time_of_two_bars = (float) 8 * time_of_one_beat;
      float time_of_four_bars = (float) 16 * time_of_one_beat;

      float time_per_step_tilt = time_of_two_beats; // seconds
      float time_per_step_pan = time_of_two_beats; // seconds
      bool loop = false;

      if(fix.get_moving_head_type() == "Continuous 8") {
        amplitude_tilt = 45 / fix.get_degrees_per_tilt();
        amplitude_pan = 90 / fix.get_degrees_per_pan();
        /* weak 8
        pan_steps.push_back({0.0, pan_tilt_center});
        pan_steps.push_back({1.0, (int) (pan_tilt_center + amplitude_pan)});
        pan_steps.push_back({2.0, pan_tilt_center});
        pan_steps.push_back({3.0, (int) (pan_tilt_center - amplitude_pan)});
        pan_steps.push_back({4.0, pan_tilt_center});
        pan_steps.push_back({5.0, (int) (pan_tilt_center + amplitude_pan)});
        pan_steps.push_back({6.0, pan_tilt_center});
        pan_steps.push_back({7.0, (int) (pan_tilt_center - amplitude_pan)});*/

        if (fix.get_position_on_stage() == "Left") {
          std::cout << "position on stage: left" << std::endl;
          pan_steps.push_back({0.0, (int) (pan_center + amplitude_pan)});
          pan_steps.push_back({1.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({2.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({3.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({4.0f * time_of_two_beats, (int) (pan_center - amplitude_pan)});
          pan_steps.push_back({5.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({6.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({7.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
        } else {
          std::cout << "position on stage: right/center" << std::endl;
          pan_steps.push_back({0.0, (int) (pan_center - amplitude_pan)});
          pan_steps.push_back({1.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({2.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({3.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({4.0f * time_of_two_beats, (int) (pan_center + amplitude_pan)});
          pan_steps.push_back({5.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({6.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({7.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
        }

        tilt_steps.push_back({0.0, tilt_center});
        tilt_steps.push_back({1.0f * time_of_two_beats, (int) (tilt_center + (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({2.0f * time_of_two_beats, (int) (tilt_center + amplitude_tilt)});
        tilt_steps.push_back({3.0f * time_of_two_beats, (int) (tilt_center + (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({4.0f * time_of_two_beats, tilt_center});
        tilt_steps.push_back({5.0f * time_of_two_beats, (int) (tilt_center - (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({6.0f * time_of_two_beats, (int) (tilt_center - amplitude_tilt)});
        tilt_steps.push_back({7.0f * time_of_two_beats, (int) (tilt_center - (amplitude_tilt * 2 / 3))});

        loop = true;

      }if(fix.get_moving_head_type() == "Continuous Circle") {
        time_per_step_tilt = time_of_one_beat;
        amplitude_tilt = 45 / fix.get_degrees_per_tilt();
        amplitude_pan = 90 / fix.get_degrees_per_pan();

        if (fix.get_position_on_stage() == "Left") {
          std::cout << "position on stage: left" << std::endl;
          pan_steps.push_back({0.0, (int) (pan_center + amplitude_pan)});
          pan_steps.push_back({1.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({2.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({3.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({4.0f * time_of_two_beats, (int) (pan_center - amplitude_pan)});
          pan_steps.push_back({5.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({6.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({7.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
        } else {
          std::cout << "position on stage: right/center" << std::endl;
          pan_steps.push_back({0.0, (int) (pan_center - amplitude_pan)});
          pan_steps.push_back({1.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
          pan_steps.push_back({2.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({3.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({4.0f * time_of_two_beats, (int) (pan_center + amplitude_pan)});
          pan_steps.push_back({5.0f * time_of_two_beats, (int) (pan_center + (amplitude_pan * 2 / 3))});
          pan_steps.push_back({6.0f * time_of_two_beats, pan_center});
          pan_steps.push_back({7.0f * time_of_two_beats, (int) (pan_center - (amplitude_pan * 2 / 3))});
        }

        tilt_steps.push_back({0.0, tilt_center});
        tilt_steps.push_back({1.0f * time_of_two_beats, (int) (tilt_center + (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({2.0f * time_of_two_beats, (int) (tilt_center + amplitude_tilt)});
        tilt_steps.push_back({3.0f * time_of_two_beats, (int) (tilt_center + (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({4.0f * time_of_two_beats, tilt_center});
        tilt_steps.push_back({5.0f * time_of_two_beats, (int) (tilt_center - (amplitude_tilt * 2 / 3))});
        tilt_steps.push_back({6.0f * time_of_two_beats, (int) (tilt_center - amplitude_tilt)});
        tilt_steps.push_back({7.0f * time_of_two_beats, (int) (tilt_center - (amplitude_tilt * 2 / 3))});

        loop = true;

      } else if(fix.get_moving_head_type() == "Backlight, drop on action") {
        amplitude_tilt = 100;
        amplitude_pan = 0;
        time_per_step_tilt = time_of_one_beat;
        time_step = 0.025f;
        vc_tilt.push_back({0.0, 127});
        vc_tilt.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        vc_pan.push_back({0.0, 127});
        vc_pan.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        vc_zoom.push_back({0.0, 200});
        vc_zoom.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        vc_focus.push_back({0.0, 200});
        vc_focus.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});

        std::vector<time_value_int> segment_changes = lightshow->get_timestamps_colorchanges();

        std::cout << "all segment changes:" << std::endl;
        //for(time_value_int f: segment_changes) {
          //std::cout << f.time << "    " << f.value << std::endl;
        //}
        std::vector<float> timestamps_of_drops;
        for(int i = 1; i < segment_changes.size(); i++) {
          //if(segment_changes[i].value > segment_changes[i-1].value + 30 && segment_changes[i].time > segment_changes[i-1].time + time_of_four_bars) {
          if(segment_changes[i].value >= 90) {
            if(segment_changes[i].time - time_of_two_bars > 0)
              timestamps_of_drops.push_back(segment_changes[i].time - time_of_two_bars);
            else if(segment_changes[i].time - 2 * time_of_two_beats > 0)
              timestamps_of_drops.push_back(segment_changes[i].time - 2 * time_of_two_beats);
          }
        }
        //for(float f: timestamps_of_drops)
          //std::cout << f << std::endl;


        int start_value = tilt_center;
        // TODO hier switch case plus oder minus
        int end_value = tilt_center - amplitude_tilt;
        float value_step = (float) (end_value - start_value) * time_step / time_of_two_bars;
        for(float begin_timestamp: timestamps_of_drops) {
          float time = begin_timestamp;
          float current_value = start_value;
          while(time <= begin_timestamp + time_of_two_bars) {
            current_value += value_step;

            vc_tilt.push_back({time, (int) current_value});
            time = time + time_step;
          }
          vc_tilt.push_back({time + 0.5f, tilt_center});

        }
        fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
        fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
        fix.add_value_changes_to_channel(vc_zoom, fix.get_channel_zoom());
        fix.add_value_changes_to_channel(vc_focus, fix.get_channel_focus());

        loop = false;

      } else {


        vc_tilt.push_back({0.0, 127});
        vc_tilt.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 126});
        vc_pan.push_back({0.0, 127});
        vc_pan.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 126});

        fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
        fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
      }

      if(fix.get_moving_head_type() != "Nothing" && loop) {
        int j = 0;
        //for(int j = 0; j < 400; j = j + tilt_steps.size() * (time_per_step_tilt * freq)) {
        if(tilt_steps.size() > 0) {
          while (j
              < (lightshow->get_length() - 3) / (tilt_steps.size() * (time_per_step_tilt * frequency))) {
            for (int i = 0; i < tilt_steps.size(); i++) {
              float start_time = tilt_steps[i].time + j * tilt_steps.size() * time_per_step_tilt;
              int start_value = tilt_steps[i].value;
              int end_value = tilt_steps[0].value;
              if (i < tilt_steps.size() - 1)
                end_value = tilt_steps[i + 1].value;

              float value_step = (float) (end_value - start_value) / (time_per_step_tilt * frequency);

              for (int i = 0; i < time_per_step_tilt * frequency; i++) {
                vc_tilt.push_back({start_time + i * time_step, (int) (start_value + i * value_step)});

              }
            }
            j++;
          }
        }
        j = 0;
        if(pan_steps.size() > 0) {
          while (j
              < (lightshow->get_length() - 3) / (pan_steps.size() * (time_per_step_pan * frequency))) {
            for (int i = 0; i < pan_steps.size(); i++) {
              float start_time = pan_steps[i].time + j * pan_steps.size() * time_per_step_pan;
              int start_value = pan_steps[i].value;
              int end_value = pan_steps[0].value;
              if (i < pan_steps.size() - 1)
                end_value = pan_steps[i + 1].value;

              float value_step = (float) (end_value - start_value) / (time_per_step_pan * frequency);

              for (int i = 0; i < time_per_step_pan * frequency; i++) {
                vc_pan.push_back({start_time + i * time_step, (int) (start_value + i * value_step)});

              }
            }
            j++;
          }
        }
        fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
        fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
      }
    }

    if (fix_type == "bass") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_bass(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        //std::cout << colors.size() << std::endl;
        this->generate_color_fades(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_bass(), fix.get_channel_red());
      }
      //lightshow->add_fixture_bass(fix);
    } else if (fix_type == "mid") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_middle(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_middle(), fix.get_channel_blue());
      }
      //lightshow->add_fixture_middle(fix);
    } else if (fix_type == "high") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_high(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_high(), fix.get_channel_green());
      }
      //lightshow->add_fixture_high(fix);
    } else if (fix_type == "ambient") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);

      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "color_change") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        if(fix.get_timestamps_type().find("action") != string::npos)
          this->generate_color_changes(lightshow, fix, colors, true, timestamps);
        else
          this->generate_color_changes(lightshow, fix, colors, false, timestamps);

      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "flash") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes_flash;
        value_changes_flash.push_back({0.0, 0});

        if(fix.has_shutter) {
          for (int i = 0; i < timestamps.size(); i++) {
            if (timestamps[i] - 0.050f > 0) {
              value_changes_flash.push_back({timestamps[i] - 0.050f, 0});
            }
            value_changes_flash.push_back({timestamps[i], 255});
            if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
              value_changes_flash.push_back({timestamps[i] + 0.050f, 0});
            }
          }
        } else {
          for (int i = 0; i < timestamps.size(); i++) {
            if (timestamps[i] - 0.050f > 0) {
              value_changes_flash.push_back({timestamps[i] - 0.050f, 0});
              value_changes_flash.push_back({timestamps[i] - 0.025f, 100});
            }
            value_changes_flash.push_back({timestamps[i], 200});
            if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
              value_changes_flash.push_back({timestamps[i] + 0.025f, 100});
              value_changes_flash.push_back({timestamps[i] + 0.050f, 0});
            }
          }
        }

        if(fix.has_shutter) {
          std::vector<time_value_int> v;
          v.push_back({0.0, 200});
          v.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
          fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_shutter());
        } else {
          fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_dimmer());
        }

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }

    } else if (fix_type == "flash_reverse") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes_flash_reverse;

        if(fix.has_shutter) {
          value_changes_flash_reverse.push_back({0.0, 255});
          value_changes_flash_reverse.push_back({((float) lightshow->get_length() - 3)
                                                   / lightshow->get_resolution(), 0});
          for (int i = 0; i < timestamps.size(); i++) {
            if (timestamps[i] - 0.050f > 0) {
              value_changes_flash_reverse.push_back({timestamps[i] - 0.050f, 255});
            }
            value_changes_flash_reverse.push_back({timestamps[i], 0});
            if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
              value_changes_flash_reverse.push_back({timestamps[i] + 0.050f, 255});
            }
          }
        } else {
          value_changes_flash_reverse.push_back({0.0, 200});
          value_changes_flash_reverse.push_back({((float) lightshow->get_length() - 3)
                                                   / lightshow->get_resolution(), 0});
          for (int i = 0; i < timestamps.size(); i++) {
            if (timestamps[i] - 0.050f > 0) {
              value_changes_flash_reverse.push_back({timestamps[i] - 0.050f, 200});
              value_changes_flash_reverse.push_back({timestamps[i] - 0.025f, 100});
            }
            value_changes_flash_reverse.push_back({timestamps[i], 0});
            if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
              value_changes_flash_reverse.push_back({timestamps[i] + 0.025f, 100});
              value_changes_flash_reverse.push_back({timestamps[i] + 0.050f, 200});
            }
          }
        }

        if(fix.has_shutter) {
          std::vector<time_value_int> v;
          v.push_back({0.0, 200});
          v.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
          fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_shutter());
        } else {
          fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_dimmer());
        }

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
    }else if (fix_type == "blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        for (int i = 0; i < timestamps.size(); i++) {
            value_changes.push_back({timestamps[i], 200});
            if(i < timestamps.size() - 1) {
              this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], ((float) lightshow->get_length() - 3) / lightshow->get_resolution());
            }
            else {
              value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
            }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_one_after_another") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes_onset_blink;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          for (int i = 0; i < timestamps.size(); i++) {
            if(i % fixtures_in_group_one_after_another + 1 == fix.get_position_in_group()) {
              value_changes_onset_blink.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1)
                value_changes_onset_blink.push_back({timestamps[i + 1], 0});
              else
                value_changes_onset_blink.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
            }
          }

          fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_one_after_another_back_and_forth") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes_onset_blink;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          uint8_t counter = 1;
          bool left_to_right = true;
          for (int i = 0; i < timestamps.size(); i++) {
            if(counter == fix.get_position_in_group()) {
              value_changes_onset_blink.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1)
                value_changes_onset_blink.push_back({timestamps[i + 1], 0});
              else
                value_changes_onset_blink.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
            }

            if(left_to_right)
              counter++;
            else
              counter--;

            if(counter == fixtures_in_group_one_after_another_back_and_forth)
              left_to_right = false;
            else if(counter == 1)
              left_to_right = true;
          }

          fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_one_after_another_back_and_forth_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          uint8_t counter = 1;
          bool left_to_right = true;
          for (int i = 0; i < timestamps.size(); i++) {
            if(counter == fix.get_position_in_group()) {
              value_changes.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1)
                this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], ((float) lightshow->get_length() - 3) / lightshow->get_resolution());
              else
                value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
            }

            if(left_to_right)
              counter++;
            else
              counter--;

            if(counter == fixtures_in_group_one_after_another_back_and_forth_blink)
              left_to_right = false;
            else if(counter == 1)
              left_to_right = true;
          }

          fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_one_after_another_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        //std::vector<float> timestamps = {0.452789, 0.917188, 1.38159, 1.84599, 2.31039, 2.77478, 3.22757, 3.68036, 4.13315, 4.60916, 5.08517, 5.54957, 6.0488, 6.54803, 7.04726, 7.55809, 8.06893, 8.57977, 9.12544, 9.67111, 10.2168, 10.7741, 11.3313, 11.8886, 12.4343, 12.98, 13.5605, 14.141, 14.7215, 15.2787, 15.836, 16.3933, 16.9506, 17.4962, 18.0419, 18.5876, 19.1332, 19.5512, 19.9692, 20.3871, 20.8051, 21.223, 21.641, 22.059, 22.4885, 22.9181, 23.3593, 23.7888, 24.2184, 24.6364, 25.0659, 25.5536, 26.0412, 26.5288, 27.0164, 27.5156, 27.9917, 28.4561, 28.9205, 29.3965, 29.7448, 30.0931, 30.4414, 30.7897, 31.1263, 31.4746, 31.8113, 32.1364, 32.4731, 32.8098, 33.1349, 33.46, 33.785, 34.1101, 34.4352, 34.7835, 35.1202, 35.4453, 35.7819, 36.1186, 36.4437, 36.7688, 37.0939, 37.419, 37.7556, 38.0923, 38.429, 38.7773, 39.1256, 39.4855, 39.8338, 40.1821, 40.5304, 40.8787, 41.227, 41.5753, 41.9236, 42.2603, 42.6086, 42.9569, 43.2936, 43.6303, 43.967, 44.2921, 44.6171, 44.9422, 45.2673, 45.5924, 45.9175, 46.2425, 46.5676, 46.9043, 47.241, 47.6009, 47.9608, 48.3323, 48.7039, 49.145, 49.5746, 50.0158, 50.4454, 50.9446, 51.4322, 51.9198, 52.4307, 52.9299, 53.4291, 53.9283, 54.4392, 54.9384, 55.4376, 55.9369, 56.4477, 56.9702, 57.5042, 58.1195, 58.7233, 59.3154, 59.9075, 60.4996, 61.0336, 61.5677, 62.1018, 62.6358, 63.1815, 63.7156, 64.2728, 64.8185, 65.4687, 66.1072, 66.7458, 67.3959, 68.0345, 68.673, 69.3116, 69.9385, 70.5538, 71.1808, 71.8309, 72.4695, 73.108, 73.7466, 74.3851, 75.0121, 75.639, 76.2659, 76.9045, 77.485, 78.0655, 78.6344, 79.2265, 79.8302, 80.4107, 80.9912, 81.5833, 82.1986, 82.8024, 83.3596, 83.8937, 84.4278, 84.9618, 85.5075, 86.0415, 86.5756, 87.098, 87.6205, 88.1429, 88.6538, 89.1762, 89.6987, 90.2095, 90.7204, 91.2196, 91.7188, 92.2297, 92.7289, 93.2281, 93.7273, 94.2382, 94.7606, 95.225, 95.6778, 96.119, 96.5602, 96.9317, 97.3032, 97.6631, 98.0346, 98.4062, 98.7777, 99.1492, 99.5207, 99.9039, 100.287, 100.67, 101.053, 101.436, 101.808, 102.179, 102.632, 103.097, 103.573, 104.049, 104.525, 105.001, 105.477, 105.941, 106.394, 106.858, 107.334, 107.799, 108.263, 108.82, 109.378, 109.946, 110.55, 111.154, 111.746, 112.338, 112.93, 113.429, 113.94, 114.44, 114.939, 115.438, 115.926, 116.413, 116.901, 117.388, 117.946, 118.515, 119.084, 119.641, 120.198, 120.767, 121.324, 121.87, 122.427, 122.996, 123.577, 124.134, 124.703, 125.272, 125.841, 126.398, 126.967, 127.536, 128.104, 128.662, 129.231, 129.8, 130.357, 130.914, 131.483, 132.052, 132.621, 133.19, 133.77, 134.339, 134.908, 135.465, 136.034, 136.591, 137.149, 137.718, 138.286, 138.855, 139.413, 139.97, 140.55, 141.119, 141.688, 142.257, 142.826, 143.383, 143.94, 144.498, 145.067, 145.624, 146.193, 146.773, 147.342, 147.911, 148.48, 149.037, 149.606, 150.175, 150.732, 151.301, 151.87, 152.451, 153.02, 153.577, 154.134, 154.691, 155.26, 155.829, 156.398, 156.967, 157.536, 158.093, 158.662, 159.242, 159.823, 160.403, 160.961, 161.518, 162.087, 162.656, 163.213, 163.77, 164.328, 164.896, 165.465, 166.057, 166.626, 167.195, 167.753, 168.321, 168.89, 169.448, 170.005, 170.574, 171.154, 171.735, 172.327, 172.942, 173.546, 174.057, 174.568, 175.078, 175.589, 176.089, 176.599, 177.11, 177.621, 178.143, 178.596, 179.049, 179.49, 179.943, 180.384, 180.825, 181.267, 181.696, 182.126, 182.544, 182.938, 183.333, 183.728, 184.123, 184.506, 184.889, 185.272, 185.678, 186.073, 186.468, 186.863, 187.373, 187.884, 188.395, 188.918, 189.428, 189.928, 190.427, 190.926, 191.425, 191.994, 192.563, 193.132, 193.712, 194.293, 194.862, 195.431, 196, 196.557, 197.126, 197.683, 198.252, 198.821, 199.39, 199.959, 200.528, 201.096, 201.665, 202.234, 202.803, 203.372, 203.941, 204.51, 205.079, 205.648, 206.216, 206.774, 207.331, 207.888, 208.457, 209.026, 209.595, 210.164, 210.733, 211.29, 211.859, 212.428, 212.997, 213.554, 214.111, 214.68, 215.249, 215.818, 216.375, 216.944, 217.525, 218.093, 218.662, 219.22, 219.788, 220.357, 220.926, 221.495, 222.064, 222.621, 223.179, 223.736, 224.305, 224.874, 225.431, 226, 226.569, 227.138, 227.706, 228.275, 228.844, 229.425, 229.994, 230.563, 231.131, 231.689, 232.258, 232.826, 233.395, 233.953, 234.522, 235.079, 235.648, 236.205, 236.774, 237.343, 237.912, 238.481, 239.026, 239.583, 240.152, 240.71, 241.279, 241.836, 242.405, 242.974, 243.554, 244.123, 244.703, 245.261, 245.83, 246.399, 246.967, 247.525, 248.094, 248.662, 249.243, 249.8, 250.358, 250.926, 251.507, 252.076, 252.645, 253.214, 253.771, 254.328, 254.885, 255.454, 256.023, 256.592, 257.161, 257.718, 258.287, 258.856, 259.413, 259.971, 260.539, 261.097, 261.666, 262.235, 262.803, 263.361, 263.918, 264.475, 265.044, 265.613, 266.17, 266.739, 267.308, 267.877, 268.457, 269.038, 269.607, 270.164, 270.721, 271.29, 271.859, 272.428, 272.997, 273.554, 274.123, 274.692, 275.261, 275.83, 276.399, 276.968, 277.536, 278.105, 278.674, 279.232, 279.789, 280.346, 280.915, 281.472, 282.041, 282.61, 283.179, 283.748, 284.305, 284.874, 285.513, 286.139, 286.766, 287.393, 288.02, 288.624, 289.239, 289.855, 290.458, 291.051, 291.643, 292.223, 292.792, 293.361, 293.941, 294.51, 295.091, 295.671, 296.252, 296.844, 297.482, 298.121, 298.771, 299.421, 300.06, 300.71, 301.36, 302.022, 302.684, 303.345, 304.007, 304.681, 305.354, 306.027, 306.701, 307.386, 308.117, 308.849, 309.568, 310.288, 310.996, 311.705, 312.366, 313.04, 313.678, 314.317, 314.955, 315.594, 316.233, 316.859, 317.486, 318.113, 318.74, 319.356, 320.029, 320.702, 321.306, 321.921, 322.537, 323.141, 323.744, 324.29, 324.836, 325.381, 325.915, 326.461, 327.053, 327.645, 328.237, 328.76, 329.282, 329.805, 330.304, 330.815, 331.314, 331.802, 332.289, 332.777, 333.264, 333.775, 334.286, 334.809, 335.343, 335.865, 336.387, 336.91, 337.432, 337.966};
        std::vector<time_value_int> value_changes;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "fixtures_in_group_one_after_another_blink: " << fixtures_in_group_one_after_another_blink << std::endl;
        std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          for (int i = 0; i < timestamps.size(); i++) {
            if(i % fixtures_in_group_one_after_another_blink + 1 == fix.get_position_in_group()) {
              value_changes.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1) {

                this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], ((float) lightshow->get_length() - 3) / lightshow->get_resolution());

              }
              else {
                value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
              }
            }
          }

          fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_two_after_another") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes.push_back({timestamps[i] - 0.050f, 200});
            value_changes.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes.push_back({timestamps[i], 0});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes.push_back({timestamps[i] + 0.025f, 100});
            value_changes.push_back({timestamps[i] + 0.050f, 200});
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes.push_back({timestamps[i] - 0.050f, 200});
            value_changes.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes.push_back({timestamps[i], 0});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes.push_back({timestamps[i] + 0.025f, 100});
            value_changes.push_back({timestamps[i] + 0.050f, 200});
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes.push_back({timestamps[i] - 0.050f, 200});
            value_changes.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes.push_back({timestamps[i], 0});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes.push_back({timestamps[i] + 0.025f, 100});
            value_changes.push_back({timestamps[i] + 0.050f, 200});
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes.push_back({timestamps[i] - 0.050f, 200});
            value_changes.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes.push_back({timestamps[i], 0});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes.push_back({timestamps[i] + 0.025f, 100});
            value_changes.push_back({timestamps[i] + 0.050f, 200});
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "group_random_flashes") {
      if (fix.has_global_dimmer) {

        std::vector<float> onset_timestamps = lightshow->get_onset_timestamps();
        std::vector<float> begin_and_end_of_flashing_timestamps;
        int onset_counter = 0;
        float last_time = 0;
        float begin_timestamp = 0;
        float time_of_one_eigth = ((float) 60 / (float) lightshow->get_bpm()) / 2;
        std::cout << "time_of_one_eigth: " << time_of_one_eigth << std::endl;
        for(float onset_timestamp: onset_timestamps) {
          if(onset_counter == 0 && onset_timestamp - last_time < time_of_one_eigth) {
            begin_timestamp = last_time;
            onset_counter = 1;
          }

          if(onset_counter > 0 && onset_timestamp - last_time < time_of_one_eigth) {
            onset_counter++;
          }

          if(onset_counter > 0 && onset_timestamp - last_time >= time_of_one_eigth) {
            if(onset_counter > 12) {
              begin_and_end_of_flashing_timestamps.push_back(begin_timestamp);
              begin_and_end_of_flashing_timestamps.push_back(last_time);
            }
            onset_counter = 0;
          }
          last_time = onset_timestamp;
        }


        //begin_and_end_of_flashing_timestamps.push_back(1);
        //begin_and_end_of_flashing_timestamps.push_back(1.5f);
        //begin_and_end_of_flashing_timestamps.push_back(3.5f);
        //begin_and_end_of_flashing_timestamps.push_back(6);

        std::vector<time_value_int> value_changes;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "begin_and_end_of_flashing_timestamps.size(): " << begin_and_end_of_flashing_timestamps.size() << std::endl;
        std::cout << "old bpm analysis: " << lightshow->get_bpm() << std::endl;

        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(1, fixtures_in_group_random_flashes); // guaranteed unbiased
        float begin_flashing = 0;
        float end_flashing = 0;
        float time_of_next_flash = 0;

        if(fix.get_position_in_group() > 0 && begin_and_end_of_flashing_timestamps.size() >= 2) {
          for (int i = 0; i < begin_and_end_of_flashing_timestamps.size() - 1; i = i + 2) {
            begin_flashing = begin_and_end_of_flashing_timestamps[i];
            end_flashing = begin_and_end_of_flashing_timestamps[i + 1];
            time_of_next_flash = begin_flashing;

            while(time_of_next_flash < end_flashing) {
              auto random_integer = uni(rng);
              //std::cout << "random_integer: " << random_integer << std::endl;
              if(random_integer == fix.get_position_in_group()) {
                value_changes.push_back({time_of_next_flash, 200});
                if(((float) lightshow->get_length() - 3) / lightshow->get_resolution() > time_of_next_flash + 0.025f)
                  value_changes.push_back({time_of_next_flash + 0.025f, 0});
                else
                  value_changes.push_back({((float) lightshow->get_length() - 3) / lightshow->get_resolution(), 0});
              }
              time_of_next_flash += 0.025f;
            }
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow, fix, colors);
      } else {

      }
      //lightshow->add_fixture(fix);
    } else if (fix_type == "strobe_if_many_onsets") {

      std::vector<float> onset_timestamps = lightshow->get_onset_timestamps();
      std::vector<float> begin_and_end_of_flashing_timestamps;
      int onset_counter = 0;
      float last_time = 0;
      float begin_timestamp = 0;
      float time_of_one_eigth = ((float) 60 / (float) lightshow->get_bpm()) / 2;
      //std::cout << "time_of_one_eigth: " << time_of_one_eigth << std::endl;
      for (float onset_timestamp: onset_timestamps) {
        if (onset_counter == 0 && onset_timestamp - last_time < time_of_one_eigth) {
          begin_timestamp = last_time;
          onset_counter = 1;
        }

        if (onset_counter > 0 && onset_timestamp - last_time < time_of_one_eigth) {
          onset_counter++;
        }

        if (onset_counter > 0 && onset_timestamp - last_time >= time_of_one_eigth) {
          if (onset_counter > 12) {
            begin_and_end_of_flashing_timestamps.push_back(begin_timestamp);
            begin_and_end_of_flashing_timestamps.push_back(last_time);
          }
          onset_counter = 0;
        }
        last_time = onset_timestamp;
      }

      std::vector<time_value_int> value_changes;

      std::cout << "begin_and_end_of_flashing_timestamps.size(): " << begin_and_end_of_flashing_timestamps.size()
                << std::endl;
      //std::cout << "old bpm analysis: " << lightshow->get_bpm() << std::endl;

      float begin_flashing = 0;
      float end_flashing = 0;

      if(fix.is_blinder) {
        if (begin_and_end_of_flashing_timestamps.size() > 0) {
          for (int i = 0; i < begin_and_end_of_flashing_timestamps.size() - 1; i = i + 2) {
            begin_flashing = begin_and_end_of_flashing_timestamps[i];
            end_flashing = begin_and_end_of_flashing_timestamps[i + 1];

            value_changes.push_back({begin_flashing, fix.get_blinder_value()});
            value_changes.push_back({end_flashing, 0});
          }
          fix.add_value_changes_to_channel(value_changes, fix.get_channel_blinder());
        }
      }
      else {
        if (fix.has_global_dimmer) {
          if (fix.has_strobe) {


            if (begin_and_end_of_flashing_timestamps.size() > 0) {
              for (int i = 0; i < begin_and_end_of_flashing_timestamps.size() - 1; i = i + 2) {
                begin_flashing = begin_and_end_of_flashing_timestamps[i];
                end_flashing = begin_and_end_of_flashing_timestamps[i + 1];

                value_changes.push_back({begin_flashing, 255});
                value_changes.push_back({end_flashing, 0});
              }
              fix.add_value_changes_to_channel(value_changes, fix.get_channel_strobo());
            }

            fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

            std::vector<std::string> colors = fix.get_colors();
            this->generate_color_fades(lightshow, fix, colors);

          } else if (fix.has_shutter) {

          } else {

          }
        }
      }
      //lightshow->add_fixture(fix);
    }

  }
  return lightshow;
}

void LightshowGenerator::generate_color_fades(std::shared_ptr<Lightshow> lightshow_from_analysis, LightshowFixture& fix, std::vector<std::string>& colors) {
  std::vector<color_change> color_changes;
  std::vector<time_value_int> timestamps = lightshow_from_analysis->get_timestamps_colorchanges();
  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());
  int c = 0;
  color_changes.push_back({ 0, colors[0] });

  for (int i = 0; i < timestamps.size(); i++) {
    // Alles, was vor Sekunde 1 passiert, wird ignoriert
    if (timestamps[i].time < this->fade_duration)
      timestamps[i].time = 0 + this->fade_duration / 2;

    // Wenn zwischen dem aktuellen und dem n�chsten Timestamp weniger als 4 Sekunden liegen, ignoriere den aktuellen
    if (i >= 0 && timestamps.size() > i + 1) {
      if (timestamps[i + 1].time <= timestamps[i].time + 4)
        continue;
    }

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if (c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps[i].time);
    color_changes.push_back({ timestamps[i].time, colors[c] });
  }
  if(fix.has_colorwheel) {
    //this->set_colorwheel_color_changes(lightshow_from_analysis, fix, color_changes);
    this->set_hard_color_changes(lightshow_from_analysis, fix, color_changes);
  } else {
    this->set_soft_color_changes(lightshow_from_analysis, fix, color_changes, this->fade_duration);
  }
}

void LightshowGenerator::set_soft_color_changes(std::shared_ptr<Lightshow> lightshow_from_analysis, LightshowFixture& fix, std::vector<color_change> color_changes, float fade_duration) {
  if (color_changes.empty())
    return;

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;


  int counter = 0;
  float timestamp_color_change = 0;
  float timestamp_begin_fade = 0;
  float timestamp_end_fade = 0;
  float end_of_color = 0;
  std::string previous_color;
  float end_of_song = (lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution();

  while (counter < color_changes.size()) {

    timestamp_color_change = color_changes[counter].timestamp;
    timestamp_begin_fade = timestamp_color_change - fade_duration / 2;

    if (counter == 0 && color_changes.size() > 1) {
        timestamp_end_fade = color_changes[counter + 1].timestamp + fade_duration / 2;
        this->set_color_for_fixed_time(fix, color_changes[counter].color, color_changes[counter].timestamp, timestamp_end_fade);
        previous_color = color_changes[counter].color;
        counter++;
        continue;
    }

    color_values values_previous_color = color_to_rgb(previous_color);
    color_values values_new_color = color_to_rgb(color_changes[counter].color);

    for (time_value_int tv : this->calculate_single_fade(lightshow_from_analysis, fade_duration, values_previous_color.r, values_new_color.r)) {
        tv.time = tv.time + timestamp_begin_fade;
        data_pairs_red.push_back(tv);
    }
    for (time_value_int tv : this->calculate_single_fade(lightshow_from_analysis, fade_duration, values_previous_color.g, values_new_color.g)) {
        tv.time = tv.time + timestamp_begin_fade;
        if (tv.time < 0)
            tv.time = 0;
        data_pairs_green.push_back(tv);
    }
    for (time_value_int tv : this->calculate_single_fade(lightshow_from_analysis, fade_duration, values_previous_color.b, values_new_color.b)) {
        tv.time = tv.time + timestamp_begin_fade;
        data_pairs_blue.push_back(tv);
    }
    previous_color = color_changes[counter].color;

    counter++;
  }

  data_pairs_red.push_back({ end_of_song, -1 });
  data_pairs_green.push_back({ end_of_song, -1 });
  data_pairs_blue.push_back({ end_of_song, -1 });
  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}


void LightshowGenerator::set_hard_color_changes(const std::shared_ptr<Lightshow> lightshow_from_analysis, LightshowFixture &fix, std::vector<color_change> color_changes) {
  if (color_changes.empty())
    return;


  if (fix.has_colorwheel) {
    std::vector<time_value_int> color_changes_colorwheel;
    for(color_change cc: color_changes) {
      color_changes_colorwheel.push_back({cc.timestamp, fix.get_colorwheel_value(cc.color)});
    }
    fix.add_value_changes_to_channel(color_changes_colorwheel, fix.get_channel_colorwheel());
  } else {

    std::vector<time_value_int> data_pairs_red;
    std::vector<time_value_int> data_pairs_green;
    std::vector<time_value_int> data_pairs_blue;

    int counter = 0;
    float start_of_color = 0;
    float end_of_color = 0;

    while (counter < color_changes.size()) {
      start_of_color = color_changes[counter].timestamp;
      if (counter == color_changes.size() - 1)
        end_of_color = ((lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution());
      else
        end_of_color = color_changes[counter + 1].timestamp;

      color_values cv = this->color_to_rgb(color_changes[counter].color);
      if (cv.r > 0) {
        data_pairs_red.push_back({start_of_color, cv.r});
        data_pairs_red.push_back({end_of_color, 0});
      }
      if (cv.g > 0) {
        data_pairs_green.push_back({start_of_color, cv.g});
        data_pairs_green.push_back({end_of_color, 0});
      }
      if (cv.b > 0) {
        data_pairs_blue.push_back({start_of_color, cv.b});
        data_pairs_blue.push_back({end_of_color, 0});
      }

      counter++;
    }

    fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
    fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
    fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
  }
}

std::vector<time_value_int> LightshowGenerator::calculate_single_fade(const std::shared_ptr<Lightshow> lightshow_from_analysis, float fade_duration, int c_old, int c_new)
{
	std::vector<time_value_int> v;
	float change_per_step = (c_old - c_new) / ((lightshow_from_analysis->get_resolution() - 5) * fade_duration);
	int color = c_old;
	float timestamp = 0;
	float time_step = 0;
	if (lightshow_from_analysis->get_resolution() == 40)
		time_step = 0.025;
	else if (lightshow_from_analysis->get_resolution() == 20)
		time_step = 0.05;
	else if (lightshow_from_analysis->get_resolution() == 10)
		time_step = 0.1;


	//for(float timestamp = 0; timestamp < fade_duration; timestamp = timestamp + 1 / this->get_resolution()) {
	for (int i = 0; i < fade_duration * lightshow_from_analysis->get_resolution(); i++) {
		color = c_old - i * change_per_step;
		if (i == fade_duration * lightshow_from_analysis->get_resolution() - 1)
			color = c_new;
		if (color < 0)
			color = 0;
		else if (color > 255)
			color = 255;
		v.push_back({ timestamp, color });
		timestamp = timestamp + time_step;
		//timestamp = timestamp + 0.025;
	}
	//std::cout << "calculated fade with " << v.size() << " steps." << std::endl;
	return v;
}

void LightshowGenerator::set_color_for_fixed_time(LightshowFixture& fix, std::string& color, float begin, float end) {
  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;

  color_values cv = this->color_to_rgb(color);
  if(cv.r > 0) {
    data_pairs_red.push_back({begin, cv.r});
    data_pairs_red.push_back({end, 0});
  }
  if(cv.g > 0) {
    data_pairs_green.push_back({begin, cv.g});
    data_pairs_green.push_back({end, 0});
  }
  if(cv.b > 0) {
    data_pairs_blue.push_back({begin, cv.b});
    data_pairs_blue.push_back({end, 0});
  }

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}

void LightshowGenerator::set_color_of_fixture_during_song(const std::shared_ptr<Lightshow> lightshow_from_analysis, LightshowFixture& fix, std::initializer_list<std::string> a_args) {
  int number_of_color_changes = a_args.size();
  int number_of_current_color = 0;
  float length_of_song = (lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution();
  float time_of_one_color = length_of_song / number_of_color_changes;
  Logger::debug("time of one color: {}", time_of_one_color);
  float begin = 0;
  float end = 0;

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;
  int counter = 0;

  for (const std::string s : a_args) {
    begin = counter * (length_of_song / number_of_color_changes);
    end = (counter + 1) * (length_of_song / number_of_color_changes);

    color_values cv = this->color_to_rgb(s);
    if(cv.r > 0) {
      data_pairs_red.push_back({begin, cv.r});
      data_pairs_red.push_back({end, 0});
    }
    if(cv.g > 0) {
      data_pairs_green.push_back({begin, cv.g});
      data_pairs_green.push_back({end, 0});
    }
    if(cv.b > 0) {
      data_pairs_blue.push_back({begin, cv.b});
      data_pairs_blue.push_back({end, 0});
    }

    counter++;
  }

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}

color_values LightshowGenerator::color_to_rgb(string color) {
  color_values cv{0, 0, 0};

  if(color == "red") {
    cv.r = 255;
    cv.g = 0;
    cv.b = 0;
  } else if(color == "green") {
    cv.r = 0;
    cv.g = 255;
    cv.b = 0;
  } else if(color == "blue") {
    cv.r = 0;
    cv.g = 0;
    cv.b = 255;
  } else if(color == "white") {
    cv.r = 255;
    cv.g = 255;
    cv.b = 255;
  } else if(color == "yellow") {
    cv.r = 255;
    cv.g = 130; // looked to green on Cameo, maybe change back if mostly other fixtures are used
    //cv.g = 100;
    cv.b = 0;
  } else if(color == "orange") {
    cv.r = 255;
    cv.g = 80;
    cv.b = 0;
  } else if(color == "cyan") {
    cv.r = 0;
    cv.g = 255;
    cv.b = 255;
  } else if(color == "pink") {
    cv.r = 255;
    cv.g = 0;
    // cv.b = 255; // looked to blue on Cameo, maybe change back if mostly other fixtures are used
    cv.b = 75;
  } else if(color == "light-green") {
    cv.r = 50;
    cv.g = 255;
    cv.b = 50;
  } else { Logger::warning("Tried to use unknown color!"); }

  return cv;
}

void LightshowGenerator::generate_color_changes(std::shared_ptr<Lightshow> lightshow_from_analysis,
                                                LightshowFixture &fix,
                                                std::vector<std::string> &colors, bool only_change_color_if_action, std::vector<float> timestamps) {

  std::vector<color_change> color_changes;
  //std::vector<double> timestamps = lightshow_from_analysis->get_all_beats();
/*
  std::vector<float> timestamps_float;

  if(only_change_color_if_action) {
    std::vector<time_value_int> bass_values = lightshow_from_analysis->get_value_changes_bass();
    std::vector<time_value_int> mid_values = lightshow_from_analysis->get_value_changes_middle();
    for (float timestamp: timestamps) {
      for (time_value_int tvi_bass: bass_values) {
        if (tvi_bass.time >= (float) timestamp - 0.02 && tvi_bass.time <= (float) timestamp + 0.02) {
          if (tvi_bass.value > 75)
            timestamps_float.push_back((float) timestamp);
          break;
        }
      }
      for (time_value_int tvi_mid: mid_values) {
        if (tvi_mid.time >= (float) timestamp - 0.02 && tvi_mid.time <= (float) timestamp + 0.02) {
          if (tvi_mid.value > 175)
            timestamps_float.push_back((float) timestamp);
          break;
        }
      }
    }
  } else {
    //for(float timestamp: timestamps) {
      //timestamps_float.push_back( timestamp);
    //}
    timestamps_float = timestamps;
  }*/

  std::cout << "Number of color changes in this lightshow: " << timestamps.size() << std::endl;

  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());

  //timestamps.erase( unique( timestamps.begin(), timestamps.end() ), timestamps.end() );

  int c = 0;
  color_changes.push_back({ 0, colors[0] });

  for (int i = 0; i < timestamps.size(); i++) {

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if (c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps[i]);
    color_changes.push_back({ timestamps[i], colors[c] });
  }
  this->set_hard_color_changes(lightshow_from_analysis, fix, color_changes);
}

LightshowGenerator::~LightshowGenerator() {

}

void LightshowGenerator::generate_onset_color_changes(std::shared_ptr<Lightshow> lightshow_from_analysis,
                                                      LightshowFixture &fix,
                                                      std::vector<std::string> &colors) {
  std::vector<color_change> color_changes;
  std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();

  std::cout << "Number of onsets in this lightshow: " << timestamps.size() << std::endl;

  int c = 0;
  color_changes.push_back({ 0, colors[0] });

  for (int i = 0; i < timestamps.size(); i++) {

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if (c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps[i]);
    color_changes.push_back({ timestamps[i], colors[c] });
  }
  this->set_hard_color_changes(lightshow_from_analysis, fix, color_changes);
}

void LightshowGenerator::generate_blink_fade_outs(std::vector<time_value_int> &value_changes, float current_timestamp, float next_timestamp, float lightshow_length) {
  // TODO make nicer with loop(s)
  if(next_timestamp - current_timestamp > 0.5 && lightshow_length > current_timestamp + 0.5f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({current_timestamp + 0.275f, 113});
    value_changes.push_back({current_timestamp + 0.3f, 100});
    value_changes.push_back({current_timestamp + 0.325f, 88});
    value_changes.push_back({current_timestamp + 0.35f, 75});
    value_changes.push_back({current_timestamp + 0.375f, 63});
    value_changes.push_back({current_timestamp + 0.4f, 50});
    value_changes.push_back({current_timestamp + 0.425f, 38});
    value_changes.push_back({current_timestamp + 0.45f, 25});
    value_changes.push_back({current_timestamp + 0.475f, 13});
    value_changes.push_back({current_timestamp + 0.5f, 0});
  } else if(next_timestamp - current_timestamp > 0.45 && lightshow_length > current_timestamp + 0.45f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({current_timestamp + 0.275f, 113});
    value_changes.push_back({current_timestamp + 0.3f, 100});
    value_changes.push_back({current_timestamp + 0.325f, 88});
    value_changes.push_back({current_timestamp + 0.35f, 75});
    value_changes.push_back({current_timestamp + 0.375f, 63});
    value_changes.push_back({current_timestamp + 0.4f, 50});
    value_changes.push_back({current_timestamp + 0.425f, 38});
    value_changes.push_back({current_timestamp + 0.45f, 25});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.4 && lightshow_length > current_timestamp + 0.4f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({current_timestamp + 0.275f, 113});
    value_changes.push_back({current_timestamp + 0.3f, 100});
    value_changes.push_back({current_timestamp + 0.325f, 88});
    value_changes.push_back({current_timestamp + 0.35f, 75});
    value_changes.push_back({current_timestamp + 0.375f, 63});
    value_changes.push_back({current_timestamp + 0.4f, 50});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.35 && lightshow_length > current_timestamp + 0.35f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({current_timestamp + 0.275f, 113});
    value_changes.push_back({current_timestamp + 0.3f, 100});
    value_changes.push_back({current_timestamp + 0.325f, 88});
    value_changes.push_back({current_timestamp + 0.35f, 75});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.3 && lightshow_length > current_timestamp + 0.3f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({current_timestamp + 0.275f, 113});
    value_changes.push_back({current_timestamp + 0.3f, 100});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.25 && lightshow_length > current_timestamp + 0.25f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({current_timestamp + 0.225f, 133});
    value_changes.push_back({current_timestamp + 0.25f, 125});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.2 && lightshow_length > current_timestamp + 0.2f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({current_timestamp + 0.175f, 160});
    value_changes.push_back({current_timestamp + 0.2f, 150});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.15 && lightshow_length > current_timestamp + 0.15f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({current_timestamp + 0.125f, 178});
    value_changes.push_back({current_timestamp + 0.15f, 170});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.1 && lightshow_length > current_timestamp + 0.1f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({current_timestamp + 0.075f, 190});
    value_changes.push_back({current_timestamp + 0.1f, 185});
    value_changes.push_back({next_timestamp, 0});
  } else if(next_timestamp - current_timestamp > 0.05 && lightshow_length > current_timestamp + 0.05f) {
    value_changes.push_back({current_timestamp + 0.025f, 198});
    value_changes.push_back({current_timestamp + 0.05f, 195});
    value_changes.push_back({next_timestamp, 0});
  }
  else
    value_changes.push_back({next_timestamp, 0});
}
