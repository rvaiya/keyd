#!/bin/bash
modprobe libcomposite
cd /sys/kernel/config/usb_gadget || exit
mkdir g1
cd g1
mkdir configs/c.1
mkdir functions/hid.usb0
echo 1 > functions/hid.usb0/protocol
echo 1 > functions/hid.usb0/subclass
echo 8 > functions/hid.usb0/report_length
echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > functions/hid.usb0/report_desc
mkdir strings/0x409
mkdir configs/c.1/strings/0x409
echo 0x0100 > bcdDevice
echo 0x0200 > bcdUSB
echo 0x00 > bDeviceClass
echo 0x00 > bDeviceProtocol
echo 0x00 > bDeviceSubClass
echo 0x08 > bMaxPacketSize0
echo 0x0104 > idProduct
echo 0x1d6b > idVendor
echo "0123456789" > strings/0x409/serialnumber
echo "Tux" > strings/0x409/manufacturer
echo "USB Gadget Keyboard" > strings/0x409/product
echo "Conf 1" > configs/c.1/strings/0x409/configuration
echo 0x80 > configs/c.1/bmAttributes
echo 500 > configs/c.1/MaxPower
ln -s functions/hid.usb0 configs/c.1
ls /sys/class/udc > UDC
