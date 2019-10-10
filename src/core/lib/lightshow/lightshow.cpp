//
// Created by Johannes on 05.05.2019.
//

#include "lightshow.h"

#if defined(_WIN32) || defined(WIN32)

#include <mingw.thread.h>

#else

#include <thread>

#endif

Lightshow::Lightshow() {

}

Lightshow::~Lightshow() {

}

std::string Lightshow::get_sound_src() {
  return this->sound_src;
}

std::vector<LightshowFixture> Lightshow::get_fixtures() {
  return this->fixtures;
}

void Lightshow::set_sound_src(std::string sound_src) {
  this->sound_src = sound_src;
}

void Lightshow::add_fixture(LightshowFixture fixture) {
  this->fixtures.push_back(fixture);
  this->update_channel_count();
}

void Lightshow::empty_fixtures() {
  this->fixtures.clear();
}

std::vector<std::vector<std::uint8_t>> Lightshow::read_channel_values() {
  Logger::debug("reading channel values, number of fixtures: {} ", this->fixtures.size());
  Logger::debug("length of lightshow: {}", this->length);
  Logger::debug("channel count of lightshow: {}", this->channel_count);

  std::vector<std::vector<std::uint8_t>>
      all_channel_values(this->length, std::vector<std::uint8_t>(this->channel_count, 0));

  std::vector<std::uint8_t> dimmer_channels;
  int index_of_last_value_change = 0;
  std::uint8_t value_of_last_value_change = 0;
  Logger::debug("pre reading channel values from fixtures");
  for (LightshowFixture fix: this->fixtures) {

    for (Channel ch: fix.get_channels()) {
      for (ValueChange vc: ch.get_value_changes()) {
        if (vc.get_timestamp() * this->resolution < all_channel_values.size()) {
          //std::cout << "reading channel value" << std::endl;
          all_channel_values[vc.get_timestamp() * this->resolution][fix.get_start_channel() + ch.get_channel() - 2] = vc.get_value();

          // zwischen letztem ValueChange und aktuellem ValueChange alles mit dem Value des letzten ValueChanges vollschreiben
          for (index_of_last_value_change = index_of_last_value_change + 1;
               index_of_last_value_change < vc.get_timestamp() * this->resolution; index_of_last_value_change++) {
            all_channel_values[index_of_last_value_change][fix.get_start_channel() + ch.get_channel() - 2] =
                value_of_last_value_change;
          }
          index_of_last_value_change = vc.get_timestamp() * this->resolution;
          value_of_last_value_change = vc.get_value();
        }
      }
    }

  }
  Logger::debug("finished reading channel values");
  return all_channel_values;
}

void Lightshow::set_length(int length) {
  this->length = length;
  Logger::debug("Set length of lightshow to: {}", length);
}

int Lightshow::get_length() {
  return this->length;
}

int Lightshow::get_channel_count() {
  return this->channel_count;
}

void Lightshow::set_channel_count(int channel_count) {
  this->channel_count = channel_count;
  Logger::debug("set channel count of lightshow to: {}", channel_count);
}

void Lightshow::update_channel_count() {
  int max_channel = 0;
  int temp_channel = 0;
  for (LightshowFixture fix: this->fixtures) {
    std::vector<Channel> channels = fix.get_channels();
    temp_channel = fix.get_start_channel() + fix.get_number_of_channels() - 1;
    if (temp_channel > max_channel)
      max_channel = temp_channel;

  }
  Logger::debug("updated channel count: {}", max_channel);
  this->channel_count = max_channel;
}

void Lightshow::add_multiple_fixtures(std::initializer_list<LightshowFixture> a_args) {
  for (LightshowFixture fixture: a_args) {
    this->fixtures.push_back(fixture);
  }
  this->update_channel_count();
}

bool compare_by_timestamp(const time_value_float &a, const time_value_float &b)
{
  return a.time < b.time;
}

