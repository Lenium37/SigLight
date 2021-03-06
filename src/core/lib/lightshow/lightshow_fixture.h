//
// Created by Johannes on 05.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_XML_XML_FIXTURE_H_
#define RASPITOLIGHT_SRC_UI_XML_XML_FIXTURE_H_

#include <list>
#include <cstdint>
#include "channel.h"
#include <string>
#include "analysis.h"
#include <map>

/**
 * Represents a DMX Fixture in the context of a Lightshow.
 * Contains the DMX start channel that is set on the Fixture in real life,
 * the number of channels it has,
 * a vector of its Channel objects,
 * its name (e.g. "Cameo Flat RGB 10" ...),
 * its type (e.g. "bass", "high" ...)
 * and which channel number represents which functionality (red, green, blue, global dimmer, strobo).
 */
class LightshowFixture {
 public:
  /**
   * Sets name, start_channel, number_of_channels and type.
   * Depending on the name the channel numbers of the channels with functionality (red, green, blue, global dimmer, strobo) are set.
   * This is hard coded!
   * @param name
   * @param start_channel
   * @param number_of_channels
   * @param type
   */
  LightshowFixture(std::string name, int start_channel, int number_of_channels, std::string type, std::string colors, int _position_inside_group, std::string _position_on_stage, std::string _moving_head_type, int _modifier_pan, int _modifier_tilt, std::string _timestamps_type, int _position_inside_mh_group, bool _invert_tilt, int _amplitude_pan, int _amplitude_tilt);
  ~LightshowFixture();

  /**
   * Returns the DMX start channel of the Fixture.
   * @return
   */
  int get_start_channel() const;

  /**
   * Returns the vector of the Channel objects the Fixture contains
   * @return
   */
  std::vector<Channel> get_channels() const;

  /**
   * Sets the start channel of the Fixture.
   * @param start_channel
   */
  void set_start_channel(int start_channel);

  /**
   * Adds a Channel to the vector of Channel objets the Fixture contains.
   * @param channel
   */
  void add_channel(Channel channel);

  /**
   * Sets the number of channels the Fixture has.
   * @param number_of_channels
   */
  void set_number_of_channels(int number_of_channels);

  /**
   * Returns the number of channels the Fixture has.
   * @return
   */
  int get_number_of_channels();

  /**
   * Returns the channel number of the global dimmer.
   * @return global dimmer
   */
  std::uint8_t get_channel_dimmer();

  /**
   * Returns the channel number of the strobo channel.
   * @return strobo
   */
  std::uint8_t get_channel_strobo();

  /**
   * Returns the channel number of the red channel.
   * @return red
   */
  std::uint8_t get_channel_red();

  /**
   * Returns the channel number of the green channel.
   * @return green
   */
  std::uint8_t get_channel_green();

  /**
   * Returns the channel number of the blue channel.
   * @return blue
   */
  std::uint8_t get_channel_blue();

  std::uint8_t get_channel_shutter();
  std::uint8_t get_channel_pan();
  std::uint8_t get_channel_tilt();
  std::uint8_t get_channel_blinder();
  std::uint8_t get_channel_flash_duration();
  std::uint8_t get_channel_flash_rate();
  std::uint8_t get_blinder_value();
  std::uint8_t get_channel_colorwheel();
  std::uint8_t get_channel_focus();
  std::uint8_t get_channel_zoom();

  /**
   * Sets the channel number of the global dimmer.
   * @param channel_dimmer
   */
  void set_channel_dimmer(std::uint8_t channel_dimmer);

  /**
   * Sets the channel number of the strobo channel.
   * @param channel_strobo
   */
  void set_channel_strobo(std::uint8_t channel_strobo);

  /**
   * Sets the channel number of the red channel.
   * @param channel_red
   */
  void set_channel_red(std::uint8_t channel_red);

  /**
   * Sets the channel number of the green channel.
   * @param channel_green
   */
  void set_channel_green(std::uint8_t channel_green);

  /**
   * Sets the channel number of the blue channel.
   * @param channel_blue
   */
  void set_channel_blue(std::uint8_t channel_blue);

