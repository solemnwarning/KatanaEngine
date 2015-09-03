/*	Copyright (C) 2011-2015 OldTimes Software

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

#include "EngineBase.h"

#include "EngineEditor.h"
#include "EngineVideo.h"

bool bMaterialEditorInitialized = false;

void MaterialEditor_Initialize(void)
{
	if (bMaterialEditorInitialized)
		return;

	CL_Disconnect();
	Host_ShutdownServer(false);

	cls.state = CLIENT_STATE_EDITOR;

	key_dest = KEY_EDITOR_MATERIAL;

	bMaterialEditorInitialized = true;
}

void MaterialEditor_Input(int iKey)
{
	switch (iKey)
	{
	case K_DOWNARROW:
		break;
	}
}
