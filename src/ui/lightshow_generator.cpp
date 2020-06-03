//
// Created by Jan on 06.06.2019.
//

#include <random>
#include <math.h>
#include "lightshow_generator.h"

#define PI 3.14159265

LightshowGenerator::LightshowGenerator() {

}

std::shared_ptr<Lightshow> LightshowGenerator::generate(int resolution, Song *song, std::shared_ptr<Lightshow> lightshow, int user_bpm, float onset_value) {

  bool need_bass = false;
  bool need_mid = false;
  bool need_high = false;
  bool need_onsets = false;

  lightshow->set_onset_value(onset_value);

  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    std::string fix_type = lightshow->get_fixtures()[i].get_type();
    std::string timestamp_type = lightshow->get_fixtures()[i].get_timestamps_type();
    std::string fix_mh_type = lightshow->get_fixtures()[i].get_moving_head_type();
    if(fix_type == "bass" || fix_type.find("auto") != string::npos || timestamp_type.find("action") != string::npos || fix_mh_type.find("auto") != string::npos)
      need_bass = true;

    if(fix_type == "mid" || fix_type.find("auto") != string::npos || timestamp_type.find("action") != string::npos || fix_mh_type.find("auto") != string::npos)
      need_mid = true;

    if(fix_type == "high")
      need_high = true;

    if(timestamp_type == "Onsets" || fix_type.find("onsets") != string::npos || fix_type.find("auto") != string::npos || fix_mh_type.find("auto") != string::npos) { // TODO das letzte wieder entfernen, war nur testweise
      need_onsets = true;
    }
  }

  lightshow->set_resolution(resolution);
  lightshow->set_sound_src(song->get_file_path());
  lightshow->prepare_analysis_for_song((char*)song->get_file_path().c_str(), need_bass, need_mid, need_high, need_onsets, user_bpm, onset_value, lightshow->get_onset_bass_lower_frequency(), lightshow->get_onset_bass_upper_frequency(), lightshow->get_onset_bass_threshold(), lightshow->get_onset_snare_lower_frequency(), lightshow->get_onset_snare_upper_frequency(), lightshow->get_onset_snare_threshold(), lightshow->get_custom_segments());