  void set_channel_shutter(std::uint8_t channel_shutter);
  void set_channel_pan(std::uint8_t _channel_pan);
  void set_channel_tilt(std::uint8_t _channel_tilt);
  void set_channel_blinder(std::uint8_t _channel_blinder);
  void set_channel_flash_duration(std::uint8_t _channel_flash_duration);
  void set_channel_flash_rate(std::uint8_t _channel_flash_rate);
  void set_blinder_value(std::uint8_t _blinder_value);
  void set_channel_colorwheel(std::uint8_t _channel_colowheel);
  void set_channel_focus(std::uint8_t _channel_focus);
  void set_channel_zoom(std::uint8_t _channel_zoom);

  /**
   * Sets the name of the Fixture.
   * @param name
   */
  void set_name(std::string name);

  /**
   * Returns the name of the Fixture
   * @return
   */
  std::string get_name();

  /**
   * Sets the type of the Fixture.
   * @param type
   */
  void set_type(std::string type);

  /**
   * Returns the type of the Fixture.
   * @return
   */
  std::string get_type();

  /**
   * Adds ValueChanges to a specific, already existing Channel within this Fixture.
   * Used for adding color fades.
   * @param value_changes Vector of ColorChanges to be added to the channel
   * @param ch The number of the channel the ValueChanges should be added to
   */
  void add_value_changes_to_channel(std::vector<time_value_int> value_changes, int ch);

  /**
   * If the Fixture has a global dimmer channel, this gets set to true.
   */
  bool has_global_dimmer = false;

  bool has_shutter = false;

  bool has_pan = false;
  bool has_tilt = false;
  bool has_strobe = false;
  bool is_blinder = false;
  bool has_flash_duration = false;
  bool has_flash_rate = false;
  bool has_colorwheel = false;
  bool has_focus = false;
  bool has_zoom = false;

  std::vector<std::string> get_colors();
  std::string get_colors_string();

  void set_position_in_group(int _position);
  int get_position_in_group();

  void set_temp_position_in_group(int _position);
  int get_temp_position_in_group();

  void set_position_in_mh_group(int _position);
  int get_position_in_mh_group();

  void set_position_on_stage(std::string _position);
  std::string get_position_on_stage();

  void set_moving_head_type(std::string _moving_head_type);
  std::string get_moving_head_type();

  std::uint8_t get_colorwheel_value(std::string color);

  int get_modifier_pan();
  int get_modifier_tilt();

  float get_degrees_per_pan();
  float get_degrees_per_tilt();

  void set_timestamps_type(std::string _timestamps_type);
  std::string get_timestamps_type();
  bool get_invert_tilt();
  int get_amplitude_pan();
  int get_amplitude_tilt();
  void set_invert_tilt(bool _invert_tilt);
  void set_amplitude_pan(int _amplitude_pan);
  void set_amplitude_tilt(int _amplitude_tilt);

 private:
  bool channel_already_exists(int channel);
  Channel & get_channel(int channel);
  int start_channel;
  int number_of_channels;
  std::vector<Channel> channels;
  std::string name;
  std::string type;
  std::uint8_t channel_dimmer;
  std::uint8_t channel_strobo;
  std::uint8_t channel_red;
  std::uint8_t channel_green;
  std::uint8_t channel_blue;
  std::uint8_t channel_shutter;
  std::uint8_t channel_pan;
  std::uint8_t channel_tilt;
  std::uint8_t channel_blinder;
  std::uint8_t blinder_value;
  std::uint8_t channel_colorwheel;
  std::uint8_t channel_focus;
  std::uint8_t channel_zoom;
  std::uint8_t channel_flash_duration;
  std::uint8_t channel_flash_rate;
  Channel dimmer = Channel(0);
  Channel strobo = Channel(0);
  Channel red = Channel(0);
  Channel green = Channel(0);
  Channel blue = Channel(0);
  std::vector<std::string> colors;
  std::string colors_string;
  int position_inside_group;
  int temp_position_inside_group;
  std::string position_on_stage;
  std::string moving_head_type;
  std::map<const std::string, std::uint8_t> colorwheel_values;
  int modifier_pan;
  int modifier_tilt;
  float degrees_per_pan;
  float degrees_per_tilt;
  std::string timestamps_type;
  int position_in_mh_group;
  bool invert_tilt;
  int amplitude_pan;
  int amplitude_tilt;

};

#endif //RASPITOLIGHT_SRC_UI_XML_XML_FIXTURE_H_
