colcycle - color cycling image viewer
=====================================

About
-----
This is a small program which can be used to view the excellent color cycling
pictures of Mark J. Ferrari (http://markferrari.com), showcased in this
web-based viewer by Joseph Huckaby: http://www.effectgames.com/demos/canvascycle

I do not have access to the original LBM files, so I've written a parser of the
converted JSON images which can be retrieved from the canvascycle website. Use
the included `download_images` script to grab all of them easily.

License
-------
Copyright (C) 2016 John Tsiombikas <nuclear@member.fsf.org>

This program is free software; feel free to use it, modify and/or redistribute
it, under the terms of the GNU General Public License version 3, or at your
option, any later version published by the Free Software Foundation. See COPYING
for details.

Build instructions
------------------
On UNIX, make sure you have SDL 1.2 installed, and type `make`.

On DOS you need the Watcom C compiler (OpenWatcom should do, although I've only
tested Watcom 11.0c). Copy DOS4GW.EXE to the project directory, and type `wmake`
to build.

Run instructions
----------------
Just pass the image you want to view as a command-line argument to `colcycle`.
If you pass a directory name as an argument, the program will cycle through all
images in that directory, showing each one for a few seconds.

For the SDL version, you may set the environment variable `FULLSCREEN` to 0, if
you with to run in a window.
