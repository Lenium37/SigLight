//
// Created by Jan on 02.06.2019.
//
/*#include <dmx_device_k_8062.h>
#include <logger.h>

int main(){
  DmxDeviceK8062 dmx_device_k_8062;
  Logger::info("Device connected: {}", dmx_device_k_8062.is_connected());
  dmx_device_k_8062.start_device();

}*/

/*
 * libusb example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <logger.h>

#include "libusb.h"




static const unsigned int URB_TIMEOUT_MS = 500;
static const uint8_t DMX_LABEL = 6;
static const uint8_t START_OF_MESSAGE = 0x7e;
static const uint8_t END_OF_MESSAGE = 0xe7;
static const unsigned char ENDPOINT = 0x02;
static const uint8_t MK2_SET_BAUD_RATE = 0x03;
static const unsigned int MK2_TIMEOUT_MS = 500;
enum { EUROLITE_PRO_FRAME_SIZE = 518 };

static const uint8_t ENAPLE_API2 = 0x0D;
static const uint8_t DMX_ZERO = 0x0D;


static void print_devs(libusb_device **devs)
{
  libusb_device *dev;
  libusb_device_handle *devh;
  int i = 0, j = 0;
  uint8_t path[8];

  while ((dev = devs[i++]) != NULL) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stderr, "failed to get device descriptor");
      return;
    }

    printf("%04x:%04x (bus %d, device %d)",
           desc.idVendor, desc.idProduct,
           libusb_get_bus_number(dev), libusb_get_device_address(dev));

    if(desc.idVendor == 0x0403 || desc.idProduct == 0x6001) {
      printf("\neurolite found!\n");
      int status = libusb_open(dev, &devh);
      printf("status of libusb_open: %d\n", status);
      if(status == 0) {

        libusb_set_auto_detach_kernel_driver(devh, 1);
        status = libusb_claim_interface(devh, 0);
        printf("status of claim_interface: %d\n", status);

        int actual = 0;

        printf("LIBUSB_ENDPOINT_OUT: %08x\n", LIBUSB_ENDPOINT_OUT);
        printf("LIBUSB_REQUEST_TYPE_VENDOR: %08x\n", LIBUSB_REQUEST_TYPE_VENDOR);
        printf("LIBUSB_RECIPIENT_DEVICE: %08x\n", LIBUSB_RECIPIENT_DEVICE);
        printf("LIBUSB_REQUEST_TYPE_VENDOR  | LIBUSB_RECIPIENT_DEVICE  | LIBUSB_ENDPOINT_OUT: %08x\n", LIBUSB_REQUEST_TYPE_VENDOR  | LIBUSB_RECIPIENT_DEVICE  | LIBUSB_ENDPOINT_OUT);
        /*
        libusb_control_transfer(devh,
                                LIBUSB_REQUEST_TYPE_VENDOR  | LIBUSB_RECIPIENT_DEVICE  | LIBUSB_ENDPOINT_OUT,
                                0x4,
                                0x5008,
                                0,
                                NULL,
                                0,
                                0
        );
        std::cout << "control transfer 1 completed." << std::endl;
        */
        /*
        libusb_control_transfer(devh,
                                LIBUSB_REQUEST_TYPE_VENDOR  | LIBUSB_RECIPIENT_DEVICE  | LIBUSB_ENDPOINT_OUT,
                                0x4,
                                0x1008,
                                0,
                                NULL,
                                0,
                                0
        );
        std::cout << "control transfer 2 completed." << std::endl;
        */
        // MK2 Specific Baud Rate Control Transfer
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
        std::cout << "control transfer 3 completed." << std::endl;


        // HERE WE GO

        unsigned char first_transfer[518];
        first_transfer[0] = 0x7e; // START OF MESSAGE
        first_transfer[1] = 6; // DMX LABEL ?
        first_transfer[2] = 0b00000001; // gesplitted aus DMX_UNIVERSE_SIZ+1 uint_16_t zu uint_8_t
        first_transfer[3] = 0b00000010; // gesplitted aus DMX_UNIVERSE_SIZ+1 uint_16_t zu uint_8_t
        first_transfer[4] = 0;
        std::memset(first_transfer + 5, 0, 512);
        first_transfer[517] = 0xe7;
        first_transfer[5] = 255; // DIMMER
        first_transfer[7] = 255; // ROT
        libusb_interrupt_transfer(
            devh,                 // dev handle
            (0x2 | LIBUSB_ENDPOINT_OUT), // EP
            first_transfer,                        // data
            sizeof(first_transfer),                   // size
            &actual,                        // & bytes sent
            0                      // timeout (ms)
        );
        std::cout << "actual written first_transfer: " << actual << std::endl;

        usleep(1000000);

        first_transfer[5] = 0;

        libusb_interrupt_transfer(
            devh,                 // dev handle
            (0x2 | LIBUSB_ENDPOINT_OUT), // EP
            first_transfer,                        // data
            sizeof(first_transfer),                   // size
            &actual,                        // & bytes sent
            0                      // timeout (ms)
        );
        std::cout << "actual written first_transfer: " << actual << std::endl;


        /*
 * Create a Eurolite Pro message to match the supplied DmxBuffer.
 */