//  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  auto seed = lightshow->get_seed();
  std::cout << "seed: " << seed << std::endl;
  std::mt19937 rng(seed);    // random-number engine used (Mersenne-Twister in this case)
  std::uniform_int_distribution<> random_int_0_1(0, 1);
  std::uniform_int_distribution<> random_int_0_2(0, 2);
  std::uniform_int_distribution<> random_int_0_3(0, 3);
  std::uniform_int_distribution<> random_int_0_4(0, 4);
  std::uniform_int_distribution<> random_int_0_5(0, 5);
  std::uniform_int_distribution<> random_int_0_6(0, 6);
  std::uniform_int_distribution<> random_int_0_7(0, 7);



  float end_of_song = ((float) lightshow->get_length() - 3) / lightshow->get_resolution();

  int fixtures_in_group_one_after_another = 0;
  int fixtures_in_group_one_after_another_fade = 0;
  int fixtures_in_group_one_after_another_fade_reverse = 0;
  int fixtures_in_group_one_after_another_fade_single = 0;
  int fixtures_in_group_one_after_another_fade_single_reverse = 0;
  int fixtures_in_group_one_after_another_back_and_forth = 0;
  int fixtures_in_group_one_after_another_back_and_forth_blink = 0;
  int fixtures_in_group_one_after_another_blink = 0;
  int fixtures_in_group_two_after_another = 0;
  int fixtures_in_group_two_after_another_blink = 0;
  int fixtures_in_group_alternate_odd_even = 0;
  int fixtures_in_group_alternate_odd_even_blink = 0;
  int fixtures_in_group_random_flashes = 0;
  int fixtures_in_group_auto_beats = 0;
  int fixtures_in_group_auto_onsets = 0;

  int fixtures_in_mh_group_continuous_8 = 0;
  int fixtures_in_mh_group_continuous_circle = 0;
  int fixtures_in_mh_group_continuous_line_vertical = 0;
  int fixtures_in_mh_group_auto_background = 0;
  int fixtures_in_mh_group_auto_action = 0;
  int fixtures_in_mh_group_auto_special = 0;


  std::vector<std::vector<std::string>> color_palettes = {
      {"orange", "red", "yellow"},
      {"red", "white"},
      {"orange", "light-green"},
      {"blue", "red"},
      {"orange", "pink", "red"},
      {"cyan", "pink"},
      {"cyan", "green"},
      {"blue", "pink", "orange"}
  };

  //std::uniform_int_distribution<int> uni(0, 5); // guaranteed unbiased
  int random_color_index = random_int_0_7(rng);

  //srand (time(NULL));
  //random_color_index = rand() % 5 + 0;
  std::cout << "random_color_index: " << random_color_index << std::endl;
  std::vector<std::string> color_palette = color_palettes[random_color_index];

  std::vector<std::string> colors_1 = color_palette;
  auto first_color = color_palette.begin();
  std::rotate(first_color, first_color + 1, color_palette.end());
  std::vector<std::string> colors_2 = color_palette;

  std::map<std::string, std::vector<std::string>> fix_types_with_colors;
  std::map<std::string, int> fix_names_with_temp_position;
  int temp_position_counter = 1;


  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    LightshowFixture &fix = lightshow->get_fixtures_reference()[i];
    std::string fix_type = fix.get_type();
    std::string fix_kind = fix.get_name();

    if(fix_kind.find("BAR TRI-LED") != std::string::npos)
      fix_kind = "BAR TRI-LED";

    if (fix_type == "group_one_after_another" && fix.get_position_in_group() > fixtures_in_group_one_after_another)
      fixtures_in_group_one_after_another = fix.get_position_in_group();
    else if (fix_type == "group_one_after_another_fade" && fix.get_position_in_group() > fixtures_in_group_one_after_another_fade)
      fixtures_in_group_one_after_another_fade = fix.get_position_in_group();
    else if (fix_type == "group_one_after_another_fade_reverse" && fix.get_position_in_group() > fixtures_in_group_one_after_another_fade_reverse)
      fixtures_in_group_one_after_another_fade_reverse = fix.get_position_in_group();
    else if (fix_type == "group_one_after_another_fade_single" && fix.get_position_in_group() > fixtures_in_group_one_after_another_fade_single)
      fixtures_in_group_one_after_another_fade_single = fix.get_position_in_group();
    else if (fix_type == "group_one_after_another_fade_single_reverse" && fix.get_position_in_group() > fixtures_in_group_one_after_another_fade_single_reverse)
      fixtures_in_group_one_after_another_fade_single_reverse = fix.get_position_in_group();
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
    else if (fix_type == "group_auto_beats" && fix.get_position_in_group() > fixtures_in_group_auto_beats)
      fixtures_in_group_auto_beats = fix.get_position_in_group();
    else if (fix_type == "group_auto_onsets" && fix.get_position_in_group() > fixtures_in_group_auto_onsets)
      fixtures_in_group_auto_onsets = fix.get_position_in_group();

    std::string fix_mh_type = fix.get_moving_head_type();
    if(fix_mh_type == "Continuous 8 group" && fix.get_position_in_mh_group() > fixtures_in_mh_group_continuous_8)
      fixtures_in_mh_group_continuous_8 = fix.get_position_in_mh_group();
    else if(fix_mh_type == "Continuous Circle group" && fix.get_position_in_mh_group() > fixtures_in_mh_group_continuous_circle)
      fixtures_in_mh_group_continuous_circle = fix.get_position_in_mh_group();
    else if(fix_mh_type == "Continuous Line vertical group" && fix.get_position_in_mh_group() > fixtures_in_mh_group_continuous_line_vertical)
      fixtures_in_mh_group_continuous_line_vertical = fix.get_position_in_mh_group();
    else if(fix_mh_type == "group_auto_background" && fix.get_position_in_mh_group() > fixtures_in_mh_group_auto_background)
      fixtures_in_mh_group_auto_background = fix.get_position_in_mh_group();
    else if(fix_mh_type == "group_auto_action" && fix.get_position_in_mh_group() > fixtures_in_mh_group_auto_action)
      fixtures_in_mh_group_auto_action = fix.get_position_in_mh_group();
    else if(fix_mh_type == "group_auto_special" && fix.get_position_in_mh_group() > fixtures_in_mh_group_auto_special)
      fixtures_in_mh_group_auto_special = fix.get_position_in_mh_group();


    if(fix_types_with_colors.find(fix_type) == fix_types_with_colors.end() ) { // alternatively could also divide by fix name
      // not found
      if(random_int_0_1(rng) == 0)
        fix_types_with_colors.insert(std::pair<std::string, std::vector<std::string>>(fix_type, colors_1));
      else
        fix_types_with_colors.insert(std::pair<std::string, std::vector<std::string>>(fix_type, colors_2));

      /*if(!fix_types_with_colors.empty() && fix_types_with_colors.rbegin().operator*().second == colors_1)
        fix_types_with_colors.insert(std::pair<std::string, std::vector<std::string>>(fix_type, colors_2));
      else
        fix_types_with_colors.insert(std::pair<std::string, std::vector<std::string>>(fix_type, colors_1));*/
    } else {
      // found
    }


    if(fix_names_with_temp_position.find(fix_kind) == fix_names_with_temp_position.end()) { // alternatively could also divide by fix name
      if(fix_type != "OFF" && fix_type != "off" && !fix_type.empty() && !fix.is_blinder) {
        // not found
        fix_names_with_temp_position.insert(std::pair<std::string, int>(fix_kind, temp_position_counter));
        temp_position_counter++;
      }
    } else {
      // found
    }

  }

  for (auto const& x : fix_types_with_colors) {
    std::cout << x.first << std::endl;
    for(auto const& y: x.second)
      std::cout << y << std::endl;
    std::cout << std::endl;
  }

  for (auto const& x : fix_names_with_temp_position) {
    std::cout << x.first << std::endl;
    std::cout << x.second << std::endl;
    std::cout << std::endl;
  }


  auto segment_changes = lightshow->get_timestamps_segment_changes();
  std::cout << "numer of segments: " << segment_changes.size() << std::endl;

  // randomize some auto stuff for every segment
  std::vector<int> auto_beats_choice;
  std::vector<int> auto_onsets_choice;
  std::vector<int> auto_background_choice;
  //int auto_beats_choice = rand() % 3 + 0;
  //int auto_onsets_choice = rand() % 5 + 0;
  //int auto_background_choice = rand() % 5 + 0;

  for(int i = 0; i < segment_changes.size() + 2; i++) {
    int random_auto_beats_choice = random_int_0_3(rng);
    int random_auto_onsets_choice = random_int_0_3(rng);
    int random_auto_background_choice = random_int_0_3(rng);


    //std::cout << "random_auto_onsets_choice for segment " << i << " is: " << random_auto_onsets_choice << std::endl;
//    std::cout << "segment_changes[i].time: " << segment_changes[i].time << std::endl;
//    std::cout << "segment_changes[i].value: " << segment_changes[i].value << std::endl;

    auto_beats_choice.push_back(random_auto_beats_choice);
    auto_onsets_choice.push_back(random_auto_onsets_choice);
    auto_background_choice.push_back(random_auto_background_choice);
  }

  Logger::debug("segment_changes.size(): {}", segment_changes.size());
  Logger::debug("auto_onsets_choice.size(): {}", auto_onsets_choice.size());

  for(int i = 0; i < auto_onsets_choice.size(); i++) {
    std::cout << "auto_onsets_choice[" << i << "]: " << auto_onsets_choice[i] << std::endl;
  }

  for(int i = 0; i < lightshow->get_fixtures().size(); i++) {
    LightshowFixture & fix = lightshow->get_fixtures_reference()[i];
    std::string fix_type = fix.get_type();
    std::transform(fix_type.begin(), fix_type.end(), fix_type.begin(), ::tolower);

    if(fix_type == "OFF" || fix_type.empty())
      continue;

    std::string timestamps_type = fix.get_timestamps_type();
    std::transform(timestamps_type.begin(), timestamps_type.end(), timestamps_type.begin(), ::tolower);
    //std::cout << "timestamps_type: " << timestamps_type << std::endl;

    std::vector<float> timestamps;
    if(timestamps_type == "onsets")
      timestamps = lightshow->get_onset_timestamps();
    else if(timestamps_type == "onsets bass")
      timestamps = lightshow->get_onset_bass_timestamps();
    else if(timestamps_type == "onsets snare")
      timestamps = lightshow->get_onset_snare_timestamps();
    else if(timestamps_type.find("Beats") != string::npos || timestamps_type.find("beats") != string::npos) { // contains beats
      timestamps = lightshow->get_specific_beats(timestamps_type);
    }

    std::cout << timestamps_type << " timestamps.size(): " << timestamps.size() << std::endl;

    std::vector<std::string> colors = fix.get_colors();
//    std::cout << "colors: " << std::endl;
//    for(int i = 0; i < colors.size(); i++) {
//      std::cout << colors[i] << std::endl;
//    }
    if(!colors.empty() && colors[0] == "auto")
      colors = fix_types_with_colors.at(fix_type);
//    std::cout << "colors after: " << std::endl;
//    for(int i = 0; i < colors.size(); i++) {
//      std::cout << colors[i] << std::endl;
//    }


    // for anton aus tirol hardstyle
    /*segment_changes.clear();
    segment_changes.shrink_to_fit();
    segment_changes.push_back({26.5, 1});
    segment_changes.push_back({51, 1});
    segment_changes.push_back({77.5, 1});
    segment_changes.push_back({103, 1});
    segment_changes.push_back({128, 1});
    segment_changes.push_back({141, 1});*/

    // for Madsen Athlet
    /*segment_changes.clear();
    segment_changes.shrink_to_fit();
    segment_changes.push_back({13, 1});
    segment_changes.push_back({23, 1});
    segment_changes.push_back({34, 1});
    segment_changes.push_back({56, 1});
    segment_changes.push_back({68, 1});
    segment_changes.push_back({90, 1});
    segment_changes.push_back({101, 1});
    segment_changes.push_back({122, 1});
    segment_changes.push_back({133, 1});
    segment_changes.push_back({166, 1});
    segment_changes.push_back({189, 1});*/

    // for Architects Doomsday
    /* 33s
     * 46s
     * 67s (maybe 65s)
     * 106s
     * 152s
     * 184s
    */

    // for Rammstein Pussy
//    segment_changes.clear();
//    segment_changes.shrink_to_fit();
//    segment_changes.push_back({7, 1});
//    segment_changes.push_back({35, 1});
//    segment_changes.push_back({50, 1});
//    segment_changes.push_back({64, 1});
//    segment_changes.push_back({71, 1});
//    segment_changes.push_back({99, 1});
//    segment_changes.push_back({114, 1});
//    segment_changes.push_back({127, 1});
//    segment_changes.push_back({134, 1});
//    segment_changes.push_back({163, 1});
//    segment_changes.push_back({177, 1});
//    segment_changes.push_back({191, 1});
//    segment_changes.push_back({206, 1});

    // for Rammstein Was Ich Liebe
    /* maybe around 30s
     * 51s
     * 86s (or 89s)
     * 111s
     * 123s
     * maybe 126s
     * 158s
     * 181.5s
     * 204s
     * 221s (225s)
     * fade out (especially moving heads)?
    */






    // ######################################################################################################

//    if(fix.get_name() == "Cameo Flat RGB 10") {
//      std::cout << "CHANGING CAMEO" << std::endl;
//      fix.set_type("group_auto_beats");
////      fix.set_moving_head_type("group_auto_action");
//      fix.set_moving_head_type("group_auto_special");
//      fix.set_position_in_mh_group(fix.get_position_in_group());
//      fix.has_pan = true;
//      fix.has_tilt = true;
//      fix.set_channel_pan(15);
//      fix.set_channel_tilt(15);
//    }

    // ######################################################################################################


    if(fix.has_shutter) {
      std::vector<time_value_int> v;
      v.push_back({0.0, 255});
      v.push_back({end_of_song,
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

      float time_step = (float) 1 / frequency;
      int pan_center = 127;
      int tilt_center = 127;

      vc_tilt.push_back({0.0, tilt_center});
      vc_tilt.push_back({end_of_song, tilt_center-1});
      vc_pan.push_back({0.0, pan_center});
      vc_pan.push_back({end_of_song, pan_center-1});

      if(fix.get_modifier_pan() && fix.get_degrees_per_pan())
        pan_center = pan_center + (fix.get_modifier_pan() / fix.get_degrees_per_pan());
      if(fix.get_modifier_tilt() && fix.get_degrees_per_tilt()) {
        if(fix.get_invert_tilt())
          tilt_center = tilt_center - (fix.get_modifier_tilt() / fix.get_degrees_per_tilt());
        else
          tilt_center = tilt_center + (fix.get_modifier_tilt() / fix.get_degrees_per_tilt());
      }

      //std::cout << "pan_center: " << pan_center << std::endl;
      //std::cout << "tilt_center: " << tilt_center << std::endl;

      int amplitude_tilt;
      int amplitude_pan;
      float time_of_one_loop_tilt;
      float time_of_one_loop_pan;

      float time_of_one_beat = (float) 60 / (float) lightshow->get_bpm();
      float time_of_two_beats = 2.0f * time_of_one_beat;
      float time_of_two_bars = (float) 8 * time_of_one_beat;
      float time_of_four_bars = (float) 16 * time_of_one_beat;

      float left_right_switch = 0;


      if(fix.get_moving_head_type().find("auto_") != std::string::npos) {


        // needed for all auto types
        //auto segment_changes = lightshow->get_timestamps_segment_changes();
        // for now, insert the first segment
        if (!segment_changes.empty() && segment_changes[0].time > 2)
          segment_changes.insert(segment_changes.begin(), {0.0, 0});
        else if(segment_changes.empty()) {
          segment_changes.push_back({0.0, 0});
        }
        float segment_start = 0;
        float segment_end = 0;

        bool auto_action_last_segment_was_crossed_tilt = false;

        // looping through all segments
        for (int j = 0; j < segment_changes.size(); j++) {

          bool crazy_finale = false;

          segment_start = segment_changes[j].time;
          if (j < segment_changes.size() - 1)
            segment_end = segment_changes[j + 1].time;
          else
            segment_end = end_of_song;

          float segment_value = segment_changes[j].value;

          //std::cout << "segment_start: " << segment_start << std::endl;
          //std::cout << "segment_end: " << segment_end << std::endl;

          std::vector<float> onset_timestamps = lightshow->get_onset_timestamps_in_segment(segment_start, segment_end);
          // duration of segment / duration of one bar * how often an onsets should occur each bar
          float time_of_one_bar = (float) 4 * ((float) 60 / (float) lightshow->get_bpm());
          float eight_per_bar = (segment_end - segment_start) / time_of_one_bar * 8;
          float six_per_bar = (segment_end - segment_start) / time_of_one_bar * 6;
          float four_per_bar = (segment_end - segment_start) / time_of_one_bar * 4;
          float two_per_bar = (segment_end - segment_start) / time_of_one_bar * 2;
          float one_per_bar = (segment_end - segment_start) / time_of_one_bar;



          // GO CRAZY and switch between fixture kinds ++++++++++
          if(segment_value > 0.9 && lightshow->get_bpm() > 120) {
            if(segment_start == segment_changes[segment_changes.size() - 1].time) {                                   // if last segment
              if(segment_end - segment_start <= 15) {                                                                 // if segment is shorter than 15s

                int pan_position = 0;
                int tilt_position = 0;

                if(fix.get_moving_head_type().find("_action") != std::string::npos) {
                  tilt_position = 45 / fix.get_degrees_per_tilt();
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 false); // default position
                } else {
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_special,
                                                 false,
                                                 false); // default position
                }
                std::cout << fix.get_moving_head_type() << " default position for GO CRAZY" << std::endl;
                crazy_finale = true;
              }
            }
          }



          if(!crazy_finale) {
            if (fix.get_moving_head_type() == "auto_background"
                || fix.get_moving_head_type() == "group_auto_background") {

            } else if (fix.get_moving_head_type() == "auto_action"
                || fix.get_moving_head_type() == "group_auto_action") {
              if (segment_value < 0.1) {
                int pan_position = 0;
                int tilt_position = 0;
                this->generate_static_position(fix,
                                               pan_center,
                                               tilt_center,
                                               pan_position,
                                               tilt_position,
                                               segment_start,
                                               segment_end,
                                               fixtures_in_mh_group_auto_action,
                                               false,
                                               false); // default position
                std::cout << fix.get_moving_head_type() << " default position" << std::endl;
                auto_action_last_segment_was_crossed_tilt = false;
              } else if (segment_value < 0.5) {
                int pan_position = 0;
                int tilt_position = 0;
                this->generate_static_position(fix,
                                               pan_center,
                                               tilt_center,
                                               pan_position,
                                               tilt_position,
                                               segment_start,
                                               segment_end,
                                               fixtures_in_mh_group_auto_action,
                                               false,
                                               false); // default position
                std::cout << fix.get_moving_head_type() << " default position" << std::endl;
                auto_action_last_segment_was_crossed_tilt = false;
              } else if (segment_value < 0.75) {
                if (j > 0 && segment_changes[j - 1].value == 0) {
                  int pan_position = 0;
                  int tilt_position = 0;
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 false); // default position
                  std::cout << fix.get_moving_head_type() << " default position" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = false;
                } else {
                  int pan_position = 90 / fix.get_degrees_per_pan();
                  //if(fix.get_modifier_pan() > 0)
                  if (fix.get_position_in_mh_group() % 2 == 0)
                    pan_position = -90 / fix.get_degrees_per_pan();
                  int tilt_position = 25 / fix.get_degrees_per_tilt();
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 auto_action_last_segment_was_crossed_tilt); // crossed straight down/up
                  if (auto_action_last_segment_was_crossed_tilt)
                    std::cout << fix.get_moving_head_type() << " crossed straight down (inverted)" << std::endl;
                  else
                    std::cout << fix.get_moving_head_type() << " crossed straight down" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = !auto_action_last_segment_was_crossed_tilt;
                }

              } else if (segment_value < 0.9) {

//              if(onset_timestamps.size() >= six_per_bar) {
                if (!auto_action_last_segment_was_crossed_tilt) {
                  // just 45/90° into crowd, rest comes from lighting type
                  int pan_position = 0 / fix.get_degrees_per_pan();
//                int pan_position = 90 / fix.get_degrees_per_pan();
//                if(fix.get_modifier_pan() > 0)
//                  pan_position = -90 / fix.get_degrees_per_pan();
                  int tilt_position = 90 / fix.get_degrees_per_tilt();
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 false); // 90° into crowd
                  std::cout << fix.get_moving_head_type() << " 90° into crowd < 0.9" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = false;
                } else {
                  int pan_position = 90 / fix.get_degrees_per_pan();
                  //if(fix.get_modifier_pan() > 0)
                  if (fix.get_position_in_mh_group() % 2 == 0)
                    pan_position = -90 / fix.get_degrees_per_pan();
                  int tilt_position = 25 / fix.get_degrees_per_tilt();
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 auto_action_last_segment_was_crossed_tilt); // crossed straight down/up
                  if (auto_action_last_segment_was_crossed_tilt)
                    std::cout << fix.get_moving_head_type() << " crossed straight down (inverted)" << std::endl;
                  else
                    std::cout << fix.get_moving_head_type() << " crossed straight down" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = !auto_action_last_segment_was_crossed_tilt;
                }

              } else if (segment_value > 0.9) {
                // always 90° into crowd (or 45°)
                if (onset_timestamps.size() >= six_per_bar) {
                  fix.set_amplitude_pan(0);
                  fix.set_amplitude_tilt(25);
                  int tilt_offset = (int) std::round((float) 45 / (float) fix.get_degrees_per_tilt()); // or 90°
                  if (fix.get_invert_tilt())
                    tilt_offset = -1 * tilt_offset;
                  this->generate_vertical_line(fix,
                                               pan_center,
                                               tilt_center + tilt_offset,
                                               time_of_one_beat * 4,
                                               segment_start,
                                               segment_end,
                                               fixtures_in_mh_group_auto_action); // vertical lines 45° into crowd, check timing (time_of_one_beat * 4)
                  std::cout << fix.get_moving_head_type() << " 45° into crowd, vertical lines" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = false;
                } else {
                  // just 45/90° into crowd, rest comes from lighting type
                  int pan_position = 0 / fix.get_degrees_per_pan();
//                int pan_position = 90 / fix.get_degrees_per_pan();
//                if(fix.get_modifier_pan() > 0)
//                  pan_position = -90 / fix.get_degrees_per_pan();
                  int tilt_position = 90 / fix.get_degrees_per_tilt();
                  this->generate_static_position(fix,
                                                 pan_center,
                                                 tilt_center,
                                                 pan_position,
                                                 tilt_position,
                                                 segment_start,
                                                 segment_end,
                                                 fixtures_in_mh_group_auto_action,
                                                 false,
                                                 false); // 90° into crowd
                  std::cout << fix.get_moving_head_type() << " 90° into crowd > 0.9" << std::endl;
                  auto_action_last_segment_was_crossed_tilt = false;
                }

              }

            } else if (fix.get_moving_head_type() == "auto_special"
                || fix.get_moving_head_type() == "group_auto_special") {

              vc_pan.push_back({segment_end, pan_center-1});
              vc_tilt.push_back({segment_end, tilt_center-1});

              if (segment_value >= 0.95) {
                int pan_position = 0;
                int tilt_position = 0;
                this->generate_static_position(fix,
                                               pan_center,
                                               tilt_center,
                                               pan_position,
                                               tilt_position,
                                               segment_start,
                                               segment_end,
                                               fixtures_in_mh_group_auto_special,
                                               false,
                                               false); // default position
                std::cout << fix.get_moving_head_type() << " default position" << std::endl;
              } else if (segment_value < 0.1 && j > 0) {
                fix.set_amplitude_pan(60);
                fix.set_amplitude_tilt(40);
                //if(fix.get_moving_head_type() == "group_auto_background")
                this->generate_continuous_8(fix,
                                            pan_center,
                                            tilt_center,
                                            time_of_four_bars,
                                            time_of_four_bars,
                                            segment_start,
                                            segment_end,
                                            fixtures_in_mh_group_auto_special);
                std::cout << fix.get_moving_head_type() << " continuous_8" << std::endl;
              } else {
                int pan_position = 0;
                int tilt_position = 0;
                this->generate_static_position(fix,
                                               pan_center,
                                               tilt_center,
                                               pan_position,
                                               tilt_position,
                                               segment_start,
                                               segment_end,
                                               fixtures_in_mh_group_auto_special,
                                               false,
                                               false); // default position
                std::cout << fix.get_moving_head_type() << " default position" << std::endl;
              }

            }
          }


