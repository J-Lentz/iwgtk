## About
iwgtk is a lightweight, graphical wifi management utility for Linux. It controls
iwd, with supported functionality similar to that of iwctl. It is particularly
useful in a system where iwd is used as a standalone network management daemon
(i.e., without NetworkManager).

![Screenshot](screenshot/iwgtk-station-mode.png)

## Usage
Launch the application window: `iwgtk`

Launch the indicator daemon: `iwgtk -i`

### Autostarting
The most common use case for iwgtk is to start the indicator daemon every time
you log into your desktop. If your desktop environment supports the XDG
autostart standard, this should happen automatically due to the
`iwgtk-indicator.desktop` file which is placed in `/etc/xdg/autostart/` during
installation.

A systemd unit file to start the indicator daemon is also provided. If your
distro uses systemd and your desktop environment supports systemd's
`graphical-session.target` unit, then iwgtk can be started at the beginning of
every desktop session by enabling the `iwgtk.service` unit.

### Configuration
Icon colors can be customized by editing the application's config file. The
system-wide config file is located at `/etc/iwgtk.conf`. This file can be copied
to `~/.config/iwgtk.conf` if desired, in which case the system-wide config file
will be ignored. For further instructions, please refer to the comments in
`iwgtk.conf`.

## Dependencies
* iwd (>=1.28)
* gtk4 (>= 4.6)
* libqrencode
* adwaita-icon-theme (or an equivalent icon package)

## Installation
To build iwgtk and install it to /usr/local, run:

```
make
sudo make install
make clean
```

To install to /usr instead of /usr/local use:
```
sudo make PREFIX=/usr install
```

## Troubleshooting

### The indicator icon doesn't show up
iwgtk's icon should show up on any system tray which supports the
StatusNotifierItem API. If your tray only supports the older XEmbed API, then a
compatibility layer such as [snixembed](https://git.sr.ht/~steef/snixembed) is
required.

The following trays support StatusNotifierItem:
* KDE Plasma
* swaybar
* xfce4-panel (*must be built with the optional libdbusmenu-gtk3 dependency*)

The following trays only support XEmbed, and require a compatibility layer:
* AwesomeWM
* i3bar

### iwgtk and iwctl only work with superuser privileges
As of iwd 1.23, membership in either the `netdev` or `wheel` group is required
to control iwd:
```
# usermod -a -G netdev YOUR_USER_ACCOUNT
```
If no `netdev` group exists on your system, then you'll need to create it prior
to running the above `usermod` command:
```
# groupadd netdev
```

## License
Copyright 2020-2022 Jesse Lentz <jesse@twosheds.org> and contributors (see below)

iwgtk is licensed under the GPL version 3 or later.

The application icon is from the [wifi
states](https://thenounproject.com/iconsguru/collection/wifi-states/) collection
by [i cons](https://thenounproject.com/iconsguru/) from the Noun Project. This
icon is licensed under the [Creative Commons BY
license](https://creativecommons.org/licenses/by/3.0/us/legalcode).

## Contributors
* Jove Yu
* Jaron Viëtor (Thulinma)
* tinywrkb
* Érico Nogueira Rolim
* VaguePenguin
* Andrew Benson
* Alex Piechowski (grepsedawk)
