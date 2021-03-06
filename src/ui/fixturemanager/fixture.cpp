#include <utility>

#include <QtCore>
#include <QString>
#include "fixture.h"
#include <cstring>
#include <utility>
#include <iostream>

using namespace std;

Fixture::Fixture(string _type, string _name, string _description, QStringList _channels, std::string icon, std::string colors) {

  Fixture(std::move(_type), std::move(_name), std::move(_description), 1, std::move(_channels), std::move(icon), colors);
}

Fixture::Fixture(string _type, string _name, string _description, int _start_channel, QStringList _channels, std::string _icon, std::string colors) {
  type = std::move(_type);
  name = std::move(_name);
  description = std::move(_description);
  start_channel = _start_channel;
  icon = std::move(_icon);
  for (int i = 0; i < _channels.size(); i++) {
      channels.push_back(Channel_gui(_channels.at(i)));
  }
  this->colors = colors;
  this->channel_count = _channels.size();
}

string Fixture::get_description_view() {
  string channels_text = "Functions:\n";
  for (int i = 0; i < channels.size(); i++) {
    string single_channel = "Channel " + std::to_string(i + 1) + ":";
    QStringList funcs = (std::next(channels.begin(), i))->get_functions().split("|");
    string function = "";
    for (int j = 0; j < funcs.size(); j++) {
      if (!function.empty()) {
        function.append("\n");
      }
      function.append("\t" +funcs.at(j).toStdString());

    }
    single_channel.append(function);

    channels_text.append("\n" + single_channel);
  }
  return this->description + "\n\n" + "Type: " + this->type + "\n\n" + channels_text;
}


string Fixture::get_name()
{
    return name;
}

string Fixture::get_description()
{
    return description;
}

string Fixture::get_type()
{
    return type;
}

int Fixture::get_channel_count() {
  if(Fixture::channels.size() > 0)
    return Fixture::channels.size();
  else
    return this->channel_count;
}

int Fixture::get_start_channel() {
    return start_channel;
}

int Fixture::get_last_channel() {
    return start_channel + (get_channel_count() -1);
}

std::list<Channel_gui> Fixture::get_channels()
{
    return Fixture::channels;
}

std::string Fixture::get_icon() {
    return icon;
}

void Fixture::set_start_channel(int _start_channel) {
    Fixture::start_channel = _start_channel;
}

string Fixture::to_string() {
    string channels_text = "";
      for (int i = 0; i < channels.size(); i++) {
        if (!channels_text.empty()) {
          channels_text.append("^");
        }
        channels_text.append((*std::next(channels.begin(), i)).to_string());
      }

      return this->get_name() + ";" + this->get_type() + ";" + this->get_description() + ";"
              + channels_text + ";" + this->get_icon();
}

void Fixture::set_type(const string &type) {
  Fixture::type = type;
}

void Fixture::set_name(const string &name) {
  Fixture::name = name;
}

void Fixture::set_description(const string &description) {
  Fixture::description = description;
}

void Fixture::set_icon(const std::string &icon)
{
    Fixture::icon = icon;
}

bool Fixture::has_global_dimmer()
{
 return global_dimmer;
}

void Fixture::set_channels(QStringList _channels) {
  for (int i = 0; i < _channels.size(); i++) {
    channels.push_back(Channel_gui(_channels.at(i)));
    if(global_dimmer == false) {
        global_dimmer = Channel_gui(_channels.at(i)).check_for_global_dimmer();
    }
  }
}


QDebug operator<<(QDebug dbg, Fixture &f)
{
    QDebugStateSaver saver(dbg);
  dbg.nospace() << QString::fromStdString(f.to_string());

    return dbg;
}

std::string Fixture::get_colors() {
  if(!this->colors.empty())
    return this->colors;
  else
    return "LG/P/C/R/G/Y/W/B";
}

static inline void ReplaceAllSubstrings(std::string &str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
}

void Fixture::set_colors(std::string colors) {
//  std::cout << colors << " " << std::any_of(colors.begin(), colors.end(), ::islower) << std::endl;
  if(std::any_of(colors.begin(), colors.end(), ::islower) && colors != "auto") { // if there are any lower case characters
    ReplaceAllSubstrings(colors, "light-green", "LG");
    ReplaceAllSubstrings(colors, "pink", "P");
    ReplaceAllSubstrings(colors, "cyan", "C");
    ReplaceAllSubstrings(colors, "red", "R");
    ReplaceAllSubstrings(colors, "green", "G");
    ReplaceAllSubstrings(colors, "yellow", "Y");
    ReplaceAllSubstrings(colors, "white", "W");
    ReplaceAllSubstrings(colors, "orange", "O");
    ReplaceAllSubstrings(colors, "blue", "B");
    std::cout << "colors after replacing: " << colors << std::endl;
  }
  this->colors = colors;
}

