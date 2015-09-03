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

#ifndef __SHAREDENGINE__
#define __SHAREDENGINE__

#include "platform.h"

#include "SharedFlags.h"
#include "SharedMaterial.h"
#include "shared_client.h"

//johnfitz -- stuff for 2d drawing control
typedef enum
{
	CANVAS_NONE,
	CANVAS_DEFAULT,
	CANVAS_CONSOLE,
	CANVAS_MENU,
	CANVAS_SBAR,
	CANVAS_WARPIMAGE,
	CANVAS_CROSSHAIR,
	CANVAS_BOTTOMLEFT,
	CANVAS_BOTTOMRIGHT,
	CANVAS_TOPRIGHT
} VideoCanvasType_t;

/*	Functions exported from the engine.
*/
typedef struct
{
	bool(*Initialize)(int argc, char *argv[], bool bEmbedded);	// Initializes the engine.
	bool(*IsRunning)();

	char*(*GetBasePath)();		// Gets the currently active game path.
	char*(*GetMaterialPath)();	// Gets the set material path.
	char*(*GetVersion)();		// Return the engine version.

	void(*Loop)();										// Main loop.
	void(*SetViewportSize)(int iWidth, int iHeight);
	void(*Shutdown)();									// Shutdown.

	Material_t*(*LoadMaterial)(const char *cPath);
	void(*UnloadMaterial)(Material_t *mMaterial);

	model_t*(*LoadModel)(const char *ModelPath);

	// Client...

	double(*GetClientTime)();

	ClientEntity_t*(*CreateClientEntity)();	// Creates a "temp" client entity.

	DynamicLight_t*(*CreateDynamicLight)(int Key);

	// Console...

	void(*InsertConsoleCommand)(const char *cCommand);										// Sends the given command to the console.
	void(*RegisterConsoleVariable)(ConsoleVariable_t *cvVariable, void(*Function)(void));	// Register a new console variable.
	void(*SetConsoleVariable)(char *cVariableName, char *cValue);							// Set the value of an existing console variable.
	void(*ResetConsoleVariable)(char *cVariableName);										// Resets the value of a console variable.
	void(*Print)(char *cMessage, ...);														// Prints a message to the console.
	void(*PrintDev)(char *cMessage, ...);													// Prints a developer message to the console.

	// Video...

	void(*Video_PreFrame)();
	void(*Video_PostFrame)();

	void(*DrawConsole)();
	void(*DrawGradientBackground)();
	void(*DrawFPS)();
	void(*DrawString)(int x, int y, char *cMsg);
	void(*DrawLine)(MathVector3f_t mvStart, MathVector3f_t mvEnd);
	void(*DrawMaterialSurface)(Material_t *mMaterial, int iSkin, int x, int y, int w, int h, float fAlpha);
	void(*DrawEntity)(ClientEntity_t *Entity);
	void(*DrawSetCanvas)(VideoCanvasType_t Canvas);
	void(*DrawResetCanvas)();

	// Material Editor
	void(*MaterialEditorInitialize)(void);					// Initializes the material editor.

	int	iVersion;
} EngineExport_t;

/*	Functions imported by the engine.
*/
typedef struct
{
	void(*PrintMessage)(char *text);
	void(*PrintWarning)(char *text);
	void(*PrintError)(char *text);

	int	iVersion;
} EngineImport_t;

#define ENGINE_VERSION	(sizeof(EngineImport_t)+sizeof(EngineExport_t))

#endif