void Lightshow::get_bpm_and_beats(bool &finished) {
  this->bpm = analysis.get_bpm(); //dauert lang
  Logger::debug("5");
  if(this->bpm == -1) {
    Logger::error("song too short to get bpm or do segmentation analysis");
    this->timestamps_colorchanges.push_back({0.0, 0});
  } else {
    //Logger::info("analysed bpm of: {}", this->bpm);
    double first_beat = analysis.get_first_beat();
    this->all_beats = analysis.get_all_beats(this->bpm, first_beat);

    std::vector<time_value_int> segment_intensities = analysis.get_intensity_average_for_next_segment(this->all_beats, this->bpm, first_beat);
    /*
     * TODO: threshold wert damit nur changes in intensity_changes gepeichert werden, die um threshold % vom letzten change abweichen
     */
    std::vector<time_value_int> intensity_changes = analysis.get_intensity_changes(segment_intensities, 15);
    this->timestamps_colorchanges = intensity_changes;
  }
  finished = true;
}

void Lightshow::prepare_analysis_for_song(char *song_path) {
  this->analysis.set_resolution(this->resolution);
  this->analysis.read_wav(song_path);
  this->analysis.stft(); //dauert lang

  bool bpm_analysis_finished = false;
  try {
    std::thread t(&Lightshow::get_bpm_and_beats, this, std::ref(bpm_analysis_finished));
    t.detach();
  } catch (const std::bad_alloc &e) {
    Logger::error("Allocation failed during bpm thread creation: {}", e.what());
  }

  this->analysis.normalize();


  this->set_length((this->analysis.get_length_of_song() + 1) * this->resolution + 3);
  //this->value_changes_bass = this->analysis.peaks_per_band(10, 45);

  Logger::debug("6");
  this->value_changes_bass = this->analysis.peaks_per_band(20, 50);
  Logger::debug("7");

  BoxFIR box1(3);
  std::vector<int> v1;
  for (int i = 0; i < this->value_changes_bass.size(); i++) {
    v1.push_back(this->value_changes_bass[i].value);
  }
  box1.filter(v1);
  for (int i = 0; i < this->value_changes_bass.size(); i++) {
    this->value_changes_bass[i].value = v1[i];
  }

  this->value_changes_middle = this->analysis.peaks_per_band(100, 600);
  BoxFIR box2(12);
  std::vector<int> v2;
  for (int i = 0; i < this->value_changes_middle.size(); i++) {
    v2.push_back(this->value_changes_middle[i].value);
  }
  box2.filter(v2);
  for (int i = 0; i < this->value_changes_middle.size(); i++) {
    this->value_changes_middle[i].value = v2[i];
  }

  this->value_changes_high = this->analysis.peaks_per_band(600, 2000);

  BoxFIR box3(8);
  std::vector<int> v3;
  for (int i = 0; i < this->value_changes_high.size(); i++) {
    v3.push_back(this->value_changes_high[i].value);
  }
  box3.filter(v3);
  for (int i = 0; i < this->value_changes_high.size(); i++) {
    this->value_changes_high[i].value = v3[i];
  }

  this->value_changes_everything = this->analysis.peaks_per_band(0, 20000);

  while(!bpm_analysis_finished) {
    //std::cout << "waiting for bpm" << std::endl;
    ;
  }
}

std::vector<LightshowFixture> Lightshow::get_fixtures_bass() {
  return this->fixtures_bass;
}

std::vector<LightshowFixture> Lightshow::get_fixtures_middle() {
  return this->fixtures_middle;
}

std::vector<LightshowFixture> Lightshow::get_fixtures_high() {
  return this->fixtures_high;
}

std::vector<LightshowFixture> Lightshow::get_fixtures_action() {
  return this->fixtures_action;
}

std::vector<LightshowFixture> Lightshow::get_fixtures_ambient() {
  return this->fixtures_ambient;
}

void Lightshow::add_fixture_bass(LightshowFixture fixture) {
  this->fixtures_bass.push_back(fixture);
  this->add_fixture(fixture);
}

void Lightshow::add_fixture_middle(LightshowFixture fixture) {
  this->fixtures_middle.push_back(fixture);
  this->add_fixture(fixture);
}

