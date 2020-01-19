//
// Created by Jan on 26.05.2019.
//

#include "dmx_device_k_8062.h"
#include "logger.h"
#include <libusb.h>
#include <iostream>
#include <unistd.h>

//#include <chrono>  // for high_resolution_clock

#if defined(_WIN32) || defined(WIN32)
#include <mingw.mutex.h>
#else
#include <mutex>
#endif

// mutex for the vector of DMX channel data
std::mutex channel_data_mutex;

int DmxDeviceK8062::write_data(std::vector<std::uint8_t> &channels_to_write_, int max_channel_) {
  if (device_status == DmxDevice::DeviceStatus::WAIT_FOR_DATA) {
    std::lock_guard<std::mutex> guard(channel_data_mutex);
    this->channels_to_write = std::vector<std::uint8_t>(channels_to_write_);
    this->max_channel = max_channel_;
  } else {
    Logger::warning("Dmx Device received data, but status is not WAIT_FOR_DATA");
  }
  return 0;
}
int DmxDeviceK8062::write_data(std::vector<std::uint8_t> &channels_to_write_) {
  if (device_status == DmxDevice::DeviceStatus::WAIT_FOR_DATA) {
    this->channels_to_write = std::vector<std::uint8_t>(channels_to_write_);
  } else {
    Logger::warning("Dmx Device received data, but status is not WAIT_FOR_DATA");
  }
  return 0;
}
int DmxDeviceK8062::start_device() {
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
      if (desc.idVendor == K_8062_VENDOR_ID && desc.idProduct == K_8062_PRODUCT_ID) {
        status = libusb_open(dev, &devh);
        Logger::debug("status open: {}", status);
        if (devh) {
          Logger::debug("opened Velleman K8062 DMX interface");
          device_status = DmxDevice::DeviceStatus::STARTED;
          //start_daemon_thread();
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

bool DmxDeviceK8062::device_already_started() const { return (device_status == DeviceStatus::STARTED || device_status == DeviceStatus::WAIT_FOR_DATA || device_status == DeviceStatus::STOPPED); }

int DmxDeviceK8062::stop_device() {
  //int status = 0;
  //status = libusb_release_interface(devh, 0);
  //Logger::debug("status of release_interface: {}", status);
  //device_status = DmxDevice::DeviceStatus::NOT_STARTED; // this will also stop the daemon thread
  device_status = DmxDevice::DeviceStatus::STOPPED; // this will also stop the daemon thread
  return 0;
}

bool DmxDeviceK8062::is_connected() const {
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
    if (desc.idVendor == K_8062_VENDOR_ID && desc.idProduct == K_8062_PRODUCT_ID) {
      return true;
    }
  }
  return false;
}
void DmxDeviceK8062::start_daemon_thread() {
  if (!daemon_thread) {
    Logger::info("starting daemon thread");
    daemon_thread = std::unique_ptr<std::thread>(new std::thread(&DmxDeviceK8062::copy_data_to_device, this));
    daemon_thread->detach();
    Logger::info("daemon thread started");
  } else {
    Logger::info("daemon thread already started");
  }
}

void DmxDeviceK8062::copy_data_to_device() {
  device_status = DmxDevice::DeviceStatus::WAIT_FOR_DATA;

  while (device_status == DmxDevice::DeviceStatus::WAIT_FOR_DATA) {
    // Record start time
    //auto start = std::chrono::high_resolution_clock::now();
    if (this->channels_to_write.size() < 6) {
      Logger::debug("skip copy_data_to_device, size: {}", this->channels_to_write.size());
    } else {
      Logger::trace("copy_data_to_device, size: {}", this->channels_to_write.size());
      unsigned char data[8];

      int i, n;
      int m = max_channel;
      //int actual; //used to find out how many bytes were written


      channel_data_mutex.lock();
      for (i = 0; (i < 100) && !channels_to_write[i] && (i < m - 6); i++);
      data[0] = 4; // send start code + 6 bytes of data
      data[1] = i + 1; // number of zeroes (incl. start code = 0)
      data[2] = channels_to_write[i];
      data[3] = channels_to_write[i + 1];
      data[4] = channels_to_write[i + 2];
      data[5] = channels_to_write[i + 3];
      data[6] = channels_to_write[i + 4];
      data[7] = channels_to_write[i + 5];
      channel_data_mutex.unlock();
      //Logger::debug("data prepared");

      this->write_test(data);
      i += 6; // ersten sechs Elemente wurden gesendet, i kann erhöht werden
      //Logger::debug("transfered");
      while (i < m - 7) {   // z.B.  6 < 96 - 7 = 6 < 89
        channel_data_mutex.lock();
        if (!channels_to_write[i]) { // ist channel-array hier noch nicht zu Ende?
          for (n = i + 1; (n < m - 6) && (n - i < 100) && !channels_to_write[n];
               n++); // beim ersten Mal: n=7; 7<96-6 && 7-6<100 && channels_to_write leer;n = 8
          data[0] = 5;  // send n zeroes plus 6 bytes of data
          data[1] = n
              - i; // number of zeroes to send, beim ersten Mal 6 (weil die ja schon in der Schleife oben geschickt wurden)
          data[2] = channels_to_write[n];
          data[3] = channels_to_write[n + 1];
          data[4] = channels_to_write[n + 2];
          data[5] = channels_to_write[n + 3];
          data[6] = channels_to_write[n + 4];
          data[7] = channels_to_write[n + 5];
          channel_data_mutex.unlock();
          this->write_test(data);
          i = n + 6;
        } else {
          data[0] = 2; // 7 channels_to_write
          data[1] = channels_to_write[i];
          data[2] = channels_to_write[i + 1];
          data[3] = channels_to_write[i + 2];
          data[4] = channels_to_write[i + 3];
          data[5] = channels_to_write[i + 4];
          data[6] = channels_to_write[i + 5];
          data[7] = channels_to_write[i + 6];
          channel_data_mutex.unlock();
          this->write_test(data);
          i += 7;
        }
      }

      for (; i < m; i++) {

        data[0] = 3; // send one channel
        channel_data_mutex.lock();
        data[1] = channels_to_write[i];
        channel_data_mutex.unlock();
        this->write_test(data);
        /*libusb_bulk_transfer(devh,
                             (1 | LIBUSB_ENDPOINT_OUT),
                             data,
                             8,
                             &actual,
                             0); */
      }
      data_changed = false;
    }

// Record end time
    //auto finish = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> elapsed = finish - start;
    //std::cout << "copying data to device took: " << elapsed.count() << " s\n";

  }
  daemon_thread = nullptr;
  Logger::info("daemon thread ended");
}

void DmxDeviceK8062::write_test(unsigned char *data) {

  int actual = 0;
  libusb_interrupt_transfer(devh,
                       (1 | LIBUSB_ENDPOINT_OUT),
                       data,
                       8,
                       &actual,
                       0);


  /*libusb_bulk_transfer(devh,
                       (1 | LIBUSB_ENDPOINT_OUT),
                       data,
                       8,
                       &actual,
                       0);
*/

}



int DmxDeviceK8062::turn_off_all_channels(std::vector<int> pan_tilt_channels) {
  usleep(25000);
  std::vector<std::uint8_t> v;
  for(int i = 0; i < this->max_channel; i++) {
    v.push_back(0);
  }
  this->channels_to_write = v;
  usleep(25000);
  Logger::info("turned off all DMX channels");
  return 0;
}
