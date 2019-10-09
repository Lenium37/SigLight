//
// Created by Johannes on 08.09.2019.
//

#include "dmx_device_eurolite_pro.h"
#include "logger.h"
#include <iostream>
#include <unistd.h>


int DmxDeviceEurolitePro::write_data(std::vector<std::uint8_t> &channels_to_write_, int max_channel_) {
    for(int i = 0; i < max_channel_; i++) {
      this->dmx_frame[i + 5] = channels_to_write_[i];
    }
    libusb_bulk_transfer(
        devh,                 // dev handle
        (0x2 | LIBUSB_ENDPOINT_OUT), // EP
        this->dmx_frame,                        // data
        sizeof(this->dmx_frame),                   // size
        nullptr,                        // & bytes sent
        0                      // timeout (ms)
    );
  return 0;
}
int DmxDeviceEurolitePro::write_data(std::vector<std::uint8_t> &channels_to_write_) {
    for(int i = 0; i < channels_to_write_.size(); i++) {
      this->dmx_frame[i + 5] = channels_to_write_[i];
    }
    libusb_bulk_transfer(
        devh,                 // dev handle
        (0x2 | LIBUSB_ENDPOINT_OUT), // EP
        this->dmx_frame,                        // data
        sizeof(this->dmx_frame),                   // size
        nullptr,                        // & bytes sent
        0                      // timeout (ms)
    );
  return 0;
}
int DmxDeviceEurolitePro::start_device() {
  if (device_already_started()) {
    Logger::info("DmxDevice was already started");
  } else {
    int status = 0;
    libusb_context *ctx = nullptr;
    status = libusb_init(&ctx);
    if (status != LIBUSB_SUCCESS)
      Logger::error("failed to init libusb");

    // list all devices
    libusb_device **devs;
    int k = 0;
    libusb_get_device_list(nullptr, &devs);
    while ((dev = devs[k++]) != nullptr) {
      struct libusb_device_descriptor desc;
      int r = libusb_get_device_descriptor(dev, &desc);
      if (r < 0) {
        Logger::error("failed to read device descriptor");
      }

      Logger::debug("idVendor: {}, idProduct: {}", desc.idVendor, desc.idProduct);
      if (desc.idVendor == FTDI_VENDOR_ID && desc.idProduct == FTDI_PRODUCT_ID) {
        status = libusb_open(dev, &devh);
        Logger::debug("status open: {}", status);
        if (devh) {
          Logger::debug("opened Eurolite Pro DMX interface");
          device_status = DmxDevice::DeviceStatus::STARTED;

          // MK2 (also Eurolite Pro Cable) Specific Baud Rate Control Transfer
          uint16_t divisor = 3000000 / 250000;
          uint16_t value = divisor;  // divisor & 0xFFFF
          uint16_t index = (divisor >> 8) & 0xFF00;
          libusb_control_transfer(devh,
                                  LIBUSB_REQUEST_TYPE_VENDOR  | LIBUSB_RECIPIENT_DEVICE  | LIBUSB_ENDPOINT_OUT,
                                  0x03,
                                  value,
                                  index,
                                  NULL,
                                  0,
                                  500
          );
          Logger::debug("Set Baud Rate of Eurolite Pro.");

          // prepare DMX Frame
          dmx_frame[0] = 0x7e; // START OF MESSAGE
          dmx_frame[1] = 6; // DMX LABEL ?
          dmx_frame[2] = 0b00000001; // gesplitted aus DMX_UNIVERSE_SIZ+1 uint_16_t zu uint_8_t
          dmx_frame[3] = 0b00000010; // gesplitted aus DMX_UNIVERSE_SIZ+1 uint_16_t zu uint_8_t
          dmx_frame[4] = 0;
          std::memset(dmx_frame + 5, 0, 512);
          dmx_frame[517] = 0xe7;

        } else {
          Logger::error("could not open DMX interface. error id: {}", status);
          return status;
        }
      }

    }
    if (devh == nullptr) {
      Logger::error("did not find DMX interface in USB devices");
    }
    libusb_free_device_list(devs, 1);
    libusb_set_auto_detach_kernel_driver(devh, 1);
#ifdef __arm__
    libusb_detach_kernel_driver(devh, 0);
#endif
    status = libusb_claim_interface(devh, 0);
    Logger::debug("status of claim_interface: {}", status);
    return status;
  }

  return 0;
}

bool DmxDeviceEurolitePro::device_already_started() const { return (device_status == DeviceStatus::STARTED || device_status == DeviceStatus::WAIT_FOR_DATA || device_status == DeviceStatus::STOPPED); }

int DmxDeviceEurolitePro::stop_device() {
  //int status = 0;
  //status = libusb_release_interface(devh, 0);
  //Logger::debug("status of release_interface: {}", status);
  //device_status = DmxDevice::DeviceStatus::NOT_STARTED; // this will also stop the daemon thread
  device_status = DmxDevice::DeviceStatus::STOPPED; // this will also stop the daemon thread
  return 0;
}

bool DmxDeviceEurolitePro::is_connected() const {
  libusb_context *ctx = nullptr;
  int status = libusb_init(&ctx);
  if (status != LIBUSB_SUCCESS) {
    Logger::error("Failed to init libusb: {}", libusb_error_name(status));
  }

  libusb_device **devs;
  libusb_device *device = nullptr;

  int k = 0;
  libusb_get_device_list(nullptr, &devs);
  while ((device = devs[k++]) != nullptr) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(device, &desc);
    if (r < 0) {
      Logger::error("failed to read device descriptor!");
    }
    if (desc.idVendor == FTDI_VENDOR_ID && desc.idProduct == FTDI_PRODUCT_ID) {
      return true;
    }
  }
  return false;
}
void DmxDeviceEurolitePro::start_daemon_thread() {
    Logger::info("Eurolite not using deamon thread");
}

int DmxDeviceEurolitePro::turn_off_all_channels() {
  usleep(25000);
  int actual = 0;
  std::memset(dmx_frame + 5, 0, 512);
  libusb_interrupt_transfer(
      devh,                 // dev handle
      (0x2 | LIBUSB_ENDPOINT_OUT), // EP
      this->dmx_frame,                        // data
      sizeof(this->dmx_frame),                   // size
      &actual,                        // & bytes sent
      0                      // timeout (ms)
  );
  usleep(25000);
  Logger::info("turned off all DMX channels");
  return 0;
}
