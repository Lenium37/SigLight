//
// Created by Johannes on 05.05.2019.
//

#include "lightshow.h"
#include<map>

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
    this->first_beat = analysis.get_first_beat();
    this->all_beats = analysis.get_all_beats(this->bpm, this->first_beat);

    std::vector<time_value_int> segment_intensities = analysis.get_intensity_average_for_next_segment(this->all_beats, this->bpm, this->first_beat);
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
  this->onset_timestamps = this->analysis.get_onset_timestamps();


  // trying to get bpm from onsets
  std::map<float, int> onset_distance_occurrences;
  float timestamp_difference = 0;
  if(this->onset_timestamps.size() > 0) {
    for (int i = 0; i < this->onset_timestamps.size() - 1; i++) {
      timestamp_difference = onset_timestamps[i + 1] - onset_timestamps[i];
      timestamp_difference = roundf(timestamp_difference * 1000) / 1000;
      if (timestamp_difference >= 0.2 && timestamp_difference <= 0.75) {
        if (onset_distance_occurrences.find(timestamp_difference) == onset_distance_occurrences.end())
          onset_distance_occurrences.insert({timestamp_difference, 1});
        else
          onset_distance_occurrences.find(timestamp_difference)->second++;
      }
    }
  }
  auto most_occured_onset_difference = std::max_element(onset_distance_occurrences.begin(), onset_distance_occurrences.end(), [](const std::pair<float, int>& p1, const std::pair<float, int>& p2) { return p1.second < p2.second; });
  std::cout << "most occured onset difference: " << most_occured_onset_difference->first  << " (" << most_occured_onset_difference->second << " times)" << std::endl;

  for(std::pair<float, int> difference_occurence: onset_distance_occurrences) {
    if (onset_distance_occurrences.find(roundf(difference_occurence.first * 2 * 1000) / 1000) != onset_distance_occurrences.end()) {
      onset_distance_occurrences.erase(difference_occurence.first);
      //onset_distance_occurrences.find(roundf(difference_occurence.first * 2 * 1000) / 1000)->second *= 2;
    }
  }
  float time_of_one_beat = 0;
  int number_of_beats = 0;
  for(std::pair<float, int> difference_occurence: onset_distance_occurrences) {
    if(difference_occurence.second > 50) {
      time_of_one_beat += difference_occurence.first * difference_occurence.second;
      number_of_beats += difference_occurence.second;
    }
    std::cout << "time difference: " << difference_occurence.first  << " (" << difference_occurence.second << " times)" << std::endl;
  }
  time_of_one_beat /= (float) number_of_beats;
  float bpm_from_onsets = 60/time_of_one_beat;
  //std::cout << "bpm from onsets: " << bpm_from_onsets << std::endl;
  bpm_from_onsets = roundf(bpm_from_onsets);
  std::cout << "bpm from onsets: " << bpm_from_onsets << std::endl;



  this->set_length((this->analysis.get_length_of_song() + 1) * this->resolution + 3);
  //this->value_changes_bass = this->analysis.peaks_per_band(10, 45);

  this->value_changes_bass = this->analysis.peaks_per_band(20, 50);

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

  while(!bpm_analysis_finished) {
    //std::cout << "waiting for bpm" << std::endl;
    ;
  }

  if(bpm_from_onsets < 200 && this->bpm * 2  - 15 < bpm_from_onsets && this->bpm * 2 + 15 > bpm_from_onsets) {
    this->bpm = this->bpm * 2;
    this->all_beats = analysis.get_all_beats(this->bpm, this->first_beat);

    std::vector<time_value_int> segment_intensities = analysis.get_intensity_average_for_next_segment(this->all_beats, this->bpm, this->first_beat);
    /*
     * TODO: threshold wert damit nur changes in intensity_changes gepeichert werden, die um threshold % vom letzten change abweichen
     */
    std::vector<time_value_int> intensity_changes = analysis.get_intensity_changes(segment_intensities, 15);
    this->timestamps_colorchanges = intensity_changes;
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

int Lightshow::get_wait_time_standard() {
  return this->wait_time_standard;
}

int Lightshow::get_wait_time_special() {
  return this->wait_time_special;
}

std::vector<time_value_int> Lightshow::get_timestamps_colorchanges() {
  return this->timestamps_colorchanges;
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

std::vector<double> Lightshow::get_all_beats() {
  return this->all_beats;
}

std::vector<float> Lightshow::get_onset_timestamps() {
  return this->onset_timestamps;
}

int Lightshow::get_bpm() {
  return this->bpm;
}