void Lightshow::add_fixture_high(LightshowFixture fixture) {
  this->fixtures_high.push_back(fixture);
  this->add_fixture(fixture);
}

void Lightshow::add_fixture_action(LightshowFixture fixture) {
  this->fixtures_action.push_back(fixture);
  this->add_fixture(fixture);
}

void Lightshow::add_fixture_ambient(LightshowFixture fixture) {
  this->fixtures_ambient.push_back(fixture);
  this->add_fixture(fixture);
}

std::vector<time_value_int> Lightshow::get_value_changes_bass() {
  return this->value_changes_bass;
}

std::vector<time_value_int> Lightshow::get_value_changes_middle() {
  return this->value_changes_middle;
}

std::vector<time_value_int> Lightshow::get_value_changes_high() {
  return this->value_changes_high;
}

std::vector<time_value_int> Lightshow::get_value_changes_action() {
  return this->value_changes_action;
}

std::vector<time_value_int> Lightshow::get_value_changes_everything() {
  return this->value_changes_everything;
}

std::vector<time_value_int> Lightshow::clear_outlier_values(std::vector<time_value_int> vector, int time_interval) {
  int counter1 = 0;
  int counter2 = time_interval;
  int avg = 0;

  while (counter2 < vector[vector.size() - 1].time * this->resolution - time_interval) {
    int i = 0;
    for (i = counter1; i < counter2; i++) {
      avg = avg + vector[i].value;
    }
    avg = avg / (i - counter1);

    for (counter1; counter1 < counter2; counter1++) {
      if (vector[counter1].value < avg / 2) {

        vector[counter1].value = avg;
      }
    }
    avg = 0;
    counter1 = counter2;
    counter2 = counter1 + time_interval;
  }
  std::cout << "Cleared outlier values!" << std::endl;
  return vector;
}

void Lightshow::generalize_values(std::vector<time_value_int> &v,
                                  std::uint8_t lower_border,
                                  std::uint8_t upper_border,
                                  std::uint8_t value) {
  for (int i = 0; i < v.size(); i++) {
    if (v[i].value >= lower_border && v[i].value <= upper_border)
      v[i].value = value;
  }
}

// todo we can do this better with enums
void Lightshow::set_color_of_fixture_during_song(LightshowFixture &fix, std::initializer_list<std::string> a_args) {
  int number_of_color_changes = a_args.size();
  int number_of_current_color = 0;
  float length_of_song = (this->get_length() - 3) / this->resolution;
  float time_of_one_color = length_of_song / number_of_color_changes;
  Logger::debug("time of one color: {}", time_of_one_color);
  float begin = 0;
  float end = 0;

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;
  int counter = 0;

  for (std::string s: a_args) {
    begin = counter * (length_of_song / number_of_color_changes);
    end = (counter + 1) * (length_of_song / number_of_color_changes);
    if (s == "red") {
      data_pairs_red.push_back({begin, 255});
      data_pairs_red.push_back({end, 0});
    } else if (s == "green") {
      data_pairs_green.push_back({begin, 255});
      data_pairs_green.push_back({end, 0});
    } else if (s == "blue") {
      data_pairs_blue.push_back({begin, 255});
      data_pairs_blue.push_back({end, 0});
    } else if (s == "white") {
      data_pairs_red.push_back({begin, 255});
      data_pairs_red.push_back({end, 0});
      data_pairs_green.push_back({begin, 255});
      data_pairs_green.push_back({end, 0});
      data_pairs_blue.push_back({begin, 255});
      data_pairs_blue.push_back({end, 0});
    } else if (s == "yellow") {
      data_pairs_red.push_back({begin, 255});
      data_pairs_red.push_back({end, 0});
      //data_pairs_green.push_back({begin, 255}); // sah zu gruen aus
      data_pairs_green.push_back({begin, 100});
      data_pairs_green.push_back({end, 0});
    } else if (s == "cyan") {
      data_pairs_green.push_back({begin, 255});
      data_pairs_green.push_back({end, 0});
      data_pairs_blue.push_back({begin, 255});
      data_pairs_blue.push_back({end, 0});
    } else if (s == "pink") {
      data_pairs_red.push_back({begin, 255});
      data_pairs_red.push_back({end, 0});
      //data_pairs_blue.push_back({begin, 255}); // sah zu blau aus
      data_pairs_blue.push_back({begin, 75});
      data_pairs_blue.push_back({end, 0});
    } else if (s == "light-green") {
      data_pairs_red.push_back({begin, 50});
      data_pairs_red.push_back({end, 0});
      data_pairs_green.push_back({begin, 255});
      data_pairs_green.push_back({end, 0});
      data_pairs_blue.push_back({begin, 50});
      data_pairs_blue.push_back({end, 0});
    } else Logger::error("Tried to set unknown color." );

    counter++;
  }

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}

