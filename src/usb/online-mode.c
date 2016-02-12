#include "online-mode.h"
#include "../datatypes/inklingOSC.h"
#include "../osc_broadcaster/osc_broadcaster.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

//Client UDP
#include <arpa/inet.h>
#include <sys/socket.h>

/*----------------------------------------------------------------------------.
 | READ ME                                                                    |
 | -------------------------------------------------------------------------  |
 | In the code that follows there are a few key parts that send undocumented  |
 | packets to the Inkling device. The packets have been obtained by sniffing  |
 | the USB packets from the official Wacom Inkling SketchManager. The reason  |
 | They are undocumented is because I don't know why certain values are sent  |
 | to the device. I tried to document the code with as much information as I  |
 | have.                                                                      |
 '----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------.
 | USB SETTINGS                                                               |
 | -------------------------------------------------------------------------- |
 |                                                                            |
 | Vendor ID:        056a                                                     |
 | Product ID:       0221                                                     |
 | Configuration:    1                                                        |
 | Interface:        0                                                        |
 | Alternate:        0                                                        |
 | bEndpointAddress: 0x83                                                     |
 '----------------------------------------------------------------------------*/

int
usb_online_mode_mouse_enable_feature (int virtual_mouse_desc, int bit, int feature)
{
  if (ioctl (virtual_mouse_desc, bit, feature) < 0)
    {
      perror ("ioctl");
      return 0;
    }

  return 1;  
}


void usb_online_mode_init()
{
  usb_network_mode_init(NULL, 0);
}

void
usb_network_mode_init (char* ipAddress, int port)
{
    const int wacomVendorID = 0x056a;
    const int wacomInclingID = 0x0221;

    #ifdef BUILDING_ON_MAC
      //Find device using hidapi
      int res;
      unsigned char buf[65];
      #define MAX_STR 255
      wchar_t wstr[MAX_STR];
      hid_device *found = NULL;
      int i;
 
      // Enumerate and print the HID devices on the system
      struct hid_device_info *devs, *cur_dev;
        
      devs = hid_enumerate(0x0, 0x0);
      cur_dev = devs;
      while (cur_dev) {
          if(cur_dev->vendor_id == wacomVendorID && cur_dev->product_id == wacomInclingID){
            found = hid_open(cur_dev->vendor_id, cur_dev->product_id, NULL);
            if(found == NULL)
              puts("Failed to open hid to Inkling");
            break;
          }
          cur_dev = cur_dev->next;
      }
      hid_free_enumeration(devs);
    
      if(found){
        mac_hidapi_listener(found, ipAddress, port);
      }else{
        puts("Inkling device not found");
      }
    #else
      //FIND DEVICE USING libusb
      if (libusb_init (NULL))
        puts ("Initializing libUSB failed.");

      // Traverse the list of connected USB devices. We are only interested in the
      // Inkling which has the vendor ID "0x056a" and the product ID "0x0221".
      libusb_device **list;
      libusb_device *found = NULL;
      ssize_t device_count = libusb_get_device_list (NULL, &list);
      ssize_t iterator = 0;
      int error = 0;
      libusb_device *device;
      if (device_count < 0)
        puts ("Cannot get the USB device list. Did you plug in your Inkling?");

      for (iterator = 0; iterator < device_count; iterator++)
        {
          device = list[iterator];
          struct libusb_device_descriptor descriptor;
          libusb_get_device_descriptor (device, &descriptor);
          if (descriptor.idVendor == wacomVendorID && descriptor.idProduct == wacomInclingID)
          {
            found = device;
            break;
          }
          device = NULL;
        }

        if (found)
        {
        
          libusb_device_handle* handle;
          error = libusb_open (found, &handle);
          if (error || handle == NULL)
          {
            puts ("Opening the USB device failed. You might need super user "
              "permissions.");
            goto free_device_list;
          }
          
          linux_libusb_listener(handle, ipAddress, port);
        }else{
          puts("Inkling device not found");
        }
    
        free_device_list:
          libusb_free_device_list (list, 1);
      #endif


}


