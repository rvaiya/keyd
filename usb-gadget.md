# USB HID gadget

Linux devices with host and either USB OTG or device port can be used as USB to USB converter boards, with keyboard connected to USB host port and PC to USB OTG or device port.

In that kind of setup Linux USB HID gadget driver can be used to emulate HID device and `keyd` can be configured to translate evdev input events to HID reports.


# Installation

    sudo apt-get install libudev-dev # Debian specific, install the corresponding package on your distribution

    git clone https://github.com/rvaiya/keyd
    cd keyd
    make vkbd-usb-gadget && sudo make install && sudo make install-usb-gadget
    sudo systemctl enable usb-gadget && sudo systemctl start usb-gadget
    sudo systemctl enable keyd && sudo systemctl start keyd

Device should show up on `lsusb` list as `1d6b:0104 Linux Foundation Multifunction Composite Gadget`.
One can also see it in `/dev/input/by-id/` under `Tux_USB_Gadget_Keyboard` name.