//          if(fix.get_moving_head_type() == "auto_background" || fix.get_moving_head_type() == "group_auto_background") {
//            if (onset_timestamps.size() >= eight_per_bar) {
//              fix.set_amplitude_pan(0);
//              fix.set_amplitude_tilt(25);
//              int tilt_offset = (int) std::round((float) 90 / (float) fix.get_degrees_per_tilt());
//              this->generate_vertical_line(fix, pan_center, tilt_center + tilt_offset, time_of_one_beat * 4, segment_start, segment_end, fixtures_in_mh_group_auto_background);
//
//            } else if (onset_timestamps.size() >= four_per_bar) {
//              if(random_int_0_1(rng) == 1) {
//                fix.set_amplitude_pan(30);
//                fix.set_amplitude_tilt(20);
//                int tilt_offset = (int) std::round((float) 75 / (float) fix.get_degrees_per_tilt());
//                this->generate_continuous_circle(fix, pan_center, tilt_center + tilt_offset, time_of_two_bars, time_of_two_bars, segment_start, segment_end, fixtures_in_mh_group_auto_background);
//              } else {
//                fix.set_amplitude_pan(0);
//                fix.set_amplitude_tilt(25);
//                int tilt_offset = (int) std::round((float) 90 / (float) fix.get_degrees_per_tilt());
//                this->generate_vertical_line(fix, pan_center, tilt_center + tilt_offset, time_of_two_beats * 4, segment_start, segment_end, fixtures_in_mh_group_auto_background);
//
//              }
//            } else if(onset_timestamps.size() >= two_per_bar) {
//              fix.set_amplitude_pan(30);
//              fix.set_amplitude_tilt(20);
//              this->generate_continuous_8(fix, pan_center, tilt_center, time_of_two_bars * 2, time_of_two_bars, segment_start, segment_end, fixtures_in_mh_group_auto_background);
//
//            } else {
//              /*fix.set_amplitude_pan(60);
//              fix.set_amplitude_tilt(40);
//              //if(fix.get_moving_head_type() == "group_auto_background")
//              this->generate_continuous_8(fix, pan_center, tilt_center, time_of_four_bars, time_of_four_bars, segment_start, segment_end, fixtures_in_mh_group_auto_background);
//              //else
//              //this->generate_continuous_8(fix, pan_center, tilt_center, time_of_four_bars * 2, time_of_four_bars * 2, segment_start, segment_end, 0);*/
//              int pan_position = 90 / fix.get_degrees_per_pan();
//              if(fix.get_modifier_pan() > 0)
//                pan_position = -90 / fix.get_degrees_per_pan();
//              int tilt_position = (float) 30 / fix.get_degrees_per_tilt();
//              this->generate_continuous_cross_change(fix, pan_center, tilt_center, pan_position, tilt_position, segment_start, segment_end, fixtures_in_mh_group_auto_background, time_of_two_bars);
//
//            }
//
//          }


          if(fix.get_type().find("auto_") != std::string::npos) {


            // GO CRAZY and switch between fixture kinds ++++++++++
            //std::cout << "pre GO CRAZY and switch between fixture kinds" << std::endl;
            //std::cout << "segment_value: " << segment_value << std::endl;
            //std::cout << "segment_start: " << segment_start << std::endl;
            //std::cout << "segment_end: " << segment_end << std::endl;
            if(segment_value > 0.9 && lightshow->get_bpm() > 120) {
              if(segment_start == segment_changes[segment_changes.size() - 1].time) {                                   // if last segment
                if(segment_end - segment_start <= 15) {                                                                 // if segment is shorter than 15s
                  for (auto const &x: fix_names_with_temp_position) {
                    std::cout << "x.first: " << x.first << std::endl;
                    std::cout << "fix.get_name(): " << fix.get_name() << std::endl;
                    //if (fix.get_name() == x.first)
                    if(fix.get_name().find(x.first) != std::string::npos)
                      fix.set_temp_position_in_group(x.second);
                  }
                  //std::cout << fix.get_temp_position_in_group() << std::endl;

//                std::vector<float> beats1234 = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
//                this->generate_group_one_kind_after_another(fix, beats1234, segment_start, segment_end, fix_names_with_temp_position.size());
                  this->generate_group_one_kind_after_another(fix, onset_timestamps, segment_start, segment_end, fix_names_with_temp_position.size());
                  std::cout << fix.get_type() << " lighting for GO CRAZY" << std::endl;
                  crazy_finale = true;
                }
              }
            }

            if(!crazy_finale) {
              if (fix.get_type() == "auto_beats" || fix.get_type() == "group_auto_beats") {
                if (fix.get_moving_head_type() == "auto_action" || fix.get_moving_head_type() == "group_auto_action") {

                  if (segment_value < 0.1) {

                    this->set_dimmer_values_in_segment(fix, segment_start, 0, segment_end, 0); // nothing
                    std::cout << fix.get_type() << " nothing" << std::endl;

                  } else if (segment_value < 0.5) {

                    this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 200); // ambient
                    std::cout << fix.get_type() << " ambient" << std::endl;

                  } else if (segment_value < 0.75) {

                    this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 200); // ambient
                    std::cout << fix.get_type() << " ambient" << std::endl;

                  } else if (segment_value < 0.9) {

                    if (lightshow->get_bpm() > 160) {

                      timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
                      if (fixtures_in_mh_group_auto_action % 4 == 0) { // group ABBA
                        this->generate_group_ABBA(fix, timestamps, segment_start, segment_end);
                        std::cout << fix.get_type() << " ABBA 2/4" << std::endl;
                      } else if (fixtures_in_mh_group_auto_action % 3 == 0) { // group ABA
                        this->generate_group_ABA(fix, timestamps, segment_start, segment_end);
                        std::cout << fix.get_type() << " ABA 2/4" << std::endl;
                      } else { // alternate odd even
                        this->generate_group_alternate_odd_even(fix, timestamps, segment_start, segment_end);
                        std::cout << fix.get_type() << " alternate_odd_even 2/4" << std::endl;
                      }

                    } else {
                      this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 200); // ambient
                      std::cout << fix.get_type() << " ambient" << std::endl;
                    }

                  } else if (segment_value > 0.9) {

                    std::string temp = "2/4";
                    if (onset_timestamps.size() >= six_per_bar) {

                      timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);

                      //this->generate_group_one_after_another_fade(fix, timestamps, segment_start, segment_end, fixtures_in_mh_group_auto_action, lightshow->get_resolution(), true);
                      this->generate_group_one_after_another_fade(fix,
                                                                  timestamps,
                                                                  segment_start,
                                                                  segment_end,
                                                                  fixtures_in_mh_group_auto_action,
                                                                  lightshow->get_resolution(),
                                                                  false);
                      //this->generate_group_one_after_another(fix, timestamps, segment_start, segment_end, fixtures_in_mh_group_auto_action); // on/off at every timestamp

                      //timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
                      //temp = "1/2/3/4";
                    } else {

                      if(j == segment_changes.size() - 1)
                        timestamps = lightshow->get_specific_beats("beats 2/4 action", segment_start, segment_end);
                      else
                        timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);

                      if (fix.get_type() == "auto_beats") {
                        this->generate_group_one_after_another(fix,
                                                               timestamps,
                                                               segment_start,
                                                               segment_end,
                                                               2); // on/off at every timestamp
                        std::cout << fix.get_type() << " one_after_another " << temp << std::endl;
                      } else {
                        if (fixtures_in_mh_group_auto_action % 4 == 0) { // group ABBA
                          this->generate_group_ABBA(fix, timestamps, segment_start, segment_end);
                          std::cout << fix.get_type() << " ABBA " << temp << std::endl;
                        } else if (fixtures_in_mh_group_auto_action % 3 == 0) { // group ABA
                          this->generate_group_ABA(fix, timestamps, segment_start, segment_end);
                          std::cout << fix.get_type() << " ABA " << temp << std::endl;
                        } else { // alternate odd even
                          this->generate_group_alternate_odd_even(fix, timestamps, segment_start, segment_end);
                          std::cout << fix.get_type() << " alternate_odd_even " << temp << std::endl;
                        }
                      }
                    }

                  }

                } else if (fix.get_moving_head_type() == "auto_special"
                    || fix.get_moving_head_type() == "group_auto_special") {

                  if (segment_value >= 0.95) {
                    this->set_dimmer_values_in_segment(fix, segment_start, 150, segment_end, 0); // ambient
                    std::cout << fix.get_type() << " ambient" << std::endl;
                  } else if (segment_value < 0.1 && j > 0) {
                    this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0); // ambient
                    std::cout << fix.get_type() << " ambient" << std::endl;
                  } else {
                    this->set_dimmer_values_in_segment(fix, segment_start, 0, segment_end, 0); // nothing
                    std::cout << fix.get_type() << " nothing" << std::endl;
                  }

                }

              } else if (fix.get_type() == "auto_onsets" || fix.get_type() == "group_auto_onsets") {

              }
            }

            this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
          }

        }

      } else if(fix.get_moving_head_type() == "Continuous 8" || fix.get_moving_head_type() == "Continuous 8 group") {
        this->generate_continuous_8(fix, pan_center, tilt_center, time_of_four_bars * 2, time_of_four_bars * 2, 0, end_of_song, fixtures_in_mh_group_continuous_8);

      } else if(fix.get_moving_head_type() == "Continuous Circle" || fix.get_moving_head_type() == "Continuous Circle group") {

        int offset_pan = fix.get_modifier_pan();
        int offset_tilt = fix.get_modifier_tilt();

        if(offset_tilt > 0 && offset_tilt < amplitude_tilt * 1.5)
          offset_tilt = offset_tilt * 1.6;
        else if(offset_tilt < 0 && offset_tilt > amplitude_tilt * 1.5)
          offset_tilt = offset_tilt * 1.6;

        if(offset_pan > 0 && offset_pan < amplitude_pan * 1.5)
          offset_pan = offset_pan * 1.6;
        else if(offset_pan < 0 && offset_pan > amplitude_pan * 1.5)
          offset_pan = offset_pan * 1.6;

        //pan_center = 127 + offset_pan;
        //tilt_center = 127 + offset_tilt;

        this->generate_continuous_circle(fix, pan_center, tilt_center, time_of_four_bars, time_of_four_bars, 0, end_of_song, fixtures_in_mh_group_continuous_circle);

      } else if(fix.get_moving_head_type() == "Continuous Line vertical" || fix.get_moving_head_type() == "Continuous Line vertical group") {
        this->generate_vertical_line(fix, pan_center, tilt_center, time_of_two_beats * 2, 0, end_of_song, fixtures_in_mh_group_continuous_line_vertical);

      } else if(fix.get_moving_head_type() == "Backlight, drop on action") {
        //amplitude_tilt = 100 / fix.get_degrees_per_tilt();
        //amplitude_pan = 0;
        amplitude_pan = (int) std::round(fix.get_amplitude_pan() / fix.get_degrees_per_pan());
        amplitude_tilt = (int) std::round(fix.get_amplitude_tilt() / fix.get_degrees_per_tilt());

        if(tilt_center + amplitude_tilt > 255)
          amplitude_tilt = 255 - tilt_center;
        else if(tilt_center - amplitude_tilt < 0)
          amplitude_tilt = tilt_center;
        if(pan_center + amplitude_pan > 255)
          amplitude_pan = 255 - pan_center;
        else if(pan_center - amplitude_pan < 0)
          amplitude_pan = pan_center;

        float time_per_step_tilt = time_of_one_beat;
        time_step = 0.025f;
        vc_tilt.push_back({0.0, tilt_center});
        vc_tilt.push_back({end_of_song, tilt_center-1});
        vc_pan.push_back({0.0, pan_center});
        vc_pan.push_back({end_of_song, pan_center-1});
        vc_zoom.push_back({0.0, 200});
        vc_zoom.push_back({end_of_song, 0});
        vc_focus.push_back({0.0, 200});
        vc_focus.push_back({end_of_song, 0});

        //std::vector<time_value_float> segment_changes = lightshow->get_timestamps_segment_changes();

        //std::cout << "all segment changes:" << std::endl;
        //for(time_value_int f: segment_changes) {
        //std::cout << f.time << "    " << f.value << std::endl;
        //}
        std::vector<float> timestamps_of_drops;
        for(int i = 1; i < segment_changes.size(); i++) {
          //if(segment_changes[i].value > segment_changes[i-1].value + 30 && segment_changes[i].time > segment_changes[i-1].time + time_of_four_bars) {
          if(segment_changes[i].value >= 0.9) {
            if(segment_changes[i].time - time_of_two_bars > 0)
              timestamps_of_drops.push_back(segment_changes[i].time - time_of_two_bars);
            else if(segment_changes[i].time - 2 * time_of_two_beats > 0)
              timestamps_of_drops.push_back(segment_changes[i].time - 2 * time_of_two_beats);
          }
        }
        for(float f: timestamps_of_drops)
          std::cout << "timestamp of drop: " << f << std::endl;


        int start_value = tilt_center;
        // TODO hier switch case plus oder minus
        int end_value = tilt_center - amplitude_tilt;
        float value_step = (float) (end_value - start_value) * time_step / time_of_two_bars;
        for(float begin_timestamp: timestamps_of_drops) {
          float time = begin_timestamp;
          float current_value = start_value;
          while(time <= begin_timestamp + time_of_two_bars) {
            if(fix.get_invert_tilt())
              current_value -= value_step;
            else
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

      } else {


        vc_tilt.push_back({0.0, tilt_center});
        vc_tilt.push_back({end_of_song, tilt_center-1});
        vc_pan.push_back({0.0, pan_center});
        vc_pan.push_back({end_of_song, pan_center-1});

        //fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
        //fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
      }


      fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
      fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
    }


    if(fix_type.find("auto_") != std::string::npos && fix.get_moving_head_type().find("auto_") == std::string::npos) {

      // for now, insert the first segment
      if (!segment_changes.empty() && segment_changes[0].time > 2)
        segment_changes.insert(segment_changes.begin(), {0.0, 0});
      else if(segment_changes.empty()) {
        segment_changes.push_back({0.0, 0});
      }
      float segment_start = 0;
      float segment_end = 0;
      float segment_value = 0;



      // looping through all segments
      for (int j = 0; j < segment_changes.size(); j++) {

        std::cout << "segment_changes[j].value: " << segment_changes[j].value << std::endl;

        segment_start = segment_changes[j].time;
        if (j < segment_changes.size() - 1)
          segment_end = segment_changes[j + 1].time;
        else
          segment_end = end_of_song;

        segment_value = segment_changes[j].value;

        //std::cout << "segment_start_seconds: " << segment_start << std::endl;
        //std::cout << "segment_end_seconds: " << segment_end << std::endl;

        std::vector<float> onset_timestamps = lightshow->get_onset_timestamps_in_segment(segment_start, segment_end);
        // duration of segment / duration of one bar * how often an onsets should occur each bar
        float time_of_one_bar = (float) 4 * ((float) 60 / (float) lightshow->get_bpm());
        float eight_per_bar = (segment_end - segment_start) / time_of_one_bar * 8;
        float six_per_bar = (segment_end - segment_start) / time_of_one_bar * 6;
        float four_per_bar = (segment_end - segment_start) / time_of_one_bar * 4;
        float two_per_bar = (segment_end - segment_start) / time_of_one_bar * 2;
        float one_per_bar = (segment_end - segment_start) / time_of_one_bar;
        //std::cout << "four_per_bar: " << four_per_bar << std::endl;
        //std::cout << "number of onset timestamps in segment: " << onset_timestamps.size() << std::endl;

        if(onset_timestamps.size() >= eight_per_bar)
          std::cout << ">= eight_per_bar" << std::endl;
        else if(onset_timestamps.size() >= six_per_bar)
          std::cout << ">= six_per_bar" << std::endl;
        else if(onset_timestamps.size() >= four_per_bar)
          std::cout << ">= four_per_bar" << std::endl;
        else if(onset_timestamps.size() >= two_per_bar)
          std::cout << ">= two_per_bar" << std::endl;
        else if(onset_timestamps.size() >= one_per_bar)
          std::cout << ">= one_per_bar" << std::endl;
        else
          std::cout << "< one_per_bar" << std::endl;

        // GO CRAZY and switch between fixture kinds ++++++++++
        if(segment_value > 0.9 && lightshow->get_bpm() > 120) {
          if(segment_start == segment_changes[segment_changes.size() - 1].time) {                                   // if last segment
            if(segment_end - segment_start <= 15) {                                                                 // if segment is shorter than 15s
              for (auto const &x: fix_names_with_temp_position) {
                if (fix.get_name() == x.first)
                  fix.set_temp_position_in_group(x.second);
              }
              //std::cout << fix.get_temp_position_in_group() << std::endl;

//          std::vector<float> beats1234 = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
//          this->generate_group_one_kind_after_another(fix, beats1234, segment_start, segment_end, fix_names_with_temp_position.size());
              this->generate_group_one_kind_after_another(fix, onset_timestamps, segment_start, segment_end, fix_names_with_temp_position.size());
              continue;
            }
          }
        }



        if(fix_type == "auto_beats") {

          if(segment_value < 0.1) {
            if(segment_start == segment_changes[0].time) {                                                            // if first segment
              this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);                            // ambient
            }
            else if(segment_start > end_of_song/2) {                                                                  // if segment is in 2nd half of song
              // nothing
            }

          }
          else if(segment_value < 0.5) {
            this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);                              // ambient
          }
          else if(segment_value < 0.75) {
            if(lightshow->get_bpm() < 100) {
              if(segment_start != segment_changes[0].time && segment_changes[j - 1].value == 1) {
                this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);                          // ambient
              } else {
                timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
                this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
                this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());    // pulse 2/4
              }
            } else {
              this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);                            // ambient
            }
          }
          else if(segment_value < 0.9) {
            if(onset_timestamps.size() >= six_per_bar) {
//              timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
              timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
              this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
              this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());      // pulse 2/4
            } else if(onset_timestamps.size() >= four_per_bar) {
              timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
              this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
              this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());      // pulse 2/4
            } else {
              this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);                            // ambient
            }
          }
          else if(segment_value > 0.9) {

            if(lightshow->get_bpm() < 100) {
              timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
            } else {
              if(segment_start == segment_changes[segment_changes.size() - 1].time)
                timestamps = lightshow->get_specific_beats("beats 1/2/3/4 action", segment_start, segment_end);       // only take action beats in last segment to catch fade out at the end
              else
                timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
            }


            this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
            this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());        // pulse 1/2/3/4
          }


