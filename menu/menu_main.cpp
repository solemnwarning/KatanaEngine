/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "menu_main.h"

/*
	Main menu entry!
	Functions called by the engine can be found here.
*/

// Platform library
#include "platform_module.h"

CMenu	*mGlobal;

/*	TODO:
		#	Get menu elements to be handled like objects.
		#	Let us handle models (for 3D menu interface).
		#	Get basic window environment done, move windows
			if being clicked and mousepos changing, allow
			windows to be resized, let us show a "virtual"
			window within another window, "Katana Menu Scripts",
			allow us to manipulate windows within a 3D space
		#	Allow for texture maps!
		etc
		etc
*/

MenuExport_t	Export;
ModuleImport_t	Engine;

int	iMenuState				= 0;

cvar_t	cvShowMenu = { "menu_show", "1", false, false, "Toggle the display of any menu elements." },
cvDebugMenu = { "menu_debug", "0", false, false, "Toggle the display of any debugging information." };

// [13/8/2013] Fixed ~hogsy
int	iMousePosition[2];

// [2/8/2012] TODO: Why are we doing this!? Should be using the one from the lib ~hogsy
char *va(char *format,...)
{
	va_list		argptr;
	static char	string[1024];

	va_start(argptr,format);
	vsprintf(string,format,argptr);
	va_end(argptr);

	return string;
}

CMenu::CMenu()
{
	uiWidth	= 0;
	uiHeight = 0;
}

CMenu::~CMenu()
{
}

void CMenu::Initialize()
{
	Engine.Con_Printf("Initializing menu...\n");

	// Set the width and height.
	uiWidth = Engine.GetScreenWidth();
	uiHeight = Engine.GetScreenHeight();
}

void Menu_Initialize(void)
{
	mGlobal = new CMenu;
	if (!mGlobal)
		Engine.Sys_Error("Failed to initialize global menu class!\n");

	// Initialize the global menu class.
	mGlobal->Initialize();

	for (int i = 0; i < 10; i++)
		Engine.Client_PrecacheResource(RESOURCE_TEXTURE, va(MENU_HUD_PATH"num%i", i));
}

void Menu_Draw(void)
{
#if 0
	/*	TODO:
			Draw inactive windows last.

		~hogsy
	*/

	if(cvShowMenu.value <= 0)
		return;

	// [17/9/2013] Set the canvas to the default ~hogsy
	Engine.Client_SetMenuCanvas(CANVAS_DEFAULT);

	// [3/8/2012] Find out the current position of our mouse on each frame ~hogsy
	// [3/10/2012] Updated to use what's provided by the engine instead ~hogsy
	Engine.GetCursorPosition(&iMousePosition[pX],&iMousePosition[pY]);

#if 1
	if(iMenuState & MENU_STATE_LOADING)
	{
		// [21/5/2013] TODO: Switch to element ~hogsy
		Engine.DrawPic(MENU_BASE_PATH"loading",1.0f,(iMenuWidth-256)/2,(iMenuHeight-32)/2,256,32);
		return;
	}
	else if(iMenuState & MENU_STATE_PAUSED)
	{
		Engine.DrawPic(MENU_BASE_PATH"paused",1.0f,0,0,32,64);
		return;
	}

	if ((iMenuState & MENU_STATE_HUD) && (!(iMenuState & MENU_STATE_SCOREBOARD) && !(iMenuState & MENU_STATE_MENU)))
	{

	}

	if(iMenuState & MENU_STATE_SCOREBOARD && !(iMenuState & MENU_STATE_MENU))
	{
		// [10/11/2013] TEMP: Added for MP test ~hogsy
		Engine.DrawFill((iMenuWidth/2),(iMenuHeight/2),128,35,0,0,0,0.7f);
		Engine.DrawString(255,10,"Player Score:");
		Engine.DrawString(270,20,va("%d",Engine.Client_GetStat(STAT_FRAGS)));
	}

	if(iMenuState & MENU_STATE_MENU)
	{
		Engine.DrawFill(0,0,iMenuWidth,iMenuHeight,0,0,0,0.8f);

		Engine.Client_SetMenuCanvas(CANVAS_MENU);

		Engine.DrawString(110,80,">");
		Engine.DrawString(190,80,"<");

		switch(iMenuDisplaySelection)
		{
		case MENU_MAIN:
			Engine.DrawString(120,80,"New Game");
			Engine.DrawString(120,90,"Load Game");
			Engine.DrawString(120,100,"Settings");
			Engine.DrawString(120,110,"Quit");
			break;
		case MENU_NEW:
			break;
		case MENU_LOAD:
		case MENU_OPTIONS:
			break;
		case MENU_QUIT:
			// Do we even need to draw anything for this? ~hogsy
			break;
		}
	}
#if 0
	for(i = 0; i < iMenuElements; i++)
	{
		if(mMenuElements[i].bActive)
			return;

		switch((int)mMenuElements[i].mMenuType)
		{
		case MENU_IMAGE:
			Engine.DrawPic(
				mMenuElements[i].cResource,
				mMenuElements[i].fAlpha,
				mMenuElements[i].iPosition[X],
				mMenuElements[i].iPosition[Y],
				mMenuElements[i].iScale[WIDTH],
				mMenuElements[i].iScale[HEIGHT]);
		case MENU_WINDOW:
		case MENU_BUTTON:
		case MENU_MODEL:
			break;
		}

		// [3/8/2012] MUST be done last to prevent something drawing over us! ~hogsy
		if(cvDebugMenu.value >= 1)
		{
			int p = mMenuElements[i].iPosition[Y];

			Engine.DrawString(iMousePosition[X],iMousePosition[Y],va
			(
				"(%i,%i)",
				iMousePosition[X],
				iMousePosition[Y]
			));

			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"Name: %s",
				mMenuElements[i].cName
			));
			p += 12;
			// [27/8/2012] Let us know who the parent is ~hogsy
			if(mMenuElements[i].mParent)
			{
				Engine.DrawString((int)mMenuElements[i].iPosition[X],p,va
				(
					"Parent: %s",
					mMenuElements[i].mParent->cName
				));
				p += 12;
			}
			if(mMenuElements[i].mMenuType == MENU_IMAGE)
			{
				Engine.DrawString(mMenuElements[i].iPosition[X],p,va
				(
					"Image: %s",
					mMenuElements[i].cName
				));
				p += 12;
			}
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"X: %f",
				mMenuElements[i].iPosition[0]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"Y: %f",
				mMenuElements[i].iPosition[1]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"W: %i",
				mMenuElements[i].iScale[WIDTH]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"H: %i",
				mMenuElements[i].iScale[HEIGHT]
			));
		}
	}
