/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef CMD
#undef CMD
#endif	/* defined(CMD) */

#ifdef NEED_COMMAND_STRINGS
#define CMD(command)	#command
#else
#define CMD(command)	CMD_##command
#endif	/* weird hack */

/* Here is the place to add/remove commands accepted by the demosystem */
#define COMMANDS	\
	CMD(START_PART),\
	CMD(END_PART),\
	CMD(RENAME_PART),\
	CMD(SET_RTARGET),\
	CMD(SET_CLEAR),\
	CMD(END),\
	CMD(FX)
