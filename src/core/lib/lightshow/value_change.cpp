//
// Created by Johannes on 05.05.2019.
//

#include "value_change.h"

ValueChange::ValueChange(float timestamp, int value) {
  this->timestamp = timestamp;
  this->value = value;
}

ValueChange::~ValueChange() {

}

float ValueChange::get_timestamp() const {
  return this->timestamp;
}

int ValueChange::get_value() const {
  return this->value;
}

void ValueChange::set_timestamp(float timestamp) {
  this->timestamp = timestamp;
}

void ValueChange::set_value(int value) {
  this->value = value;
}