#ifndef BUILDING_ON_MAC
void linux_libusb_listener(libusb_device_handle *handle, char* ipAddress, int port)
{


    // Reset the device so we have a predictable state.
    libusb_reset_device (handle);

    // Check whether the kernel has assigned a driver to it.
    if (libusb_kernel_driver_active (handle, 0))
    {
      printf ("Detaching kernel driver..\t");
      (libusb_detach_kernel_driver (handle, 0) == 0)
        ? puts ("OK")
        : puts ("FAILED");
    }

    // Set the one and only configuration on the device (to be sure).
    libusb_set_configuration (handle, 1);



    // Claim the HID interface.
    printf("claim_interface: %i\n", libusb_claim_interface(handle,0));
    if (!libusb_claim_interface (handle, 0) == 0)
      puts ("Failed to claim HID interface.");

    // Some kind of handshaking.
    // Values obtained by sniffing the USB connection between SketchManager and the device.
    unsigned char usb_data[33];
    memset (&usb_data, '\0', 33);
    memcpy (&usb_data, "\x80\x01\x03\x01\x02\x00\x00\x00", 8);

    int bytes = 0;
    bytes += libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);

    memcpy (&usb_data, "\x80\x01\x0a\x01\x01\x0b\x01\x00", 8);
    bytes += libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);

    memset (&usb_data, '\0', 33);
    bytes += libusb_control_transfer (handle, 0xa1, 1, 0x0380, 0, usb_data, 33, 0);

    memcpy (&usb_data, "\x80\x01\x0b\x01\x00\x00\x00\x00", 8);
    bytes += libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);

    memcpy (&usb_data, "\x80\x01\x02\x01\x01\x00\x00\x00", 8);
    bytes += libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);

    memcpy (&usb_data, "\x80\x01\x0a\x01\x01\x02\x01\x00", 8);
    bytes += libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);

    memset (&usb_data, '\0', 33);
    bytes += libusb_control_transfer (handle, 0xa1, 1, 0x0380, 0, usb_data, 33, 0);

    // Assume that the incorrect amount of bytes returned indicates a
    // handshake failure.
    if (bytes != 231)
    {
      puts ("Device handshake failed.");
      goto device_release;
    }
    /*----------------------------------------------------------------------.
    | SET UP A VIRTUAL MOUSE DEVICE USING UINPUT.                          |
    '----------------------------------------------------------------------*/

    int virtual_mouse_desc;
    virtual_mouse_desc = open ("/dev/uinput", O_WRONLY | O_NONBLOCK);
      
      
    // Enable the virtual mouse device features we need.
    if (virtual_mouse_desc < 0
      // Key events are the base for the rest.
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_EVBIT, EV_KEY) == 0

      // Enable support for mouse buttons.
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_KEYBIT, BTN_MOUSE) == 0
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_KEYBIT, BTN_LEFT) == 0
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_KEYBIT, BTN_RIGHT) == 0

      // Enable absolute coordinate positioning.
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_EVBIT, EV_ABS) == 0
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_ABSBIT, ABS_X) == 0
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_ABSBIT, ABS_Y) == 0

      // Enable tilt features.
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_ABSBIT, ABS_TILT_X) == 0
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_ABSBIT, ABS_TILT_Y) == 0

      // Enable pressure features.
      || usb_online_mode_mouse_enable_feature (virtual_mouse_desc, UI_SET_ABSBIT, ABS_PRESSURE) == 0)
       
       goto device_release;

    struct uinput_user_dev uidev;
    memset (&uidev, 0, sizeof (uidev));

    snprintf (uidev.name, UINPUT_MAX_NAME_SIZE, "inkling-virtual");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;//0x056a;
    uidev.id.product = 0x1;//0x0221;
    uidev.id.version = 1;

    if (write (virtual_mouse_desc, &uidev, sizeof (uidev)) < 0)
    {
      puts ("Failed to set up the virtual mouse device");
      goto device_release;
    }

    if (ioctl (virtual_mouse_desc, UI_DEV_CREATE) < 0)
    {
      puts ("Failed to register the virtual mouse device");
      goto device_release;
    }

    // Define the device ranges.
    uidev.absmin[ABS_X] = 0;
    uidev.absmax[ABS_X] = 1920;
    uidev.absmin[ABS_Y] = 0;
    uidev.absmax[ABS_Y] = 1920;
    uidev.absmin[ABS_PRESSURE] = 0;
    uidev.absmax[ABS_PRESSURE] = 1024;
    uidev.absmin[ABS_TILT_X] = 0;
    uidev.absmax[ABS_TILT_X] = 255;
    uidev.absmin[ABS_TILT_Y] = 0;
    uidev.absmax[ABS_TILT_Y] = 255;

    int click_state = 0;
    int sockfd = 0;

    bool isNetworkBroadcast = (ipAddress != NULL && port != 0);
      
    if(isNetworkBroadcast){
        struct sockaddr_in remoteAddr;
        sockfd = sizeof(remoteAddr);
        if((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
           puts("killing osc");
           kill_osc("socket");
        }
        
        memset((char *) &remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_addr.s_addr = inet_addr(ipAddress);
        remoteAddr.sin_port = htons(port);

        if(inet_aton(ipAddress, &remoteAddr.sin_addr) == 0)
        {
          fprintf(stderr, "inet_aton() failed\n");
          exit(1);
        }
        connect(sockfd, (struct sockaddr_in *)&remoteAddr, sizeof(remoteAddr));

        int error = 0;
        socklen_t len = sizeof(error);
        int retval = getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

        if(retval != 0){
          printf("%s","error getting socket code");
        }

        if(error != 0){
          printf("%s", "socket error");
        }

        //currently no check at end of while loop for close
    }

    while (1)
    {
      // Allocate some space for receiving data.
      unsigned char data[10];
      memset (&data, '\0', 10);

      int transferred;
      int inter_status;

      // Request coordinate and pressure data.
      inter_status = libusb_interrupt_transfer (handle, 0x83, data, 10, &transferred, 0);
      if (inter_status == LIBUSB_ERROR_NO_DEVICE)
        {
          puts ("Device disconnected.");
          goto device_release;
        }

      // Only process complete packets.
      if (transferred != 10) continue;

      // Only process "Pen" packets.
      if (data[0] != 0x02) continue;

      // The coordinate are segmented in 7 pieces of 1 byte.
      // An extra byte indicates which in which segment the pen is.
      // Thus, the right value can be obtained by this formula:
      // segment * 2^8 + x
      int x = data[2] * 256 + data[1];
      int y = (data[4] * 256 + data[3]) * -1;

      int tilt_x = data[8];
      int tilt_y = data[9];

      // Same formula applies to pressure data.
      // segment * 2^8 + pressure
      // "segment" can vary between 0 and 3, giving us values
      // between 0 and 1024.
      int pressure = data[6];
      pressure = pressure + 256 * data[7];

      if(isNetworkBroadcast){
          dt_inkling_osc oscData;
          oscData.coordinate.x = x;
          oscData.coordinate.y = y;
          oscData.coordinate.pressure = pressure;
          oscData.tilt.x = tilt_x;
          oscData.tilt.y = tilt_y;
          sendInklingOSC(sockfd, oscData);
      }else{
          //printf ("%-5d %-5d %-5d %-5d %-5d %-5d\n", data[5], x, y, pressure, tilt_x, tilt_y);

          // Move the mouse pointer with our virtual mouse device.
          struct input_event ev[5];
          memset (ev, 0, sizeof (ev));

          ev[0].type = EV_ABS;
          ev[0].code = ABS_X;
          ev[0].value = abs (x);
          ev[1].type = EV_ABS;
          ev[1].code = ABS_Y;
          ev[1].value = abs (y);
          ev[2].type = EV_ABS;
          ev[2].code = ABS_PRESSURE;
          ev[2].value = pressure;
          ev[3].type = EV_ABS;
          ev[3].code = ABS_TILT_X;
          ev[3].value = tilt_x;
          ev[4].type = EV_ABS;
          ev[4].code = ABS_TILT_Y;
          ev[4].value = tilt_y;

          if (write (virtual_mouse_desc, ev, sizeof (ev)) < 1)
            puts ("Failed to move the mouse pointer.");

          // Do a mouse click when needed.
          if (pressure > 5)
            {
              click_state = 1;
              struct input_event click;
              click.type = EV_KEY;
              click.code = BTN_LEFT;
              click.value = 1;

              if (write (virtual_mouse_desc, &click, sizeof (click)) < 1)
            puts ("Failed to do a click event.");

            }
          else if (click_state == 1)
            {
              click_state = 0;
              struct input_event click;
              click.type = EV_KEY;
              click.code = BTN_LEFT;
              click.value = 0;

              if (write (virtual_mouse_desc, &click, sizeof (click)) < 1)
            puts ("Failed to do a click event.");
            }
          }
    }
    
    device_release:
      libusb_release_interface (handle, 0);
      libusb_reset_device (handle);
      ioctl (virtual_mouse_desc, UI_DEV_DESTROY);
      close (virtual_mouse_desc);

}
#endif

#ifdef BUILDING_ON_MAC
void mac_hidapi_listener(hid_device *handle, char* ipAddress, int port)
{
    // Some kind of handshaking.
    // Values obtained by sniffing the USB connection between SketchManager and the device.
    unsigned char usb_data[33];
    memset (&usb_data, '\0', 33);
    memcpy (&usb_data, "\x80\x01\x03\x01\x02\x00\x00\x00", 8);

    int bytes = 0;
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memcpy (&usb_data, "\x80\x01\x0a\x01\x01\x0b\x01\x00", 8);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memset (&usb_data, '\0', 33);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memcpy (&usb_data, "\x80\x01\x0b\x01\x00\x00\x00\x00", 8);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memcpy (&usb_data, "\x80\x01\x02\x01\x01\x00\x00\x00", 8);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memcpy (&usb_data, "\x80\x01\x0a\x01\x01\x02\x01\x00", 8);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    memset (&usb_data, '\0', 33);
    bytes += hid_send_feature_report(handle, usb_data, 33);

    // Assume that the incorrect amount of bytes returned indicates a
    // handshake failure.
    if (bytes != 163)
    {
      puts ("Device handshake failed.");
      printf("bytes:%i\n", bytes);
      goto device_release;
    }

    int sockfd = 0;

    bool isNetworkBroadcast = (ipAddress != NULL && port != 0);
      
    if(isNetworkBroadcast){
        struct sockaddr_in remoteAddr;
        sockfd = sizeof(remoteAddr);
        if((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
           puts("killing osc");
           kill_osc("socket");
        }
        
        memset((char *) &remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_addr.s_addr = inet_addr(ipAddress);
        remoteAddr.sin_port = htons(port);

        if(inet_aton(ipAddress, &remoteAddr.sin_addr) == 0)
        {
          fprintf(stderr, "inet_aton() failed\n");
          exit(1);
        }
        //THIS CAST MIGHT NOT BE VALID - If something goes wrong check here
        connect(sockfd, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));

        int error = 0;
        socklen_t len = sizeof(error);
        int retval = getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

        if(retval != 0){
          printf("%s","error getting socket code");
        }

        if(error != 0){
          printf("%s", "socket error");
        }

        //currently no check at end of while loop for close
    }
    
    while (1)
    {
      // Allocate some space for receiving data.
      unsigned char data[10];
      memset (&data, '\0', 10);

      int transferred;
      int inter_status;

      // Request coordinate and pressure data.
      //puts("reading bytes");
      int bytesRead = hid_read_timeout(handle, data, 10, 1000);
      //printf("read:%i\n", bytesRead);
      
      if (bytesRead == -1)
        {
          puts ("Device disconnected.");
          goto device_release;
        }

      // Only process complete packets.
      if (transferred != 10) continue;
      
      // Only process "Pen" packets.
      if (data[0] != 0x02) continue;

      // The coordinate are segmented in 7 pieces of 1 byte.
      // An extra byte indicates which in which segment the pen is.
      // Thus, the right value can be obtained by this formula:
      // segment * 2^8 + x
      int x = data[2] * 256 + data[1];
      int y = (data[4] * 256 + data[3]) * -1;

      int tilt_x = data[8];
      int tilt_y = data[9];

      // Same formula applies to pressure data.
      // segment * 2^8 + pressure
      // "segment" can vary between 0 and 3, giving us values
      // between 0 and 1024.
      int pressure = data[6];
      pressure = pressure + 256 * data[7];

      if(isNetworkBroadcast){
          dt_inkling_osc oscData;
          oscData.coordinate.x = x;
          oscData.coordinate.y = y;
          oscData.coordinate.pressure = pressure;
          oscData.tilt.x = tilt_x;
          oscData.tilt.y = tilt_y;
          sendInklingOSC(sockfd, oscData);
      }else{
          //printf ("%-5d %-5d %-5d %-5d %-5d %-5d\n", data[5], x, y, pressure, tilt_x, tilt_y);
          // Move the mouse pointer with our virtual mouse device.
      }
    }
    
    device_release:
      hid_close(handle);
    
}
#endif

void
usb_online_mode_exit ()
{
  libusb_exit (NULL);
}

void
usb_network_mode_exit ()
{

}

void kill_osc(char* s)
{
  perror(s);
  exit(1);
}