#endif
#else
	for(i = 0; i < iMenuElements; i++)
	{
//		mMenuElements[i].vPos[0] = mousepos[0];
//		mMenuElements[i].vPos[1] = mousepos[1];
		// [3/8/2012] Make sure elements can't go off-screen ~hogsy
#if 0
		if(mMenuElements->.vPos[0] > (screenwidth-mMenuElements[i].vScale[WIDTH]))
			mMenuElements[i].vPos[0] = (float)(screenwidth-mMenuElements[i].vScale[WIDTH]);
		else if(mMenuElements[i].vPos[0] < (screenwidth+mMenuElements[i].vScale[WIDTH]))
			mMenuElements[i].vPos[0] = (float)(screenwidth+mMenuElements[i].vScale[WIDTH]);
		if(mMenuElements[i].vPos[1] > (iMenuHeight-mMenuElements[i].height))
			mMenuElements[i].vPos[1] = (float)(screenwidth-mMenuElements[i].height);
		else if(mMenuElements[i].vPos[1] < (screenwidth+mMenuElements[i].height))
			mMenuElements[i].vPos[1] = (float)(screenwidth+mMenuElements[i].height);
#endif

		// [3/8/2012] Don't show if this element isn't visible ~hogsy
		// [27/8/2012] Don't bother rendering if the alpha is stupidly low ~hogsy
		if(mMenuElements[i].fAlpha > 0)
		{
			// [27/8/2012] Check if we have a parent ~hogsy
			if(mMenuElements[i].mParent)
			{
				// [27/8/2012] TODO: I know this isn't right, just temporary until parents are actually in ~hogsy
				mMenuElements[i].iPosition[X] += mMenuElements[i].mParent->iPosition[0]-mMenuElements[i].iPosition[X];
				mMenuElements[i].iPosition[Y] += mMenuElements[i].mParent->iPosition[1]-mMenuElements[i].iPosition[Y];
			}

			// [3/8/2012] Set depending on the type of window ~hogsy
			switch(mMenuElements[i].mMenuType)
			{
			// [3/8/2012] Draw a window element ~hogsy
			case MENU_WINDOW:
				// [3/8/2012] Check if this window is active or not ~hogsy
				if(mMenuElements[i].bActive)
					mMenuElements[i].fAlpha = 1.0f;
				else
				{
					// [4/8/2012] Simplified ~hogsy
					if(Menu_IsMouseOver(mMenuElements[i].iPosition,mMenuElements[i].iScale[WIDTH],mMenuElements[i].iScale[HEIGHT]))
						// [3/8/2012] We're being hovered over, make us more clear ~hogsy
						mMenuElements[i].fAlpha = 1.0f;
					else
						// [4/8/2012] BUG: Anything too low and we don't even show! ~hogsy
						mMenuElements[i].fAlpha = 0.7f;
				}

				// [3/8/2012] Draw the window top ~hogsy
//				Engine.DrawPic(MENU_BASEPATH"topright",m_menu[i].fAlpha,m_menu[i].pos[0],m_menu[i].pos[1],(int)m_menu[i].vScale[WIDTH]-2,20);
//				Engine.DrawPic(MENU_BASEPATH"topleft",m_menu[i].fAlpha,m_menu[i].pos[0],m_menu[i].pos[1],(int)m_menu[i].vScale[WIDTH]-2,20);
				Engine.DrawPic(
					MENU_BASEPATH"topbar",
					mMenuElements[i].fAlpha,
					mMenuElements[i].iPosition[X],
					mMenuElements[i].iPosition[1],
					mMenuElements[i].iScale[WIDTH]-2,
					20);
				Engine.DrawPic(MENU_BASEPATH"topclose",
					mMenuElements[i].fAlpha,
					mMenuElements[i].iPosition[X]+(mMenuElements[i].iScale[WIDTH]-20),
					mMenuElements[i].iPosition[Y],
					22,
					20);

				Engine.DrawString(mMenuElements[i].iPosition[X]+5,mMenuElements[i].iPosition[Y]+5,mMenuElements[i].cName);

				// [3/8/2012] Do the fill for the contents ~hogsy
				Engine.DrawFill(
					mMenuElements[i].iPosition[X],
					mMenuElements[i].iPosition[Y]+20,
					mMenuElements[i].iScale[WIDTH],
					mMenuElements[i].iScale[HEIGHT]-20,
					0.5f,
					0.5f,
					0.5f,
					mMenuElements[i].fAlpha);
				break;
			case MENU_BUTTON:
				if(Menu_IsMouseOver(mMenuElements[i].iPosition,mMenuElements[i].iScale[WIDTH],mMenuElements[i].iScale[HEIGHT]))
				{}
			default:
				Engine.Con_Warning("Unknown menu element type (%i) for %s!\n",mMenuElements[i].mMenuType,mMenuElements[i].cName);
				// [3/8/2012] TODO: This isn't right... Clear it properly! ~hogsy
				free(mMenuElements);
				// [3/8/2012] TODO: Don't just return, we should be changing to the next slot ~hogsy
				return;
			}
		}
	}