std::vector<time_value_int> Lightshow::get_combined_values_changes(std::initializer_list<std::vector<time_value_int>> a_args) {
  std::vector<time_value_int> combined;
  for (std::vector<time_value_int> data_pairs: a_args) {
    if (combined.size() < 1)
      combined = data_pairs;
    for (int i = 0; i < combined.size(); i++) {
      if (data_pairs[i].value > combined[i].value)
        combined[i].value = data_pairs[i].value;
    }
  }
  return combined;
}

void Lightshow::set_resolution(int resolution_) {
  this->resolution = resolution_;
  this->wait_time_standard = 1000000 / resolution_;
  this->wait_time_special = 832000 / resolution_;
  Logger::debug("Set resolution of lightshow to {} and wait times to {} (standard) and {} (special)", this->resolution, this->wait_time_standard, this->wait_time_special);
}

int Lightshow::get_resolution() {
  return this->resolution;
}

// todo move function to class LightshowFixture
void Lightshow::color_change_at(LightshowFixture &fix, const std::string& new_color, float timestamp) {

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;

  if (new_color == "red") {
    data_pairs_red.push_back({timestamp, 255});
    data_pairs_green.push_back({timestamp, 0});
    data_pairs_blue.push_back({timestamp, 0});
  } else if (new_color == "green") {
    data_pairs_red.push_back({timestamp, 0});
    data_pairs_green.push_back({timestamp, 255});
    data_pairs_blue.push_back({timestamp, 0});
  } else if (new_color == "blue") {
    data_pairs_red.push_back({timestamp, 0});
    data_pairs_green.push_back({timestamp, 0});
    data_pairs_blue.push_back({timestamp, 255});
  } else if (new_color == "white") {
    data_pairs_red.push_back({timestamp, 255});
    data_pairs_green.push_back({timestamp, 255});
    data_pairs_blue.push_back({timestamp, 255});
  } else if (new_color == "yellow") {
    data_pairs_red.push_back({timestamp, 255});
    //data_pairs_green.push_back({timestamp, 255}); // sah zu gruen aus
    data_pairs_green.push_back({timestamp, 100});
    data_pairs_blue.push_back({timestamp, 0});
  } else if (new_color == "cyan") {
    data_pairs_red.push_back({timestamp, 0});
    data_pairs_green.push_back({timestamp, 255});
    data_pairs_blue.push_back({timestamp, 255});
  } else if (new_color == "pink") {
    data_pairs_red.push_back({timestamp, 255});
    data_pairs_green.push_back({timestamp, 0});
    //data_pairs_blue.push_back({timestamp, 255}); // sah zu blau aus
    data_pairs_blue.push_back({timestamp, 75});
  } else if (new_color == "light-green") {
    data_pairs_red.push_back({timestamp, 50});
    data_pairs_green.push_back({timestamp, 255});
    data_pairs_blue.push_back({timestamp, 50});
  } else Logger::error("Tried to set unknown color.");

  float end_of_song = ((this->get_length() - 3) / this->get_resolution());
  data_pairs_red.push_back({end_of_song, 0});
  data_pairs_green.push_back({end_of_song, 0});
  data_pairs_blue.push_back({end_of_song, 0});

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}