//          if (onset_timestamps.size() >= eight_per_bar) {
//
//          } else if(onset_timestamps.size() >= six_per_bar) {
//
//          } else if(onset_timestamps.size() >= four_per_bar) {
//
//          } else if(onset_timestamps.size() >= two_per_bar) {
//
//          } else if(onset_timestamps.size() >= one_per_bar) {
//
//          } else {
//
//          }














//
//          if (onset_timestamps.size() >= four_per_bar)
//            timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
//          else if (onset_timestamps.size() >= two_per_bar)
//            timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
//          else if (onset_timestamps.size() >= one_per_bar)
//            timestamps = lightshow->get_specific_beats("beats 1", segment_start, segment_end);
//          else
//            continue;
//
//          if (fix.has_global_dimmer) {
//            if(auto_beats_choice[j] == 0 || !fix.has_global_dimmer) { // color change
//              this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
//              colors = fix_types_with_colors.at(fix_type);
//              this->generate_color_changes(fix, colors, timestamps, segment_end);
//            } else if(auto_beats_choice[j] == 1) { // pulse
//              this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
//              this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());
//            } else if(auto_beats_choice[j] == 2 || auto_beats_choice[j] == 3) {
//              if(onset_timestamps.size() >= four_per_bar) {
//                if(auto_beats_choice[j] == 2) { // blink
//                  this->generate_blink(fix, timestamps, segment_end);
//                } else if(auto_beats_choice[j] == 3) {
//                  if(lightshow->get_bpm() > 150) // reverse flash
//                    this->generate_flash_reverse(fix, timestamps, segment_start, segment_end);
//                  else { // pulse
//                    this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
//                    this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());
//                  }
//                }
//              } else {
//                if(auto_beats_choice[j] == 2) { // color change
//                  this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
//                  colors = fix_types_with_colors.at(fix_type);
//                  this->generate_color_changes(fix, colors, timestamps, segment_end);
//                } else if(auto_beats_choice[j] == 3) { // pulse
//                  this->set_dimmer_values_in_segment(fix, segment_start, 100, segment_end, 0);
//                  this->generate_pulse(fix, timestamps, segment_start, segment_end, 0, lightshow->get_resolution());
//                }
//              }
//            }
//          } else {
//          }
        } else if(fix_type == "group_auto_beats") {
          if (onset_timestamps.size() >= four_per_bar)
            timestamps = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
          else if (onset_timestamps.size() >= two_per_bar)
            timestamps = lightshow->get_specific_beats("beats 2/4", segment_start, segment_end);
          else if (onset_timestamps.size() >= one_per_bar)
            timestamps = lightshow->get_specific_beats("beats 1", segment_start, segment_end);
          else
            continue;

          if (fix.has_global_dimmer) {
            this->generate_group_one_after_another(fix, timestamps, segment_start, segment_end, fixtures_in_group_auto_beats);
            //std::vector<std::string> colors = fix.get_colors();
            //this->generate_color_changes(fix, colors, timestamps, segment_end);
          } else if(fix.has_shutter) {
            this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
            this->generate_group_one_after_another(fix, timestamps, segment_start, segment_end, fixtures_in_group_auto_beats);
          }


        } else if(fix_type == "auto_onsets" || fix_type == "group_auto_onsets") {
          if(onset_timestamps.size() >= eight_per_bar) {
            std::cout << ">= eight_per_bar" << std::endl;
            if(fix.has_pan && fix.has_tilt) {
              /*if(fix_type == "group_auto_onsets") {
                if(auto_onsets_choice[j] == 0) // flash reverse
                  this->generate_flash_reverse(fix, onset_timestamps, segment_start, segment_end);
                else if(auto_onsets_choice[j] == 1) // one after another
                  this->generate_group_one_after_another(fix, onset_timestamps, segment_start, segment_end, fixtures_in_group_auto_onsets);
                else if(auto_onsets_choice[j] == 2) // one after another fade
                  this->generate_group_one_after_another_fade(fix, onset_timestamps, segment_start, segment_end, fixtures_in_group_auto_onsets, lightshow->get_resolution(), true);
                else if(auto_onsets_choice[j] == 3) // one after another fade reverse
                  this->generate_group_one_after_another_fade_reverse(fix, onset_timestamps, segment_start, segment_end, fixtures_in_group_auto_onsets, lightshow->get_resolution(), true);
              } else {
                this->generate_flash_reverse(fix, onset_timestamps, segment_start, segment_end);
              }*/
            } else {
              if(fix_type == "group_auto_onsets") {
                if(auto_onsets_choice[j] == 0) {

                  if(fixtures_in_group_auto_onsets % 4 == 0) // group ABBA
                    this->generate_group_ABBA(fix, onset_timestamps, segment_start, segment_end);

                  else if(fixtures_in_group_auto_onsets % 3 == 0) // group ABA
                    this->generate_group_ABA(fix, onset_timestamps, segment_start, segment_end);

                  else if(fixtures_in_group_auto_onsets % 2 == 1) // alternate odd even
                    this->generate_group_alternate_odd_even(fix, onset_timestamps, segment_start, segment_end);

                  else // alternate odd even
                    this->generate_flash(fix, onset_timestamps, segment_start, segment_end);
//                    this->generate_group_alternate_odd_even(fix, onset_timestamps, segment_start, segment_end);

                } else if(auto_onsets_choice[j] == 1) { // flash
                  this->generate_flash(fix, onset_timestamps, segment_start, segment_end);
                } else if(auto_onsets_choice[j] == 2) { // flash reverse
                  this->generate_flash_reverse(fix, onset_timestamps, segment_start, segment_end);
                } else if(auto_onsets_choice[j] == 3) { // pulse on beats 1234
                  //std::vector<float> beats1234 = lightshow->get_specific_beats("beats 1/2/3/4", segment_start, segment_end);
                  //this->generate_pulse(fix, beats1234, segment_start, segment_end, fixtures_in_group_auto_onsets, lightshow->get_resolution()); // doesnt work that well
                  this->generate_flash(fix, onset_timestamps, segment_start, segment_end);
                }
              } else {
                //if(auto_onsets_choice[j] == 0) // blink
                //this->generate_blink(fix, onset_timestamps, segment_end);
                //else // flash
                this->generate_flash(fix, onset_timestamps, segment_start, segment_end);
              }
            }
          } else if (onset_timestamps.size() >= four_per_bar) {
            std::cout << ">= four_per_bar" << std::endl;
            if (fix.has_pan && fix.has_tilt) { // flash_reverse
              //this->generate_flash_reverse(fix, onset_timestamps, segment_start, segment_end);
              //this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
            } else {
              if (fix_type == "auto_onsets") {
//                if(auto_onsets_choice[j] == 0) // pulse
//                  this->generate_pulse(fix, onset_timestamps, segment_start, segment_end, fixtures_in_group_auto_onsets, lightshow->get_resolution());
//                else // blink
                if (segment_value < 0.6 || segment_value >= 0.9)
                  this->generate_blink(fix, onset_timestamps, segment_end);
              } else if (fix_type == "group_auto_onsets") {

                if (segment_value < 0.6 || segment_value >= 0.9) {
                  if (auto_onsets_choice[j] == 0) // blink back and forth
                    this->generate_group_blink_alternate_odd_even(fix, onset_timestamps, segment_end);
                    //this->generate_blink_back_and_forth(fix, onset_timestamps, fixtures_in_group_auto_onsets, segment_end); // TODO: blink alternate odd even
                  else if (auto_onsets_choice[j] == 1 || auto_onsets_choice[j] == 2) {

                    if (fixtures_in_group_auto_onsets % 4 == 0) // group ABBA
                      this->generate_group_ABBA(fix, onset_timestamps, segment_start, segment_end);

                    else if (fixtures_in_group_auto_onsets % 3 == 0) // group ABA
                      this->generate_group_ABA(fix, onset_timestamps, segment_start, segment_end);

                    else if (fixtures_in_group_auto_onsets % 2 == 1) // alternate odd even
                      this->generate_group_alternate_odd_even(fix, onset_timestamps, segment_start, segment_end);

                    else // blink back and forth
                      this->generate_blink(fix, onset_timestamps, segment_end);
//                    this->generate_blink_back_and_forth(fix, onset_timestamps, fixtures_in_group_auto_onsets, segment_end);
                  }
                   /*else if(auto_onsets_choice[j] == 2) // pulse
                    this->generate_pulse(fix, onset_timestamps, segment_start, segment_end, fixtures_in_group_auto_onsets, lightshow->get_resolution());
                  else if(auto_onsets_choice[j] == 3) // blink
                    this->generate_blink(fix, onset_timestamps, segment_end);*/
                  else // blink
                    this->generate_blink(fix, onset_timestamps, segment_end);

                }
              }
            }

          } else if (onset_timestamps.size() >= one_per_bar) {
            std::cout << ">= one_per_bar" << std::endl;
            if (fix.has_pan && fix.has_tilt && fix.has_colorwheel) { // do nothing
              if (fix.has_global_dimmer)
                this->set_dimmer_values_in_segment(fix, segment_start, 0, segment_end, 200);
            } else if (fix.has_pan && fix.has_tilt) { // color_change on onsets
              if (fix.has_global_dimmer) {
                this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
                std::vector<std::string> colors1 = fix_types_with_colors.at(fix_type);
                this->generate_color_changes(fix, colors1, onset_timestamps, segment_end);

              } else {

              }
            } else {
              if (fix_type == "auto_onsets") {
                // blink
                this->generate_blink(fix, onset_timestamps, segment_end);
              } else if (fix_type == "group_auto_onsets") {
                // blink back and forth
                //this->generate_blink_back_and_forth(fix, onset_timestamps, fixtures_in_group_auto_onsets, segment_end);
                this->generate_blink(fix, onset_timestamps, segment_end);
              }
            }
          } else {
            std::cout << "< one_per_bar (just blink)" << std::endl;
            if (fix.has_pan && fix.has_tilt) {
              // do nothing
              if (fix.has_global_dimmer)
                this->set_dimmer_values_in_segment(fix, segment_start, 0, segment_end, 200);
            } else {
              // blink
              this->generate_blink(fix, onset_timestamps, segment_end);
            }
          }

          //std::vector<std::string> colors = fix_types_with_colors.at(fix_type);// = fix.get_colors();
          //this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

        }
      }

