//
// Created by Johannes on 08.09.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_EUROLITE_PRO_H_
#define RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_EUROLITE_PRO_H_

static const int FTDI_VENDOR_ID = 0x0403;
static const int FTDI_PRODUCT_ID = 0x6001;

#if defined(_WIN32) || defined(WIN32)
#include <mingw.thread.h>
#else
#include <thread>
#endif

#include "dmx_device.h"
#include <string>
#include <libusb.h>
#include <atomic>

/**
 * This class works for Eurolite Pro Mk2 and Eurolite Pro Cable Version.
 */
class DmxDeviceEurolitePro : public DmxDevice{
 public:
  int write_data(std::vector<std::uint8_t> &channels_to_write, int max_channel) override;
  int write_data(std::vector<std::uint8_t> &channels_to_write) override;
  int start_device() override;
  int stop_device() override;
  bool is_connected() const override;
  int turn_off_all_channels() override;
  void start_daemon_thread() override;

 private:
  libusb_device_handle *devh;
  libusb_device *dev;
  std::unique_ptr<std::thread> daemon_thread;
  std::vector<std::uint8_t> channels_to_write;
  unsigned char dmx_frame[518];
  std::atomic<bool> data_changed;
  bool device_already_started() const;
  void copy_data_to_device();

};

#endif //RASPITOLIGHT_SRC_CORE_LIB_DMX_DEVICE_EUROLITE_PRO_H_