/*
        unsigned char first_transfer[9];
        first_transfer[0] = START_OF_MESSAGE; // DMX start code (Which constitutes the + 1 below)
        first_transfer[1] = ENAPLE_API2; // Enable API2
        first_transfer[2] = 0x04; // data length LSB
        first_transfer[3] = DMX_ZERO; // data length MSB
        first_transfer[4] = 0xAD; // Magic number. WTF ??
        first_transfer[5] = 0x88; // Magic number. WFT ??
        first_transfer[6] = 0xD0; // Magic number. WTF ??
        first_transfer[7] = 0xC8; // Magic number. WTF ??
        first_transfer[8] = END_OF_MESSAGE; // Stop byte
        libusb_interrupt_transfer(
            devh,                 // dev handle
            (0x2 | LIBUSB_ENDPOINT_OUT), // EP
            first_transfer,                        // data
            sizeof(first_transfer),                   // size
            &actual,                        // & bytes sent
            0                      // timeout (ms)
        );
        std::cout << "actual written first_transfer: " << actual << std::endl;
*/
        /*
        unsigned char start_byte[1];
        start_byte[0] = 0x0;
        libusb_interrupt_transfer(
            devh,                 // dev handle
            (0x2 | LIBUSB_ENDPOINT_OUT), // EP
            start_byte,                        // data
            1,                   // size
            &actual,                        // & bytes sent
            0                      // timeout (ms)
        );
        std::cout << "actual written start_bytes: " << actual << std::endl;

        unsigned char test_data[8];
        test_data[0] = 0xFF;
        test_data[1] = 0x0;
        test_data[2] = 0xFF;
        test_data[3] = 0xFF;
        test_data[4] = 0xFF;
        test_data[5] = 0x0;
        test_data[6] = 0x0;
        test_data[7] = 0x0;

        libusb_bulk_transfer(
            devh,                 // dev handle
            (0x2 | LIBUSB_ENDPOINT_OUT), // EP
            test_data,                        // data
            8,                   // size
            &actual,                        // & bytes sent
            0                      // timeout (ms)
        );
        std::cout << "actual written data_bytes: " << actual << std::endl;*/

      }
      libusb_close(devh);
    }

    r = libusb_get_port_numbers(dev, path, sizeof(path));
    if (r > 0) {
      printf(" path: %d", path[0]);
      for (j = 1; j < r; j++)
        printf(".%d", path[j]);
    }
    printf("\n");
  }
}

int main(void)
{
  libusb_device **devs;
  int r;
  ssize_t cnt;

  r = libusb_init(NULL);
  if (r < 0)
    return r;

  cnt = libusb_get_device_list(NULL, &devs);
  if (cnt < 0){
    libusb_exit(NULL);
    return (int) cnt;
  }

  print_devs(devs);
  libusb_free_device_list(devs, 1);

  libusb_exit(NULL);
  return 0;
}





/*
 * Find the interface with the endpoint we're after. Usually this is interface
 * 1 but we check them all just in case.
 */
