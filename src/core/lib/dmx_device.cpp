//
// Created by Jan on 21.05.2019.
//

#include "dmx_device.h"
DmxDevice::~DmxDevice() {

}
void DmxDevice::set_device_status(DmxDevice::DeviceStatus device_status) {
  DmxDevice::device_status = device_status;
}
