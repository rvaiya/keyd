#!/usr/bin/python3

# Copyright © 2019 Raheman Vaiya.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

import selectors
import fcntl
import glob
import time
import struct
import os
from ctypes import *
import keys
import sys
import signal
import random
import re


class VirtualKeyboard():
    def __init__(self, name, product_id=0x9234, vendor_id=0x567a):
        EV_SYN = 0x00
        EV_KEY = 0x01
        UI_SET_EVBIT = 0x40045564
        UI_SET_KEYBIT = 0x40045565
        UI_DEV_SETUP = 0x405c5503
        UI_DEV_CREATE = 0x5501

        BUS_USB = 0x03
        version = 0

        self.uinp = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
        fcntl.ioctl(self.uinp, UI_SET_EVBIT, EV_KEY)
        fcntl.ioctl(self.uinp, UI_SET_EVBIT, EV_SYN)

        for _, key in keys.names.items():
            if not keys.is_mouse_button(key):
                fcntl.ioctl(self.uinp, UI_SET_KEYBIT, key.code)

        setup_struct = struct.pack('HHHH80bI',
                                   BUS_USB,
                                   vendor_id,
                                   product_id,
                                   version,
                                   *([ord(c) for c in name] +
                                     ([0] * (80 - len(name)))),
                                   0)

        fcntl.ioctl(self.uinp, UI_DEV_SETUP, setup_struct)
        fcntl.ioctl(self.uinp, UI_DEV_CREATE)

        # Kludge to give the new device some time to propagate up the
        # input stack
        time.sleep(.3)

    def write_code(self, code, pressed):
        EV_KEY = 0x01
        EV_SYN = 0x00

        b = struct.pack("llHHi", 0, 0, EV_KEY, code, pressed)
        os.write(self.uinp, b)
        b = struct.pack("llHHi", 0, 0, EV_SYN, 0, 0)

        os.write(self.uinp, b)

    def send_key(self, name, pressed):
        code = 0

        if name in keys.names:
            code = keys.names[name].code
        elif name in keys.alt_names:
            code = keys.alt_names[name].code
        else:
            raise Exception(f'Could not find corresponding key for \"{name}\"')

        self.write_code(code, pressed)

    def send_string(self, s):
        for c in s:
            shifted = False
            code = 0

            if c in keys.names:
                code = keys.names[c].code
            elif c in keys.alt_names:
                code = keys.alt_names[c].code
            else:
                code = keys.shifted_names[c].code
                shifted = True

            if shifted:
                self.write_code(keys.names["shift"].code, 1)

            self.write_code(code, 1)
            self.write_code(code, 0)

            if shifted:
                self.write_code(keys.names["shift"].code, 0)


class KeyStream():
    def grab(self):
        EVIOCGRAB = 0x40044590
        fcntl.ioctl(self.fh, EVIOCGRAB, 1)

    def ungrab(self):
        EVIOCGRAB = 0x40044590
        fcntl.ioctl(self.fh, EVIOCGRAB, 0)

    def get_name(self, fh):
        EVIOCGNAME = 0x81004506
        buf = bytes(256)

        name = fcntl.ioctl(fh, EVIOCGNAME, buf)
        return c_char_p(name).value.decode('utf8')

    def get_ids(self, fh):
        EVIOCGID = 0x80084502
        buf = bytes(8)

        resp = fcntl.ioctl(fh, EVIOCGID, buf)

        (_, vendor, product, _) = struct.unpack("HHHH", resp)
        return (product, vendor)

    def __init__(self, product=0x00, vendor=0x09, name=""):
        self.fh = None
        for f in glob.glob("/dev/input/event*"):
            fh = open(f, 'rb')
            fh.devname = self.get_name(fh)
            p, v = self.get_ids(fh)

            if (p == product and v == vendor) or (name != "" and fh.devname == name):
                self.fh = fh

        if not self.fh:
            raise Exception(
                'Could not find keyboard with id %04x:%04x' % (vendor, product))

    # Collect all events currently sitting on the input stream.
    def collect(self):
        EV_KEY = 0x01

        events = []

        flags = fcntl.fcntl(self.fh, fcntl.F_GETFL)
        fcntl.fcntl(self.fh, fcntl.F_SETFL, flags | os.O_NONBLOCK)

        while True:
            ev = self.fh.read(24)
            if not ev:
                fcntl.fcntl(self.fh, fcntl.F_SETFL, flags & ~os.O_NONBLOCK)
                return events

            _, _, type, code, value = struct.unpack("llhhi", ev)

            if type == EV_KEY:
                key = keys.codes[code]

                events.append((key, value))

    # Block until the next event
    def next(self):
        EV_KEY = 0x01

        while True:
            ev = self.fh.read(24)
            _, _, type, code, value = struct.unpack("llhhi", ev)

            if type == EV_KEY:
                key = keys.codes[code]

                return key, value


