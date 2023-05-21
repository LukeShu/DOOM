# heirloom linuxxdoom v1.10 - The original open source DOOM

<!--
  Copyright (C) 2023 by Luke Shumaker <lukeshu@lukeshu.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
-->

In 1994, id Software's Dave Taylor released two ports of DOOM v1.8 to
the then-new Linux kernel, which had just hit "1.0" earlier that year.
One port was `linuxsdoom`, which used SVGAlib; and the other was
`linuxxdoom`, which used X11.

In 1997, Bernd Kreimeier was given a 1997-01-10 snapshot of the DOOM
source code, which contained the code for the MS-DOS version,
linuxsdoom, and linuxxdoom (and maybe the NeXTSTEP version?).  That
snapshot does not precisely correspond to any released version of
DOOM.  He stripped out parts that id wasn't allowed to or declined to
share, stripped out the versions other than linuxxdoom, cleaned it up,
got it working again (bit-rot is a terrible thing).  Bernd Kreimeier's
modified linuxxdoom sources were then published by id on 1997-12-23,
with a README from id's John Carmack.

That 1997 Bernd-Kreimeier-modified source release is the basis for all
DOOM source ports (... until 2009 when Bethesda bought id, and
subsequently started releasing "official" but inferior ports based on
pre-open-source code).

heirloom linuxxdoom is a project to keep that original source release
from bit-rotting, while remaining as faithful as possible to that
original source release; out of historical interest.  It does not aim
to be faithful to any historical released binary; see [Chocolate
Doom][] if you are interested in that.

## Limitations

 - It does not support the mouse, it's keyboard-only (TODO: linuxxdoom
   v1.8 didn't support the mouse, but there's code there for the
   mouse.  Did it never work, or is this bit-rot?)

 - There is no music (but sound effects do work).  (Neither linuxxdoom
   nor linuxsdoom ever had music.)

 - It only supports OSS audio; rather than modern ALSA/PipeWire; you
   will need to enable OSS.

 - It only supports 8-bit PseudoColor X11; while modern versions of
   the Xorg server still support 8-bit PseudoColor, they can't enable
   it and modern 24-bit TrueColor at the same time.

 - It only supports x86-32 (TODO: Get it compiling for other
   architectures.  The change in `int` sizes is bit-rot.)

 - Demos only play if the WAD file version matches the executable
   version.  The latest official id WAD files, as well as most
   community WAD files, have version `109` (1.9), while linuxxdoom
   1.10 naturally has version `110`, so demos will generally not play.

 - It only supports rendering at 320x200, 640x400, or 960x600; it
   cannot go larger than that, and it cannot render at fractions.

 - DOOM's graphics are designed for non-square pixels (tall 5:6
   pixels); your modern display probably has square pixels; we make no
   attempt to stretch the image or otherwise compensate for this.

heirloom linuxxdoom has no intention to add music, move away from OSS,
move away from 8-bit PseudoColor X11, fuss with `VERSION`, et c.; See
the [TODO: link-here] project if you are interested in any of those
things.  Adding/fixing mouse support is dependent on why it's not
working.

## Compiling

If you are on x86-64, you will need do install the normal dev tools
(Make, GCC, ...), as well as the 32-bit versions of the libraries that
DOOM needs:

 - Arch Linux and derivatives: `sudo pacman -S --needed base-devel lib32-libxext lib43-libnsl`

If you are on a non-x86 platform, you won't be able to build this
without the help of an x86 emulator.

Once the dependencies are installed, simply run `make` in the
`linuxdoom-1.0/` and `sndserv/` directories:

    make -C linuxdoom-1.10/ && make -C sndserv/

The two executables that it builds are
`./linuxdoom-1.10/linux/linuxxdoom` and `./sndserv/linux/sndserver`.

## Installing

 - You will likely want to put the `linuxxdoom` executable somewhere
   in your `$PATH`.

 - I wish I had a good place to tell you to put the `sndserver`
   executable, but it needs to be in either the current-directory when
   you run `linuxxdoom`, or in the directory pointed to by your
   `$DOOMWADDIR` environment variable; adding it to your `$PATH` does
   NOT work.  Or, if you don't care about sound effects, you can just
   ignore the `sndserver`

 - Similarly, in either the current-directory or the `$DOOMWADDIR`
   directory, you will need a WAD file, with one of the following
   filenames:

   | filename       | what that filename usually is                  | notes                              |
   |----------------|------------------------------------------------|------------------------------------|
   | `doom1.wad`    | "DOOM" Shareware version (1993)                | just the 1st episode               |
   | `doom.wad`     | "DOOM" (1993)                                  | all 3 episodes                     |
   | `doomu.wad`    | "The Ultimate DOOM" (1995)                     | the 3 episodes, plus a 4th episode |
   | `plutonia.wad` | "Final DOOM: The Plutonia Experiment" (1996)   | a standalone episode               |
   | `tnt.wad`      | "Final DOOM: TNT: Evilution" (1996)            | a standalone episode               |
   | `doom2.wad`    | "DOOM II: Hell on Earth" (1994)                |                                    |
   | `doom2f.wad`   | "DOOM II: Hell on Earth" French version (1994) |                                    |

   Of course, any of those may be a community-created/modified WAD,
   rather than what it "usually" is.

 - Assuming that you don't want to turn off 24-bit color, you'll need
   a nested X server to run with 8-bit color, such as Xephyr:

   + Arch Linux and derivatives: `sudo pacman -S --needed xorg-server-xephyr`

