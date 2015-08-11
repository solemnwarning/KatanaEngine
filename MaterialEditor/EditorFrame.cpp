#include "EditorBase.h"

#include "EditorFrame.h"
#include "EditorRenderCanvas.h"

// Tools
#include "WADFrame.h"
#include "MaterialFrame.h"

enum
{
	FRAME_EVENT_SHOWCONSOLE,

	ID_WINDOW_SCRIPTEDITOR,	// Edit the material script.
	ID_WINDOW_PROPERTIES,
	FRAME_EVENT_RELOAD,		// Reload the current material.
	ID_WINDOW_PLAY,			// Play simulation.
	ID_WINDOW_PAUSE,		// Pause simulation.

	FRAME_EVENT_SCRIPT,

	// View
	FRAME_EVENT_WIREFRAME,
	FRAME_EVENT_FLAT,
	FRAME_EVENT_TEXTURED,
	FRAME_EVENT_LIT,

	// Tools
	FRAME_EVENT_WADTOOL,		// Open WAD tool
	FRAME_EVENT_MATERIALTOOL,	// Open Material tool

	FRAME_EVENT_TRANSFORM,

	FRAME_EVENT_CUBE,
	ID_BUTTON_SPHERE,
	ID_BUTTON_PLANE
};

wxBEGIN_EVENT_TABLE(CEditorFrame, wxFrame)

	EVT_MENU(wxID_OPEN, CEditorFrame::OnOpen)
	EVT_MENU(wxID_SAVE, CEditorFrame::OnSave)
	EVT_MENU(wxID_EXIT, CEditorFrame::OnExit)
	EVT_MENU(wxID_ABOUT, CEditorFrame::OnAbout)
	EVT_MENU(FRAME_EVENT_SHOWCONSOLE, CEditorFrame::OnConsole)
	EVT_MENU(ID_WINDOW_PROPERTIES, CEditorFrame::OnProperties)
	EVT_MENU(FRAME_EVENT_RELOAD, CEditorFrame::OnReload)
	EVT_MENU(ID_WINDOW_PLAY, CEditorFrame::OnPlay)
	EVT_MENU(ID_WINDOW_PAUSE, CEditorFrame::OnPause)

	// View
	EVT_MENU(FRAME_EVENT_WIREFRAME, CEditorFrame::OnView)
	EVT_MENU(FRAME_EVENT_FLAT, CEditorFrame::OnView)
	EVT_MENU(FRAME_EVENT_TEXTURED, CEditorFrame::OnView)
	EVT_MENU(FRAME_EVENT_LIT, CEditorFrame::OnView)

	// Tools
	EVT_MENU(FRAME_EVENT_WADTOOL, CEditorFrame::OnTool)

	EVT_TIMER(-1, CEditorFrame::OnTimer)

wxEND_EVENT_TABLE()

ConsoleVariable_t
cvEditorAutoReload = { "editor_ar", "1", true, false, "Enable or disable automatic reloading." },
cvEditorAutoReloadDelay = { "editor_ar_delay", "5", true, false, "Delay before attempting to automatically reload content." },
cvEditorShowProperties = { "editor_showproperties", "1", true, false, "Can show/hide the properties." },
cvEditorShowConsole = { "editor_showconsole", "1", true, false, "Can show/hide the console." };

