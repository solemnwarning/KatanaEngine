/*	Copyright (C) 2011-2016 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef PLATFORMSTRING_H
#define	PLATFORMSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

	char *p_strcpy(char *destination, const char *source);
	char *p_strncpy(char *destination, const char *source, size_t num);

	// TODO: These need to be implemented...
#define p_strlen(str) strlen(str)

#ifdef __cplusplus
}
#endif

#endif // !PLATFORMSTRING_H