![3dengfx](http://nuclear.mutantstargoat.com/sw/3dengfx/logo.png)
=======

![shots](http://nuclear.mutantstargoat.com/sw/3dengfx/shots/shotbar.png)

The original README at the end of 3dengfx development had the single line:

    TODO: create a new readme for release 0.5

Before that there was the release 0.1 README file, which is still available in
the repository. You can find it by going back to the `oldsvn` tag. The rest of
this file was written many years after the end of this project's development.

3dengfx was a 3D engine I started hacking on, right after I decided to move away
from Windows and my previous DirectX 8 engine, to GNU/Linux and OpenGL, back in
the summer of 2003. Soon after, samurai (Michael Georgoulopoulos) joined the
project, and we continued to improve and use 3dengfx in demoscene productions
until roughly the spring of 2007. The last production using 3dengfx was
Mindflush / MLFC, at which point the code had become crufty, full of bugs from
half-finished half-baked features, and very cumbersome, and we decided to stop.

Originally 3dengfx was hosted on my home computer, running a subversion server.
A couple of years later the project was moved to BerliOS, which was the only
free software code hosting service supporting subversion at that point.
Sourceforge, which was the more mainstream alternative only provided CVS repos.

Unfortunately BerliOS shut down in 2011, taking with it all the commit history
of 3dengfx after the move. That's why this repository has a detailed commit
history up to about mid-2004, which was the last commit in my own repo (tag:
`oldsvn`), and two bulk commits after that. The first one being a copy of
3dengfx as it was in 2007 in the Mindflush source tree (tag: `mindflush`), and
the next one being the last state of the berlios repository working copy as it
was on my disk, but augmented with a few minor fixes from the Mindflush source,
which were apparently never merged back (tag: `lastsvn`).

This git repository was created for archival reasons. No further development is
going to happen in this project. But there might be a few fixes here and there,
as I intend to resurect some of my old demos, and I'll probably have to fix a
few things to even manage to build them.

License
-------
Copyright (C) 2003-2007 John Tsiombikas <nuclear@member.fsf.org>

3dengfx is free software. Feel free to use, modify and/or redistribute it under
the terms of the GNU General Public License version 3, or at your option, any
later version published by the Free Software Foundation. See COPYING for
details.
