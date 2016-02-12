#ifndef USB_ONLINE_MODE_H
#define USB_ONLINE_MODE_H

#include "../constants.h"
#include <libusb.h>

#ifdef BUILDING_ON_MAC
  #include "../libraries/hidapi/hidapi/hidapi.h"
  /**
   * Utilize hidapi on Mac builds to pull data from Inkling
   */

  void mac_hidapi_listener(hid_device *handle, char* ipAddress, int port);

#else
  #include <linux/input.h>
  #include <linux/uinput.h>

  /**
   * Utilize libusb on linux builds to pull data from Inkling
   */
  void linux_libusb_listener(libusb_device_handle *found, char* ipAddress, int port);

#endif

/**
 * This function initializes the "online mode" of the device.
 */
void usb_online_mode_init();

/**
 * This function initializes sending "online mode" OSC data from of the device.
 */

void usb_network_mode_init(char* ipAddress, int port);



/**
 * This function stops the "online mode" of the device.
 */
void usb_online_mode_exit ();

/**
* This function stops broadcasting OSC data from the device
*/
void usb_network_mode_exit ();


void kill_osc(char* s);

#endif//USB_ONLINE_MODE_H
