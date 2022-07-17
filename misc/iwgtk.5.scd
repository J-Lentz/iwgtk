iwgtk(5)

# NAME

iwgtk - Configuration file

# LOCATION

The system-wide configuration file is located at *$(sysconfdir)/iwgtk.conf*.
*$(sysconfdir)* is usually either */etc* or */usr/local/etc*, depending on your
build-time *prefix* setting. *iwgtk.conf* can be copied to
*~/.config/iwgtk.conf* if user-specific configuration is desired. If a user
configuration file exists, then the system-wide configuration file will be
ignored.

# SYNTAX

The configuration file uses the "key file" syntax, which consists of groups of
key-value pairs. Blank lines are ignored, and lines beginning with a "#" are
considered comments.

Groups are started by a header line containing the group name enclosed in square
brackets. A group is ended implicitly by the start of the next group or by the
end of the file. Every option must be contained in a group. If a group contains
the same option key multiple times, the last occurrence wins. Unrecognized group
or option keys are silently ignored. Any option which is not explicitly
specified will take on its default value. Group names, option keys, and values
are all case-sensitive.

Options have the form key=value. Spaces before or after the '=' character are
ignored. Newline, tab, carriage return and backslash characters in values must
be escaped as \\n, \\t, \\r, and \\\\, respectively. To preserve leading spaces
in values, these may be escaped as \\s.

Possible option types include strings, integers, and booleans. Boolean options
must be specified as either "true" or "false". Although colors are technically
specified as strings, they are regarded herein as a distinct type.

# COLOR VALUES

Color values may consist of:
- A standard name (Taken from the CSS specification).
- A hexadecimal value in the form "#rgb", "#rrggbb", "#rrrgggbbb" or
  "#rrrrggggbbbb"
- A hexadecimal value in the form "#rgba", "#rrggbbaa", or "#rrrrggggbbbbaaaa"
- A RGB color in the form "rgb(r,g,b)" (In this case the color will have full
  opacity)
- A RGBA color in the form "rgba(r,g,b,a)"

Where "r", "g", "b", and "a" are respectively the red, green, blue and alpha
color values. In the last two cases, "r", "g", and "b" are either integers in
the range 0 to 255 or percentage values in the range 0% to 100%, and "a" is a
floating point value in the range 0 to 1.

See: https://docs.gtk.org/gdk4/method.RGBA.parse.html

List of standard color names:++
https://developer.mozilla.org/en-US/docs/Web/CSS/color_value/color_keywords

# OPTIONS

## [indicator.colors.station]

Indicator icon colors: station mode

[- *Option*
:- *Type*
:- *Default value*
|[ connected
:[ color
:[ lime
|  connecting
:  color
:  yellow
|  disconnected
:  color
:  gray

## [indicator.colors.ap]

Indicator icon colors: AP mode

[- *Option*
:- *Type*
:- *Default value*
|[ up
:[ color
:[ lime
|  down
:  color
:  gray

## [indicator.colors.adhoc]

Indicator icon colors: ad-hoc mode

[- *Option*
:- *Type*
:- *Default value*
|[ up
:[ color
:[ lime
|  down
:  color
:  gray

## [indicator.colors.disabled]

Indicator icon colors: disabled device or adapter

[- *Option*
:- *Type*
:- *Default value*
|[ device
:[ color
:[ gray
|  adapter
:  color
:  gray

## [network.colors]

Available network list - Signal strength icon colors

[- *Option*
:- *Type*
:- *Default value*
|[ connected
:[ color
:[ lime
|  connecting
:  color
:  yellow
|  known
:  color
:  darkgreen
|  unknown
:  color
:  gray
|  hidden
:  color
:  gray

## [known-network]

Known network list - last connection date/time - format string

[- *Option*
:- *Type*
:- *Default value*
|[ last-connection-time.format
:[ string
:[ %x\\n%l:%M %p

## [theme]

Theme - enable dark mode

[- *Option*
:- *Type*
:- *Default value*
|[ dark
:[ boolean
:[ false

# SEE ALSO

_iwgtk_(1)
