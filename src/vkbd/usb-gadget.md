# USB HID gadget

Linux devices with host and either USB OTG or device ports can be used as USB
to USB converter boards, with the keyboard connected to the USB host port and
the PC to the USB OTG or device port.

Under this kind of setup, the Linux USB HID gadget driver can be used to emulate
a HID device and `keyd` can be configured to translate evdev input events to
HID reports.


# Installation

    git clone https://github.com/rvaiya/keyd
    cd keyd
    make VKBD=usb-gadget && sudo make install VKBD=usb-gadget
    sudo systemctl enable keyd-usb-gadget && sudo systemctl start keyd-usb-gadget
    sudo systemctl enable keyd && sudo systemctl start keyd

The device should show up as` 1d6b:0104 Linux Foundation Multifunction Composite Gadget`
on the host machine. This can be observed on a Linux host by checking the output of
`lsof` or the existence of `/dev/input/by-id/Tux_USB_Gadget_Keyboard`.