void Lightshow::set_hard_color_changes(LightshowFixture &fix, std::vector<color_change> color_changes) {
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
      end_of_color = ((this->get_length() - 3) / this->get_resolution());
    else
      end_of_color = color_changes[counter + 1].timestamp;

    if (color_changes[counter].color == "red") {
      data_pairs_red.push_back({start_of_color, 255});
      data_pairs_red.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "green") {
      data_pairs_green.push_back({start_of_color, 255});
      data_pairs_green.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "blue") {
      data_pairs_blue.push_back({start_of_color, 255});
      data_pairs_blue.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "white") {
      data_pairs_red.push_back({start_of_color, 255});
      data_pairs_red.push_back({end_of_color, 0});
      data_pairs_green.push_back({start_of_color, 255});
      data_pairs_green.push_back({end_of_color, 0});
      data_pairs_blue.push_back({start_of_color, 255});
      data_pairs_blue.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "yellow") {
      data_pairs_red.push_back({start_of_color, 255});
      data_pairs_red.push_back({end_of_color, 0});
      //data_pairs_green.push_back({start_of_color, 255}); // sah zu gruen aus
      data_pairs_green.push_back({start_of_color, 100});
      data_pairs_green.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "cyan") {
      data_pairs_green.push_back({start_of_color, 255});
      data_pairs_green.push_back({end_of_color, 0});
      data_pairs_blue.push_back({start_of_color, 255});
      data_pairs_blue.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "pink") {
      data_pairs_red.push_back({start_of_color, 255});
      data_pairs_red.push_back({end_of_color, 0});
      //data_pairs_blue.push_back({start_of_color, 255}); // sah zu blau aus
      data_pairs_blue.push_back({start_of_color, 75});
      data_pairs_blue.push_back({end_of_color, 0});
    } else if (color_changes[counter].color == "light-green") {
      data_pairs_red.push_back({start_of_color, 50});
      data_pairs_red.push_back({end_of_color, 0});
      data_pairs_green.push_back({start_of_color, 255});
      data_pairs_green.push_back({end_of_color, 0});
      data_pairs_blue.push_back({start_of_color, 50});
      data_pairs_blue.push_back({end_of_color, 0});
    } else Logger::error("Tried to set unknown color.");

    counter++;
  }

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}



