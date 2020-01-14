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


LightshowFixture::LightshowFixture(std::string name, int start_channel, int number_of_channels, std::string type, std::string colors, int _position_inside_group, std::string _position_on_stage) {
  this->name = name;
  this->start_channel = start_channel;
  this->number_of_channels = number_of_channels;
  this->position_inside_group = _position_inside_group;
  this->position_on_stage = _position_on_stage;
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
    this->has_global_dimmer = true;
    this->has_pan = true;
    this->has_tilt = true;
    this->has_shutter = true;
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
    this->has_global_dimmer = false;
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
  Channel ch(channel);
  ValueChange vc(0.0, 0);

  // loop through all ValueChanges and add them to the channel
  for (int i = 0; i < value_changes.size(); i++) {
    vc.set_timestamp(value_changes[i].time);
    vc.set_value(value_changes[i].value);
    if(vc.get_value() == -1) { // end of song special
      vc.set_value(0);
      ch.add_value_change(vc);
    } else if (vc.get_value() != ch.get_value_of_last_added_value_change()) // if ValueChange is different than the last one added to the channel, add it to the channel
        ch.add_value_change(vc);
  }
  if(ch.get_value_of_last_added_value_change() != -1)
    this->add_channel(ch);
}

void LightshowFixture::set_type(std::string type) {
  std::transform (type.begin(), type.end(), type.begin(), ::tolower);
  if (type == "bass"
  || type == "mid"
  || type == "high"
  || type == "action"
  || type == "everything"
  || type == "ambient"
  || type == "color_change_beats"
  || type == "color_change_beats_action"
  || type == "color_change_onsets"
  || type == "onset_flash"
  || type == "onset_flash_reverse"
  || type == "onset_blink"
  || type == "group_one_after_another"
  || type == "group_one_after_another_blink"
  || type == "group_two_after_another"
  || type == "group_alternate_odd_even"
  || type == "group_random_flashes"
  || type == "strobe_if_many_onsets") {
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
