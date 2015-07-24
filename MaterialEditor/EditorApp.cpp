#include "MaterialEditorBase.h"

#include "EditorApp.h"

wxIMPLEMENT_APP(CEditorApp);

char cApplicationTitle[512];

bool CEditorApp::OnInit()
{
	pLog_Clear(MATERIALEDITOR_LOG);
	pLog_Write(MATERIALEDITOR_LOG, "Initializing engine interface...\n");

	EngineInterface_Load();

	sprintf(cApplicationTitle, EDITOR_TITLE" (%s)", engine->GetVersion());

	pLog_Write(MATERIALEDITOR_LOG, "Creating main frame...\n");

	// Create the main frame.
	mainFrame = new CMaterialEditorFrame(cApplicationTitle, wxPoint(50, 50), wxSize(1024, 768));

	pLog_Write(MATERIALEDITOR_LOG, "Displaying main frame...\n");
	
	// Show it!
	mainFrame->Show(true);

	pLog_Write(MATERIALEDITOR_LOG, "Proceeding with engine initialization...\n");

	// Initialize the engine.
	if (!engine->Initialize(argc, argv, true))
	{
		wxMessageBox("Failed to initialize engine!", EDITOR_TITLE" Error");
		wxExit();
	}

	engine->MaterialEditorInitialize();

	// Initialize any console variables we're using.
	mainFrame->InitializeConsoleVariables();

	pLog_Write(MATERIALEDITOR_LOG, "Starting main loop...\n");

	// Start rendering.
	mainFrame->StartEngineLoop();

	// We launched without problems, return true.
	return true;
}

int CEditorApp::OnExit()
{
	pLog_Write(MATERIALEDITOR_LOG, "Unloading engine interface...\n");
	EngineInterface_Unload();
	pLog_Write(MATERIALEDITOR_LOG, "Done!\n");
	return 0;
}