## Running

 - You'll need to ensure that OSS audio is enabled:

        sudo modprobe snd_pcm_oss

 - You'll need to start that nested X server:

        Xephyr :1 -screen 640x400x8 &

   That `:1` should be set to an available display number; your main X
   display is probably `:0`, so `:1` is probably available.

   For the resolution, you can choose any of the supported
   resolutions: `320x200`, `640x400`, or `960x600`; but you *must*
   have the `x8` at the end to indicate 8-bit color.

 - Either `cd` to, or set `DOOMWADDIR` to the directory where you
   placed `sndserver` and the WAD file(s).

 - Set `DISPLAY` to whatever you chose for Xephyr above, and run the
   `linuxxdoom` executable; see [`./README.linuxx-1.8.txt`][] for the
   various options when running it.  By default it will run at
   320x200; to run at 640x400, give `-2` as an argument; to run at
   960x600, give `-3` as an argument.

        DISPLAY=:1 linuxxdoom -2

## The original documentation

Dave Taylor's Linux DOOM:

 - [`./README.linuxs-1.8.txt`][]: Dave Taylor's README for linuxsdoom
   v1.8.

 - [`./README.linuxx-1.8.txt`][]: Dave Taylor's README for linuxxdoom
   v1.8 (see below for a description of ways this file is
   out-of-date).

 - [`./README.dos-1.6.txt`][]: The README for DOS DOOM v1.6, which was
   included with linuxsdoom and linuxxdoom v1.8, and covers details
   that are not specific to Linux DOOM.


Bernd Kreimeier's source release:

 - [`./README.carmack.txt`][]: John Carmack's notes on the source release.

 - Bernd Kreimeier's notes on the state of the code, and the changes
   he made:

   + [`./linuxdoom-1.10/README.b`][]: His main README.
   + [`./linuxdoom-1.10/README.book`][]: The book he was writing, and
     how the source-release came to be.
   + [`./linuxdoom-1.10/README.sound`][]: Describes how sound works.
   + [`./linuxdoom-1.10/README.asm`][]: Some assembly routines from
     the DOS version of DOOM.
   + [`./linuxdoom-1.10/README.gl`][]: Notes on what would need to be
     done to port DOOM to OpenGL.
   + [`./sndserv/README.sndserv`][]: Basic introduction of the sound
     server.


## Differences from the linuxxdoom v1.8 README to v1.10

Surely the state of bugs and such between v1.8 and v1.10 has changed
(TODO: document them), but what follows are notes about things in
README.linuxx-1.8.txt that are not true for v1.10 (were they even true
for 1.8?).

 - The X server supporting the MIT-SHM extension is no longer
   required; it will be used if it does support it, but will
   gracefully fall back if the server doesn't support it.

 - The `sndserver` binary doesn't need to be in your path, instead it
   needs to be in either the current directory or the `$DOOMWADDIR`
   directory.

## License

In 1999, id Software authorized Andrew Stine to re-license the source
release to the GNU General Public License (GPL), and so that is the
license that heirloom linuxxdoom is under.

Unless the copyright statement in a specific file is edited, the
heirloom linuxxdoom project does not consider its patches to be
sufficiently creative to qualify for copyright, and are in the public
domain.  Feel free to cherry-pick them in to your own DOOM project
without the legal need for attribution (but attribution would be
appreciated).

[Chocolate Doom]: https://github.com/chocolate-doom/chocolate-doom
[`./README.linuxs-1.8.txt`]: ./README.linuxs-1.8.txt
[`./README.linuxx-1.8.txt`]: ./README.linuxx-1.8.txt
[`./README.dos-1.6.txt`]: ./README.dos-1.6.txt
[`./README.carmack.txt`]: ./README.carmack.txt
[`./linuxdoom-1.10/README.b`]: ./linuxdoom-1.10/README.b
[`./linuxdoom-1.10/README.book`]: ./linuxdoom-1.10/README.book
[`./linuxdoom-1.10/README.sound`]: ./linuxdoom-1.10/README.sound
[`./linuxdoom-1.10/README.asm`]: ./linuxdoom-1.10/README.asm
[`./linuxdoom-1.10/README.gl`]: ./linuxdoom-1.10/README.gl
[`./sndserv/README.sndserv`]: ./sndserv/README.sndserv
