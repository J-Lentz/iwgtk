## About
iwgtk is a lightweight, graphical wifi management utility for Linux. Its only
dependencies are iwd and GTK3. Supported functionality is similar to that of
iwctl.

## Usage
iwgtk is used to control iwd via its D-Bus API. It is particularly useful in a
system where iwd is being used as a standalone network management daemon (i.e.,
without NetworkManager).

## Installation
To build the iwgtk binary and install it to /usr/local/bin, run:

```
make
sudo make install
make clean
```

To install to /usr/bin instead of /usr/local/bin, use:
```
sudo make prefix=/usr install
```

## License
Copyright 2020 Jesse Lentz <jesselnz@gmail.com>

iwgtk is licensed under the GPL version 3 or later.

The iwgtk application icon, network icons, and signal strength icons are from
the "wifi states" collection by i cons from the Noun Project. These icons are
licensed under the Creative Commons BY license.
<https://thenounproject.com/iconsguru/collection/wifi-states/>
