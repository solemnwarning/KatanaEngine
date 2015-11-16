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

/*
	Dynamic variable tracking.
*/

ConsoleVariable_t *cConsoleVariables;

/*
	User Commands
*/

void Cvar_List_f (void)
{
	ConsoleVariable_t *cvar;
	char 	*partial;
	int		len, count;

	if (Cmd_Argc() > 1)
	{
		partial = Cmd_Argv (1);
		len = Q_strlen(partial);
	}
	else
	{
		partial = NULL;
		len = 0;
	}

	count=0;
	for (cvar=cConsoleVariables ; cvar ; cvar=cvar->next)
	{
		if (partial && Q_strncmp (partial,cvar->name, len))
			continue;

		Con_SafePrintf ("%s%s %s \"%s\"\n",
			cvar->bArchive ? "*" : " ",
			cvar->server ? "s" : " ",
			cvar->name,
			cvar->string);
		count++;
	}

	Con_SafePrintf ("%i cvars", count);
	if (partial)
		Con_SafePrintf (" beginning with \"%s\"", partial);

	Con_SafePrintf ("\n");
}

void Cvar_Inc_f (void)
{
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("inc <cvar> [amount] : increment cvar\n");
		break;
	case 2:
		Cvar_SetValue (Cmd_Argv(1), Cvar_VariableValue(Cmd_Argv(1)) + 1);
		break;
	case 3:
		Cvar_SetValue (Cmd_Argv(1), Cvar_VariableValue(Cmd_Argv(1)) + Q_atof(Cmd_Argv(2)));
		break;
	}
}

void Cvar_Toggle_f (void)
{
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("toggle <cvar> : toggle cvar\n");
		break;
	case 2:
		if (Cvar_VariableValue(Cmd_Argv(1)))
			Cvar_Set (Cmd_Argv(1), "0");
		else
			Cvar_Set (Cmd_Argv(1), "1");
		break;
	}
}

void Cvar_Cycle_f (void)
{
	int i;

	if (Cmd_Argc() < 3)
	{
		Con_Printf("cycle <cvar> <value list>: cycle cvar through a list of values\n");
		return;
	}

	//loop through the args until you find one that matches the current cvar value.
	//yes, this will get stuck on a list that contains the same value twice.
	//it's not worth dealing with, and i'm not even sure it can be dealt with.

	for (i=2;i<Cmd_Argc();i++)
	{
		//zero is assumed to be a string, even though it could actually be zero.  The worst case
		//is that the first time you call this command, it won't match on zero when it should, but after that,
		//it will be comparing strings that all had the same source (the user) so it will work.
		if (atof(Cmd_Argv(i)) == 0)
		{
			if (!strcmp(Cmd_Argv(i), Cvar_VariableString(Cmd_Argv(1))))
				break;
		}
		else
		{
			if (atof(Cmd_Argv(i)) == Cvar_VariableValue(Cmd_Argv(1)))
				break;
		}
	}

	if (i == Cmd_Argc())
		Cvar_Set (Cmd_Argv(1), Cmd_Argv(2)); // no match
	else if (i + 1 == Cmd_Argc())
		Cvar_Set (Cmd_Argv(1), Cmd_Argv(2)); // matched last value in list
	else
		Cvar_Set (Cmd_Argv(1), Cmd_Argv(i+1)); // matched earlier in list
}

void Cvar_Reset_f (void)
{
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf ("reset <cvar> : reset cvar to default\n");
		break;
	case 2:
		Cvar_Reset (Cmd_Argv(1));
		break;
	}
}

void Cvar_ResetAll_f (void)
{
	ConsoleVariable_t *var;

	for (var = cConsoleVariables; var; var = var->next)
		Cvar_Reset (var->name);
}

//==============================================================================
//
//  INIT
//
//==============================================================================