void Lightshow::set_soft_color_changes(LightshowFixture &fix, std::vector<color_change> color_changes, float fade_duration) {
  if(color_changes.empty())
    return;

  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;


  int counter = 0;
  float timestamp_color_change = 0;
  float timestamp_begin_fade = 0;
  float timestamp_end_fade = 0;
  float end_of_color= 0;
  std::string previous_color;
  int value_prev_red = 0;
  int value_prev_green = 0;
  int value_prev_blue = 0;
  int value_new_red = 0;
  int value_new_green = 0;
  int value_new_blue = 0;
  float end_of_song = (this->get_length() - 3) / this->get_resolution();

  while(counter < color_changes.size()) {

    timestamp_color_change = color_changes[counter].timestamp;
    timestamp_begin_fade = timestamp_color_change - fade_duration / 2;

    if(counter == 0 && color_changes.size() > 1) {
      timestamp_end_fade = color_changes[counter + 1].timestamp + fade_duration / 2;
      this->set_color_for_time(fix, color_changes[counter].color, color_changes[counter].timestamp, timestamp_end_fade);
      previous_color = color_changes[counter].color;
      counter++;
      continue;
    }

    if(previous_color == "red") {
      value_prev_red = 255;
      value_prev_green = 0;
      value_prev_blue = 0;
    } else if(previous_color == "green") {
      value_prev_red = 0;
      value_prev_green = 255;
      value_prev_blue = 0;
    } else if(previous_color == "blue") {
      value_prev_red = 0;
      value_prev_green = 0;
      value_prev_blue = 255;
    } else if(previous_color == "white") {
      value_prev_red = 255;
      value_prev_green = 255;
      value_prev_blue = 255;
    } else if(previous_color == "yellow") {
      value_prev_red = 255;
      // value_prev_green = 255; // sah zu gruen aus
      value_prev_green = 100;
      value_prev_blue = 0;
    } else if(previous_color == "cyan") {
      value_prev_red = 0;
      value_prev_green = 255;
      value_prev_blue = 255;
    } else if(previous_color == "pink") {
      value_prev_red = 255;
      value_prev_green = 0;
      // value_prev_blue = 255; // sah zu blau aus
      value_prev_blue = 75;
    } else if(previous_color == "light-green") {
      value_prev_red = 50;
      value_prev_green = 255;
      value_prev_blue = 50;
    } else Logger::error("Tried to set unknown color.");

    if(color_changes[counter].color == "red") {
      value_new_red = 255;
      value_new_green = 0;
      value_new_blue = 0;
    } else if(color_changes[counter].color == "green") {
      value_new_red = 0;
      value_new_green = 255;
      value_new_blue = 0;
    } else if(color_changes[counter].color == "blue") {
      value_new_red = 0;
      value_new_green = 0;
      value_new_blue = 255;
    } else if(color_changes[counter].color == "white") {
      value_new_red = 255;
      value_new_green = 255;
      value_new_blue = 255;
    } else if(color_changes[counter].color == "yellow") {
      value_new_red = 255;
      // value_new_green = 255; // sah zu gruen aus
      value_new_green = 100;
      value_new_blue = 0;
    } else if(color_changes[counter].color == "cyan") {
      value_new_red = 0;
      value_new_green = 255;
      value_new_blue = 255;
    } else if(color_changes[counter].color == "pink") {
      value_new_red = 255;
      value_new_green = 0;
      // value_new_blue = 255; // sah zu blau aus
      value_new_blue = 75;
    } else if(color_changes[counter].color == "light-green") {
      value_new_red = 50;
      value_new_green = 255;
      value_new_blue = 50;
    } else Logger::error("Tried to set unknown color.");

    for(time_value_int tv : this->calculate_fade(fade_duration, value_prev_red, value_new_red)) {
      tv.time = tv.time + timestamp_begin_fade;
      data_pairs_red.push_back(tv);
    }
    for(time_value_int tv : this->calculate_fade(fade_duration, value_prev_green, value_new_green)) {
      tv.time = tv.time + timestamp_begin_fade;
      if(tv.time < 0)
        tv.time = 0;
      data_pairs_green.push_back(tv);
    }
    for(time_value_int tv : this->calculate_fade(fade_duration, value_prev_blue, value_new_blue)) {
      tv.time = tv.time + timestamp_begin_fade;
      data_pairs_blue.push_back(tv);
    }
    previous_color = color_changes[counter].color;

    counter++;
  }

  data_pairs_red.push_back({end_of_song, -1});
  data_pairs_green.push_back({end_of_song, -1});
  data_pairs_blue.push_back({end_of_song, -1});
  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());

}

std::vector<time_value_int> Lightshow::calculate_fade(float fade_duration, int c_old, int c_new) {
  std::vector<time_value_int> v;
  float change_per_step = (c_old - c_new) / ((this->get_resolution() - 5) * fade_duration);
  int color = c_old;
  float timestamp = 0;
  float time_step = 0;
  if(this->get_resolution() == 40)
    time_step = 0.025;
  else if(this->get_resolution() == 20)
    time_step = 0.05;
  else if(this->get_resolution() == 10)
    time_step = 0.1;


  //for(float timestamp = 0; timestamp < fade_duration; timestamp = timestamp + 1 / this->get_resolution()) {
  for(int i = 0; i < fade_duration * this->get_resolution(); i++) {
    color = c_old - i * change_per_step;
    if(i == fade_duration * this->get_resolution() - 1)
      color = c_new;
    if(color < 0)
      color = 0;
    else if(color > 255)
      color = 255;
    v.push_back({timestamp, color});
    timestamp = timestamp + time_step;
    //timestamp = timestamp + 0.025;
  }
  //std::cout << "calculated fade with " << v.size() << " steps." << std::endl;
  return v;
}

