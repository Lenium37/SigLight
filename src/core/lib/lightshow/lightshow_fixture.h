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
  LightshowFixture(std::string name, int start_channel, int number_of_channels, std::string type, std::string colors, int _position_inside_group);
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

  std::vector<std::string> get_colors();

  void set_position_in_group(int _position);

  int get_position_in_group();

 private:
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
  Channel dimmer = Channel(0);
  Channel strobo = Channel(0);
  Channel red = Channel(0);
  Channel green = Channel(0);
  Channel blue = Channel(0);
  std::vector<std::string> colors;
  int position_inside_group;

};

#endif //RASPITOLIGHT_SRC_UI_XML_XML_FIXTURE_H_
