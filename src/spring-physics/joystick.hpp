
#include <hidapi.h>
#include <iostream>

class Joystick {
public:
  Joystick(unsigned short vendor_id, unsigned short product_id) {
    // Initialize the hidapi library
    hid_init();

    // Open the device using the VID, PID, and optionally the Serial number.
    device = hid_open(vendor_id, product_id, NULL);

    if (device == NULL) {
      std::cout << "Unable to open device" << std::endl;
      return;
    }

    // Make device non-blocking
    hid_set_nonblocking(device, 1);
  }

  ~Joystick() {
    // Close the HID device
    hid_close(device);

    // Clean up the hidapi library
    hid_exit();
  }

  void read() {
    if(!device) {
        return;
    }
    // Read input data from the joystick
    while(hid_read(device, buf, sizeof(buf)) > 0) {}
  }

  bool ready() {
      return device != NULL;
  }

  unsigned char* get_buffer() {
    return buf;
  }

private:
  hid_device *device;
  unsigned char buf[256];
};