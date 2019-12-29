//
// Created by Jan on 06.06.2019.
//

#include "lightshow_generator.h"


LightshowGenerator::LightshowGenerator() {

}

std::shared_ptr<Lightshow> LightshowGenerator::generate(int resolution, Song *song, std::list<Fixture> fixtures) {
  std::shared_ptr<Lightshow> lightshow_from_analysis = std::make_shared<Lightshow>();
  lightshow_from_analysis->set_resolution(resolution);

  lightshow_from_analysis->set_sound_src(song->get_file_path());
  lightshow_from_analysis->prepare_analysis_for_song((char*)song->get_file_path().c_str());

  std::vector<LightshowFixture> my_fixtures;
  for (Fixture fix: fixtures) {
    //std::cout << "new fix. name: " << fix.get_name() << ". start address: " << fix.get_start_channel() << ", number of addresses: " << fix.get_channel_count() << std::endl;
    if (fix.get_name() == "Cameo Flat RGB 10"
    || fix.get_name() == "Helios 7"
    || fix.get_name() == "Cobalt Plus Spot 5R"
    || fix.get_name() == "Varytec PAD7 seventy"
    || fix.get_name() == "TOURSPOT PRO"
    || fix.get_name() == "BAR TRI-LED") {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type(), fix.get_colors()));
    } else std::cout << "Fixture type unknown." << std::endl;
  }


  for (LightshowFixture fix: my_fixtures) {
    std::string fix_type = fix.get_type();
    std::transform(fix_type.begin(), fix_type.end(), fix_type.begin(), ::tolower);

    if (fix_type == "bass") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(), fix.get_channel_dimmer());

        std::vector<std::string> colors; // = fix.get_colors();
        for (std::string c: fix.get_colors()) {
          colors.push_back(c);
          std::cout << c << std::endl;
        }
        std::cout << colors.size() << std::endl;
        /*colors.push_back("red");
        colors.push_back("cyan");
        colors.push_back("light-green");
        colors.push_back("blue");
        colors.push_back("pink");*/
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(), fix.get_channel_red());
      }
      lightshow_from_analysis->add_fixture_bass(fix);
    } else if (fix_type == "mid") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        /*colors.push_back("blue");
        colors.push_back("light-green");
        colors.push_back("cyan");
        colors.push_back("red");
        colors.push_back("green");*/
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(), fix.get_channel_blue());
      }
      lightshow_from_analysis->add_fixture_middle(fix);
    } else if (fix_type == "high") {

      if (fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(), fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        /*colors.push_back("green");
        colors.push_back("yellow");
        colors.push_back("red");
        colors.push_back("white");
        colors.push_back("blue");*/
        this->generate_color_fades(lightshow_from_analysis, fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(), fix.get_channel_green());
      }
      lightshow_from_analysis->add_fixture_high(fix);
    } else if (fix_type == "action") {
      this->set_color_of_fixture_during_song(lightshow_from_analysis, fix, {"white"});
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_action(), fix.get_channel_dimmer());
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_action(), fix.get_channel_strobo());
      lightshow_from_analysis->add_fixture_action(fix);
    } else if (fix_type == "everything") {
      this->set_color_of_fixture_during_song(lightshow_from_analysis, fix, {"white"});
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_everything(),
                                       fix.get_channel_dimmer());
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "ambient") {
      if (fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float) lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution(),
                     0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();

        /*colors.push_back("blue");
        colors.push_back("light-green");
        colors.push_back("pink");
        colors.push_back("cyan");
        colors.push_back("red");
        colors.push_back("green");
        colors.push_back("yellow");
        colors.push_back("white");*/
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
        //lightshow_from_analysis->generate_ambient_color_fades(fix, colors);  // funktioniert noch gar nicht gut

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

        std::vector<std::string> colors;
        for (std::string c: fix.get_colors()) {
          colors.push_back(c);
          //std::cout << c << std::endl;
        }
        //std::cout << colors.size() << std::endl;
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

        std::vector<std::string> colors;
        for (std::string c: fix.get_colors()) {
          colors.push_back(c);
          //std::cout << c << std::endl;
        }
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

        std::vector<std::string> colors;
        for (std::string c: fix.get_colors()) {
          colors.push_back(c);
          //std::cout << c << std::endl;
        }
        //std::cout << colors.size() << std::endl;
        this->generate_onset_color_changes(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "onset_blink") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::cout << "timestamps.size() onset_blink: " << timestamps.size() << std::endl;
        std::vector<time_value_int> value_changes_onset_blink;

        std::vector<int> values_onset_blink;

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

        std::cout << value_changes_onset_blink.size() << std::endl;
        /*for (int i = 0; i < value_changes_onset_blink.size(); i++) {
          std::cout << value_changes_onset_blink[i].value << std::endl;
        }*/

        fix.add_value_changes_to_channel(value_changes_onset_blink, fix.get_channel_dimmer());

        std::vector<std::string> colors = fix.get_colors();
        this->generate_color_fades(lightshow_from_analysis, fix, colors);
      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type == "onset_blink_reverse") {
      if (fix.has_global_dimmer) {

        std::vector<float> timestamps = lightshow_from_analysis->get_onset_timestamps();
        std::cout << "timestamps.size() onset_blink: " << timestamps.size() << std::endl;
        std::vector<time_value_int> value_changes_onset_blink;

        std::vector<int> values_onset_blink;

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
    // cv.g = 255; // looked to green on Cameo, maybe change back if mostly other fixtures are used
    cv.g = 100;
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
