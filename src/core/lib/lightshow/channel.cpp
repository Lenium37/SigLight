//
// Created by Johannes on 05.05.2019.
//

#include "channel.h"

Channel::Channel(int channel) {
  this->channel = channel;
}

Channel::~Channel() {

}

int Channel::get_channel() const {
  return this->channel;
}

std::vector<ValueChange> Channel::get_value_changes() {
  return this->value_changes;
}

void Channel::set_channel(int channel) {
  this->channel = channel;
}

void Channel::add_value_change(ValueChange value_change) {
  this->value_changes.push_back(value_change);
}

int Channel::get_value_of_last_added_value_change() {
  if (!this->value_changes.empty())
    return this->value_changes.back().get_value();
  else
    return -1;
}