/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

/**
 * Simple header to print colored/bold text etc., to ANSI terminals.
 *
 * Modified from https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
 * */

#ifndef STAMINA_ANSI_COLORS_H
#define STAMINA_ANSI_COLORS_H

// Windows by default does not support ANSI
#ifdef WINDOWS
	#define IGNORE_COLORS
#endif // WINDOWS

/* FOREGROUND */
#ifndef IGNORE_COLORS
	#define RST  "\x1B[0m"
	#define KRED "\x1B[31m"
	#define KGRN "\x1B[32m"
	#define KYEL "\x1B[33m"
	#define KBLU "\x1B[34m"
	#define KMAG "\x1B[35m"
	#define KCYN "\x1B[36m"
	#define KWHT "\x1B[37m"
#else // IGNORE_COLORS defined
	#define RST
	#define KRED
	#define KGRN
	#define KYEL
	#define KBLU
	#define KMAG
	#define KCYN
	#define KWHT
#endif // IGNORE_COLORS

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#ifndef IGNORE_COLORS
	#define BOLD(x) "\x1B[1m" x RST
	#define UNDL(x) "\x1B[4m" x RST
#else // IGNORE_COLORS defined
	#define BOLD(x)
	#define UNDL(x)
#endif // IGNORE_COLORS

#endif // STAMINA_ANSI_COLORS_H