void Fixture::set_position_in_group(int _position) {
  if(_position > 0)
    this->position_in_group = _position;
  else
    this->position_in_group = 0;
}

int Fixture::get_position_in_group() {
  return this->position_in_group;
}

std::vector<int> Fixture::get_pan_tilt_channels() {
  std::vector<int> pan_tilt_channels;
  if(this->name == "JBLED A7 (S8)"
  || this->name == "JBLED A7 (S16)"
  || this->name == "JBLED P4 (M1)"
  || this->name == "JBLED Sparx 7 (M3)") {
    pan_tilt_channels.push_back(1);
    pan_tilt_channels.push_back(2);
    pan_tilt_channels.push_back(3);
    pan_tilt_channels.push_back(4);
  }
  return pan_tilt_channels;
}

void Fixture::set_position_on_stage(std::string _position) {
  this->position_on_stage = _position;
}

std::string Fixture::get_position_on_stage() {
  return this->position_on_stage;
}

void Fixture::set_moving_head_type(std::string _moving_head_type) {
  this->moving_head_type = _moving_head_type;
}

std::string Fixture::get_moving_head_type() {
  return this->moving_head_type;
}
int Fixture::get_modifier_pan() {
  return this->modifier_pan;
}
int Fixture::get_modifier_tilt() {
  return this->modifier_tilt;
}
void Fixture::set_modifier_pan(int _modifier_pan) {
  this->modifier_pan = _modifier_pan;
}
void Fixture::set_modifier_tilt(int _modifier_tilt) {
  this->modifier_tilt = _modifier_tilt;
}
vector<channel_value> Fixture::get_pan_tilt_channels_with_default_positions() {
  std::vector<channel_value> pan_tilt_channels_with_default_value;
  for(int pan_channel: this->get_pan_channels())
    pan_tilt_channels_with_default_value.push_back({pan_channel, 127 + this->modifier_pan});
  for(int tilt_channel: this->get_tilt_channels())
    pan_tilt_channels_with_default_value.push_back({tilt_channel, 127 + this->modifier_tilt});
  return pan_tilt_channels_with_default_value;
}

channel_value Fixture::get_control_channel_with_ignite_value() {
  if(this->name == "JBLED P4 (M1)")
    return {5, 250};
  return {0, 0};
}

channel_value Fixture::get_control_channel_with_turn_off_value() {
  if(this->name == "JBLED P4 (M1)")
    return {5, 235};
  return {0, 0};
}

void Fixture::set_channel_count(int _channel_count) {
  this->channel_count = _channel_count;
}

void Fixture::set_timestamps_type(std::string _timestamps_type) {
  this->timestamps_type = _timestamps_type;
}

std::string Fixture::get_timestamps_type() {
  return this->timestamps_type;
}

std::vector<int> Fixture::get_pan_channels() {
  std::vector<int> pan_channels;
  if(this->name == "JBLED A7 (S8)"
  || this->name == "JBLED A7 (S16)"
  || this->name == "JBLED P4 (M1)"
  || this->name == "JBLED Sparx 7 (M3)") {
    pan_channels.push_back(1);
    pan_channels.push_back(2);
  }
  return pan_channels;
}

std::vector<int> Fixture::get_tilt_channels() {
  std::vector<int> tilt_channels;
  if(this->name == "JBLED A7 (S8)"
  || this->name == "JBLED A7 (S16)"
  || this->name == "JBLED P4 (M1)"
  || this->name == "JBLED Sparx 7 (M3)") {
    tilt_channels.push_back(3);
    tilt_channels.push_back(4);
  }
  return tilt_channels;
}

int Fixture::get_position_in_mh_group() {
  return this->position_in_mh_group;
}

void Fixture::set_position_in_mh_group(int _position) {
  this->position_in_mh_group = _position;
}

bool Fixture::get_invert_tilt() {
  return this->invert_tilt;
}

void Fixture::set_invert_tilt(bool _invert_tilt) {
  this->invert_tilt = _invert_tilt;
}

int Fixture::get_amplitude_pan() {
  return this->amplitude_pan;
}

void Fixture::set_amplitude_pan(int _amplitude_pan) {
  this->amplitude_pan = _amplitude_pan;
}

int Fixture::get_amplitude_tilt() {
  return this->amplitude_tilt;
}

void Fixture::set_amplitude_tilt(int _amplitude_tilt) {
  this->amplitude_tilt = _amplitude_tilt;
}