//      std::vector<std::string> colors = fix.get_colors();
//      if(fix_type == "auto")
//        colors = fix_types_with_colors.at(fix_type);// = fix.get_colors();
      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
    }

    else if (fix_type == "bass") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_bass(), fix.get_channel_dimmer());

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_bass(), fix.get_channel_red());
      }
    } else if (fix_type == "mid") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_middle(), fix.get_channel_dimmer());

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_middle(), fix.get_channel_blue());
      }
    } else if (fix_type == "high") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_high(), fix.get_channel_dimmer());

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow->get_value_changes_high(), fix.get_channel_green());
      }
    } else if (fix_type == "ambient") {
      if (fix.has_global_dimmer) {
        this->set_dimmer_values_in_segment(fix, 0.0, 200, end_of_song, 0);

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

      } else {

      }
    } else if (fix_type == "color_change") {
      if (fix.has_global_dimmer) {
        this->set_dimmer_values_in_segment(fix, 0.0, 200, end_of_song, 0);

        this->generate_color_changes(fix, colors, timestamps, end_of_song);

      } else {

      }
    } else if (fix_type == "color_fade") {
      if (fix.has_global_dimmer) {
        this->set_dimmer_values_in_segment(fix, 0.0, 200, end_of_song, 0);

        this->generate_color_fades(fix, colors, timestamps, end_of_song, lightshow->get_resolution());

      } else {

      }
    } else if (fix_type == "flash") {
      if (fix.has_global_dimmer) {
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
          this->set_dimmer_values_in_segment(fix, 0.0, 200, end_of_song, 0);
          fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_shutter());
        } else {
          fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_dimmer());
        }

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }

    } else if (fix_type == "flash_reverse") {
      if (fix.has_global_dimmer) {
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
          this->set_dimmer_values_in_segment(fix, 0.0, 200, end_of_song, 0);
          fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_shutter());
        } else {
          fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_dimmer());
        }

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        for (int i = 0; i < timestamps.size(); i++) {
          value_changes.push_back({timestamps[i], 200});
          if(i < timestamps.size() - 1) {
            this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], end_of_song);
          }
          else {
            value_changes.push_back({end_of_song, 0});
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "pulse") {
      if (fix.has_global_dimmer) {
        this->generate_pulse(fix, timestamps, 0, end_of_song, 0, lightshow->get_resolution());
        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_one_after_another") {

      //fix.get_timestamps_type();
      this->generate_group_one_after_another(fix, timestamps, 0, end_of_song, fixtures_in_group_one_after_another);
      //this->generate_group_ABBA(fix, timestamps, 0, end_of_song);


      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
    } else if (fix_type == "group_one_after_another_fade") {
      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      this->generate_group_one_after_another_fade(fix, timestamps, 0, end_of_song, fixtures_in_group_one_after_another_fade, lightshow->get_resolution(), false);
    } else if (fix_type == "group_one_after_another_fade_reverse") {
      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      this->generate_group_one_after_another_fade_reverse(fix, timestamps, 0, end_of_song, fixtures_in_group_one_after_another_fade_reverse, lightshow->get_resolution(), false);
    } else if (fix_type == "group_one_after_another_fade_single") {
      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      this->generate_group_one_after_another_fade(fix, timestamps, 0, end_of_song, fixtures_in_group_one_after_another_fade_single, lightshow->get_resolution(), true);
    } else if (fix_type == "group_one_after_another_fade_single_reverse") {
      this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      this->generate_group_one_after_another_fade_reverse(fix, timestamps, 0, end_of_song, fixtures_in_group_one_after_another_fade_single_reverse, lightshow->get_resolution(), true);
    } else if (fix_type == "group_one_after_another_back_and_forth") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes_onset_blink;

        //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          uint8_t counter = 1;
          bool left_to_right = true;
          for (int i = 0; i < timestamps.size(); i++) {
            if(counter == fix.get_position_in_group()) {
              value_changes_onset_blink.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1)
                value_changes_onset_blink.push_back({timestamps[i + 1], 0});
              else
                value_changes_onset_blink.push_back({end_of_song, 0});
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
        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_one_after_another_back_and_forth_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          uint8_t counter = 1;
          bool left_to_right = true;
          for (int i = 0; i < timestamps.size(); i++) {
            if(counter == fix.get_position_in_group()) {
              value_changes.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1)
                this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], end_of_song);
              else
                value_changes.push_back({end_of_song, 0});
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
        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_one_after_another_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        //std::vector<float> timestamps = {0.452789, 0.917188, 1.38159, 1.84599, 2.31039, 2.77478, 3.22757, 3.68036, 4.13315, 4.60916, 5.08517, 5.54957, 6.0488, 6.54803, 7.04726, 7.55809, 8.06893, 8.57977, 9.12544, 9.67111, 10.2168, 10.7741, 11.3313, 11.8886, 12.4343, 12.98, 13.5605, 14.141, 14.7215, 15.2787, 15.836, 16.3933, 16.9506, 17.4962, 18.0419, 18.5876, 19.1332, 19.5512, 19.9692, 20.3871, 20.8051, 21.223, 21.641, 22.059, 22.4885, 22.9181, 23.3593, 23.7888, 24.2184, 24.6364, 25.0659, 25.5536, 26.0412, 26.5288, 27.0164, 27.5156, 27.9917, 28.4561, 28.9205, 29.3965, 29.7448, 30.0931, 30.4414, 30.7897, 31.1263, 31.4746, 31.8113, 32.1364, 32.4731, 32.8098, 33.1349, 33.46, 33.785, 34.1101, 34.4352, 34.7835, 35.1202, 35.4453, 35.7819, 36.1186, 36.4437, 36.7688, 37.0939, 37.419, 37.7556, 38.0923, 38.429, 38.7773, 39.1256, 39.4855, 39.8338, 40.1821, 40.5304, 40.8787, 41.227, 41.5753, 41.9236, 42.2603, 42.6086, 42.9569, 43.2936, 43.6303, 43.967, 44.2921, 44.6171, 44.9422, 45.2673, 45.5924, 45.9175, 46.2425, 46.5676, 46.9043, 47.241, 47.6009, 47.9608, 48.3323, 48.7039, 49.145, 49.5746, 50.0158, 50.4454, 50.9446, 51.4322, 51.9198, 52.4307, 52.9299, 53.4291, 53.9283, 54.4392, 54.9384, 55.4376, 55.9369, 56.4477, 56.9702, 57.5042, 58.1195, 58.7233, 59.3154, 59.9075, 60.4996, 61.0336, 61.5677, 62.1018, 62.6358, 63.1815, 63.7156, 64.2728, 64.8185, 65.4687, 66.1072, 66.7458, 67.3959, 68.0345, 68.673, 69.3116, 69.9385, 70.5538, 71.1808, 71.8309, 72.4695, 73.108, 73.7466, 74.3851, 75.0121, 75.639, 76.2659, 76.9045, 77.485, 78.0655, 78.6344, 79.2265, 79.8302, 80.4107, 80.9912, 81.5833, 82.1986, 82.8024, 83.3596, 83.8937, 84.4278, 84.9618, 85.5075, 86.0415, 86.5756, 87.098, 87.6205, 88.1429, 88.6538, 89.1762, 89.6987, 90.2095, 90.7204, 91.2196, 91.7188, 92.2297, 92.7289, 93.2281, 93.7273, 94.2382, 94.7606, 95.225, 95.6778, 96.119, 96.5602, 96.9317, 97.3032, 97.6631, 98.0346, 98.4062, 98.7777, 99.1492, 99.5207, 99.9039, 100.287, 100.67, 101.053, 101.436, 101.808, 102.179, 102.632, 103.097, 103.573, 104.049, 104.525, 105.001, 105.477, 105.941, 106.394, 106.858, 107.334, 107.799, 108.263, 108.82, 109.378, 109.946, 110.55, 111.154, 111.746, 112.338, 112.93, 113.429, 113.94, 114.44, 114.939, 115.438, 115.926, 116.413, 116.901, 117.388, 117.946, 118.515, 119.084, 119.641, 120.198, 120.767, 121.324, 121.87, 122.427, 122.996, 123.577, 124.134, 124.703, 125.272, 125.841, 126.398, 126.967, 127.536, 128.104, 128.662, 129.231, 129.8, 130.357, 130.914, 131.483, 132.052, 132.621, 133.19, 133.77, 134.339, 134.908, 135.465, 136.034, 136.591, 137.149, 137.718, 138.286, 138.855, 139.413, 139.97, 140.55, 141.119, 141.688, 142.257, 142.826, 143.383, 143.94, 144.498, 145.067, 145.624, 146.193, 146.773, 147.342, 147.911, 148.48, 149.037, 149.606, 150.175, 150.732, 151.301, 151.87, 152.451, 153.02, 153.577, 154.134, 154.691, 155.26, 155.829, 156.398, 156.967, 157.536, 158.093, 158.662, 159.242, 159.823, 160.403, 160.961, 161.518, 162.087, 162.656, 163.213, 163.77, 164.328, 164.896, 165.465, 166.057, 166.626, 167.195, 167.753, 168.321, 168.89, 169.448, 170.005, 170.574, 171.154, 171.735, 172.327, 172.942, 173.546, 174.057, 174.568, 175.078, 175.589, 176.089, 176.599, 177.11, 177.621, 178.143, 178.596, 179.049, 179.49, 179.943, 180.384, 180.825, 181.267, 181.696, 182.126, 182.544, 182.938, 183.333, 183.728, 184.123, 184.506, 184.889, 185.272, 185.678, 186.073, 186.468, 186.863, 187.373, 187.884, 188.395, 188.918, 189.428, 189.928, 190.427, 190.926, 191.425, 191.994, 192.563, 193.132, 193.712, 194.293, 194.862, 195.431, 196, 196.557, 197.126, 197.683, 198.252, 198.821, 199.39, 199.959, 200.528, 201.096, 201.665, 202.234, 202.803, 203.372, 203.941, 204.51, 205.079, 205.648, 206.216, 206.774, 207.331, 207.888, 208.457, 209.026, 209.595, 210.164, 210.733, 211.29, 211.859, 212.428, 212.997, 213.554, 214.111, 214.68, 215.249, 215.818, 216.375, 216.944, 217.525, 218.093, 218.662, 219.22, 219.788, 220.357, 220.926, 221.495, 222.064, 222.621, 223.179, 223.736, 224.305, 224.874, 225.431, 226, 226.569, 227.138, 227.706, 228.275, 228.844, 229.425, 229.994, 230.563, 231.131, 231.689, 232.258, 232.826, 233.395, 233.953, 234.522, 235.079, 235.648, 236.205, 236.774, 237.343, 237.912, 238.481, 239.026, 239.583, 240.152, 240.71, 241.279, 241.836, 242.405, 242.974, 243.554, 244.123, 244.703, 245.261, 245.83, 246.399, 246.967, 247.525, 248.094, 248.662, 249.243, 249.8, 250.358, 250.926, 251.507, 252.076, 252.645, 253.214, 253.771, 254.328, 254.885, 255.454, 256.023, 256.592, 257.161, 257.718, 258.287, 258.856, 259.413, 259.971, 260.539, 261.097, 261.666, 262.235, 262.803, 263.361, 263.918, 264.475, 265.044, 265.613, 266.17, 266.739, 267.308, 267.877, 268.457, 269.038, 269.607, 270.164, 270.721, 271.29, 271.859, 272.428, 272.997, 273.554, 274.123, 274.692, 275.261, 275.83, 276.399, 276.968, 277.536, 278.105, 278.674, 279.232, 279.789, 280.346, 280.915, 281.472, 282.041, 282.61, 283.179, 283.748, 284.305, 284.874, 285.513, 286.139, 286.766, 287.393, 288.02, 288.624, 289.239, 289.855, 290.458, 291.051, 291.643, 292.223, 292.792, 293.361, 293.941, 294.51, 295.091, 295.671, 296.252, 296.844, 297.482, 298.121, 298.771, 299.421, 300.06, 300.71, 301.36, 302.022, 302.684, 303.345, 304.007, 304.681, 305.354, 306.027, 306.701, 307.386, 308.117, 308.849, 309.568, 310.288, 310.996, 311.705, 312.366, 313.04, 313.678, 314.317, 314.955, 315.594, 316.233, 316.859, 317.486, 318.113, 318.74, 319.356, 320.029, 320.702, 321.306, 321.921, 322.537, 323.141, 323.744, 324.29, 324.836, 325.381, 325.915, 326.461, 327.053, 327.645, 328.237, 328.76, 329.282, 329.805, 330.304, 330.815, 331.314, 331.802, 332.289, 332.777, 333.264, 333.775, 334.286, 334.809, 335.343, 335.865, 336.387, 336.91, 337.432, 337.966};
        std::vector<time_value_int> value_changes;

        //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        //std::cout << "fixtures_in_group_one_after_another_blink: " << fixtures_in_group_one_after_another_blink << std::endl;
        //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          for (int i = 0; i < timestamps.size(); i++) {
            if(i % fixtures_in_group_one_after_another_blink + 1 == fix.get_position_in_group()) {
              value_changes.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1) {

                this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], end_of_song);

              }
              else {
                value_changes.push_back({end_of_song, 0});
              }
            }
          }

          fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
        }
        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_two_after_another") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({end_of_song, 0});
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

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_alternate_odd_even") {
      if(fix.has_global_dimmer) {
        this->generate_group_alternate_odd_even(fix, timestamps, 0, end_of_song);
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

      }
      /*if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({end_of_song, 0});
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

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }*/
    } else if (fix_type == "group_alternate_odd_even") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({end_of_song, 0});
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

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_alternate_odd_even_blink") {
      if (fix.has_global_dimmer) {

        //std::vector<float> timestamps = lightshow->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({end_of_song, 0});
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

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
    } else if (fix_type == "group_random_flashes") {
      if (fix.has_global_dimmer) {

        std::vector<float> onset_timestamps = lightshow->get_onset_timestamps();
        std::vector<float> begin_and_end_of_flashing_timestamps;
        int onset_counter = 0;
        float last_time = 0;
        float begin_timestamp = 0;
        float time_of_one_eigth = ((float) 60 / (float) lightshow->get_bpm()) / 2;
        //std::cout << "time_of_one_eigth: " << time_of_one_eigth << std::endl;
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

        //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        //std::cout << "begin_and_end_of_flashing_timestamps.size(): " << begin_and_end_of_flashing_timestamps.size() << std::endl;
        //std::cout << "old bpm analysis: " << lightshow->get_bpm() << std::endl;

        //std::random_device rd;     // only used once to initialise (seed) engine
        //std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
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
                if(end_of_song > time_of_next_flash + 0.025f)
                  value_changes.push_back({time_of_next_flash + 0.025f, 0});
                else
                  value_changes.push_back({end_of_song, 0});
              }
              time_of_next_flash += 0.025f;
            }
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        //std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades_on_segment_changes(lightshow, fix, colors);
      } else {

      }
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
          if(fix.has_flash_duration && fix.has_flash_rate) {
            fix.add_value_changes_to_channel(value_changes, fix.get_channel_flash_duration());
            fix.add_value_changes_to_channel(value_changes, fix.get_channel_flash_rate());
          }
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

            //std::vector<std::string> colors = fix.get_colors();
            this->generate_color_fades_on_segment_changes(lightshow, fix, colors);

          } else if (fix.has_shutter) {

          } else {

          }
        }
      }
    }

  }
  return lightshow;
}

