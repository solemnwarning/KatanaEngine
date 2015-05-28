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

#include "EngineBase.h"

#include "EngineVideoMaterial.h"
#include "EngineEditor.h"
#include "EngineVideo.h"

Material_t *mActiveMaterial = NULL; // This is the material we're currently editing.
Material_t *mFloorMaterial;			// Material used for the floor plane.
model_t *mPreviewModel = NULL;		// Model we're previewing material on.
MD2_t *mPreviewMesh = NULL;			// Mesh we're previewing the material on.

bool bMaterialEditorInitialized = false;

void MaterialEditor_Initialize(void)
{
	if (bMaterialEditorInitialized)
		return;

	CL_Disconnect();
	Host_ShutdownServer(false);

	key_dest = KEY_EDITOR_MATERIAL;

	mFloorMaterial = Material_Load("engine/grid");
	if (!mFloorMaterial)
	{
		Con_Warning("Failed to load grid material!\n");
		return;
	}

	mPreviewModel = Mod_ForName("models/placeholders/cube.md2");
	if (!mPreviewModel)
	{
		Con_Warning("Failed to load preview mesh!\n");
		return;
	}

	mPreviewMesh = (MD2_t*)Mod_Extradata(mPreviewModel);

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

#define	MATERIALEDITOR_FLOOR_SCALE	10.0f

void MaterialEditor_Draw(void)
{
	VideoObjectVertex_t	voGrid[4] = { 0 };

	MathVector3f_t mvPosition = { 0, 0, -5 };
	Draw_Grid(mvPosition, 3);

	Video_ResetCapabilities(false);

#if 0
	glPushMatrix();
	glTranslatef(0, -2.0f, 0);
	Alias_DrawGenericFrame(mPreviewMesh, mPreviewModel, mActiveMaterial, 0);
	glPopMatrix();
#endif

	glPushMatrix();
	glTranslatef(10.0f, 0, 0);
	glRotatef(-90.0f, 0, 1, 0);
	glRotatef(-90.0f, 0, 0, 1);
	Video_SetBlend(VIDEO_BLEND_IGNORE, VIDEO_DEPTH_FALSE);
	Video_SetColour(1.0f, 1.0f, 1.0f, 1.0f);
	Video_ObjectVertex(&voGrid[0], -5, 5, 0);
	Video_ObjectTexture(&voGrid[0], VIDEO_TEXTURE_DIFFUSE, 0, 0);
	Video_ObjectVertex(&voGrid[1], 5, 5, 0);
	Video_ObjectTexture(&voGrid[1], VIDEO_TEXTURE_DIFFUSE, 1.0f, 0);
	Video_ObjectVertex(&voGrid[2], 5, -5, 0);
	Video_ObjectTexture(&voGrid[2], VIDEO_TEXTURE_DIFFUSE, 1.0f, 1.0f);
	Video_ObjectVertex(&voGrid[3], -5, -5, 0);
	Video_ObjectTexture(&voGrid[3], VIDEO_TEXTURE_DIFFUSE, 0, 1.0f);
	Video_DrawFill(voGrid, mActiveMaterial);
	glPopMatrix();

	Video_ResetCapabilities(true);

	GL_SetCanvas(CANVAS_DEFAULT);
	Draw_String(10, 10, va("Camera: origin(%i %i %i), angles(%i %i %i)",
		(int)r_refdef.vieworg[pX],
		(int)r_refdef.vieworg[pY],
		(int)r_refdef.vieworg[pZ],
		(int)r_refdef.viewangles[pX],
		(int)r_refdef.viewangles[pY],
		(int)r_refdef.viewangles[pZ]));
}

void MaterialEditor_Display(Material_t *mDisplayMaterial)
{
	if (!bMaterialEditorInitialized)
	{
		Con_Warning("The editor hasn't been initialized!\n");
		return;
	}

	if (mActiveMaterial)
		Material_Clear(mActiveMaterial);

	mActiveMaterial = mDisplayMaterial;
	if (!mActiveMaterial)
	{
		Con_Warning("Failed to display material!");
		return;
	}

	mPreviewModel->mAssignedMaterials = mActiveMaterial;
}

void MaterialEditor_Shutdown(void)
{}