CEditorFrame::CEditorFrame(const wxString & title, const wxPoint & pos, const wxSize & size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	manager = new wxAuiManager(this);

	pLog_Write(EDITOR_LOG, "Setting frame icon...\n");

	SetIcon(wxIcon("resource/icon-material.png", wxBITMAP_TYPE_PNG));

	// Display the splash screen...

	pLog_Write(EDITOR_LOG, "Displaying splash screen...\n");

	wxBitmap bmSplashImage;
	if (bmSplashImage.LoadFile(PATH_RESOURCES"material_editor/splash.png", wxBITMAP_TYPE_PNG))
	{
		new wxSplashScreen(
			bmSplashImage,
			wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
			4000,
			NULL,
			-1,
			wxDefaultPosition,
			wxDefaultSize,
			wxBORDER_SIMPLE | wxSTAY_ON_TOP);
	}

	SetSize(size);
	Maximize();

	// Set the menu up...

	wxMenu *mFile = new wxMenu;

	wxMenuItem *openMenuItem = new wxMenuItem(mFile, wxID_OPEN);
	openMenuItem->SetBitmap(smallDocumentOpen);
	mFile->Append(openMenuItem);

	mFile->AppendSeparator();

	wxMenuItem *miCloseDocument = new wxMenuItem(mFile, wxID_CLOSE);
	miCloseDocument->SetBitmap(smallDocumentClose);
	mFile->Append(miCloseDocument);

	mFile->AppendSeparator();

	wxMenuItem *saveDocumentMenuItem = new wxMenuItem(mFile, wxID_SAVE);
	saveDocumentMenuItem->SetBitmap(smallDocumentSave);
	mFile->Append(saveDocumentMenuItem);

	wxMenuItem *saveAsDocumentMenuItem = new wxMenuItem(mFile, wxID_SAVEAS);
	saveAsDocumentMenuItem->SetBitmap(smallDocumentSaveAs);
	mFile->Append(saveAsDocumentMenuItem);

	mFile->AppendSeparator();

	wxMenuItem *miExit = new wxMenuItem(mFile, wxID_EXIT);
	miExit->SetBitmap(smallApplicationExit);
	mFile->Append(miExit);

	wxMenu *mEdit = new wxMenu;
	mEdit->Append(wxID_UNDO);
	mEdit->Append(wxID_REDO);
	mEdit->AppendSeparator();

	wxMenu *mView = new wxMenu;
	viewWireframe = mView->AppendCheckItem(FRAME_EVENT_WIREFRAME, "&Wireframe");
	viewFlat = mView->AppendCheckItem(FRAME_EVENT_FLAT, "&Flat");
	viewTextured = mView->AppendCheckItem(FRAME_EVENT_TEXTURED, "Textured");
	viewLit = mView->AppendCheckItem(FRAME_EVENT_LIT, "&Lit");
	mView->AppendSeparator();
	mView->Append(FRAME_EVENT_RELOAD, "Reload material");
	mView->AppendSeparator();
	mView->AppendCheckItem(wxID_ANY, "&Transform");
	mView->AppendCheckItem(wxID_ANY, "&Rotate");
	mView->AppendCheckItem(wxID_ANY, "&Scale");
	mView->AppendSeparator();
	mView->AppendCheckItem(ID_BUTTON_SPHERE, "Sphere");
	mView->AppendCheckItem(FRAME_EVENT_CUBE, "&Cube");
	mView->AppendCheckItem(ID_BUTTON_PLANE, "&Plane");

	viewLit->Check(true);

	wxMenu *mTools = new wxMenu;
	mTools->Append(FRAME_EVENT_WADTOOL, "WAD Tool...");
	mTools->Append(FRAME_EVENT_MATERIALTOOL, "Material Tool...");

	wxMenu *mWindow = new wxMenu;
	windowShowConsole = mWindow->AppendCheckItem(FRAME_EVENT_SHOWCONSOLE, "&Console");
	windowShowProperties = mWindow->AppendCheckItem(ID_WINDOW_PROPERTIES, "&Properties");

	wxMenu *mHelp = new wxMenu;
	mHelp->Append(wxID_ABOUT);

	wxMenuBar *mbMainMenu = new wxMenuBar;
	mbMainMenu->Append(mFile, "&File");
	mbMainMenu->Append(mEdit, "&Edit");
	mbMainMenu->Append(mView, "&View");
	mbMainMenu->Append(mTools, "&Tools");
	mbMainMenu->Append(mWindow, "&Window");
	mbMainMenu->Append(mHelp, "&Help");
	SetMenuBar(mbMainMenu);

	CreateStatusBar(3);
	SetStatusText("Initialized");
	Center();

	Show();

	// Initialize the timer...
	timer = new wxTimer(this);

	// Create the toolbar...

	wxAuiPaneInfo toolbarInfo;
	toolbarInfo.Caption("Toolbar");
	toolbarInfo.ToolbarPane();
	toolbarInfo.Top();

	// File
	wxAuiToolBar *tbFile = new wxAuiToolBar(this);
	tbFile->AddTool(wxID_NEW, "New material", iconDocumentNew);
	tbFile->AddTool(wxID_OPEN, "Open material", smallDocumentOpen, "Open an existing material");
	tbFile->AddTool(wxID_SAVE, "Save material", smallDocumentSave, "Save the current material");
	tbFile->Realize();

	toolbarInfo.Position(0);
	manager->AddPane(tbFile, toolbarInfo);

	// Edit
	wxAuiToolBar *tbEdit = new wxAuiToolBar(this);
	tbEdit->AddTool(wxID_UNDO, "Undo", iconDocumentUndo, "Undo changes");
	tbEdit->AddTool(wxID_REDO, "Redo", iconDocumentRedo, "Redo changes");
	tbEdit->AddSeparator();
	tbEdit->AddTool(FRAME_EVENT_SCRIPT, "Script", iconScriptEdit, "Modify script");
	tbEdit->Realize();

	toolbarInfo.Position(1);
	manager->AddPane(tbEdit, toolbarInfo);

	// View
	wxAuiToolBar *tbView = new wxAuiToolBar(this);
	tbView->AddTool(FRAME_EVENT_RELOAD, "Reload material", iconDocumentRefresh, "Reload the material");
	tbView->AddSeparator();
	tbView->AddTool(ID_BUTTON_SPHERE, "Sphere", iconShapeSphere, "Sphere shape");
	tbView->AddTool(FRAME_EVENT_CUBE, "Cube", iconShapeCube, "Cube shape");
	tbView->AddTool(ID_BUTTON_PLANE, "Plane", iconShapePlane, "Plane shape");
	tbView->AddSeparator();
	tbView->AddTool(ID_WINDOW_PAUSE, "Pause", iconMediaPause, "Pause simulation");
	tbView->AddTool(ID_WINDOW_PLAY, "Play", iconMediaPlay, "Play simulation");
	tbView->Realize();

    toolbarInfo.Position(2);
	manager->AddPane(tbView, toolbarInfo);

	int attributes[] = {
		WX_GL_DEPTH_SIZE, 24,
		WX_GL_STENCIL_SIZE, 8,
		WX_GL_MIN_RED, 8,
		WX_GL_MIN_GREEN, 8,
		WX_GL_MIN_BLUE, 8,
		WX_GL_MIN_ALPHA, 8,
		WX_GL_MIN_ACCUM_RED, 8,
		WX_GL_MIN_ACCUM_GREEN, 8,
		WX_GL_MIN_ACCUM_BLUE, 8,
		WX_GL_MIN_ACCUM_ALPHA, 8,
#if 0
		WX_GL_SAMPLES, 4,
		WX_GL_SAMPLE_BUFFERS, 1
		WX_GL_DOUBLEBUFFER,	1,
#endif
	};

    // Create the engine viewport...
	editorViewport = new CEditorRenderCanvas(this, attributes);
	wxAuiPaneInfo viewportInfo;
	viewportInfo.Caption("Viewport");
	viewportInfo.Center();
	viewportInfo.Movable(true);
	viewportInfo.Floatable(true);
	viewportInfo.Dockable(true);
	viewportInfo.MaximizeButton(true);
	viewportInfo.CloseButton(false);
	manager->AddPane(editorViewport, viewportInfo);

	// Create the console...
	editorConsolePanel = new CEditorConsolePanel(this);
	wxAuiPaneInfo consoleInfo;
	consoleInfo.Caption("Console");
	consoleInfo.Bottom();
	consoleInfo.Movable(true);
	consoleInfo.Floatable(true);
	consoleInfo.MaximizeButton(true);
	consoleInfo.CloseButton(false);
	manager->AddPane(editorConsolePanel, consoleInfo);

	manager->Update();

	dAutoReloadDelay = 0;
	dClientTime = 0;
}