#endif
#endif
}

/*	Called by the engine.
	Add state to the current state set.
*/
void Menu_AddState(int iState)
{
	// [17/9/2013] Return since the state is already added ~hogsy
	if(iMenuState & iState)
		return;

	iMenuState |= iState;
}

/*	Called by the engine.
	Remove a state from the menus state set.
*/
void Menu_RemoveState(int iState)
{
	// [17/9/2013] Return since the state is already removed ~hogsy
	if(!(iMenuState & iState))
		return;

	iMenuState &= ~iState;
}

/*	Called by the engine.
	Force the current menu state to something new.
*/
void Menu_SetState(int iState)
{
	iMenuState = iState;
}

/*	Called by the engine.
	Returns the menus currently active states.
*/
int Menu_GetState(void)
{
	return iMenuState;
}

/*	Called by the engine.
	Shuts down the menu system.
*/
void Menu_Shutdown(void)
{
	Engine.Con_Printf("Shutting down menu...\n");

	delete mGlobal;
}

void Input_Key(int iKey)
{

}

extern "C" pMODULE_EXPORT MenuExport_t *Menu_Main(ModuleImport_t *Import)
{
	Engine.Con_DPrintf				= Import->Con_DPrintf;
	Engine.Con_Printf				= Import->Con_Printf;
	Engine.Con_Warning				= Import->Con_Warning;
	Engine.Sys_Error				= Import->Sys_Error;
	Engine.Cvar_RegisterVariable	= Import->Cvar_RegisterVariable;
	Engine.DrawPic					= Import->DrawPic;
	Engine.DrawString				= Import->DrawString;
	Engine.DrawFill					= Import->DrawFill;
	Engine.Cvar_SetValue			= Import->Cvar_SetValue;
	Engine.GetScreenHeight			= Import->GetScreenHeight;
	Engine.GetScreenWidth			= Import->GetScreenWidth;
	Engine.GetCursorPosition		= Import->GetCursorPosition;
	Engine.ShowCursor				= Import->ShowCursor;
	Engine.Client_PrecacheResource	= Import->Client_PrecacheResource;
	Engine.Cmd_AddCommand			= Import->Cmd_AddCommand;
	Engine.Client_GetStat			= Import->Client_GetStat;
	Engine.Client_SetMenuCanvas		= Import->Client_SetMenuCanvas;

	Export.iVersion		= MENU_VERSION;
	Export.Initialize	= Menu_Initialize;
	Export.Shutdown		= Menu_Shutdown;
	Export.Draw			= Menu_Draw;
	Export.Input		= Input_Key;
	Export.SetState		= Menu_SetState;
	Export.RemoveState	= Menu_RemoveState;
	Export.AddState		= Menu_AddState;
	Export.GetState		= Menu_GetState;

	return &Export;
}