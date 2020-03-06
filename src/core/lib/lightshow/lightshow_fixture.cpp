#include <utility>

//
// Created by Johannes on 05.05.2019.
//

#include <iostream>
#include <logger.h>
#include "lightshow_fixture.h"
#include "dmx_device.h"
#include <string>
#include <libusb.h>
#include <atomic>


LightshowFixture::LightshowFixture(std::string name, int start_channel, int number_of_channels, std::string type, std::string colors, int _position_inside_group, std::string _position_on_stage, std::string _moving_head_type, int _modifier_pan, int _modifier_tilt, std::string _timestamps_type, int _position_inside_mh_group, bool _invert_tilt, int _amplitude_pan, int _amplitude_tilt) {
  this->name = name;
  this->start_channel = start_channel;
  this->number_of_channels = number_of_channels;
  this->position_inside_group = _position_inside_group;
  this->position_on_stage = _position_on_stage;
  this->moving_head_type = _moving_head_type;
  this->modifier_pan = _modifier_pan;
  this->modifier_tilt = _modifier_tilt;
  this->colors_string = colors;
  this->timestamps_type = _timestamps_type;
  this->position_in_mh_group = _position_inside_mh_group;
  this->invert_tilt = _invert_tilt;
  this->amplitude_pan = _amplitude_pan;
  this->amplitude_tilt = _amplitude_tilt;
  std::istringstream ss(colors);
  std::string color;
  if(!colors.empty()) {
    if(colors.find("/") != std::string::npos) {
      while (std::getline(ss, color, '/')) {
        if (color == "R" || color == "red")
          this->colors.push_back("red");
        else if (color == "G" || color == "green")
          this->colors.push_back("green");
        else if (color == "B" || color == "blue")
          this->colors.push_back("blue");
        else if (color == "LG" || color == "light-green")
          this->colors.push_back("light-green");
        else if (color == "C" || color == "cyan")
          this->colors.push_back("cyan");
        else if (color == "P" || color == "pink")
          this->colors.push_back("pink");
        else if (color == "W" || color == "white")
          this->colors.push_back("white");
        else if (color == "Y" || color == "yellow")
          this->colors.push_back("yellow");
        else if (color == "O" || color == "orange")
          this->colors.push_back("orange");
      }
    } else {
      if (colors == "R" || colors == "red")
        this->colors.push_back("red");
      else if (colors == "G" || colors == "green")
        this->colors.push_back("green");
      else if (colors == "B" || colors == "blue")
        this->colors.push_back("blue");
      else if (colors == "LG" || colors == "light-green")
        this->colors.push_back("light-green");
      else if (colors == "C" || colors == "cyan")
        this->colors.push_back("cyan");
      else if (colors == "P" || colors == "pink")
        this->colors.push_back("pink");
      else if (colors == "W" || colors == "white")
        this->colors.push_back("white");
      else if (colors == "Y" || colors == "yellow")
        this->colors.push_back("yellow");
      else if (colors == "O" || colors == "orange")
        this->colors.push_back("orange");
    }
  } else { this->colors.push_back("white"); }

  Logger::debug("Constructor of LightshowFixture. Type: {}", type);
  Logger::debug("Constructor of LightshowFixture. Name: {}", name);
  this->set_type(type);

  if (name == "Cameo Flat RGB 10") {
    this->set_channel_dimmer(1);
    this->set_channel_strobo(2);
    this->set_channel_red(3);
    this->set_channel_green(4);
    this->set_channel_blue(5);
    this->has_global_dimmer = true;
    this->has_strobe = true;
  } else if(name == "JBLED A7 (S8)") {
    this->set_channel_pan(1);
    this->set_channel_tilt(3);
    this->set_channel_shutter(6);
    this->set_channel_dimmer(7);
    this->set_channel_red(9);
    this->set_channel_green(10);
    this->set_channel_blue(11);
    this->degrees_per_pan = (float) 430 / 255;
    this->degrees_per_tilt = (float) 300 / 255;
    this->has_global_dimmer = true;
    this->has_pan = true;
    this->has_tilt = true;
    this->has_shutter = true;
  } else if(name == "JBLED P4 (M1)") {
    this->set_channel_pan(1);
    this->set_channel_tilt(3);
    this->set_channel_shutter(6);
    this->set_channel_dimmer(7);
    this->set_channel_focus(8);
    this->set_channel_zoom(9);
    this->set_channel_colorwheel(14);
    this->degrees_per_pan = (float) 433.6 / 255;
    this->degrees_per_tilt = (float) 280 / 255;
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("white", 1));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("red", 4));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("yellow", 8));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("green", 16));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("orange", 20));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("pink", 32));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("cyan", 36));
    this->colorwheel_values.insert(std::pair<std::string, uint8_t>("blue", 52));
    this->has_global_dimmer = true;
    this->has_pan = true;
    this->has_tilt = true;
    this->has_shutter = true;
    this->has_colorwheel = true;
    this->has_focus = true;
    this->has_zoom = true;
  } else if(name == "JBLED Sparx 7 (M3)") {
    this->set_channel_pan(1);
    this->set_channel_tilt(3);
    this->set_channel_shutter(6);
    this->set_channel_dimmer(7);
    this->set_channel_zoom(8);
    this->set_channel_red(21);
    this->set_channel_green(22);
    this->set_channel_blue(23);
    this->degrees_per_pan = (float) 433.6 / 255;
    this->degrees_per_tilt = (float) 333.3 / 255;
    this->has_global_dimmer = true;
    this->has_pan = true;
    this->has_tilt = true;
    this->has_shutter = true;
    this->has_zoom = true;
  } else if(name == "Stairville LED Flood Panel 150 (3ch)") {
    this->set_channel_red(1);
    this->set_channel_green(2);
    this->set_channel_blue(3);
    this->has_global_dimmer = false;
  } else if(name == "Stairville LED Flood Panel 150 (4ch)") {
    this->set_channel_dimmer(1);
    this->set_channel_red(2);
    this->set_channel_green(3);
    this->set_channel_blue(4);
    this->has_global_dimmer = true;
  } else if(name == "Helios 7") {
    this->set_channel_dimmer(11);
    this->set_channel_red(6);
    this->set_channel_green(7);
    this->set_channel_blue(8);
    this->set_channel_strobo(10);
    this->has_global_dimmer = true;
    this->has_strobe = true;
  } else if(name == "Cobalt Plus Spot 5R") {
    this->set_channel_dimmer(16);
    // alle Farben ein Channel
    this->set_channel_red(6);
    this->set_channel_green(6);
    this->set_channel_blue(6);
    this->has_global_dimmer = true;
  } else if(name == "Varytec PAD7 seventy") {
    // kein Dimmer!
    this->set_channel_red(1);
    this->set_channel_green(2);
    this->set_channel_blue(3);
    this->has_global_dimmer = false;
  } else if (name == "TOURSPOT PRO") {
    this->set_channel_dimmer(1);
    this->set_channel_strobo(2);
    this->set_channel_red(3);
    this->set_channel_green(4);
    this->set_channel_blue(5);
    this->has_global_dimmer = true;
    this->has_strobe = true;
  } else if (name == "BAR TRI-LED") {
    this->set_channel_dimmer(2);
    this->set_channel_strobo(3);
    this->set_channel_red(4);
    this->set_channel_green(5);
    this->set_channel_blue(6);
    this->has_global_dimmer = true;
    this->has_strobe = true;
  } else if(name == "SGM X-5 (1CH)") {
    this->set_channel_blinder(1);
    this->set_blinder_value(249);
    this->is_blinder = true;
  } else if(name == "SGM X-5 (3CH)") {
    this->set_channel_blinder(1);
    this->set_channel_flash_duration(2);
    this->set_channel_flash_rate(3);
    this->set_blinder_value(249);
    this->is_blinder = true;
    this->has_flash_duration = true;
    this->has_flash_rate = true;
  } else if(name == "SGM X-5 (4CH)") {
    this->set_channel_blinder(1);
    this->set_channel_flash_duration(2);
    this->set_channel_flash_rate(3);
    this->set_blinder_value(249);
    this->is_blinder = true;
    this->has_flash_duration = true;
    this->has_flash_rate = true;
  } else Logger::error("Fixture with unknown name created. Channels have not been set.");
}

