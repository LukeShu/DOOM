DOOM
====

This repository contains my various DOOM source code projects.  I'd
have created separate repos for some of them, but GitHub doesn't let
you have multiple forks of the same project in one account/org.  So,
instead projects are organized by branch name.

I mostly don't use Git tags for reconstructed historical commits; as I
might update them, and `git fetch` mostly assumes that tags are
immutable.

Historical reconstruction
-------------------------

 - [`linuxxdoom/original-release`]: The original 1997 source code
   release, under the non-free DOOM Source License (DSL).  This is
   1-commit after what is at https://github.com/id-Software/DOOM/
   because the contents of the original tarballs weren't quite
   faithfully checked in; see the commit message for details.

 - [`linuxxdoom/gpl-release`]: The 1999 re-licensing to the GNU
   General Public License (GPL).  See the commit message for
   historical details (Thanks to Andrew Stine for answering my Twitter
   DMs!).  If you're going to start a new project based off of the
   original source code, you absolutely should start with this
   version, for the licensing reasions.

 - [`frosted-doom/choco`] (plus the `frosted-doom/merge` and
   `chocolate-doom/base` tags), The now-popular [doomgeneric] is based
   off of [fbDOOM], which is based off of [Frosted Doom], which is
   based off of a combination of the original source release,
   [Chocolate Doom] v2.1.0 and the then-in-progress v2.2.0.  How those
   3 versions were smashed together wasn't checked in to Git...  until
   now!  (Thanks to Maxime Vincent for answering my emails about
   tracking down the correct Chocolate Doom versions!)  You can see
   the full reconstructed history of Frosted Doom / fbDOOM /
   doomgeneric by fetching the branches and tags, and running these
   two commands:

   ```shell
	git replace --graft chocolate-doom/base linuxxdoom/original-release
	git replace --graft frosted-doom/merge frosted-doom/merge^ frosted-doom/choco
	```

[`linuxxdoom/original-release`]: https://github.com/LukeShu/DOOM/tree/linuxxdoom/original-release
[`linuxxdoom/gpl-release`]: https://github.com/LukeShu/DOOM/tree/linuxxdoom/gpl-release
[`frosted-doom/choco`]: https://github.com/LukeShu/DOOM/tree/frosted-doom/choco
[doomgeneric]: https://github.com/ozkl/doomgeneric
[fbDOOM]: https://github.com/maximevince/fbDOOM
[Frosted Doom]: https://github.com/insane-adding-machines/DOOM
[Chocolate Doom]: https://github.com/chocolate-doom/chocolate-doom

My own development, of historical interest
------------------------------------------

 - [`linuxxdoom/heirloom`]: Keeping the `linuxdoom/gpl-release`
   building and running on modern GNU/Linux systems.

[`linuxxdoom/heirloom`]: https://github.com/LukeShu/DOOM/tree/linuxxdoom/heirloom

My own development, of modern interest
--------------------------------------

 - `doomgeneric/*`: In-progress or experimental contributions to
   https://github.com/ozkl/doomgeneric
