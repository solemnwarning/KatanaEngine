/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2015 OldTimes Software

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

#ifndef __ENGINEVIDEOSHADER__
#define __ENGINEVIDEOSHADER__

void VideoShader_Initialize(void);
void VideoShader_Enable(void);
void VideoShader_Disable(void);
void VideoShader_SetVariablei(const char *name, int i);
void VideoShader_SetVariablef(const char *name, float f);
void VideoShader_SetVariable3f(const char *name, float x, float y, float z);
void VideoShader_SetVariable4f(const char *name, float x, float y, float z, float a);

#endif