LightshowFixture::~LightshowFixture() {
}

int LightshowFixture::get_start_channel() const {
  return this->start_channel;
}

std::vector<Channel> LightshowFixture::get_channels() const {
  return this->channels;
}

void LightshowFixture::set_start_channel(int start_channel) {
  this->start_channel = start_channel;
}

void LightshowFixture::add_channel(Channel channel) {
  this->channels.push_back(channel);
}

void LightshowFixture::set_number_of_channels(int number_of_channels) {
  this->number_of_channels = number_of_channels;
}

int LightshowFixture::get_number_of_channels() {
  return this->number_of_channels;
}
std::uint8_t LightshowFixture::get_channel_dimmer() {
  return this->channel_dimmer;
}
std::uint8_t LightshowFixture::get_channel_strobo() {
  return this->channel_strobo;
}
std::uint8_t LightshowFixture::get_channel_red() {
  return this->channel_red;
}
std::uint8_t LightshowFixture::get_channel_green() {
  return this->channel_green;
}
std::uint8_t LightshowFixture::get_channel_blue() {
  return this->channel_blue;
}
void LightshowFixture::set_channel_dimmer(std::uint8_t channel_dimmer) {
  Channel ch(channel_dimmer);
  this->dimmer = ch;
  this->channel_dimmer = channel_dimmer;
  this->add_channel(this->dimmer);
}
void LightshowFixture::set_channel_strobo(std::uint8_t channel_strobo) {
  Channel ch(channel_strobo);
  this->strobo = ch;
  this->channel_strobo = channel_strobo;
  this->add_channel(this->strobo);
}
void LightshowFixture::set_channel_red(std::uint8_t channel_red) {
  Channel ch(channel_red);
  this->red = ch;
  this->channel_red = channel_red;
  this->add_channel(this->red);
}
void LightshowFixture::set_channel_green(std::uint8_t channel_green) {
  Channel ch(channel_green);
  this->green = ch;
  this->channel_green = channel_green;
  this->add_channel(this->green);
}
void LightshowFixture::set_channel_blue(std::uint8_t channel_blue) {
  Channel ch(channel_blue);
  this->blue = ch;
  this->channel_blue = channel_blue;
  this->add_channel(this->blue);
}

