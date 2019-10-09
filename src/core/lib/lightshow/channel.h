//
// Created by Johannes on 05.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_XML_CHANNEL_H_
#define RASPITOLIGHT_SRC_UI_XML_CHANNEL_H_

#include "value_change.h"
#include <vector>

/**
 * Represents a channel of a DMX fixture.
 * It contains its channel number inside of the DMX fixture and a vector of its ValueChanges.
 */
class Channel {
 public:
  /**
   * Sets the channel number.
   * @param channel
   */
  Channel(int channel);
  ~Channel();

  /**
   * Returns the channel number.
   * @return channel
   */
  int get_channel() const;

  /**
   * Returns a vector of its ValueChanges.
   * @return
   */
  std::vector<ValueChange> get_value_changes();

  /**
   * Sets the channel number.
   * @param channel
   */
  void set_channel(int channel);

  /**
   * Adds a ValueChange to the vector of its ValueChanges.
   * @param value_change
   */
  void add_value_change(ValueChange value_change);

  /**
   * Returns the value of the last ValueChange that was added.
   * @return value_of_last_ladded_value_change
   */
  int get_value_of_last_added_value_change();

 private:
  /**
   * The channel number.
   */
  int channel;

  /**
   * Vector of its ValueChanges.
   */
  std::vector<ValueChange> value_changes;

};

#endif //RASPITOLIGHT_SRC_UI_XML_CHANNEL_H_
