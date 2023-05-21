This is Linux X DOOM v1.8.  DOOM is a nifty 3D game originally
released for NEXTSTEP and then DOS.  Read the attached DOS README
for more.

It was compiled under Linux v1.1.64.  You need Hannu's 3.0ish sound
driver if you want sound.

This executable should also work with the registered DOOM and DOOM
II wad (v1.8).  Be a pal and don't pirate doom.wad and doom2.wad
it if you'd be so kind.

Included [README.dos-1.6.txt] is the stock README file which came with DOS
DOOM v1.6.  If you don't know jack about DOOM, you might want to
read it.  It mostly applies except where it doesn't.

I did this 'cause Linux gives me a woody.  It doesn't generate
revenue.  Please don't call or write us with bug reports.  They
cost us money, and I get sorta ragged on for wasting my time on
UNIX ports anyway.

There are two executables, linuxxdoom and sndserver.  They both
need to be in your path.  If sndserver isn't, you won't get sound.
This is good if you don't have a 16-bit stereo sound card.
The data file doom1.wad, which can be had from ftp.uwp.edu, must
either be in the current directory or must be in the directory
pointed to by the environment variable DOOMWADDIR.  The default.cfg
file is now in ~/.doomrc.  It'll be created the first time you run
DOOM.  Wouldn't bother messing with it except for chat macros if
you like to use them.

CTRL fires, SHIFT makes you go fast, ALT lets you sidle, and the
arrow keys move you around.

There are a few new/different options.  To play a net game, you do
this: "linuxxdoom -net <myplayernumber> <otherguyhostname ...>".  So
for a three person game between the machines huey, dewey, and louie,
you might type:

huey> linuxxdoom -net 1 dewey louie
dewey> linuxxdoom -net 2 huey louie
louie> linuxxdoom -net 3 huey dewey

It's a slightly queer syntax, but the number lets you be a color
not determined by your machine address which is kinda nice.

You can specify a different port number to use by saying "-port
<num>".

Another new option is inflating the screen size.  You'll quickly
discover that 320x200 is just REAL small on 1280x1024 screens.
You can use -2, -3, or -4 to double, triple, or quadruple the window
size.  This turns out to be a pretty bonehead thing to do in Linux.
You can resize the X server resolution.  I understand there's now
a 320x200 mode.  I'd recommend it.  The -2,-3,-4 options were broken
before (an endianness thing).  -2 and -3 are fixed.  I still say
you're a bonehead for using them.

fvwm is an unwise thing to have running if the CTRL key fires.
You'll see why after a few minutes.

This game uses the MITSHM extension to X.  It also uses Sys V shared
memory.  I'm not terribly expert with the latter, so if stale shared
memory starts to accumulate on your system, reboot I guess, and my
apologies in advance.  I think I wrote enough code to reclaim the
stuff if it sees it lying around.  Quitting gracefully will drastically
reduce the chance of bad things happening.

Thanks to Joel at InfoMagic for sending me extra nifty Linux CDROM's.
Special thanks to Hannu Savolainen who with almost painful coolness
added a nifty feature to allow real-time sound to work.  It made
this possible and has helped me write the Jag DOOM sound code.
I'm horribly grateful for that.  And of course, horribly gruesome
dripping thanks to Linus Torvalds for giving us one hell of an
operating system!

	=-ddt->

PS.  This is straight out of the source code.  You can put these
values in the .doomrc to remap your keys.  For keys not on here,
it's generally the lowercase ASCII value.

//
// most key data are simple ascii (uppercased)
//
#define KEY_RIGHTARROW          0xae
#define KEY_LEFTARROW           0xac
#define KEY_UPARROW                     0xad
#define KEY_DOWNARROW           0xaf
#define KEY_ESCAPE                      27
#define KEY_ENTER                       13
#define KEY_TAB                         9
#define KEY_F1                          (0x80+0x3b)
#define KEY_F2                          (0x80+0x3c)
#define KEY_F3                          (0x80+0x3d)
#define KEY_F4                          (0x80+0x3e)
#define KEY_F5                          (0x80+0x3f)
#define KEY_F6                          (0x80+0x40)
#define KEY_F7                          (0x80+0x41)
#define KEY_F8                          (0x80+0x42)
#define KEY_F9                          (0x80+0x43)
#define KEY_F10                         (0x80+0x44)
#define KEY_F11                         (0x80+0x57)
#define KEY_F12                         (0x80+0x58)

#define KEY_BACKSPACE           127
#define KEY_PAUSE                       0xff

#define KEY_EQUALS                      0x3d
#define KEY_MINUS                       0x2d

#define KEY_RSHIFT                      (0x80+0x36)
#define KEY_RCTRL                       (0x80+0x1d)
#define KEY_RALT                        (0x80+0x38)

#define KEY_LALT                        KEY_RALT
