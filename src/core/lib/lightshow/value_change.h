//
// Created by Johannes on 05.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_XML_VALUE_CHANGE_H_
#define RASPITOLIGHT_SRC_UI_XML_VALUE_CHANGE_H_

/**
 * Represents a ValueChange of a DMX channel.
 * It contains a timestamp and a value.
 */
class ValueChange {
 public:
  /**
   * Sets timestamp and value.
   * @param timestamp
   * @param value
   */
  ValueChange(float timestamp, int value);
  ~ValueChange();

  /**
   * Returns timestamp of ValueChange.
   * @return timestamp
   */
  float get_timestamp() const;

  /**
   * Returns value of ValueChange.
   * @return value
   */
  int get_value() const;

  /**
   * Sets timestamp of ValueChange.
   * @param timestamp
   */
  void set_timestamp(float timestamp);

  /**
   * Sets Value of ValueChange.
   * @param value
   */
  void set_value(int value);

 private:
  /**
   * The timestamp the ValueChange occurs.
   */
  float timestamp;

  /**
   * The value a channel should have after this ValueChange.
   */
  int value;
};

#endif //RASPITOLIGHT_SRC_UI_XML_VALUE_CHANGE_H_