void LightshowFixture::add_value_changes_to_channel(std::vector<time_value_int> value_changes, int channel) {
  if(this->channel_already_exists(channel)) {
    //std::cout << "channel already exists and has " << this->get_channel(channel).get_value_changes().size() << " value changes" << std::endl;

    //Channel ch(channel);
    ValueChange vc(0.0, 0);

    // loop through all ValueChanges and add them to the channel
    for (int i = 0; i < value_changes.size(); i++) {
      vc.set_timestamp(value_changes[i].time);
      vc.set_value(value_changes[i].value);
      if (vc.get_value() == -1) { // end of song special
        vc.set_value(0);
        this->get_channel(channel).add_value_change(vc);
      } else if (vc.get_value()
          != this->get_channel(channel).get_value_of_last_added_value_change()) // if ValueChange is different than the last one added to the channel, add it to the channel
        this->get_channel(channel).add_value_change(vc);
    }
  } else {
    Channel ch(channel);
    ValueChange vc(0.0, 0);

    // loop through all ValueChanges and add them to the channel
    for (int i = 0; i < value_changes.size(); i++) {
      vc.set_timestamp(value_changes[i].time);
      vc.set_value(value_changes[i].value);
      if (vc.get_value() == -1) { // end of song special
        vc.set_value(0);
        ch.add_value_change(vc);
      } else if (vc.get_value()
          != ch.get_value_of_last_added_value_change()) // if ValueChange is different than the last one added to the channel, add it to the channel
        ch.add_value_change(vc);
    }
    if (ch.get_value_of_last_added_value_change() != -1)
      this->add_channel(ch);
  }
}

void LightshowFixture::set_type(std::string type) {
  std::transform (type.begin(), type.end(), type.begin(), ::tolower);
  if (type == "bass"
  || type == "mid"
  || type == "high"
  || type == "action"
  || type == "everything"
  || type == "ambient"
  || type == "color_change"
  || type == "flash"
  || type == "flash_reverse"
  || type == "blink"
  || type == "group_one_after_another"
  || type == "group_one_after_another_fade"
  || type == "group_one_after_another_back_and_forth"
  || type == "group_one_after_another_blink"
  || type == "group_one_after_another_back_and_forth_blink"
  || type == "group_two_after_another"
  || type == "group_alternate_odd_even"
  || type == "group_random_flashes"
  || type == "strobe_if_many_onsets"
  || type == "auto_beats"
  || type == "group_auto_beats"
  || type == "auto_onsets"
  || type == "group_auto_onsets") {
    this->type = type;
    Logger::debug("Set type of fixture to {}", type);
  }
  else Logger::error("tried to set fixture type to {}", type);
}

std::string LightshowFixture::get_type() {
  return this->type;
}

std::string LightshowFixture::get_name() {
  return this->name;
}

void LightshowFixture::set_name(std::string name) {
  this->name = name;
}

std::vector<std::string> LightshowFixture::get_colors() {
  return this->colors;
}

void LightshowFixture::set_position_in_group(int _position) {
  if(_position > 0)
    this->position_inside_group = _position;
  else
    this->position_inside_group = 0;
}

int LightshowFixture::get_position_in_group() {
  return this->position_inside_group;
}

std::uint8_t LightshowFixture::get_channel_shutter() {
  return this->channel_shutter;
}

void LightshowFixture::set_channel_shutter(std::uint8_t channel_shutter) {
  this->channel_shutter = channel_shutter;
}

std::uint8_t LightshowFixture::get_channel_pan() {
  return this->channel_pan;
}

void LightshowFixture::set_channel_pan(std::uint8_t _channel_pan) {
  this->channel_pan = _channel_pan;
}


