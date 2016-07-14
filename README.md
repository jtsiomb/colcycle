colcycle - color cycling image viewer
=====================================

About
-----
This is a small program which can be used to view the excellent color cycling
pictures of Mark J. Ferrari (http://markferrari.com), showcased in this
web-based viewer by Joseph Huckaby: http://www.effectgames.com/demos/canvascycle

I do not have access to the original LBM files, so I've written a parser for the
converted JSON images used in the `canvascycle` web viewer. Use the included
`download_images` script to grab all of them directly from there (uses `wget`).

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

FAQ
---
1. Why DOS?

The whole point of all this was to use Mark Ferrari's awesome color cycling
images as a screensaver on my dos machine. The SDL backend is just there for
ease of development, so that I can write most of the code on GNU/Linux.

2. Where can I find `DOS4GW.EXE`?

If you got the source from git, there won't be any binaries inside. Get the release
tarball of `colcycle`, which contains all the necessary binaries, and copy
`dos4gw.exe` from there. Alternatively, pretty much every 90s PC game has it.

3. Why don't you include any actual images with the code?

I don't own the rights to Mark Ferrari's artwork, so I can't redistirbute it.
Use the `download_images` script to get them directly from the canvascycle
website.

4. Why don't you support LBM images?

I don't have any. Feel free to send me any good ones to tempt me to write an
LBM loader.
