iwgtk(1)

# NAME

iwgtk - Wireless network management GUI

# SYNOPSIS

*iwgtk* [_options_]

# OPTIONS

*-i, --indicators*
	Launch indicator daemon

*-n, --notifications*
	Enable desktop notifications (default)

*-N, --no-notifications*
	Disable desktop notifications

*-h, --help*
	Show command line options and quit

*-V, --version*
	Print version number and quit

# DESCRIPTION

iwgtk is a graphical utility for managing wireless network connections via iwd.
Supported functionality is similar to that of iwctl.

# CONFIGURATION

Icon colors can be customized by editing the application's config file. The
system-wide config file is located at */etc/iwgtk.conf*. This file can be copied
to *~/.config/iwgtk.conf* if desired, in which case the system-wide config file
will be ignored. For further instructions, please refer to the comments in
*iwgtk.conf*.

# REPORTING BUGS

Report bugs using the issue tracker on Github
<https://github.com/J-Lentz/iwgtk/issues>.

# COPYRIGHT

Copyright 2020-2022 Jesse Lentz <jesse@twosheds.org> and contributors

iwgtk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iwgtk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iwgtk.  If not, see <https://www.gnu.org/licenses/>.