std::uint8_t LightshowFixture::get_channel_tilt() {
  return this->channel_tilt;
}

void LightshowFixture::set_channel_tilt(std::uint8_t _channel_tilt) {
  this->channel_tilt = _channel_tilt;
}

void LightshowFixture::set_position_on_stage(std::string _position) {
  this->position_on_stage = _position;
}

std::string LightshowFixture::get_position_on_stage() {
  return this->position_on_stage;
}

std::uint8_t LightshowFixture::get_channel_blinder() {
  return this->channel_blinder;
}

std::uint8_t LightshowFixture::get_blinder_value() {
  return this->blinder_value;
}

void LightshowFixture::set_blinder_value(std::uint8_t _blinder_value) {
  this->blinder_value = _blinder_value;
}

void LightshowFixture::set_channel_blinder(std::uint8_t _channel_blinder) {
  this->channel_blinder = _channel_blinder;
}

void LightshowFixture::set_channel_colorwheel(std::uint8_t _channel_colorwheel) {
  this->channel_colorwheel = _channel_colorwheel;
}

std::uint8_t LightshowFixture::get_channel_colorwheel() {
  return this->channel_colorwheel;
}

void LightshowFixture::set_channel_focus(std::uint8_t _channel_focus) {
  this->channel_focus = _channel_focus;
}

std::uint8_t LightshowFixture::get_channel_focus() {
  return this->channel_focus;
}

void LightshowFixture::set_channel_zoom(std::uint8_t _channel_zoom) {
  this->channel_zoom = _channel_zoom;
}

std::uint8_t LightshowFixture::get_channel_zoom() {
  return this->channel_zoom;
}

void LightshowFixture::set_moving_head_type(std::string _moving_head_type) {
  this->moving_head_type = _moving_head_type;
}

std::string LightshowFixture::get_moving_head_type() {
  return this->moving_head_type;
}

std::uint8_t LightshowFixture::get_colorwheel_value(std::string color) {
  if(this->colorwheel_values.count(color) == 1)
    return this->colorwheel_values.at(color);
  else
    return 0;
}
int LightshowFixture::get_modifier_pan() {
  return this->modifier_pan;
}
int LightshowFixture::get_modifier_tilt() {
  return this->modifier_tilt;
}
float LightshowFixture::get_degrees_per_pan() {
  return this->degrees_per_pan;
}
float LightshowFixture::get_degrees_per_tilt() {
  return this->degrees_per_tilt;
}

std::string LightshowFixture::get_colors_string() {
  return this->colors_string;
}

void LightshowFixture::set_timestamps_type(std::string _timestamps_type) {
  this->timestamps_type = _timestamps_type;
}

std::string LightshowFixture::get_timestamps_type() {
  return this->timestamps_type;
}

std::uint8_t LightshowFixture::get_channel_flash_duration() {
  return this->channel_flash_duration;
}

std::uint8_t LightshowFixture::get_channel_flash_rate() {
  return this->channel_flash_rate;
}

void LightshowFixture::set_channel_flash_duration(std::uint8_t _channel_flash_duration) {
  this->channel_flash_duration = _channel_flash_duration;
}

void LightshowFixture::set_channel_flash_rate(std::uint8_t _channel_flash_rate) {
  this->channel_flash_rate = _channel_flash_rate;
}

void LightshowFixture::set_position_in_mh_group(int _position) {
  this->position_in_mh_group = _position;
}

int LightshowFixture::get_position_in_mh_group() {
  return this->position_in_mh_group;
}


bool LightshowFixture::get_invert_tilt() {
  return this->invert_tilt;
}

void LightshowFixture::set_invert_tilt(bool _invert_tilt) {
  this->invert_tilt = _invert_tilt;
}

int LightshowFixture::get_amplitude_pan() {
  return this->amplitude_pan;
}

void LightshowFixture::set_amplitude_pan(int _amplitude_pan) {
  this->amplitude_pan = _amplitude_pan;
}

int LightshowFixture::get_amplitude_tilt() {
  return this->amplitude_tilt;
}

void LightshowFixture::set_amplitude_tilt(int _amplitude_tilt) {
  this->amplitude_tilt = _amplitude_tilt;
}

bool LightshowFixture::channel_already_exists(int channel) {
  bool channel_already_exists = false;

  for(auto const &ch : this->get_channels()) {
    if(ch.get_channel() == channel)
      channel_already_exists = true;
  }

  return channel_already_exists;
}

Channel &LightshowFixture::get_channel(int channel) {
  Channel channel1(0);
  for(int i = 0; i < this->channels.size(); i++) {
    if(this->channels[i].get_channel() == channel) {
      return std::ref(this->channels[i]);
    }
  }
  std::cerr << "THIS SHOULD NEVER HAPPEN" << std::endl;
  return std::ref(channel1);
}
