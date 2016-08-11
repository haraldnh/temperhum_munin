# TemperHUM Munin plugin

This is a simple stripped-down version of the main program from
https://github.com/lp0/temperhum.git

Instead of having a server and creating rrd files, it is a one-shot
measurement producing output suitable to be a munin plugin.

To use, link it up as a plugin, and add a section to the plugin
configuration like:

    [temperhum_munin]
    env.device 1-1.2.3

See the temperhum docs for further details.

Note that this code is for the TemperHUM versions with a usb->serial
converter, not the HID versions.