CEditorFrame::~CEditorFrame()
{
	// Stop the engine from looping.
	StopEngineLoop();

	// Uninitialize the AUI manager.
	manager->UnInit();
}

void CEditorFrame::InitializeConsoleVariables()
{
	// TODO: These need to be able to update the editor, when modified.
	engine->RegisterConsoleVariable(&cvEditorShowConsole, NULL);
	engine->RegisterConsoleVariable(&cvEditorShowProperties, NULL);
	engine->RegisterConsoleVariable(&cvEditorAutoReload, NULL);
	engine->RegisterConsoleVariable(&cvEditorAutoReloadDelay, NULL);

	dAutoReloadDelay = cvEditorAutoReloadDelay.value;
}

void CEditorFrame::OpenWADTool(wxString sPath)
{
	static CWADFrame *WADTool;
	if (!WADTool)
		WADTool = new CWADFrame(this);

    WADTool->Show();
}

void CEditorFrame::OpenMaterialTool(wxString sPath)
{
	static CMaterialFrame *MaterialTool;
	if (!MaterialTool)
		MaterialTool = new CMaterialFrame(this);

    MaterialTool->Show();
}

void CEditorFrame::StartEngineLoop()
{
	// TODO: This isn't working...
	if (!cvEditorShowConsole.bValue)
	{
		manager->GetPane(editorConsolePanel).Show(false);
		manager->Update();
	}
	else
		windowShowConsole->Check(true);

	timer->Start(25);
}

