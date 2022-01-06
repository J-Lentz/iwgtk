## About
iwgtk is a lightweight, graphical wifi management utility for Linux. It is used to control
iwd, with supported functionality similar to that of iwctl. It is particularly useful in
a system where iwd is being used as a standalone network management daemon (i.e., without
NetworkManager).

![Screenshot](screenshot/station.png)

## Usage
* Launch application window: `iwgtk`
* Run in the background with indicator icons: `iwgtk -i`

## Dependencies
* iwd
* gtk3

## Installation
To build iwgtk and install it to /usr/local, run:

```
make
sudo make install
make clean
```

To install to /usr instead of /usr/local use:
```
sudo make prefix=/usr install
```

## Troubleshooting

### The indicator icon doesn't show up
iwgtk's icon should show up on any system tray which supports the StatusNotifierItem API.
If your tray only supports the older XEmbed API, then a compatibility layer such as
[snixembed](https://git.sr.ht/~steef/snixembed) is required.

The following trays support StatusNotifierItem:
* KDE Plasma
* swaybar
* xfce4-panel (*must be built with the optional libdbusmenu-gtk3 dependency*)

The following trays only support XEmbed, and require a compatibility layer:
* AwesomeWM
* i3bar

## License
Copyright 2020-2022 Jesse Lentz <jesselnz@gmail.com> and contributors (see below)

iwgtk is licensed under the GPL version 3 or later.

All icons, including the application icon, tray icons, and icons within the GUI, are from
the "wifi states" collection by i cons from the Noun Project. These icons are licensed
under the Creative Commons BY license.
<https://thenounproject.com/iconsguru/collection/wifi-states/>

## Contributors
* Jove Yu
* Jaron Viëtor (Thulinma)
* tinywrkb
* Érico Nogueira Rolim
* VaguePenguin
* Andrew Benson
* Alex Piechowski (grepsedawk)