void LightshowGenerator::generate_color_fades_on_segment_changes(std::shared_ptr<Lightshow> lightshow_from_analysis,
                                                                 LightshowFixture &fix,
                                                                 std::vector<std::string> &colors) {
  std::vector<color_change> color_changes;
  std::vector<time_value_float> timestamps = lightshow_from_analysis->get_timestamps_segment_changes();
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
  float end_of_song = (lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution();
  if(fix.has_colorwheel) {
    //this->set_colorwheel_color_changes(lightshow_from_analysis, fix, color_changes);
    this->set_hard_color_changes(fix, color_changes, end_of_song);
  } else {
    this->set_soft_color_changes(fix, color_changes, this->fade_duration, end_of_song, lightshow_from_analysis->get_resolution());
  }
}

void LightshowGenerator::set_soft_color_changes(LightshowFixture& fix, std::vector<color_change> color_changes, float _fade_duration, float end_of_song, int lightshow_resolution) {
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
//  float end_of_song = (lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution();

  while (counter < color_changes.size()) {

    timestamp_color_change = color_changes[counter].timestamp;
    timestamp_begin_fade = timestamp_color_change - _fade_duration / 2;

    if (counter == 0 && color_changes.size() > 1) {
      timestamp_end_fade = color_changes[counter + 1].timestamp + _fade_duration / 2;
      this->set_color_for_fixed_time(fix, color_changes[counter].color, color_changes[counter].timestamp, timestamp_end_fade);
      previous_color = color_changes[counter].color;
      counter++;
      continue;
    }

    color_values values_previous_color = color_to_rgb(previous_color);
    color_values values_new_color = color_to_rgb(color_changes[counter].color);

    for (time_value_int tv : this->calculate_single_color_fade(lightshow_resolution,
                                                               _fade_duration,
                                                               values_previous_color.r,
                                                               values_new_color.r)) {
      tv.time = tv.time + timestamp_begin_fade;
      data_pairs_red.push_back(tv);
    }
    for (time_value_int tv : this->calculate_single_color_fade(lightshow_resolution,
                                                               _fade_duration,
                                                               values_previous_color.g,
                                                               values_new_color.g)) {
      tv.time = tv.time + timestamp_begin_fade;
      if (tv.time < 0)
        tv.time = 0;
      data_pairs_green.push_back(tv);
    }
    for (time_value_int tv : this->calculate_single_color_fade(lightshow_resolution,
                                                               _fade_duration,
                                                               values_previous_color.b,
                                                               values_new_color.b)) {
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


void LightshowGenerator::set_hard_color_changes(LightshowFixture &fix, std::vector<color_change> color_changes, float end_of_last_color) {
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
        end_of_color = end_of_last_color;
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

std::vector<time_value_int> LightshowGenerator::calculate_single_color_fade(int lightshow_resolution,
                                                                            float fade_duration,
                                                                            int c_old,
                                                                            int c_new)
{
  std::vector<time_value_int> v;
  float change_per_step = (c_old - c_new) / ((lightshow_resolution - 5) * fade_duration);
  int color = c_old;
  float timestamp = 0;
  float time_step = 0;
  if (lightshow_resolution == 40)
    time_step = 0.025;
  else if (lightshow_resolution == 20)
    time_step = 0.05;
  else if (lightshow_resolution == 10)
    time_step = 0.1;


  //for(float timestamp = 0; timestamp < fade_duration; timestamp = timestamp + 1 / this->get_resolution()) {
  for (int i = 0; i < fade_duration * lightshow_resolution; i++) {
    color = c_old - i * change_per_step;
    if (i == fade_duration * lightshow_resolution - 1)
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

void LightshowGenerator::generate_color_changes(LightshowFixture &fix,
                                                std::vector<std::string> &colors, std::vector<float> timestamps,
                                                float end_of_last_color) {

  std::vector<color_change> color_changes;

  //std::cout << "Number of color changes in this lightshow: " << timestamps.size() << std::endl;

  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());

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
  this->set_hard_color_changes(fix, color_changes, end_of_last_color);


}

void LightshowGenerator::generate_color_fades(LightshowFixture &fix,
                                              std::vector<std::string> &colors, std::vector<float> timestamps,
                                              float end_of_song,
                                              int lightshow_resolution) {

  std::vector<color_change> color_changes;

  //std::cout << "Number of color changes in this lightshow: " << timestamps.size() << std::endl;

  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());

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

  this->set_soft_color_changes(fix, color_changes, 0.2, end_of_song, lightshow_resolution);

}

LightshowGenerator::~LightshowGenerator() {

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

void LightshowGenerator::set_dimmer_values_in_segment(LightshowFixture & fix,
                                                      float segment_start,
                                                      int start_value,
                                                      float segment_end,
                                                      int end_value) {
  std::vector<time_value_int> v;
  v.push_back({segment_start, start_value});
  v.push_back({segment_end, end_value});
  fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
}

void LightshowGenerator::generate_flash(LightshowFixture & fix, std::vector<float> & onset_timestamps, float segment_start, float segment_end) {
  Logger::debug("generate_flash");
  if (fix.has_global_dimmer) {
    std::vector<time_value_int> value_changes_flash;
    value_changes_flash.push_back({segment_start, 0});

    if(fix.has_shutter) {
      for (int i = 0; i < onset_timestamps.size(); i++) {
        if (onset_timestamps[i] - 0.050f > 0) {
          value_changes_flash.push_back({onset_timestamps[i] - 0.050f, 0});
        }
        value_changes_flash.push_back({onset_timestamps[i], 255});
        if (i < onset_timestamps.size() && onset_timestamps[i] + 0.050f < onset_timestamps[onset_timestamps.size() - 1]) {
          value_changes_flash.push_back({onset_timestamps[i] + 0.050f, 0});
        }
      }
    } else {
      for (int i = 0; i < onset_timestamps.size(); i++) {
        if (onset_timestamps[i] - 0.050f > 0) {
          value_changes_flash.push_back({onset_timestamps[i] - 0.050f, 0});
          value_changes_flash.push_back({onset_timestamps[i] - 0.025f, 100});
        }
        value_changes_flash.push_back({onset_timestamps[i], 200});
        if(i < onset_timestamps.size() - 1) {
          if (i < onset_timestamps.size() && onset_timestamps[i] + 0.050f < onset_timestamps[onset_timestamps.size() - 1]) {
            value_changes_flash.push_back({onset_timestamps[i] + 0.025f, 100});
            value_changes_flash.push_back({onset_timestamps[i] + 0.050f, 0});
          }
        } else {
          if(onset_timestamps[i] + 0.050f < segment_end) {
            value_changes_flash.push_back({onset_timestamps[i] + 0.025f, 100});
            value_changes_flash.push_back({onset_timestamps[i] + 0.050f, 0});
          }
          else {
            value_changes_flash.push_back({onset_timestamps[i] + 0.025f, 0});
          }
        }
      }
    }

    if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_shutter());
    } else {
      fix.add_value_changes_to_channel(value_changes_flash, fix.get_channel_dimmer());
    }

  } else {

  }
}

void LightshowGenerator::generate_flash_reverse(LightshowFixture &fix,
                                                std::vector<float> &onset_timestamps,
                                                float segment_start,
                                                float segment_end) {
  Logger::debug("generate_flash_reverse");
  if (fix.has_global_dimmer) {
    std::vector<time_value_int> value_changes_flash_reverse;

    if(fix.has_shutter) {
      value_changes_flash_reverse.push_back({segment_start, 255});
      value_changes_flash_reverse.push_back({segment_end, 0});
      for (int i = 0; i < onset_timestamps.size(); i++) {
        if (onset_timestamps[i] - 0.050f > 0) {
          value_changes_flash_reverse.push_back({onset_timestamps[i] - 0.050f, 255});
        }
        value_changes_flash_reverse.push_back({onset_timestamps[i], 0});
        if (i < onset_timestamps.size() && onset_timestamps[i] + 0.050f < onset_timestamps[onset_timestamps.size() - 1]) {
          value_changes_flash_reverse.push_back({onset_timestamps[i] + 0.050f, 255});
        }
      }
    } else {
      value_changes_flash_reverse.push_back({segment_start, 200});
      value_changes_flash_reverse.push_back({segment_end, 0});
      for (int i = 0; i < onset_timestamps.size(); i++) {
        if (onset_timestamps[i] - 0.050f > 0) {
          value_changes_flash_reverse.push_back({onset_timestamps[i] - 0.050f, 200});
          value_changes_flash_reverse.push_back({onset_timestamps[i] - 0.025f, 100});
        }
        value_changes_flash_reverse.push_back({onset_timestamps[i], 0});
        if (i < onset_timestamps.size() && onset_timestamps[i] + 0.050f < onset_timestamps[onset_timestamps.size() - 1]) {
          value_changes_flash_reverse.push_back({onset_timestamps[i] + 0.025f, 100});
          value_changes_flash_reverse.push_back({onset_timestamps[i] + 0.050f, 200});
        }
      }
    }

    if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 200, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_shutter());
    } else {
      fix.add_value_changes_to_channel(value_changes_flash_reverse, fix.get_channel_dimmer());
    }

  } else {

  }
}

void LightshowGenerator::generate_blink(LightshowFixture & fix, std::vector<float> & timestamps, float segment_end) {
  Logger::debug("generate_blink");
  if (fix.has_global_dimmer) {
    std::vector<time_value_int> value_changes;
    for (int k = 0; k < timestamps.size(); k++) {
      value_changes.push_back({timestamps[k], 200});
      if (k < timestamps.size() - 1) {
        this->generate_blink_fade_outs(value_changes, timestamps[k], timestamps[k + 1], segment_end);
      } else {
        if(segment_end - timestamps[k] > 0.5)
          this->generate_blink_fade_outs(value_changes, timestamps[k], timestamps[k] + 0.5, segment_end);
        else
          this->generate_blink_fade_outs(value_changes, timestamps[k], segment_end, segment_end);
      }
    }
    fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
  } else {

  }
}

void LightshowGenerator::generate_blink_back_and_forth(LightshowFixture & fix, std::vector<float> & timestamps, int group_counter, float segment_end) {
  Logger::debug("generate_blink_back_and_forth");
  if (fix.has_global_dimmer) {
    std::vector<time_value_int> value_changes;
    if(fix.get_position_in_group() > 0) {
      uint8_t counter = 1;
      bool left_to_right = true;
      for (int i = 0; i < timestamps.size(); i++) {
        if(counter == fix.get_position_in_group()) {
          value_changes.push_back({timestamps[i], 200});
          if(i < timestamps.size() - 1)
            this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i+1], segment_end);
          else {
            if (segment_end - timestamps[i] > 0.5)
              this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i] + 0.5, segment_end);
            else
              this->generate_blink_fade_outs(value_changes, timestamps[i], segment_end, segment_end);
          }
        }

        if(left_to_right)
          counter++;
        else
          counter--;

        if(counter == group_counter)
          left_to_right = false;
        else if(counter == 1)
          left_to_right = true;
      }

      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    }
  } else {

  }
}

void LightshowGenerator::generate_continuous_8(LightshowFixture & fix, int pan_center, int tilt_center, float time_of_one_loop_pan, float time_of_one_loop_tilt, float start_timestamp, float end_timestamp, int number_of_fixtures_in_group) {
  Logger::debug("generate_continuous_8");
  int amplitude_pan = (int) std::round(fix.get_amplitude_pan() / fix.get_degrees_per_pan());
  int amplitude_tilt = (int) std::round(fix.get_amplitude_tilt() / fix.get_degrees_per_tilt());

  std::vector<time_value_int> vc_pan;
  std::vector<time_value_int> vc_tilt;

  if(tilt_center + amplitude_tilt > 255)
    amplitude_tilt = 255 - tilt_center;
  else if(tilt_center - amplitude_tilt < 0)
    amplitude_tilt = tilt_center;
  if(pan_center + amplitude_pan > 255)
    amplitude_pan = 255 - pan_center;
  else if(pan_center - amplitude_pan < 0)
    amplitude_pan = pan_center;

  float left_right_switch = 0;
  float invert_tilt = 0;

  float current_timestamp = start_timestamp;
  uint8_t value = 0;
  if(fix.get_position_on_stage() == "Left")
    left_right_switch = PI;

  if(fix.get_invert_tilt())
    invert_tilt = PI;

  float group_offset = 0;
  if((fix.get_moving_head_type() == "Continuous 8 group" || fix.get_moving_head_type() == "group_auto_background" || fix.get_moving_head_type() == "group_auto_special" || fix.get_moving_head_type() == "group_auto_action") && number_of_fixtures_in_group > 0) {
    group_offset = (float) 2 * (float) (fix.get_position_in_mh_group() - 1) / number_of_fixtures_in_group;
  }

  while(current_timestamp < end_timestamp) {
    // ( cos(2 * PI * t * f + left_right_switch + (group_offset * PI)) * amplitude_pan ) + pan_center
    // x * cos(2 * PI * f * t + phi) + pan_center mit phi = (group_offset * PI)
    value = (int) (cos((2*PI*current_timestamp)/(time_of_one_loop_pan) + left_right_switch + (group_offset * PI)) * amplitude_pan) + pan_center;
    vc_pan.push_back({current_timestamp, value});
    value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + invert_tilt + (group_offset * PI)) * amplitude_tilt) + tilt_center;
    /*if(fix.get_invert_tilt())
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI) + PI) * amplitude_tilt) + tilt_center;
    else
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI)) * amplitude_tilt) + tilt_center;*/
    vc_tilt.push_back({current_timestamp, value});
    current_timestamp += 0.025f;
  }

  fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
  fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
}