void CEditorFrame::StopEngineLoop()
{
	timer->Stop();
}

void CEditorFrame::OnPause(wxCommandEvent &event)
{
	StopEngineLoop();
}

void CEditorFrame::OnPlay(wxCommandEvent &event)
{
	timer->Start(25);
}

void CEditorFrame::OnTimer(wxTimerEvent &event)
{
	// TODO: Editor won't launch if engine isn't running... Can't we just make an assumption?
	if (engine->IsRunning())
	{
		// Perform the main loop.
		engine->Loop();

		// Keep the client-time updated.
		dClientTime = engine->GetClientTime();

		// Draw the main viewport.
		editorViewport->DrawFrame();
		editorViewport->Refresh();
	}

	// Check to see if it's time to check for changes.
	if (dAutoReloadDelay < dClientTime)
	{
		ReloadCurrentDocument();
		dAutoReloadDelay = dClientTime + cvEditorAutoReloadDelay.value;
	}
}

void CEditorFrame::OnView(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case FRAME_EVENT_WIREFRAME:
		viewFlat->Check(false);
		viewLit->Check(false);
		viewWireframe->Check(true);
		viewTextured->Check(false);

		engine->SetConsoleVariable("r_drawflat", "0");
		engine->SetConsoleVariable("r_showtris", "1");
		engine->SetConsoleVariable("r_fullbright", "0");
		break;
	case FRAME_EVENT_FLAT:
		viewFlat->Check(true);
		viewLit->Check(false);
		viewWireframe->Check(false);
		viewTextured->Check(false);

		engine->SetConsoleVariable("r_drawflat", "1");
		engine->SetConsoleVariable("r_showtris", "0");
		engine->SetConsoleVariable("r_fullbright", "0");
		break;
	case FRAME_EVENT_TEXTURED:
		viewFlat->Check(false);
		viewLit->Check(false);
		viewWireframe->Check(false);
		viewTextured->Check(true);

		engine->SetConsoleVariable("r_drawflat", "0");
		engine->SetConsoleVariable("r_showtris", "0");
		engine->SetConsoleVariable("r_fullbright", "1");
		break;
	case FRAME_EVENT_LIT:
		viewFlat->Check(false);
		viewLit->Check(true);
		viewWireframe->Check(false);
		viewTextured->Check(false);

		engine->SetConsoleVariable("r_drawflat", "0");
		engine->SetConsoleVariable("r_showtris", "0");
		engine->SetConsoleVariable("r_fullbright", "0");
		break;
	}
}

void CEditorFrame::OnTool(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case FRAME_EVENT_WADTOOL:
		OpenWADTool("");
		break;
	case FRAME_EVENT_MATERIALTOOL:
		OpenMaterialTool("");
		break;
	}
}

void CEditorFrame::OnReload(wxCommandEvent &event)
{
	ReloadCurrentDocument();
}

