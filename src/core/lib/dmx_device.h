//
// Created by Jan on 21.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_DMX_DEVICE_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_DMX_DEVICE_H_

#include <vector>
#include <cstdint>

class DmxDevice {
 protected:
  enum class DeviceStatus {
    NOT_STARTED = 0,
    STARTED = 1,
    WAIT_FOR_DATA = 2,
    STOPPED = 3
  };
 public:
  virtual int write_data(std::vector<std::uint8_t> &channels_to_write, int max_channel) = 0;
  virtual int write_data(std::vector<std::uint8_t> &channels_to_write) = 0; // write complete data vector
  virtual int start_device() = 0;
  virtual int stop_device() = 0;
  void set_device_status(DeviceStatus device_status);
  virtual int turn_off_all_channels(std::vector<int> pan_tilt_channels) = 0;
  virtual int set_channel_values(std::vector<std::uint8_t> channels_with_default_values) = 0;
  virtual void start_daemon_thread() = 0;
  /**
   * Checks if the device is connected and could be opened using start_device. This method only checks if the device
   * if correctly connected and does not do ANY initialization!!!. It is possible to contruct a DmxDevice and run
   * is_connected() without ANY sideeffects!!!
   * @return if the device is connected and could be opened using start_device
   */
  virtual bool is_connected() const = 0; // todo should be const
  virtual ~DmxDevice();
 protected:
  DeviceStatus device_status;
};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_DMX_DEVICE_H_