void Cvar_Init(void)
{
	Cmd_AddCommand ("cvarlist", Cvar_List_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("cycle", Cvar_Cycle_f);
	Cmd_AddCommand ("inc", Cvar_Inc_f);
	Cmd_AddCommand ("reset", Cvar_Reset_f);
	Cmd_AddCommand ("resetall", Cvar_ResetAll_f);
}

//==============================================================================
//
//  CVAR FUNCTIONS
//
//==============================================================================

void Cvar_UpdateValues(ConsoleVariable_t *cVariable)
{
	cVariable->value = Q_atof(cVariable->string);
	cVariable->iValue = Q_atoi(cVariable->string);

	cVariable->bValue = cVariable->iValue;
	if(cVariable->bValue < false)
		cVariable->bValue = false;
	else if(cVariable->bValue > true)
		cVariable->bValue = true;
}

ConsoleVariable_t *Cvar_FindVar (const char *var_name)
{
	ConsoleVariable_t *var;

	for(var = cConsoleVariables; var; var = var->next)
		if(!strcmp(var_name,var->name))
			return var;

	return NULL;
}

float Cvar_VariableValue (const char *var_name)
{
	ConsoleVariable_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return Q_atof (var->string);
}

char *Cvar_VariableString (char *var_name)
{
	ConsoleVariable_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->string;
}

#if 0
/*	Attempts to match a partial variable name for command line completion
	returns NULL if nothing fits
*/
char *Cvar_CompleteVariable (char *partial)
{
	ConsoleVariable_t *cvar;
	int	len;

	len = strlen(partial);
	if(!len)
		return NULL;

	// Check functions
	for(cvar=cConsoleVariables ; cvar ; cvar=cvar->next)
		if(!strncmp (partial,cvar->name, len))
			return cvar->name;

	return NULL;
}
#endif

void Cvar_Reset (const char *name)
{
	ConsoleVariable_t *var;

	var = Cvar_FindVar (name);
	if (!var)
		Con_Printf ("variable \"%s\" not found\n", name);
	else
		Cvar_Set (var->name, var->default_string);
}

void Cvar_Set (const char *var_name, char *value)
{
	ConsoleVariable_t *var;
	bool changed;

	var = Cvar_FindVar (var_name);
	if(!var)
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	changed = strcmp(var->string,value);

	Z_Free(var->string);	// free the old value string

	var->string = (char*)Z_Malloc(Q_strlen(value)+1);
	p_strcpy(var->string, value);

	Cvar_UpdateValues(var);

	//johnfitz -- during initialization, update default too
	if (!bHostInitialized)
	{
		Z_Free (var->default_string);
		var->default_string = (char*)Z_Malloc(strlen(value)+1);
		p_strcpy(var->default_string, value);
	}
	//johnfitz

	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}

	//johnfitz
	if(var->callback && changed)
		var->callback();
	//johnfitz
}

void Cvar_SetValue (const char *var_name, float value)
{
	char	val[32];

	sprintf (val, "%f",value);
	Cvar_Set (var_name, val);
}

/*	Adds a freestanding variable to the variable list.
*/
void Cvar_RegisterVariable (ConsoleVariable_t *variable,void (*Function)(void))
{
	char	*oldstr;
	ConsoleVariable_t	*cursor,*prev; //johnfitz -- sorted list insert

	// First check to see if it has allready been defined
	if (Cvar_FindVar (variable->name))
	{
		Con_Printf ("Can't register variable %s, allready defined\n", variable->name);
		return;
	}

	// Check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name);
		return;
	}

// copy the value off, because future sets will Z_Free it
	oldstr = variable->string;
	variable->string = (char*)Z_Malloc(strlen(variable->string)+1);
	p_strcpy(variable->string, oldstr);

	Cvar_UpdateValues(variable);

	//johnfitz -- save initial value for "reset" command
	variable->default_string = (char*)Z_Malloc(strlen(variable->string)+1);
	p_strcpy(variable->default_string, oldstr);
	//johnfitz

// link the variable in

	//johnfitz -- insert each entry in alphabetical order
	if(!cConsoleVariables || strcmp(variable->name, cConsoleVariables->name) < 0) //insert at front
	{
		variable->next = cConsoleVariables;
		cConsoleVariables = variable;
	}
	else //insert later
	{
		prev = cConsoleVariables;
		cursor = cConsoleVariables->next;
		while (cursor && (strcmp(variable->name, cursor->name) > 0))
		{
			prev = cursor;
			cursor = cursor->next;
		}
		variable->next = prev->next;
		prev->next = variable;
	}
	//johnfitz

	variable->callback = Function; //johnfitz
}

/*	Handles variable inspection and changing from the console
*/
bool Cvar_Command(void)
{
	cvar_t *v;

	// Check variables
	v = Cvar_FindVar(Cmd_Argv(0));
	if(!v)
		return false;

	// Perform a variable print or set
	if(Cmd_Argc() == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n",v->name,v->string);
		// [10/3/2014] Added support for descriptions ~hogsy
		if(v->cDescription)
			Con_Printf("%s\n",v->cDescription);

		return true;
	}

	Cvar_Set(v->name,Cmd_Argv(1));
	return true;
}


/*	Writes lines containing "set variable value" for all variables
	with the archive flag set to TRUE.
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;

	for (var = cConsoleVariables ; var ; var = var->next)
		if (var->bArchive)
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
}

