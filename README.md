    _    ____                  _                        
   / \  / ___| _ __   ___  ___| |_ _ __ _   _ _ __ ___  
  / _ \ \___ \| '_ \ / _ \/ __| __| '__| | | | '_ ` _ \ 
 / ___ \ ___) | |_) |  __/ (__| |_| |  | |_| | | | | | |
/_/   \_\____/| .__/ \___|\___|\__|_|   \__,_|_| |_| |_|
              |_|                                       
                    POWERED BY Linux

  Copyright (c) 2000
  Santiago Romero Iglesias  -  sromero@escomposlinux.org


  WHAT IS THIS PROGRAM FOR
=======================================================================

 ASpectrum Emulator: This is our contribution to the Spectrum World.
 We're trying to release our simple, useable and portable Spectrum
 emulator, always thinking in the GNU/Linux community.


  HOW TO USE THIS PROGRAM
=======================================================================

 By the moment, just run it without arguments to run the 48K spectrum
 ROM, or give it a .sna, .sp, .z80 or .tap filename to load snapshots
 or tap files...

    ./aspectrum file.sna     (or .sp, or .tap, or .z80).

 Remember that loading of .tap files requieres a LOAD "" in the
 BASIC interpreter of the emulator. The LOAD command is put by
 pressing the 'J' key, and the " by pressing ALT+P (like in the
 real Spectrum).

 to a resume of aspectrum command line args use "./aspectrum --help"

 You'll need Allegro 4.x (or 3.9.x WIP) to run this program.
 The font.fnt file MUST be located at the same directory where you
 run the aspectrum binary, or in /usr/share/aspectrum

 spectrum.rom MUST be in the same directory, in /usr/share/aspectrum
 or specified the ROM file by command line args.

 To use Gunstick Emulation you must have a working mouse and call
 the emulator with "./aspectrum -j G" (CASE SENSITIVE), or activate
 the gunstick emulation by Gerenal Optiones menu (F7).

 One time you start the gunstick emulation can avoid the "888"s locating
 the mouse in the border of the screen.

 NOTE THAT THIS EMULATOR IT'S NOT USER-FRIENDLY AS IT'S STILL IN
 FULL DEVELOPMENT STAGE. IT WON'T BE PACKAGED ON BINARY DEB/RPM
 FORMATS AND WON'T HAVE INSTALLATION OPTIONS WHILE DEVELOPING IT.

 To compile the debug version you'll need to install MSS (Memory
 Supervision System) that generates .log files reporting memory
 management (pointer errors, allocations, etc.).


  ABOUT THIS PROGRAM
=======================================================================

 o Currently it just emulates the 48K spectrum, until I fix all
   the wrong Z80 opcodes (bugs, bugs, bugs :) and until all the
   undocumented opcodes are implemented. Then I'll add support for
   128K spectrums.

 o There are not implemented opcodes (although almost all games work,
   so they are not used very often) and ___at least one bug___ in the
   Z80 emulation that makes some games to fail (see WORKING file).

 o I finally added a simple GUI that I'll update soon with a simple
   file selection interface...

 o The emulator has a built-in debugger (very simple) that I use to
   search bugs (I usually comparte the output of my emulation with
   the real spectrum or another emulators).

 o It can load .SNA, .SP and .Z80 snapshot files.

 o It can load .TAP files at "zero" speed (I've still not implemented
   the real-time loading routines), by using LOAD "" on the emulator.
   It still does not load protected games nor games launched by
   non-standard loaders. LOAD "" is writen pressing J and ALT+P and ALT+P.
   To delete on the spectrum keyboard use SHIFT+0.

 o I've tested the emulator with more than 100 games, and almost
   all were working right. Read the WORKING file to know which
   games have been tested.

 o All the coding style, documentation, etc. sucks :).



 LICENSE
=======================================================================
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 (...Read the file License on the emulator package...)



  THANKS TO...
=======================================================================

 o Marat Fayzullin, Ra�l G�mez, Philip Kendall and Ignacio Burgue�o:
   These people helped me a lot with the emulator: he explained me about
   Z80 emulation, pointed me to the right documentation, and so on.
   Ignacio tested my emulator against their own emulator to tell me
   which opcodes were not correctly emulated... to all THANKS A LOT.

   All these people have their own Z80 or Spectrum emulator, look for
   them on any search engine:

     Marat Fayzullin : z80 emulator (Z80 microprocessor).
     Ra�l G�mez: R80 emulator (Spectrum).
     Philip Kendall: Fuse emulator (Spectrum).
     Ignacio Burgue�o: GLECK emulator (Spectrum).
     Z80user, Kak-, Hylian, Kel: From #spectrum on IRC-Hispano.

 o Alvaro Alea, for submitting the first Aspectrum patch giving support
   to load Z80 snapshot files on the emulator.

 o Gin�s Flores, for trying to help me on the emulator.

 o Jose Manuel Claros, for the help on all the spectrum stuff.

 o Pedro Gimeno, for creating his Spectrum emulator (and for the readme!).

 o Grzegorz Adam Hankiewicz, for helping me on learning Allegro...
   and of course all the Allegro Development Team, for creating such
   a nice programming library. Read the allegro's AUTHORS file for
   all the people who has contributed to Shawn Hargreaves' initial Allegro.


  Santiago Romero AkA NoP/Compiler.
  sromero@escomposlinux.org
  