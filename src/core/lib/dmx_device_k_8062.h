//
// Created by Jan on 26.05.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_K_8062_H_
#define RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_K_8062_H_

static const int K_8062_PRODUCT_ID = 32866;
static const int K_8062_VENDOR_ID = 4303;

#if defined(_WIN32) || defined(WIN32)
#include <mingw.thread.h>
#else
#include <thread>
#endif

#include "dmx_device.h"
#include <string>
#include <libusb.h>
#include <atomic>

class DmxDeviceK8062 : public DmxDevice{
 public:
  int write_data(std::vector<std::uint8_t> &channels_to_write, int max_channel) override;
  int write_data(std::vector<std::uint8_t> &channels_to_write) override;
  int start_device() override;
  int stop_device() override;
  bool is_connected() const override;
  void write_test(unsigned char *data);
  int turn_off_all_channels(std::vector<int> pan_tilt_channels) override;
  void start_daemon_thread() override;

  int write_data_test(std::vector<std::uint8_t> &channels_to_write, int max_channel, bool &lightshow_playing, bool &lightshow_paused);

 private:
  libusb_device_handle *devh;
  libusb_device *dev;
  std::unique_ptr<std::thread> daemon_thread;
  std::vector<std::uint8_t> channels_to_write;
  int max_channel;
  std::atomic<bool> data_changed;
  bool device_already_started() const;
  void copy_data_to_device();

};

#endif //RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_K_8062_H_
