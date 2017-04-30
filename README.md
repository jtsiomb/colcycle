colcycle - color cycling image viewer
=====================================

About
-----
This is a small program which can be used to view the excellent color cycling
pictures of Mark J. Ferrari (http://markferrari.com), showcased in this
web-based viewer by Joseph Huckaby: http://www.effectgames.com/demos/canvascycle

I did not have access to the original LBM files, so I've written a parser for the
converted JSON images used in the `canvascycle` web viewer. Use the included
`download_images` script to grab all of them directly from there (uses `wget`).
Alternatively, to download them manually, follow instructions in the wiki:
https://github.com/jtsiomb/colcycle/wiki

Git version is now able to load a subset of LBM images which happens to contain
Mark Ferrari's LBM files available here:
http://www.randelshofer.ch/animations/anims_ibm/mark_j_ferrari/thu_mark_j_ferrari.html

Download
--------
Latest release (1.3):
https://github.com/jtsiomb/colcycle/releases/download/v1.3/colcycle-1.3.tar.gz

Source (git repo): git@github.com:jtsiomb/colcycle.git

License
-------
Copyright (C) 2016-2017 John Tsiombikas <nuclear@member.fsf.org>

This program is free software; feel free to use it, modify and/or redistribute
it, under the terms of the GNU General Public License version 3, or at your
option, any later version published by the Free Software Foundation. See COPYING
for details.

Build instructions
------------------
On DOS you need the Watcom C compiler (OpenWatcom should do, although I've only
tested Watcom 11.0c). Copy DOS4GW.EXE to the project directory, and type `wmake`
to build.

On UNIX you have the option of building with the OpenGL backend, or the SDL
backend:

 * On modern computers with properly installed GPU drivers, the OpenGL backend
   should be significantly faster, which is why it's the default build option
   when you just type `make`. You may need to install GLUT, and GLEW first if
   you don't already have them.

 * If you're on an older computer without a modern graphics card, make sure you
   have SDL 1.2 installed, and type `make BACKEND=sdl`.

To cross-compile the windows version, set the CC variable to point to the mingw
cross-compiler when invoking make, like so: `make CC=i686-w64-mingw32-gcc`.
While there is currently no provision for building directly on windows, that
can be done with minimal modifications to the makefile.

Run instructions
----------------
Just pass the image you want to view as a command-line argument to `colcycle`.

If you pass a directory name as an argument, the program will cycle through all
images in that directory, showing each one for a few seconds.

For the OpenGL or the SDL version, you may set the environment variable
`FULLSCREEN` to 1, if you wish to run fullscreen. Also hitting 'f' while the
program runs toggles fullscreen mode.

FAQ
---
1. Why DOS?

  The whole point of all this was to use Mark Ferrari's awesome color cycling
  images as a screensaver on my dos machine. The GL/SDL backend is just there
  for ease of development, so that I can write most of the code on GNU/Linux.

2. Where can I find `DOS4GW.EXE`?

  If you got the source from git, there won't be any binaries inside. Get the
  release tarball of `colcycle`, which contains all the necessary binaries, and
  copy `dos4gw.exe` from there. Alternatively, pretty much every 90s PC game
  has it.

3. Why don't you include any actual images with the code?

  I don't own the rights to Mark Ferrari's artwork, so I can't redistirbute it.
  Use the `download_images` script to get them directly from the `canvascycle`
  website, or download them manually: https://github.com/jtsiomb/colcycle/wiki
