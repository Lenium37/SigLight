#ifndef FIXTURE_H
#define FIXTURE_H
#include <string>

#include <QStringList>
#include "channel_gui.h"
#include <vector>

using namespace std;

class Fixture
{
public:
  /**
   * Default Constructor of Fixture.
   */
  Fixture() = default;

  /**
   * @brief Fixture Constructor of Fixture with: name, type, description, channels, icon, colors.
   */
  Fixture(string, string, string, QStringList, std::string, std::string);

  /**
   * @brief Fixture Fixture Constructor of Fixture with: name, type, description, start_channel, channels, icon, colors.
   */
  Fixture(string, string, string, int, QStringList, std::string, std::string);

  /**
   * Default destructor.
   */
  ~Fixture() = default;

  /**
   * @brief get_name Get the name of the Fixture.
   * @return Name of the Fixture.
   */
  string get_name();

  /**
   * @brief get_description Get description of the Fixture.
   * @return Description of the Fixture.
   */
  string get_description();

  /**
   * @brief get_description_view Get the Description, optimized for viewing.
   * @return Description formated.
   */
  string get_description_view();

  /**
   * @brief get_type Gets the type of the Fixture.
   * @return Type of the Fixture.
   */
  string get_type();

  /**
   * @brief get_channel_count Gets the ammount of channels of this Fixture.
   * @return Number of channels.
   */
  int get_channel_count();

  /**
   * @brief get_start_channel Get the starting channel.
   * @return Startring channel.
   */
  int get_start_channel();

  /**
   * @brief get_last_channel Get the last channel.
   * @return Last channel.
   */
  int get_last_channel();

  /**
   * @brief get_channels Get a list of the channels.
   * @return List of Channels.
   */
  std::list<Channel_gui> get_channels();

  /**
   * @brief get_icon Get the Icon identifyer.
   * @return The Icon identifyer but not its Path.
   */
  std::string get_icon();

  /**
   * @brief set_start_channel Sets the start channel.
   * @param start_channel Channel the fixture will start to listen to.
   */
  void set_start_channel(int start_channel);

  /**
   * @brief set_channels Sets the channels of the fixture.
   */
  void set_channels(QStringList);

  /**
   * @brief operator << Get a text representation of the Fixture for concatinating.
   * @param d Debugobject.
   * @param f The fixture that will be shown.
   * @return Textrepresentation of this Fixture.
   */
  friend QDebug operator<<(QDebug d, Fixture &f);

  /**
   * @brief to_string Get the to_string representation of this Fixture.
   * @return Text representation.
   */
  string to_string();

  /**
   * @brief set_type Set the type of this Fixture.
   * @param type Type.
   */
  void set_type(const string &type);

  /**
   * @brief set_name Set the name of this Fixture.
   * @param name Name.
   */
  void set_name(const string &name);

  /**
   * @brief set_description Set description of the Fixture.
   * @param description Description.
   */
  void set_description(const string &description);

  /**
   * @brief set_icon Sets the Icon of the Fixture.
   * @param icon_identifyer The icon identifyer.
   */
  void set_icon(const std::string & icon_identifyer);

  /**
   * @brief has_global_dimmer Checks if the Fixture has a global Dimmer;
   * @return
   */
  bool has_global_dimmer();

  std::string get_colors();

  void set_colors(std::string colors);

  void set_position_in_group(int _position);

  int get_position_in_group();

  std::vector<int> get_pan_tilt_channels();

private:
  /**
   * @brief name Name of the Fixture.
   */
  std::string name;

  /**
   * @brief type Type of the Fixture.
   */
  std::string type;

  /**
   * @brief description Description of the Fixture.
   */
  std::string description;

  /**
   * @brief icon Icon identifyer of the Fixture.
   */
  std::string icon;

  /**
   * @brief start_channel Start channel of the Fixture.
   */
  int start_channel;

  /**
   * @brief channels Channels of the Fixture.
   */
  std::list<Channel_gui> channels;

  /**
   * @brief global_dimmer If the Fixture has a global dimmer.
   */
  bool global_dimmer;

  std::string colors;

  int position_in_group;

};

#endif // FIXTURE_H