output = ''


def on_timeout(a, b):
    print('ERROR: test timed out')
    exit(-1)


# If we don't terminate within 5 seconds something has gone
# horribly wrong...
signal.signal(signal.SIGALRM, on_timeout)
signal.alarm(20)

vkbd = VirtualKeyboard('test keyboard', vendor_id=0x2fac, product_id=0x2ade)
stream = KeyStream(name="keyd virtual keyboard")

# Grab the virtual keyboard so we don't
# cause pandemonium.

stream.grab()

exit_on_fail = False


def diff(output, expected):
    n = max(len(expected), len(output))

    s = ''
    for i in range(n):
        e = expected[i] if i < len(expected) else ""
        o = output[i] if i < len(output) else ""

        if e != o:
            s += '\x1b[33m%-20s\x1b[0m \x1b[31m%s\x1b[0m\n' % (e, o)
        else:
            s += '%-20s %s\n' % (e, o)

    return s


class TestElement:
    def __init__(self, type, code, val):
        self.type = type
        self.code = code
        self.value = val


# Busy wait to minimize imprecision
# (sleep() is inaccurate).
def sleep(ms):
    us = ms * 1000
    start = time.time()

    while True:
        if ((time.time() - start) * 1E6) >= us:
            return


def run_test(name, input, output, verbose):
    def printerr(s):
        print(f'{name}: \x1b[31mERROR\x1b[0m: {s}')

        if verbose:
            sys.stdout.write('Input:\n%s\n\n%-20s %s\n%s' %
                             (input, "Expected Output:", "Output:", diff(result, expected)))

    elements = []

    for line in input.strip().split('\n'):
        line = line.strip()
        try:
            timeout = int(re.match('^([0-9]+)ms$', line).group(1))
            elements.append(TestElement('timeout', 0, timeout))
            continue
        except:
            pass

        key, state = line.split(' ')
        depress = 0

        if state == "down":
            depress = 1

        code = 0
        if key in keys.names:
            code = keys.names[key].code
        else:
            code = keys.alt_names[key].code

        elements.append(TestElement('code', code, depress))

    # Actually run the test, keep this separate from parsing to minimize
    # latency. The system may still preempt the thread causing spurious time
    # dependent test failures. There isn't much that can be done to mitigate
    # this :/.

    for e in elements:
        if e.type == 'timeout':
            sleep(e.value)
            continue
        else:
            vkbd.write_code(e.code, e.value)

    expected = output.strip().split('\n')
    result = []

    time.sleep(0.00003)
    results = stream.collect()

    # Try again, timeout may have been insufficient.
    if len(results) != len(expected):
        print('WARNING: Insufficient output, timing out one more time...')
        time.sleep(.05)
        results += stream.collect()

    for k, v in results:
        result.append(f'{k} {"up" if v == 0 else "down"}')

    if len(result) > len(expected):
        printerr('Extraneous keys.')
        return False

    if len(result) < len(expected):
        printerr('Missing keys.')
        return False

    for i in range(len(expected)):
        if result[i] != expected[i]:
            printerr(
                f'mismatch: expected \033[33m{expected[i]}\033[0m got \033[31m{result[i]}\033[0m.')
            return False

    print(f'{name}: \x1b[33mPASSED\x1b[0m')
    return True


import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-v', '--verbose', default=False, action='store_true')
parser.add_argument('-e', '--exit-on-fail', default=False, action='store_true')
parser.add_argument('files', nargs=argparse.REMAINDER)
args = parser.parse_args()


# Prevent gc from interfering with
# timeout precision.
import gc
import os

gc.collect()
gc.disable()
os.nice(-20)

tests = []
failed = False

for file in args.files:
    name = file
    input, output = open(file, 'r').read().split('\n\n')

    tests.append((name, input, output))

    if not run_test(name, input, output, args.verbose):
        if args.exit_on_fail:
            exit(-1)

        failed = True

if failed:
    exit(-1)

#tests = tests * 1000