/*bool LocateInterface(libusb_device *usb_device,
                     int *interface_number) {
  struct libusb_config_descriptor *device_config;
  if (libusb_get_config_descriptor(usb_device, 0, &device_config) != 0) {
    std::cout << "Failed to get device config descriptor" << std::endl;
    return false;
  }

  std::cout << static_cast<int>(device_config->bNumInterfaces)
            << " interfaces found" << std::endl;
  for (unsigned int i = 0; i < device_config->bNumInterfaces; i++) {
    const struct libusb_interface *interface = &device_config->interface[i];
    for (int j = 0; j < interface->num_altsetting; j++) {
      const struct libusb_interface_descriptor *iface_descriptor =
          &interface->altsetting[j];
      for (uint8_t k = 0; k < iface_descriptor->bNumEndpoints; k++) {
        const struct libusb_endpoint_descriptor *endpoint =
            &iface_descriptor->endpoint[k];
        std::cout << "Interface " << i << ", altsetting " << j << ", endpoint "
                  << static_cast<int>(k) << ", endpoint address "
                  << endpoint->bEndpointAddress << std::endl;
        if (endpoint->bEndpointAddress == ENDPOINT) {
          std::cout << "Using interface " << i << std::endl;
          *interface_number = i;
          libusb_free_config_descriptor(device_config);
          return true;
        }
      }
    }
  }
  std::cout << "Failed to locate endpoint for EurolitePro device." << std::endl;
  libusb_free_config_descriptor(device_config);
  return false;
}




#include <ftdi.h>


int main(void)
{

  int ret, i;
  struct ftdi_context *ftdi;
  struct ftdi_device_list *devlist, *curdev;
  char manufacturer[128], description[128];
  int retval = EXIT_SUCCESS;

  if ((ftdi = ftdi_new()) == 0)
  {
    fprintf(stderr, "ftdi_new failed\n");
    return EXIT_FAILURE;
  }

  if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
  {
    fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    retval =  EXIT_FAILURE;
    goto do_deinit;
  }

  printf("Number of FTDI devices found: %d\n", ret);

  i = 0;
  for (curdev = devlist; curdev != NULL; i++)
  {
    printf("Checking device: %d\n", i);
    if ((ret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0)
    {
      fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
      retval = EXIT_FAILURE;
      goto done;
    }
    printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
    curdev = curdev->next;
  }
  done:
  ftdi_list_free(&devlist);
  do_deinit:
  ftdi_free(ftdi);



  libusb_device *dev;
  int interface_numer;
  LocateInterface(dev, &interface_numer);







  return retval;
}
*/
/*

int main(void) {
  int status = 0;
  libusb_context *ctx = nullptr;
  libusb_device *dev;
  libusb_device_handle *devh;
  status = libusb_init(&ctx);
  if (status != LIBUSB_SUCCESS)
    Logger::error("failed to init libusb");

  // list all devices
  libusb_device **devs;
  int k = 0, j = 0;
  libusb_get_device_list(nullptr, &devs);
  while ((dev = devs[k++]) != nullptr) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      Logger::error("failed to read device descriptor");
    }

    Logger::info("idVendor: {}, idProduct: {}", desc.idVendor, desc.idProduct);
    if (desc.idVendor == 0x0403 && desc.idProduct == 0x6001) {
      status = libusb_open(dev, &devh);
      Logger::info("status open: {}", status);
      if (devh) {
        Logger::info("opened DMX interface");


        request.append(START_OF_MESSAGE); // DMX start code (Which constitutes the + 1 below)
        request.append(ENAPLE_API2); // Enable API2
        request.append(char(0x04)); // data length LSB
        request.append(DMX_ZERO); // data length MSB
        request.append(char(0xAD)); // Magic number. WTF ??
        request.append(char(0x88)); // Magic number. WFT ??
        request.append(char(0xD0)); // Magic number. WTF ??
        request.append(char(0xC8)); // Magic number. WTF ??
        request.append(END_OF_MESSAGE); // Stop byte



        break;
        //device_status = DmxDevice::DeviceStatus::STARTED;
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
  status = libusb_claim_interface(devh, 0);
  Logger::info("status of claim_interface: {}", status);
  return status;
}*/