void CEditorFrame::OnOpen(wxCommandEvent &event)
{
	char cDefaultPath[PLATFORM_MAX_PATH];

	sprintf(cDefaultPath, "%s", engine->GetBasePath());

	wxFileDialog *fileDialog = new wxFileDialog(
		this,
		"Open File",
		cDefaultPath,
		"",
		"Supported files (*.material;*.map;*.level;*.md2;*.wad)|"
		"*.material;*.level;*.md2;*.wad|"
		"MATERIAL files (*.material)|"
		"*.material|"
		"MD2 files (*.md2)|"
		"*.md2|"
		"MAP files (*.map)|"
		"*.map|"
		"LEVEL files (*.level)|"
		"*.level|"
		"WAD files (*.wad)|"
		"*.wad"
		,wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (fileDialog->ShowModal() == wxID_OK)
	{
		wxString filename = fileDialog->GetFilename();

		if (filename.EndsWith(".material"))
			OpenMaterialTool(fileDialog->GetPath());
		else if (filename.EndsWith(".map"))
		{
			SetTitle(fileDialog->GetFilename() + wxString(" - ") + cApplicationTitle);
		}
		else if (filename.EndsWith(".level"))
		{
			// TODO: Load the level up in a "viewer" mode.
			SetTitle(fileDialog->GetFilename() + wxString(" - ") + cApplicationTitle);
		}
		else if (filename.EndsWith(".wad"))
			OpenWADTool(fileDialog->GetPath());
		else if (filename.EndsWith(".md2"))
		{
		}

		currentFilePath = fileDialog->GetPath();
		lastTimeModified = currentTimeModified = pFileSystem_GetModifiedTime(currentFilePath);
	}
}

void CEditorFrame::OnExit(wxCommandEvent &event)
{
	// Stop rendering!
	StopEngineLoop();

	engine->Shutdown();

	// Close the frame and app.
	Close(true);
}

void CEditorFrame::OnAbout(wxCommandEvent &event)
{
	wxAboutDialogInfo info;
	info.SetName("Editor");
	info.SetCopyright("Copyright (C) 2011-2015 OldTimes Software");
	info.SetDescription("Editor for the Katana engine.");
	info.SetWebSite("www.oldtimes-software.com");
	info.SetVersion(engine->GetVersion());

	wxAboutBox(info, this);
}

void CEditorFrame::OnSave(wxCommandEvent &event)
{}

void CEditorFrame::OnConsole(wxCommandEvent &event)
{
	if (manager->GetPane(editorConsolePanel).IsShown())
	{
		manager->GetPane(editorConsolePanel).Show(false);
		engine->SetConsoleVariable(cvEditorShowConsole.name, "0");
	}
	else
	{
		manager->GetPane(editorConsolePanel).Show(true);
		engine->SetConsoleVariable(cvEditorShowConsole.name, "1");
	}

	manager->Update();
}

void CEditorFrame::OnProperties(wxCommandEvent &event)
{
#if 0
	propertyWindow->Show(!propertyWindow->IsShown());
	propertyWindow->SetPosition(wxPoint(GetPosition().x - 256, GetPosition().y));
#endif
}

/*	Reload the currently active document.
*/
void CEditorFrame::ReloadCurrentDocument()
{
#if 0
	Material_t *current = editorMaterialProperties->GetCurrent();
	if (!current)
		return;

	// Ensure things have actually changed.
	currentTimeModified = pFileSystem_GetModifiedTime(currentFilePath);
	if (currentTimeModified == lastTimeModified)
		return;

	char cPath[PLATFORM_MAX_PATH];
	strcpy(cPath, current->cPath);

	engine->UnloadMaterial(current);

	Material_t *reloadedMat = engine->LoadMaterial(cPath);
	if (reloadedMat)
	{
		// Keep this up to date.
		lastTimeModified = currentTimeModified;

		engine->MaterialEditorDisplay(reloadedMat);

		editorMaterialProperties->SetCurrentMaterial(reloadedMat);
		editorMaterialProperties->Update();
	}
#endif
}

void CEditorFrame::PrintMessage(char *text)
{
	if (!editorConsolePanel)
		return;

	editorConsolePanel->textConsoleOut->SetDefaultStyle(wxTextAttr(*wxGREEN));
	editorConsolePanel->textConsoleOut->AppendText(text);
}

void CEditorFrame::PrintWarning(char *text)
{
	if (!editorConsolePanel)
		return;

	editorConsolePanel->textConsoleOut->SetDefaultStyle(wxTextAttr(*wxYELLOW));
	editorConsolePanel->textConsoleOut->AppendText(text);
}

void CEditorFrame::PrintError(char *text)
{
	if (!editorConsolePanel)
		return;

	editorConsolePanel->textConsoleOut->SetDefaultStyle(wxTextAttr(*wxRED));
	editorConsolePanel->textConsoleOut->AppendText(text);
}