void LightshowGenerator::generate_continuous_circle(LightshowFixture & fix, int pan_center, int tilt_center, float time_of_one_loop_pan, float time_of_one_loop_tilt, float start_timestamp, float end_timestamp, int number_of_fixtures_in_group) {
  Logger::debug("generate_continuous_circle");
  int amplitude_pan = (int) std::round(fix.get_amplitude_pan() / fix.get_degrees_per_pan());
  int amplitude_tilt = (int) std::round(fix.get_amplitude_tilt() / fix.get_degrees_per_tilt());

  std::vector<time_value_int> vc_pan;
  std::vector<time_value_int> vc_tilt;

  if(tilt_center + amplitude_tilt > 255)
    amplitude_tilt = 255 - tilt_center;
  else if(tilt_center - amplitude_tilt < 0)
    amplitude_tilt = tilt_center;
  if(pan_center + amplitude_pan > 255)
    amplitude_pan = 255 - pan_center;
  else if(pan_center - amplitude_pan < 0)
    amplitude_pan = pan_center;

  float left_right_switch = 0;

  float current_timestamp = start_timestamp;
  uint8_t value = 0;
  if(fix.get_position_on_stage() == "Left")
    left_right_switch = PI;

  float group_offset = 0;
  if((fix.get_moving_head_type() == "Continuous Circle group" || fix.get_moving_head_type() == "group_auto_background" || fix.get_moving_head_type() == "group_auto_special" || fix.get_moving_head_type() == "group_auto_action") && number_of_fixtures_in_group > 0) {
    group_offset = (float) 2 * (float) (fix.get_position_in_mh_group() - 1) / number_of_fixtures_in_group;
  }

  while(current_timestamp < end_timestamp) {
    value = (int) (cos((2*PI*current_timestamp)/(time_of_one_loop_pan) + left_right_switch + (group_offset * PI)) * amplitude_pan) + pan_center;
    vc_pan.push_back({current_timestamp, value});
    if(fix.get_invert_tilt())
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI) + PI) * amplitude_tilt) + tilt_center;
    else
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI)) * amplitude_tilt) + tilt_center;
    vc_tilt.push_back({current_timestamp, value});
    current_timestamp += 0.025f;
  }

  fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
  fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
}

std::vector<time_value_int> LightshowGenerator::generate_single_fade(int start_value,
                                                                     int end_value,
                                                                     float start_timestamp,
                                                                     float end_timestamp,
                                                                     int lightshow_resolution) {
  std::vector<time_value_int> v;
  float time_step = 0;
  if (lightshow_resolution == 40)
    time_step = 0.025;
  else if(lightshow_resolution == 20)
    time_step = 0.05;
  //std::cout << "lightshow_resolution: " << lightshow_resolution << std::endl;
  //lightshow_resolution = 40;
  //time_step = 0.025;

  float fade_duration = end_timestamp - start_timestamp;
  float change_per_step = (end_value - start_value) / ((lightshow_resolution - 3) * fade_duration);

  int value = 0;
  float timestamp = start_timestamp;
  /*std::cout << "fade_duration: " << fade_duration << std::endl;
  std::cout << "time_step: " << time_step << std::endl;
  std::cout << "change_per_step: " << change_per_step << std::endl;
  std::cout << "start_value: " << start_value << std::endl;
  std::cout << "end_value: " << end_value << std::endl;
  std::cout << "fade_duration * lightshow_resolution - 2: " << fade_duration * lightshow_resolution - 2 << std::endl;*/

  //for (int i = 0; i < fade_duration * lightshow_resolution; i++) {
  int i = 0;
  while(timestamp < end_timestamp) {
    value = (int) (start_value + i * change_per_step);
    //if (i >= fade_duration * lightshow_resolution - 2)
    if(timestamp + time_step >= end_timestamp)
      value = end_value;

    if(start_value >= end_value) {
      if (value > start_value)
        value = start_value;
      else if (value < end_value)
        value = end_value;
    } else {
      if (value < start_value)
        value = start_value;
      else if (value > end_value)
        value = end_value;
    }
    //std::cout << timestamp << ": " << value << std::endl;
    v.push_back({ timestamp, value });
    timestamp = timestamp + time_step;
    i++;
  }

  return v;
}

void LightshowGenerator::generate_group_one_after_another(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end, int fixtures_in_group) {
  Logger::debug("generate_group_one_after_another");
  std::vector<time_value_int> value_changes;

  //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
  //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
  if(fix.get_position_in_group() > 0) {
    for (int i = 0; i < timestamps.size(); i++) {
      if(i % fixtures_in_group + 1 == fix.get_position_in_group()) {
        value_changes.push_back({timestamps[i], 255});
        if(i < timestamps.size() - 1)
          value_changes.push_back({timestamps[i + 1], 0});
        else
          value_changes.push_back({segment_end, 0});
      }
    }

    if (fix.has_global_dimmer) {
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    } else if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_shutter());
    }
  }
}

void LightshowGenerator::generate_group_one_kind_after_another(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end, int fixtures_in_group) {
  Logger::debug("generate_group_one_kind_after_another");
  std::cout << "generate_group_one_kind_after_another" << std::endl;
  std::vector<time_value_int> value_changes;

  //std::cout << "pos in grp: " << fix.get_temp_position_in_group() << std::endl;
  //std::cout << "numbers of fixture kinds: " << fixtures_in_group << std::endl;
  //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
  if(fix.get_temp_position_in_group() > 0) {
    for (int i = 0; i < timestamps.size(); i++) {
      if(i % fixtures_in_group + 1 == fix.get_temp_position_in_group()) {
        value_changes.push_back({timestamps[i], 255});
        if(i < timestamps.size() - 1)
          value_changes.push_back({timestamps[i + 1], 0});
        else
          value_changes.push_back({segment_end, 0});
      }
    }


    if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_shutter());
    }
    else if (fix.has_global_dimmer) {
      //std::cout << "adding " << value_changes.size() << " value changes to dimmer" << std::endl;
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    } else if(fix.is_blinder) {
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_blinder()); // TODO: check if X5 really stays on continuously >250 DMX value
      if(fix.has_flash_rate) {
        std::vector<time_value_int> value_changes_flash_rate;
        value_changes_flash_rate.push_back({segment_start, 0});
        value_changes_flash_rate.push_back({segment_end, fix.get_blinder_value()});
        fix.add_value_changes_to_channel(value_changes_flash_rate, fix.get_channel_flash_rate());
      }
    }
  }
}

void LightshowGenerator::generate_group_ABA(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end) {
  std::cout << "generate_group_ABA" << std::endl;
  Logger::debug("generate_group_ABA");
  std::vector<time_value_int> value_changes;

  int use = 1;
  if(fix.get_position_in_group() % 3 == 1 || fix.get_position_in_group() % 3 == 0)
    use = 0;

  if(fix.get_position_in_group() > 0) {
    for (int i = 0; i < timestamps.size(); i++) {
      if(i % 2 == use) {
        value_changes.push_back({timestamps[i], 255});
        if(i < timestamps.size() - 1)
          value_changes.push_back({timestamps[i + 1], 0});
        else
          value_changes.push_back({segment_end, 0});
      }
    }

    if (fix.has_global_dimmer) {
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    } else if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_shutter());
    }
  }
}

void LightshowGenerator::generate_group_ABBA(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end) {
  Logger::debug("generate_group_ABBA");
  std::vector<time_value_int> value_changes;

  int use = 1;
  if(fix.get_position_in_group() % 4 == 1 || fix.get_position_in_group() % 4 == 0)
    use = 0;

  if(fix.get_position_in_group() > 0) {
    for (int i = 0; i < timestamps.size(); i++) {
      if(i % 2 == use) {
        value_changes.push_back({timestamps[i], 255});
        if(i < timestamps.size() - 1)
          value_changes.push_back({timestamps[i + 1], 0});
        else
          value_changes.push_back({segment_end, 0});
      }
    }

    if (fix.has_global_dimmer) {
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    } else if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_shutter());
    }
  }
}

void LightshowGenerator::generate_group_alternate_odd_even(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end) {
  Logger::debug("generate_group_alternate_odd_even");
  std::vector<time_value_int> value_changes;

  int use = 1;
  if(fix.get_position_in_group() % 2 == 0)
    use = 0;

  if(fix.get_position_in_group() > 0) {
    for (int i = 0; i < timestamps.size(); i++) {
      if(i % 2 == use) {
        value_changes.push_back({timestamps[i], 255});
        if(i < timestamps.size() - 1)
          value_changes.push_back({timestamps[i + 1], 0});
        else
          value_changes.push_back({segment_end, 0});
      }
    }

    if (fix.has_global_dimmer) {
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    } else if(fix.has_shutter) {
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);
      fix.add_value_changes_to_channel(value_changes, fix.get_channel_shutter());
    }
  }
}

void LightshowGenerator::generate_group_blink_alternate_odd_even(LightshowFixture & fix, std::vector<float> & timestamps, float segment_end) {
  if(fix.has_global_dimmer) {
    std::cout << "generate_group_blink_alternate_odd_even" << std::endl;
    Logger::debug("generate_group_blink_alternate_odd_even");
    std::vector<time_value_int> value_changes;

    int use = 1;
    if(fix.get_position_in_group() % 2 == 0)
      use = 0;
    if (fix.get_position_in_group() > 0) {
      for (int i = 0; i < timestamps.size(); i++) {
        if (i % 2 == use) {
          value_changes.push_back({timestamps[i], 200});
          if (i < timestamps.size() - 1)
            this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i + 1], segment_end);
          else {
            if (segment_end - timestamps[i] > 0.5)
              this->generate_blink_fade_outs(value_changes, timestamps[i], timestamps[i] + 0.5, segment_end);
            else
              this->generate_blink_fade_outs(value_changes, timestamps[i], segment_end, segment_end);
          }
        }
      }

      fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
    }
  }
}

void LightshowGenerator::generate_group_one_after_another_fade(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end, int fixtures_in_group, int lightshow_resolution, bool only_single_lamp) {
  Logger::debug("generate_group_one_after_another_fade");
  std::cout << "generate_group_one_after_another_fade" << std::endl;
  std::cout << "fix.pos: " << fix.get_position_in_group() << std::endl;
  std::cout << "fixtures_in_group: " << fixtures_in_group << std::endl;
  std::cout << "timestamps.size: " << timestamps.size() << std::endl;
  if (fix.has_global_dimmer) {
    //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
    //timestamps = lightshow->get_specific_beats("beats 1/2/3/4");
    //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
    std::vector<float> timestamps_for_this_fixture;
    if(fix.get_position_in_group() > 0) {
      for (int j = 0; j < timestamps.size(); j++) {
        if(j % fixtures_in_group + 1 == fix.get_position_in_group()) {
          timestamps_for_this_fixture.push_back(timestamps[j]);
        }
      }

      for(int j = 0; j < timestamps_for_this_fixture.size(); j++) {
        // fade in
        if(j == 0) {
          if(timestamps_for_this_fixture[j] - 0.5 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] - 0.3 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.3, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] - 0.1 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.1, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.025, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
        } else {
          std::vector<time_value_int> v;
          if(only_single_lamp) {
            if(timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
              v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution);
            else
              v = this->generate_single_fade(0, 255, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);
          } else {
            //if(only_single_lamp && timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
            //v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution);
            //else
            v = this->generate_single_fade(0, 255, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);
          }
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
        }

        // fade out
        if(j < timestamps_for_this_fixture.size() - 1) {
          std::vector<time_value_int> v;
          if(only_single_lamp) {
            if(timestamps_for_this_fixture[j+1] - timestamps_for_this_fixture[j] > 0.5)
              v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution);
            else
              v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], (timestamps_for_this_fixture[j] + timestamps_for_this_fixture[j + 1]) / (float) 2, lightshow_resolution);
          } else {
            //if(only_single_lamp && timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
            //v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution);
            //else
            v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], (timestamps_for_this_fixture[j] + timestamps_for_this_fixture[j + 1]) / (float) 2, lightshow_resolution);
          }
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
        } else {
          if(timestamps_for_this_fixture[j] + 0.5 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] + 0.3 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.3, lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] + 0.1 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.1, lightshow_resolution), fix.get_channel_dimmer());
          else
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.025, lightshow_resolution), fix.get_channel_dimmer());
        }
      }
    }
  }
}

