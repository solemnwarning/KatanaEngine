/*	Copyright (C) 2011-2015 OldTimes Software
*/

#include "platform.h"

/*
	Log System
*/

#define	LOG_FILE_EXTENSION	".log"

void Log_Write(const char *ccPath, char *cMessage, ...)
{
	pFUNCTION_START;

	FILE *fLog;
	va_list vlArguments;
	static char scData[1024];
	char cPath[PLATFORM_MAX_PATH];
	unsigned int uiData;

	sprintf(cPath, "%s"LOG_FILE_EXTENSION, ccPath);

	va_start(vlArguments, cMessage);
	vsprintf(scData, cMessage, vlArguments);
	va_end(vlArguments);

	uiData = strlen(scData);

	fLog = fopen(cPath, "a");
	if (fwrite(scData, sizeof(char), uiData, fLog) != uiData)
		pError_Set("Failed to write to log! (%s)", cPath);

	pFUNCTION_END;
}