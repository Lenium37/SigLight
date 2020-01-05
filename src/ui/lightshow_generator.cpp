//
// Created by Jan on 06.06.2019.
//

#include <random>
#include "lightshow_generator.h"


LightshowGenerator::LightshowGenerator() {

}

std::shared_ptr<Lightshow> LightshowGenerator::generate(int resolution, Song *song, std::list<Fixture> fixtures) {
  std::shared_ptr<Lightshow> lightshow_from_analysis = std::make_shared<Lightshow>();
  lightshow_from_analysis->set_resolution(resolution);

  lightshow_from_analysis->set_sound_src(song->get_file_path());
  lightshow_from_analysis->prepare_analysis_for_song((char*)song->get_file_path().c_str());

  int fixtures_in_group_one_after_another = 0;
  int fixtures_in_group_one_after_another_blink = 0;
  int fixtures_in_group_two_after_another = 0;
  int fixtures_in_group_two_after_another_blink = 0;
  int fixtures_in_group_alternate_odd_even = 0;
  int fixtures_in_group_alternate_odd_even_blink = 0;
  int fixtures_in_group_random_flashes = 0;

  std::vector<LightshowFixture> my_fixtures;
  for (Fixture fix: fixtures) {
    //std::cout << "new fix. name: " << fix.get_name() << ". start address: " << fix.get_start_channel() << ", number of addresses: " << fix.get_channel_count() << std::endl;
    if (fix.get_name() == "Cameo Flat RGB 10"
    || fix.get_name() == "Stairville LED Flood Panel 150 (3ch)"
    || fix.get_name() == "Stairville LED Flood Panel 150 (4ch)"
    || fix.get_name() == "Helios 7"
    || fix.get_name() == "Cobalt Plus Spot 5R"
    || fix.get_name() == "Varytec PAD7 seventy"
    || fix.get_name() == "TOURSPOT PRO"
    || fix.get_name() == "BAR TRI-LED") {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type(), fix.get_colors(), fix.get_position_in_group()));
    } else std::cout << "Fixture type unknown." << std::endl;

    if(fix.get_type() == "group_one_after_another" && fix.get_position_in_group() > fixtures_in_group_one_after_another)
      fixtures_in_group_one_after_another = fix.get_position_in_group();
    else if(fix.get_type() == "group_one_after_another_blink" && fix.get_position_in_group() > fixtures_in_group_one_after_another_blink)
      fixtures_in_group_one_after_another_blink = fix.get_position_in_group();
    else if(fix.get_type() == "group_two_after_another" && fix.get_position_in_group() > fixtures_in_group_two_after_another)
      fixtures_in_group_two_after_another = fix.get_position_in_group();
    else if(fix.get_type() == "group_two_after_another_blink" && fix.get_position_in_group() > fixtures_in_group_two_after_another_blink)
      fixtures_in_group_two_after_another_blink = fix.get_position_in_group();
    else if(fix.get_type() == "group_alternate_odd_even" && fix.get_position_in_group() > fixtures_in_group_alternate_odd_even)
      fixtures_in_group_alternate_odd_even = fix.get_position_in_group();
    else if(fix.get_type() == "group_alternate_odd_even_blink" && fix.get_position_in_group() > fixtures_in_group_alternate_odd_even_blink)
      fixtures_in_group_alternate_odd_even_blink = fix.get_position_in_group();
    else if(fix.get_type() == "group_random_flashes" && fix.get_position_in_group() > fixtures_in_group_random_flashes)
      fixtures_in_group_random_flashes = fix.get_position_in_group();


  }


  for (LightshowFixture fix: my_fixtures) {
    std::string fix_type = fix.get_type();
    std::transform(fix_type.begin(), fix_type.end(), fix_type.begin(), ::tolower);

    if (fix_type == "bass") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        //std::cout << colors.size() << std::endl;
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(), fix.get_channel_red());
      }
      lightshow_from_analysis->add_fixture_bass(fix);
    } else if (fix_type == "mid") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(), fix.get_channel_blue());
      }
      lightshow_from_analysis->add_fixture_middle(fix);
    } else if (fix_type == "high") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(), fix.get_channel_green());
      }
      lightshow_from_analysis->add_fixture_high(fix);
    } else if (fix_type == "ambient") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "color_change_beats") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_beat_color_changes(lightshow_from_analysis, fix, colors, false);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "color_change_beats_action") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        //std::cout << colors.size() << std::endl;
        this->generate_beat_color_changes(lightshow_from_analysis, fix, colors, true);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "color_change_onsets") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        //std::cout << colors.size() << std::endl;
        this->generate_onset_color_changes(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "onset_flash") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::cout << "timestamps.size() onset_blink: " << timestamps.size() << std::endl;
        std::vector<time_value_int> value_changes_onset_blink;

        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes_onset_blink.push_back({timestamps[i] - 0.050f, 0});
            value_changes_onset_blink.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes_onset_blink.push_back({timestamps[i], 200});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes_onset_blink.push_back({timestamps[i] + 0.025f, 100});
            value_changes_onset_blink.push_back({timestamps[i] + 0.050f, 0});
          }
        }

        //std::cout << value_changes_onset_blink.size() << std::endl;
        fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "onset_flash_reverse") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::cout << "timestamps.size() onset_blink_reverse: " << timestamps.size() << std::endl;
        std::vector<time_value_int> value_changes_onset_blink;

        value_changes_onset_blink.push_back({0.0, 200});
        value_changes_onset_blink.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
        for (int i = 0; i < timestamps.size(); i++) {
          if (timestamps[i] - 0.050f > 0) {
            value_changes_onset_blink.push_back({timestamps[i] - 0.050f, 200});
            value_changes_onset_blink.push_back({timestamps[i] - 0.025f, 100});
          }
          value_changes_onset_blink.push_back({timestamps[i], 0});
          if (i < timestamps.size() && timestamps[i] + 0.050f < timestamps[timestamps.size() - 1]) {
            value_changes_onset_blink.push_back({timestamps[i] + 0.025f, 100});
            value_changes_onset_blink.push_back({timestamps[i] + 0.050f, 200});
          }
        }

        std::cout << value_changes_onset_blink.size() << std::endl;

        fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    }else if (fix_type == "onset_blink") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::cout << "timestamps.size() onset_blink: " << timestamps.size() << std::endl;
        std::vector<time_value_int> value_changes;

        for (int i = 0; i < timestamps.size(); i++) {
            value_changes.push_back({timestamps[i], 200});
            if(i < timestamps.size() - 1) {

              // TODO make nicer with loop(s)
              if(timestamps[i + 1] - timestamps[i] > 0.5 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.5f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i] + 0.275f, 113});
                value_changes.push_back({timestamps[i] + 0.3f, 100});
                value_changes.push_back({timestamps[i] + 0.325f, 88});
                value_changes.push_back({timestamps[i] + 0.35f, 75});
                value_changes.push_back({timestamps[i] + 0.375f, 63});
                value_changes.push_back({timestamps[i] + 0.4f, 50});
                value_changes.push_back({timestamps[i] + 0.425f, 38});
                value_changes.push_back({timestamps[i] + 0.45f, 25});
                value_changes.push_back({timestamps[i] + 0.475f, 13});
                value_changes.push_back({timestamps[i] + 0.5f, 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.45 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.45f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i] + 0.275f, 113});
                value_changes.push_back({timestamps[i] + 0.3f, 100});
                value_changes.push_back({timestamps[i] + 0.325f, 88});
                value_changes.push_back({timestamps[i] + 0.35f, 75});
                value_changes.push_back({timestamps[i] + 0.375f, 63});
                value_changes.push_back({timestamps[i] + 0.4f, 50});
                value_changes.push_back({timestamps[i] + 0.425f, 38});
                value_changes.push_back({timestamps[i] + 0.45f, 25});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.4 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.4f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i] + 0.275f, 113});
                value_changes.push_back({timestamps[i] + 0.3f, 100});
                value_changes.push_back({timestamps[i] + 0.325f, 88});
                value_changes.push_back({timestamps[i] + 0.35f, 75});
                value_changes.push_back({timestamps[i] + 0.375f, 63});
                value_changes.push_back({timestamps[i] + 0.4f, 50});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.35 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.35f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i] + 0.275f, 113});
                value_changes.push_back({timestamps[i] + 0.3f, 100});
                value_changes.push_back({timestamps[i] + 0.325f, 88});
                value_changes.push_back({timestamps[i] + 0.35f, 75});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.3 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.3f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i] + 0.275f, 113});
                value_changes.push_back({timestamps[i] + 0.3f, 100});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.25 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.25f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i] + 0.225f, 133});
                value_changes.push_back({timestamps[i] + 0.25f, 125});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.2 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.2f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i] + 0.175f, 160});
                value_changes.push_back({timestamps[i] + 0.2f, 150});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.15 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.15f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i] + 0.125f, 178});
                value_changes.push_back({timestamps[i] + 0.15f, 170});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.1 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.1f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i] + 0.075f, 190});
                value_changes.push_back({timestamps[i] + 0.1f, 185});
                value_changes.push_back({timestamps[i + 1], 0});
              } else if(timestamps[i + 1] - timestamps[i] > 0.05 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.05f) {
                value_changes.push_back({timestamps[i] + 0.025f, 198});
                value_changes.push_back({timestamps[i] + 0.05f, 195});
                value_changes.push_back({timestamps[i + 1], 0});
              }
              else
                value_changes.push_back({timestamps[i + 1], 0});
            }
            else {
              value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
            }
        }

        //std::cout << value_changes_onset_blink.size() << std::endl;
        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_one_after_another") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
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
                value_changes_onset_blink.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
            }
          }

          fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_one_after_another_blink") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        std::cout << "pos in grp: " << fix.get_position_in_group() << std::endl;
        std::cout << "timestamps.size(): " << timestamps.size() << std::endl;
        if(fix.get_position_in_group() > 0) {
          for (int i = 0; i < timestamps.size(); i++) {
            if(i % fixtures_in_group_one_after_another_blink + 1 == fix.get_position_in_group()) {
              value_changes.push_back({timestamps[i], 200});
              if(i < timestamps.size() - 1) {

                // TODO make nicer with loop(s)
                if(timestamps[i + 1] - timestamps[i] > 0.5 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.5f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i] + 0.275f, 113});
                  value_changes.push_back({timestamps[i] + 0.3f, 100});
                  value_changes.push_back({timestamps[i] + 0.325f, 88});
                  value_changes.push_back({timestamps[i] + 0.35f, 75});
                  value_changes.push_back({timestamps[i] + 0.375f, 63});
                  value_changes.push_back({timestamps[i] + 0.4f, 50});
                  value_changes.push_back({timestamps[i] + 0.425f, 38});
                  value_changes.push_back({timestamps[i] + 0.45f, 25});
                  value_changes.push_back({timestamps[i] + 0.475f, 13});
                  value_changes.push_back({timestamps[i] + 0.5f, 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.45 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.45f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i] + 0.275f, 113});
                  value_changes.push_back({timestamps[i] + 0.3f, 100});
                  value_changes.push_back({timestamps[i] + 0.325f, 88});
                  value_changes.push_back({timestamps[i] + 0.35f, 75});
                  value_changes.push_back({timestamps[i] + 0.375f, 63});
                  value_changes.push_back({timestamps[i] + 0.4f, 50});
                  value_changes.push_back({timestamps[i] + 0.425f, 38});
                  value_changes.push_back({timestamps[i] + 0.45f, 25});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.4 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.4f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i] + 0.275f, 113});
                  value_changes.push_back({timestamps[i] + 0.3f, 100});
                  value_changes.push_back({timestamps[i] + 0.325f, 88});
                  value_changes.push_back({timestamps[i] + 0.35f, 75});
                  value_changes.push_back({timestamps[i] + 0.375f, 63});
                  value_changes.push_back({timestamps[i] + 0.4f, 50});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.35 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.35f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i] + 0.275f, 113});
                  value_changes.push_back({timestamps[i] + 0.3f, 100});
                  value_changes.push_back({timestamps[i] + 0.325f, 88});
                  value_changes.push_back({timestamps[i] + 0.35f, 75});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.3 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.3f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i] + 0.275f, 113});
                  value_changes.push_back({timestamps[i] + 0.3f, 100});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.25 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.25f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i] + 0.225f, 133});
                  value_changes.push_back({timestamps[i] + 0.25f, 125});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.2 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.2f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i] + 0.175f, 160});
                  value_changes.push_back({timestamps[i] + 0.2f, 150});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.15 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.15f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i] + 0.125f, 178});
                  value_changes.push_back({timestamps[i] + 0.15f, 170});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.1 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.1f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i] + 0.075f, 190});
                  value_changes.push_back({timestamps[i] + 0.1f, 185});
                  value_changes.push_back({timestamps[i + 1], 0});
                } else if(timestamps[i + 1] - timestamps[i] > 0.05 && ((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > timestamps[i] + 0.05f) {
                  value_changes.push_back({timestamps[i] + 0.025f, 198});
                  value_changes.push_back({timestamps[i] + 0.05f, 195});
                  value_changes.push_back({timestamps[i + 1], 0});
                }
                else
                  value_changes.push_back({timestamps[i + 1], 0});
              }
              else {
                value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
              }
            }
          }

          fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());
        }
        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_two_after_another") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
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
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
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
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
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
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_alternate_odd_even_blink") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<time_value_int> value_changes;

        value_changes.push_back({0.0, 200});
        value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
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
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "group_random_flashes") {
      if (fix.has_global_dimmer) {

        std::vector<float> onset_timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::vector<float> begin_and_end_of_flashing_timestamps;
        int onset_counter = 0;
        float last_time = 0;
        float begin_timestamp = 0;
        float time_of_one_eigth = ((float) 60 / (float) lightshow_from_analysis->get_bpm()) / 2;
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
        std::cout << "old bpm analysis: " << lightshow_from_analysis->get_bpm() << std::endl;

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
                if(((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() > time_of_next_flash + 0.025f)
                  value_changes.push_back({time_of_next_flash + 0.025f, 0});
                else
                  value_changes.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(), 0});
              }
              time_of_next_flash += 0.025f;
            }
          }
        }

        fix.add_value_changes_to_channel(value_changes, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    }
  }
  return lightshow_from_analysis;
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
  this->set_soft_color_changes(lightshow_from_analysis, fix, color_changes, this->fade_duration);
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
  if(color_changes.empty())
    return;

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;

  int counter = 0;
  float start_of_color = 0;
  float end_of_color= 0;

  while(counter < color_changes.size()) {
    start_of_color = color_changes[counter].timestamp;
    if(counter == color_changes.size() - 1)
      end_of_color = ((lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution());
    else
      end_of_color = color_changes[counter + 1].timestamp;

    color_values cv = this->color_to_rgb(color_changes[counter].color);
    if(cv.r > 0) {
      data_pairs_red.push_back({start_of_color, cv.r});
      data_pairs_red.push_back({end_of_color, 0});
    }
    if(cv.g > 0) {
      data_pairs_green.push_back({start_of_color, cv.g});
      data_pairs_green.push_back({end_of_color, 0});
    }
    if(cv.b > 0) {
      data_pairs_blue.push_back({start_of_color, cv.b});
      data_pairs_blue.push_back({end_of_color, 0});
    }

    counter++;
  }

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
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
    cv.g = 255; // looked to green on Cameo, maybe change back if mostly other fixtures are used
    //cv.g = 100;
    cv.b = 0;
  } else if(color == "orange") {
    cv.r = 255;
    cv.g = 165;
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

void LightshowGenerator::generate_beat_color_changes(std::shared_ptr<Lightshow> lightshow_from_analysis,
                                                     LightshowFixture &fix,
                                                     std::vector<std::string> &colors, bool only_change_color_if_action) {

  std::vector<color_change> color_changes;
  std::vector<double> timestamps = lightshow_from_analysis->get_all_beats();

  std::vector<time_value_int> bass_values = lightshow_from_analysis->get_value_changes_bass();
  std::vector<time_value_int> mid_values = lightshow_from_analysis->get_value_changes_middle();

  std::cout << "Number of beats in this lightshow: " << timestamps.size() << std::endl;
  std::vector<float> timestamps_float;

  if(only_change_color_if_action) {
    for (double timestamp: timestamps) {
      for (time_value_int tvi_bass: bass_values) {
        if (tvi_bass.time >= (float) timestamp / 44100 - 0.02 && tvi_bass.time <= (float) timestamp / 44100 + 0.02) {
          if (tvi_bass.value > 75)
            timestamps_float.push_back((float) timestamp);
          break;
        }
      }
      for (time_value_int tvi_mid: mid_values) {
        if (tvi_mid.time >= (float) timestamp / 44100 - 0.02 && tvi_mid.time <= (float) timestamp / 44100 + 0.02) {
          if (tvi_mid.value > 175)
            timestamps_float.push_back((float) timestamp);
          break;
        }
      }
    }
  } else {
    for(double timestamp: timestamps) {
      timestamps_float.push_back((float) timestamp);
    }
  }


  Logger::debug("Number of beat color changes in this lightshow: {}", timestamps_float.size());

  timestamps_float.erase( unique( timestamps_float.begin(), timestamps_float.end() ), timestamps_float.end() );

  int c = 0;
  color_changes.push_back({ 0, colors[0] });

  for (int i = 0; i < timestamps_float.size(); i++) {

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if (c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps_float[i]);
    color_changes.push_back({ timestamps_float[i] / 44100, colors[c] });
  }
  //this->set_soft_color_changes(lightshow_from_analysis, fix, color_changes, 0.1);
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
  //this->set_soft_color_changes(lightshow_from_analysis, fix, color_changes, 0.1);
  this->set_hard_color_changes(lightshow_from_analysis, fix, color_changes);
}