void Lightshow::set_color_for_time(LightshowFixture &fix, std::string color, float begin, float end) {
  std::vector<time_value_int> data_pairs_red;
  std::vector<time_value_int> data_pairs_green;
  std::vector<time_value_int> data_pairs_blue;

  if (color == "red") {
    data_pairs_red.push_back({begin, 255});
    data_pairs_red.push_back({end, 0});
  } else if (color == "green") {
    data_pairs_green.push_back({begin, 255});
    data_pairs_green.push_back({end, 0});
  } else if (color == "blue") {
    data_pairs_blue.push_back({begin, 255});
    data_pairs_blue.push_back({end, 0});
  } else if (color == "white") {
    data_pairs_red.push_back({begin, 255});
    data_pairs_red.push_back({end, 0});
    data_pairs_green.push_back({begin, 255});
    data_pairs_green.push_back({end, 0});
    data_pairs_blue.push_back({begin, 255});
    data_pairs_blue.push_back({end, 0});
  } else if (color == "yellow") {
    data_pairs_red.push_back({begin, 255});
    data_pairs_red.push_back({end, 0});
    //data_pairs_green.push_back({begin, 255}); // sah zu gruen aus
    data_pairs_green.push_back({begin, 100});
    data_pairs_green.push_back({end, 0});
  } else if (color == "cyan") {
    data_pairs_green.push_back({begin, 255});
    data_pairs_green.push_back({end, 0});
    data_pairs_blue.push_back({begin, 255});
    data_pairs_blue.push_back({end, 0});
  } else if (color == "pink") {
    data_pairs_red.push_back({begin, 255});
    data_pairs_red.push_back({end, 0});
    //data_pairs_blue.push_back({begin, 255}); // sah zu blau aus
    data_pairs_blue.push_back({begin, 75});
    data_pairs_blue.push_back({end, 0});
  } else if (color == "light-green") {
    data_pairs_red.push_back({begin, 50});
    data_pairs_red.push_back({end, 0});
    data_pairs_green.push_back({begin, 255});
    data_pairs_green.push_back({end, 0});
    data_pairs_blue.push_back({begin, 50});
    data_pairs_blue.push_back({end, 0});
  } else Logger::error("Tried to set unknown color.");

  fix.add_value_changes_to_channel(data_pairs_red, fix.get_channel_red());
  fix.add_value_changes_to_channel(data_pairs_green, fix.get_channel_green());
  fix.add_value_changes_to_channel(data_pairs_blue, fix.get_channel_blue());
}


int Lightshow::get_wait_time_standard() {
  return this->wait_time_standard;
}

int Lightshow::get_wait_time_special() {
  return this->wait_time_special;
}

std::vector<time_value_int> Lightshow::get_timestamps_colorchanges() {
  return this->timestamps_colorchanges;
}

void Lightshow::generate_color_fades(LightshowFixture &fix, std::vector<std::string> &colors) {
  std::vector<color_change> color_changes;
  std::vector<time_value_int> timestamps = this->get_timestamps_colorchanges();
  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());
  int c = 0;
  color_changes.push_back({0, colors[0]});

  for(int i = 0; i < timestamps.size(); i++) {
    // Alles, was vor Sekunde 1 passiert, wird ignoriert
    if(timestamps[i].time < this->fade_duration)
      timestamps[i].time = 0 + this->fade_duration / 2;

    // Wenn zwischen dem aktuellen und dem nächsten Timestamp weniger als 4 Sekunden liegen, ignoriere den aktuellen
    if(i >= 0 && timestamps.size() > i+1) {
      if(timestamps[i+1].time <= timestamps[i].time + 4)
        continue;
    }

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if(c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps[i].time);
    color_changes.push_back({timestamps[i].time, colors[c]});
  }

  this->set_soft_color_changes(fix, color_changes, this->fade_duration);
}


/**
 * TODO
 * Funktioniert noch nicht gut.
 * Maximales Segment meist sehr kurz.
 * Durch doppeltes Hinzufügen von soft_color_changes bleibt die zu erst gesetzt Farbe (blau) für das ganze Lied an, die anderen werden nur mit drauf gelegt.
 * Timing der Beats stimmt auch nicht immer.
 */
