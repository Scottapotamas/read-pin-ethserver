# Easily Query hardware pin state over a network

This is a really simple sketch which allows HTTP GET requests to quickly grab pin high/low state.

This is intended for HiL test supervision, where the CI/CD host requests a hardware state change as part of the test suite, then checks the pin was actually set to the correct state.

## Hardware

This is written against the Arduino bundled networking library, so any board which supports that implementation should work.

I used an [Adafruit Feather 32u4](https://www.adafruit.com/product/2771) and [Adafruit Feather Ethernet](https://www.adafruit.com/product/3201) board as I had them on hand already.

## Usage

In the `read-pin-ethserver.ino`, the `request_io pin_get_strings[]` array describes the input string with the Arduino pin number. Add to this array to provide access to more pins. In the same way, ADC readings or internal variables could also be queried with minimal changes to the code.

Inbound GET requests are checked against this set. If the request is a valid string, `ON` or `OFF` are returned, otherwise `ERROR` is returned.

To get the state in a linux environment, I used the following `wget` command. This can be used inside a bash script to set a variable. 

`wget -qO- 192.168.1.125/osx`

Browsing to `192.168.1.125/osx` should return the ASCII string as well.

### Networking

I use DHCP to assign the IP address to the board, allowing deterministic use of the device. Advertisement with Zeroconf could make life easier, but wasn't needed here.