void LightshowGenerator::generate_group_one_after_another_fade_reverse(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end, int fixtures_in_group, int lightshow_resolution, bool only_single_lamp) {
  Logger::debug("generate_group_one_after_another_fade_reverse");
  if (fix.has_global_dimmer) {
    //std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
    //timestamps = lightshow->get_specific_beats("beats 1/2/3/4");
    //std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
    std::vector<float> timestamps_for_this_fixture;
    if(fix.get_position_in_group() > 0) {
      for (int j = 0; j < timestamps.size(); j++) {
        if(j % fixtures_in_group + 1 == fix.get_position_in_group()) {
          timestamps_for_this_fixture.push_back(timestamps[j]);
        }
      }
      this->set_dimmer_values_in_segment(fix, segment_start, 255, segment_end, 0);

      for(int j = 0; j < timestamps_for_this_fixture.size(); j++) {
        // fade out
        if(j == 0) {
          if(timestamps_for_this_fixture[j] - 0.5 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] - 0.3 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.3, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] - 0.1 > segment_start)
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.1, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
          else
            fix.add_value_changes_to_channel(this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.025, timestamps_for_this_fixture[j], lightshow_resolution), fix.get_channel_dimmer());
        } else {
          std::vector<time_value_int> v;
          if(only_single_lamp) {
            if(timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
              v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution);
            else
              v = this->generate_single_fade(255, 0, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);
          } else {
            //if(only_single_lamp && timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
            //v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution);
            //else
            v = this->generate_single_fade(255, 0, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);
          }
          if(only_single_lamp && timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
            v = this->generate_single_fade(255, 0, timestamps_for_this_fixture[j] - 0.5, timestamps_for_this_fixture[j], lightshow_resolution);
          else
            v = this->generate_single_fade(255, 0, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);

          //std::vector<time_value_int> v = this->generate_single_fade(255, 0, (timestamps_for_this_fixture[j - 1] + timestamps_for_this_fixture[j]) / (float) 2, timestamps_for_this_fixture[j], lightshow_resolution);
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
        }

        // fade in
        if(j < timestamps_for_this_fixture.size() - 1) {
          std::vector<time_value_int> v;
          if(only_single_lamp) {
            if(timestamps_for_this_fixture[j + 1] - timestamps_for_this_fixture[j] > 0.5)
              v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution);
            else
              v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], (timestamps_for_this_fixture[j] + timestamps_for_this_fixture[j + 1]) / (float) 2, lightshow_resolution);
          } else {
            //if(only_single_lamp && timestamps_for_this_fixture[j] - timestamps_for_this_fixture[j - 1] > 0.5)
            //v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution);
            //else
            v = this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], (timestamps_for_this_fixture[j] + timestamps_for_this_fixture[j + 1]) / (float) 2, lightshow_resolution);
          }
          fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());
        } else {
          if(timestamps_for_this_fixture[j] + 0.5 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.5, lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] + 0.3 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.3, lightshow_resolution), fix.get_channel_dimmer());
          else if(timestamps_for_this_fixture[j] + 0.1 < segment_end)
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.1, lightshow_resolution), fix.get_channel_dimmer());
          else
            fix.add_value_changes_to_channel(this->generate_single_fade(0, 255, timestamps_for_this_fixture[j], timestamps_for_this_fixture[j] + 0.025, lightshow_resolution), fix.get_channel_dimmer());
        }
      }
    }
  }
}

void LightshowGenerator::generate_vertical_line(LightshowFixture & fix, int pan_center, int tilt_center, float time_of_one_loop_tilt, float start_timestamp, float end_timestamp, int number_of_fixtures_in_group) {
  Logger::debug("generate_vertical_line");
  int amplitude_pan = (int) std::round(fix.get_amplitude_pan() / fix.get_degrees_per_pan());
  int amplitude_tilt = (int) std::round(fix.get_amplitude_tilt() / fix.get_degrees_per_tilt());

  std::vector<time_value_int> vc_pan;
  std::vector<time_value_int> vc_tilt;

  if(tilt_center + amplitude_tilt > 255)
    amplitude_tilt = 255 - tilt_center;
  else if(tilt_center - amplitude_tilt < 0)
    amplitude_tilt = tilt_center;
  if(pan_center + amplitude_pan > 255)
    amplitude_pan = 255 - pan_center;
  else if(pan_center - amplitude_pan < 0)
    amplitude_pan = pan_center;

  float current_timestamp = start_timestamp;
  uint8_t value = 0;
  if(fix.get_position_on_stage() == "Center")
    vc_pan.push_back({start_timestamp, pan_center});
  else if(fix.get_position_on_stage() == "Left")
    vc_pan.push_back({start_timestamp, pan_center - amplitude_pan});
  else if(fix.get_position_on_stage() == "Right")
    vc_pan.push_back({start_timestamp, pan_center + amplitude_pan});
  vc_pan.push_back({end_timestamp, pan_center-1});

  float group_offset = 0;
  if(fix.get_moving_head_type() == "Continuous Line vertical group" || fix.get_moving_head_type() == "group_auto_background" || fix.get_moving_head_type() == "group_auto_action" || fix.get_moving_head_type() == "group_auto_special") {
    group_offset = (float) 2 * (float) (fix.get_position_in_mh_group() - 1) / number_of_fixtures_in_group;
  }

  while(current_timestamp < end_timestamp) {
    if(fix.get_invert_tilt())
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI) + PI) * amplitude_tilt) + tilt_center;
    else
      value = (int) (sin((2*PI*current_timestamp)/(time_of_one_loop_tilt) + (group_offset * PI)) * amplitude_tilt) + tilt_center;
    vc_tilt.push_back({current_timestamp, value});
    current_timestamp += 0.025f;
  }

  vc_pan.push_back({end_timestamp, pan_center}); // maybe not needed
  vc_tilt.push_back({end_timestamp, tilt_center}); // maybe not needed

  fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
  fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());

}

void LightshowGenerator::generate_continuous_cross_change(LightshowFixture &fix,
                                                          int pan_center,
                                                          int tilt_center,
                                                          int pan_position,
                                                          int tilt_position,
                                                          float segment_start,
                                                          float segment_end,
                                                          int number_of_fixtures_in_group,
                                                          float time_of_two_bars) {
  Logger::debug("generate_continuous_cross_change");
  if(number_of_fixtures_in_group > 1) {
    std::vector<time_value_int> vc_pan;
    std::vector<time_value_int> vc_tilt;
    float current_timestamp = segment_start;
    bool swapped = false;

    if(fix.get_position_in_mh_group() % 2 == 1) {
      vc_pan.push_back({segment_start, pan_center + pan_position});
      vc_tilt.push_back({segment_start, tilt_center + tilt_position});
      while(current_timestamp + time_of_two_bars < segment_end) {
        current_timestamp += time_of_two_bars;
        if(swapped) {
          vc_tilt.push_back({current_timestamp, tilt_center + tilt_position});
        } else {
          vc_tilt.push_back({current_timestamp, tilt_center - tilt_position});
        }
        swapped = !swapped;
      }
    } else {
      vc_pan.push_back({segment_start, pan_center + pan_position});
      vc_tilt.push_back({segment_start, tilt_center - tilt_position});
      while(current_timestamp + time_of_two_bars < segment_end) {
        current_timestamp += time_of_two_bars;
        if(swapped) {
          vc_tilt.push_back({current_timestamp, tilt_center - tilt_position});
        } else {
          vc_tilt.push_back({current_timestamp, tilt_center + tilt_position});
        }
        swapped = !swapped;
      }
    }
    fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
    fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
  }
}


void LightshowGenerator::generate_static_position(LightshowFixture &fix,
                                                          int pan_center,
                                                          int tilt_center,
                                                          int pan_position,
                                                          int tilt_position,
                                                          float segment_start,
                                                          float segment_end,
                                                          int number_of_fixtures_in_group,
                                                          bool crossed,
                                                          bool tilt_inverted) {

  std::cout << "generate static position, tilt_inverted: " << tilt_inverted << " crossed: " << crossed << std::endl;
  Logger::debug("generate_continuous_cross_change");

  if(fix.get_invert_tilt())
    tilt_position = -1 * tilt_position;

  if(number_of_fixtures_in_group > 1) {
    std::vector<time_value_int> vc_pan;
    std::vector<time_value_int> vc_tilt;

    if (crossed) {
      if (fix.get_position_in_mh_group() % 2 == 1) {
        vc_pan.push_back({segment_start, pan_center + pan_position});
        if(tilt_inverted)
          vc_tilt.push_back({segment_start, tilt_center - tilt_position});
        else
          vc_tilt.push_back({segment_start, tilt_center + tilt_position});
      } else {
        vc_pan.push_back({segment_start, pan_center + pan_position});
        if(tilt_inverted)
          vc_tilt.push_back({segment_start, tilt_center + tilt_position});
        else
          vc_tilt.push_back({segment_start, tilt_center - tilt_position});
      }
    } else {
      vc_pan.push_back({segment_start, pan_center + pan_position});
      if (tilt_inverted)
        vc_tilt.push_back({segment_start, tilt_center - tilt_position});
      else
        vc_tilt.push_back({segment_start, tilt_center + tilt_position});
    }

    vc_pan.push_back({segment_end, pan_center}); // maybe not needed
    vc_tilt.push_back({segment_end, tilt_center}); // maybe not needed


    fix.add_value_changes_to_channel(vc_pan, fix.get_channel_pan());
    fix.add_value_changes_to_channel(vc_tilt, fix.get_channel_tilt());
  }
}

void LightshowGenerator::generate_pulse(LightshowFixture & fix, std::vector<float> & timestamps, float segment_start, float segment_end, int fixtures_in_group, int lightshow_resolution) {
  Logger::debug("generate_pulse");
  std::vector<time_value_int> value_changes;
  float time_difference = 0;

  value_changes.push_back({segment_start, 100});

  for(int j = 0; j < timestamps.size(); j++) {

    if (j == 0) {
//      time_difference = timestamps[j] - segment_start;
//      if(time_difference > 0.1)
//        time_difference = 0.1;
//      else if(time_difference < 0.05)
        time_difference = 0.05;

      if(timestamps[j] - segment_start > 0.2) {
        fix.add_value_changes_to_channel(this->generate_single_fade(100,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      } else {
        fix.add_value_changes_to_channel(this->generate_single_fade(200,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      }

      time_difference = timestamps[j+1] - timestamps[j] - 0.01;
      if(time_difference > 0.2)
        time_difference = 0.2;
      else if(time_difference < 0.1)
        time_difference = 0.1;
      else if(time_difference < 0.05)
        time_difference = 0.05;

      if(timestamps[j+1] - timestamps[j] > 0.2) {
        fix.add_value_changes_to_channel(this->generate_single_fade(255,
                                                                    100,
                                                                    timestamps[j] + time_difference / 2,
                                                                    timestamps[j] + time_difference,
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      } else {
        fix.add_value_changes_to_channel(this->generate_single_fade(255,
                                                                    200,
                                                                    timestamps[j] + time_difference / 4,
                                                                    timestamps[j] + time_difference * 3 / 4,
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      }


    } else if (j < timestamps.size() - 1) {
      time_difference = timestamps[j] - timestamps[j-1];
      if(time_difference > 0.1)
        time_difference = 0.1;
      else if(time_difference < 0.05)
        time_difference = 0.05;

      /*if (timestamps[j] - 0.35 > timestamps[j-1])
        time_difference = 0.35;
      else if (timestamps[j] - 0.3 > timestamps[j-1])
        time_difference = 0.3;
      else if (timestamps[j] - 0.25 > timestamps[j-1])
        time_difference = 0.25;
      else if (timestamps[j] - 0.2 > timestamps[j-1])
        time_difference = 0.2;
      else if (timestamps[j] - 0.15 > timestamps[j-1])
        time_difference = 0.15;
      else if (timestamps[j] - 0.1 > timestamps[j-1])
        time_difference = 0.1;
      else
        time_difference = 0.05;*/

      if(timestamps[j] - timestamps[j-1] > 0.2) {
        fix.add_value_changes_to_channel(this->generate_single_fade(100,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      } else {
        fix.add_value_changes_to_channel(this->generate_single_fade(200,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      }

      time_difference = timestamps[j+1] - timestamps[j] - 0.01;
      if(time_difference > 0.2)
        time_difference = 0.2;
      else if(time_difference < 0.1)
        time_difference = 0.1;
      else if(time_difference < 0.05)
        time_difference = 0.05;

      /*if (timestamps[j] + 1 < timestamps[j+1])
        time_difference = 1;
      else if (timestamps[j] + 0.7 < timestamps[j+1])
        time_difference = 0.69;
      else if (timestamps[j] + 0.35 < timestamps[j+1])
        time_difference = 0.34;
      else if (timestamps[j] + 0.3 < timestamps[j+1])
        time_difference = 0.29;
      else if (timestamps[j] + 0.25 < timestamps[j+1])
        time_difference = 0.24;
      else if (timestamps[j] + 0.2 < timestamps[j+1])
        time_difference = 0.19;
      else if (timestamps[j] + 0.15 < timestamps[j+1])
        time_difference = 0.14;
      else if (timestamps[j] + 0.1 < timestamps[j+1])
        time_difference = 0.09;
      else
        time_difference = 0.05;*/

      if(timestamps[j+1] - timestamps[j] > 0.2) {
        fix.add_value_changes_to_channel(this->generate_single_fade(255,
                                                                    100,
                                                                    timestamps[j] + time_difference / 3,
                                                                    timestamps[j] + time_difference * 3 / 4,
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      } else {
        fix.add_value_changes_to_channel(this->generate_single_fade(255,
                                                                    200,
                                                                    timestamps[j] + time_difference / 4,
                                                                    timestamps[j] + time_difference * 3 / 4,
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      }

    } else {

      time_difference = timestamps[j] - timestamps[j-1];
      if(time_difference > 0.1)
        time_difference = 0.1;
      else if(time_difference < 0.05)
        time_difference = 0.05;

      if(timestamps[j] - timestamps[j-1] > 0.2) {
        fix.add_value_changes_to_channel(this->generate_single_fade(100,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      } else {
        fix.add_value_changes_to_channel(this->generate_single_fade(200,
                                                                    255,
                                                                    timestamps[j] - time_difference / 3,
                                                                    timestamps[j],
                                                                    lightshow_resolution),
                                         fix.get_channel_dimmer());
      }

//      if (timestamps[j] + 0.35 < segment_end)
//        time_difference = 0.35;
//      else if (timestamps[j] + 0.3 < segment_end)
//        time_difference = 0.3;
//      else if (timestamps[j] + 0.25 < segment_end)
//        time_difference = 0.25;
//      else if (timestamps[j] + 0.2 < segment_end)
//        time_difference = 0.2;
//      else if (timestamps[j] + 0.15 < segment_end)
//        time_difference = 0.15;
//      else if (timestamps[j] + 0.1 < segment_end)
//        time_difference = 0.1;


      time_difference = segment_end - timestamps[j] - 0.01;
      if(time_difference > 0.2)
        time_difference = 0.2;
      else if(time_difference < 0.1)
        time_difference = 0.1;
      else if(time_difference < 0.05)
        time_difference = 0.05;


      fix.add_value_changes_to_channel(this->generate_single_fade(255,
                                                                  100,
                                                                  timestamps[j] + time_difference / 2,
                                                                  timestamps[j] + time_difference,
                                                                  lightshow_resolution),
                                       fix.get_channel_dimmer());


    }
  }

  fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

}