void Lightshow::generate_ambient_color_fades(LightshowFixture &fix, std::vector<std::string> &colors) {

  Logger::debug("0");
  // add color changes on 2 and 4 to segment with highest intensity
  for(int i = 0; i < this->all_beats.size(); i++) {
    float temp1 = (float)this->all_beats[i];
    float temp2 = temp1/analysis.get_samplerate();
    this->timestamps_of_all_beats.push_back(temp2);
  }
  Logger::debug("1");
  time_value_int segment_with_highest_intensity = {0.0, 0};
  float end_of_segment_with_highest_intensity = 0.0;
  for(int i = 0; i < this->timestamps_colorchanges.size(); i++) {
    Logger::debug("Intensity change at: {} Value: {}", this->timestamps_colorchanges[i].time, this->timestamps_colorchanges[i].value);
    if(this->timestamps_colorchanges[i].value >= segment_with_highest_intensity.value) {
      segment_with_highest_intensity = this->timestamps_colorchanges[i];
      if(i < this->timestamps_colorchanges.size() - 1)
        end_of_segment_with_highest_intensity = this->timestamps_colorchanges[i + 1].time;
      else
        end_of_segment_with_highest_intensity = timestamps_of_all_beats.back();
    }
  }
  Logger::debug("2");
  std::vector<float> beats_during_segment_with_highest_intensity;
  for(float timestamp: timestamps_of_all_beats) {
    if(timestamp >= segment_with_highest_intensity.time && timestamp < end_of_segment_with_highest_intensity)
      beats_during_segment_with_highest_intensity.push_back(timestamp);
  }

  Logger::debug("3");
  int n = 0;
  while(n < beats_during_segment_with_highest_intensity.size())
  {
    beats_during_segment_with_highest_intensity.erase( beats_during_segment_with_highest_intensity.begin() + n);
    n++;
  }

  for(time_value_int timestamp: this->timestamps_colorchanges)
    this->timestamps_colorchanges_ambient.push_back({timestamp.time, this->fade_duration}); // fade duration 1s
  for(float timestamp: beats_during_segment_with_highest_intensity)
    this->timestamps_colorchanges_ambient.push_back({timestamp, 0.0}); // fade duration 0s

  std::sort(this->timestamps_colorchanges_ambient.begin(), this->timestamps_colorchanges_ambient.end(), compare_by_timestamp);
  for(time_value_float colorchange_ambient: this->timestamps_colorchanges_ambient)
    Logger::debug("ambient color change at: {}", colorchange_ambient.time);



  std::vector<color_change> color_changes_soft;
  std::vector<color_change> color_changes_hard;
  std::vector<time_value_float> timestamps = this->timestamps_colorchanges_ambient;
  Logger::debug("Number of color changes in this lightshow: {}", timestamps.size());
  int c = 0;
  color_changes_soft.push_back({0, colors[0]});
  color_changes_hard.push_back({0, colors[0]});

  for(int i = 0; i < timestamps.size(); i++) {
    // Alles, was vor Sekunde 1 passiert, wird ignoriert
    if(timestamps[i].time < this->fade_duration)
      timestamps[i].time = 0 + this->fade_duration / 2;

    c = i;

    // Wenn keine weitern Farben mehr vorhanden sind, beginne wieder von vorne
    if(c >= colors.size())
      c = (i % colors.size());

    Logger::debug("adding color change at: {}", timestamps[i].time);
    if(timestamps[i].value > 0.0)
      color_changes_soft.push_back({timestamps[i].time, colors[c]});
    else
      color_changes_hard.push_back({timestamps[i].time, colors[c]});
  }

  this->set_soft_color_changes(fix, color_changes_soft, this->fade_duration);
  this->set_soft_color_changes(fix, color_changes_hard, 0.05);
}

void Lightshow::add_value_change_bass(time_value_int tvi) {
  this->value_changes_bass.push_back(tvi);
}

void Lightshow::add_value_change_mid(time_value_int tvi) {
  this->value_changes_middle.push_back(tvi);
}

void Lightshow::add_value_change_high(time_value_int tvi) {
  this->value_changes_high.push_back(tvi);
}