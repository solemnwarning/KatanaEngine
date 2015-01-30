/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "quakedef.h"

/*
	Material System
	This is pretty much written from scratch without using anything else for reference,
	more just for inspiration. This has been a pretty weird learning experience for me
	since I've never written anything out like this before but it works and does the job
	it was designed to do.
	~hogsy
*/

#include "engine_material.h"

#include "engine_video.h"
#include "engine_script.h"

bool	bInitialized = false;

Material_t	mMaterials[MATERIAL_MAX];	// Global array.

MaterialType_t	MaterialTypes[]=
{
	{	MATERIAL_TYPE_NONE,		"default"	},
	{	MATERIAL_TYPE_METAL,	"metal"		},
	{	MATERIAL_TYPE_GLASS,	"glass"		},
	{	MATERIAL_TYPE_CONCRETE,	"concrete"	},
	{	MATERIAL_TYPE_WOOD,		"wood"		},
	{	MATERIAL_TYPE_DIRT,		"dirt"		},
	{	MATERIAL_TYPE_RUBBER,	"rubber"	},
	{	MATERIAL_TYPE_WATER,	"water"		},
	{	MATERIAL_TYPE_FLESH,	"flesh"		},
	{	MATERIAL_TYPE_SNOW,		"snow"		},
	{	MATERIAL_TYPE_MUD,		"mud"		}
};

int	iMaterialCount = 0;

Material_t *Material_Allocate(void);

void Material_List(void);

void Material_Initialize(void)
{
	if(bInitialized)
		return;

	Con_Printf("Initializing material system...\n");

	Cmd_AddCommand("material_list", Material_List);

	// Must be set to initialized before anything else.
	bInitialized = true;

	// Load base materials.
	{
		Material_t *mBase;

		mBase = Material_Load("engine/notexture");
		if (!mBase)
			Sys_Error("Failed to load notexture material!\n");

		mBase = Material_Load("engine/conchars");
		if (!mBase)
			Sys_Error("Failed to load conchars material!\n");
	}

#ifdef _MSC_VER // This is false, since the function above shuts us down, but MSC doesn't understand that.
#pragma warning(suppress: 6011)
#endif
}

/*	Lists all the currently active materials.
*/
void Material_List(void)
{
	int i,iSkins = 0;

	Con_Printf("Listing materials...\n");

	for (i = 0; i < MATERIAL_MAX; i++)
	{
		// Probably the end, just break.
		if (!mMaterials[i].iSkins)
			break;

		iSkins += mMaterials[i].iSkins;

		Con_Printf(" %s (%s) (%i)\n", mMaterials[i].cName, mMaterials[i].cPath, mMaterials[i].iSkins);
	}

	Con_Printf("\nListed %i active materials with %i skins in total!\n", i,iSkins);
}

/*
	Management
*/

Material_t *Material_Allocate(void)
{
	int	i;

	// In the case of allocation, we go through the entire array.
	for (i = 0; i < MATERIAL_MAX; i++)
		if (!(mMaterials[i].iFlags & MATERIAL_FLAG_PRESERVE))
			if (!mMaterials[i].iIdentification || !mMaterials[i].iSkins)
			{
				// Set our new material with defaults.
				mMaterials[i].cName[0] = 0;
				mMaterials[i].iIdentification = i;
				mMaterials[i].iSkins = 0;
				mMaterials[i].iFlags = 0;
				mMaterials[i].fAlpha = 1.0f;
				mMaterials[i].bBind	= true;

				iMaterialCount++;

				return &mMaterials[i];
			}

	return NULL;
}

/*	Clears all the currently active materials.
	TODO: 
		Reorganise list!!!!!!!!!
*/
void Material_ClearAll(void)
{
	int	i;

	for (i = 0; i < MATERIAL_MAX; i++)
		if (!(mMaterials[i].iFlags & MATERIAL_FLAG_PRESERVE))
		{
			memset(&mMaterials[i], 0, sizeof(Material_t));

			// iMaterialCount--;
		}
}

MaterialSkin_t *Material_GetSkin(Material_t *mMaterial,int iSkin)
{
	if (iSkin < 0 || iSkin > MATERIAL_MAX_SKINS)
		Sys_Error("Invalid skin identification, should be greater than 0 and less than %i! (%i)\n", MATERIAL_MAX_SKINS, iSkin);
#if 0
	else if(!mMaterial->iSkins)
		Sys_Error("Material with no valid skins! (%s)\n", mMaterial->cName);
#endif
	else if (iSkin > mMaterial->iSkins)
		Sys_Error("Attempted to get an invalid skin! (%i) (%s)\n", iSkin, mMaterial->cName);

	return &mMaterial->msSkin[iSkin];
}

/*	Get an animated skin.
*/
MaterialSkin_t *Material_GetAnimatedSkin(Material_t *mMaterial)
{
	if (mMaterial->dAnimationTime < cl.time)
	{
		// Increment current frame...
		mMaterial->iAnimationFrame++;

		// If we're beyond the frame count, step back to 0...
		if (mMaterial->iAnimationFrame >= mMaterial->iSkins)
			mMaterial->iAnimationFrame = 0;

		mMaterial->dAnimationTime = cl.time + ((double)mMaterial->fAnimationSpeed);
	}

	return Material_GetSkin(mMaterial,mMaterial->iAnimationFrame);
}

/*	Returns a material from the given ID.
*/
Material_t *Material_Get(int iMaterialID)
{
	int i;

	// The identification would never be less than 0, and never more than our maximum.
	if (iMaterialID < 0 || iMaterialID > MATERIAL_MAX)
	{
		Con_Warning("Invalid material ID! (%i)\n",iMaterialID);
		return NULL;
	}

	for (i = 0; i < iMaterialCount; i++)
		if (mMaterials[i].iIdentification == iMaterialID)
		{
			mMaterials[i].bBind = true;

			return &mMaterials[i];
		}

	return NULL;
}

/*	Returns true on success.
	Unfinished
*/
Material_t *Material_GetByName(const char *ccMaterialName)
{
	int i;

	if(ccMaterialName[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid material name!\n");
		return NULL;
	}

	for (i = 0; i < iMaterialCount; i++)
		// If the material has no name, then it's not valid.
		if (mMaterials[i].cName[0])
			if (!strncmp(mMaterials[i].cName, ccMaterialName, sizeof(mMaterials[i].cName)))
				return &mMaterials[i];

	return NULL;
}

Material_t *Material_GetByPath(const char *ccPath)
{
	int i;

	if(ccPath[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid path!\n");
		return NULL;
	}

	for (i = 0; i < iMaterialCount; i++)
		if (mMaterials[i].cPath[0])
			if (!strncmp(mMaterials[i].cPath, ccPath, sizeof(mMaterials[i].cPath)))
				return &mMaterials[i];

	return NULL;
}

gltexture_t *Material_LoadTexture(Material_t *mMaterial, MaterialSkin_t *mCurrentSkin, char *cArg)
{
	int			iTextureFlags = TEXPREF_ALPHA;
	byte		*bTextureMap;

	Con_DPrintf("Loading material texture...\n");

	// Check if it's trying to use a built-in texture.
	if (cArg[0] == '@')
	{
		cArg++;

		if (!stricmp(cArg, "notexture"))
			return notexture;
		else if (!stricmp(cArg, "lightmap"))
		{
			mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].mttType = MATERIAL_TEXTURE_LIGHTMAP;
			return lightmap_textures[0];
		}
#if 0
		else if (!stricmp(cArg, "shadow"))
			return generated_shadow;
#endif
		else
			Sys_Error("Attempted to set invalid internal texture! (%s)\n", mMaterial->cPath);
	}

	{
		// Ensure we haven't loaded the texture in already...
		gltexture_t *gTexture = TexMgr_GetTexture(cArg);
		if (gTexture)
			return gTexture;
	}

	bTextureMap = Image_LoadImage(cArg,
		&mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiWidth,
		&mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiHeight);
	if (bTextureMap)
	{
		// Warn about incorrect sizes.
		if ((mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiWidth & 15) || (mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiHeight & 15))
		{
			Con_Warning("Texture is not 16 aligned! (%s) (%ix%i)\n", cArg,
				mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiWidth,
				mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiHeight);

			// Pad the image.
			iTextureFlags |= TEXPREF_PAD;
		}

		if (mMaterial->iFlags & MATERIAL_FLAG_PRESERVE)
			iTextureFlags |= TEXPREF_PERSIST;

		return TexMgr_LoadImage(NULL, cArg,
			mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiWidth,
			mCurrentSkin->mtTexture[mCurrentSkin->uiTextures].uiHeight,
			SRC_RGBA, bTextureMap, cArg, 0, iTextureFlags);
	}

	Con_Warning("Failed to load texture %s!\n", cArg);

	return notexture;
}

/*
	Scripting
*/

typedef enum
{
	MATERIAL_FUNCTION_NONE,

	MATERIAL_FUNCTION_MATERIAL,
	MATERIAL_FUNCTION_SKIN,
	MATERIAL_FUNCTION_TEXTURE,
	MATERIAL_FUNCTION_UNIVERSAL
} MaterialFunctionType_t;

MaterialFunctionType_t	mftMaterialState;	// Indicates that any settings applied are global.

void Material_CheckFunctions(Material_t *mNewMaterial);

// Material Functions...

typedef struct
{
	const char	*ccName;

	MaterialTextureType_t	mttType;
} MaterialTextureTypeX_t;

MaterialTextureTypeX_t mttMaterialTypes[] =
{
	{ "diffuse", MATERIAL_TEXTURE_DIFFUSE },
	{ "detail", MATERIAL_TEXTURE_DETAIL },
	{ "sphere", MATERIAL_TEXTURE_SPHERE },
	{ "fullbright", MATERIAL_TEXTURE_FULLBRIGHT },
	{ "lightmap", MATERIAL_TEXTURE_LIGHTMAP }
};

void _Material_SetType(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	switch (mftContext)
	{
	case MATERIAL_FUNCTION_SKIN:
	{
		int	iMaterialType = Q_atoi(cArg);

		// Ensure that the given type is valid.
		if ((iMaterialType < MATERIAL_TYPE_NONE) || (iMaterialType >= MATERIAL_TYPE_MAX))
			Con_Warning("Invalid material type! (%i)\n", iMaterialType);

		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].uiType = iMaterialType;
	}
	case MATERIAL_FUNCTION_TEXTURE:
	{
		int	i;

		// Search through and copy each flag into the materials list of flags.
		for (i = 0; i < pARRAYELEMENTS(mttMaterialTypes); i++)
			if (strstr(cArg, mttMaterialTypes[i].ccName))
				mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].uiType = mttMaterialTypes[i].mttType;
	}
	}
}

void _Material_SetAnimationSpeed(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	mCurrentMaterial->fAnimationSpeed = strtof(cArg, NULL);
}

// Skin Functions...

void _Material_AddSkin(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	Con_DPrintf("Adding material skin...\n");

	// Proceed to the next line.
	Script_GetToken(true);

	if (cToken[0] == '{')
	{
		while (true)
		{
			if (!Script_GetToken(true))
			{
				Con_Warning("End of field without closing brace! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
				break;
			}

			mftMaterialState = MATERIAL_FUNCTION_SKIN;

			if (cToken[0] == '}')
			{
				mCurrentMaterial->iSkins++;
				break;
			}
			// '$' declares that the following is a function.
			else if (cToken[0] == SCRIPT_SYMBOL_FUNCTION)
				Material_CheckFunctions(mCurrentMaterial);
			// '%' declares that the following is a variable.
			else if (cToken[0] == SCRIPT_SYMBOL_VARIABLE)
			{
				/*	TODO:
				* Collect variable
				* Check it against internal solutions
				* Otherwise declare it, figure out where/how it's used
				*/
			}
			else
			{
				Con_Warning("Invalid field! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
				break;
			}
		}
	}
	else
		Con_Warning("Invalid skin, no opening brace! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
}

// Texture Functions...

void _Material_AddTexture(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	MaterialSkin_t	*msSkin;

	Con_DPrintf("Adding material texture...\n");

	msSkin = Material_GetSkin(mCurrentMaterial, mCurrentMaterial->iSkins);
	if (!msSkin)
		Sys_Error("Failed to get skin!\n");

	msSkin->mtTexture[msSkin->uiTextures].bManipulation = false;
	msSkin->mtTexture[msSkin->uiTextures].fRotate = 0;
	msSkin->mtTexture[msSkin->uiTextures].mttType = MATERIAL_TEXTURE_DIFFUSE;
	msSkin->mtTexture[msSkin->uiTextures].vScroll[0] = 0;
	msSkin->mtTexture[msSkin->uiTextures].vScroll[1] = 0;
	msSkin->mtTexture[msSkin->uiTextures].gMap = Material_LoadTexture(mCurrentMaterial, msSkin, cArg);

	// Get following line.
	Script_GetToken(true);

	if (cToken[0] == '{')
	{
		while (true)
		{
			if (!Script_GetToken(true))
			{
				Con_Warning("End of field without closing brace! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
				break;
			}

			// Update state.
			mftMaterialState = MATERIAL_FUNCTION_TEXTURE;

			if (cToken[0] == '}')
			{
				msSkin->uiTextures++;
				break;
			}
			// '$' declares that the following is a function.
			else if (cToken[0] == SCRIPT_SYMBOL_FUNCTION)
				Material_CheckFunctions(mCurrentMaterial);
			// '%' declares that the following is a variable.
			else if (cToken[0] == SCRIPT_SYMBOL_VARIABLE)
			{
				/*	TODO:
				* Collect variable
				* Check it against internal solutions
				* Otherwise declare it, figure out where/how it's used
				*/
			}
			else
			{
				Con_Warning("Invalid field! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
				break;
			}
		}
	}
	else
		Con_Warning("Invalid skin, no opening brace! (%s) (%i)\n", mCurrentMaterial->cPath, iScriptLine);
}

void _Material_SetTextureType(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
}

void _Material_SetTextureScroll(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	MaterialSkin_t	*msSkin;
	MathVector2_t	vScroll;

	sscanf(cArg, "%f %f", &vScroll[0], &vScroll[1]);

	msSkin = Material_GetSkin(mCurrentMaterial, mCurrentMaterial->iSkins);
	msSkin->mtTexture[msSkin->uiTextures].vScroll[0] = vScroll[0];
	msSkin->mtTexture[msSkin->uiTextures].vScroll[1] = vScroll[1];
}

#if 0
void _Material_SetScrollX(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	MaterialSkin_t	*msSkin;

	msSkin = Material_GetSkin(mCurrentMaterial, mCurrentMaterial->iSkins);
	msSkin->mtTexture[msSkin->uiTextures].vScroll[0] = strtof(cArg, NULL);
}

void _Material_SetScrollY(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	MaterialSkin_t	*msSkin;

	msSkin = Material_GetSkin(mCurrentMaterial, mCurrentMaterial->iSkins);
	msSkin->mtTexture[msSkin->uiTextures].vScroll[1] = strtof(cArg, NULL);
}
#endif

void _Material_SetRotate(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	MaterialSkin_t	*msSkin;

	msSkin = Material_GetSkin(mCurrentMaterial, mCurrentMaterial->iSkins);
	msSkin->mtTexture[msSkin->uiTextures].fRotate = strtof(cArg, NULL);
}

// Universal Functions...

typedef struct
{
	int	iFlag;

	const	char	*ccName;

	MaterialFunctionType_t	mftContext;
} MaterialFlag_t;

MaterialFlag_t	mfMaterialFlags[] =
{
	// Global
	{ MATERIAL_FLAG_PRESERVE, "PRESERVE", MATERIAL_FUNCTION_MATERIAL },
	{ MATERIAL_FLAG_ANIMATED, "ANIMATED", MATERIAL_FUNCTION_MATERIAL },
	{ MATERIAL_FLAG_MIRROR, "MIRROR", MATERIAL_FUNCTION_MATERIAL },
	{ MATERIAL_FLAG_WATER, "WATER", MATERIAL_FUNCTION_MATERIAL },

	// Local
	{ MATERIAL_FLAG_ALPHA, "ALPHA", MATERIAL_FUNCTION_SKIN },
	{ MATERIAL_FLAG_BLEND, "BLEND", MATERIAL_FUNCTION_SKIN }
};

/*	Set flags for the material.
*/
void _Material_SetFlags(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg)
{
	int	i;

	Con_DPrintf("Setting material flags...\n");

	// Search through and copy each flag into the materials list of flags.
	for (i = 0; i < pARRAYELEMENTS(mfMaterialFlags); i++)
	{
		if (strstr(cArg, mfMaterialFlags[i].ccName))
		{
			if (mfMaterialFlags[i].mftContext != mftContext)
				continue;

			switch (mftContext)
			{
			case MATERIAL_FUNCTION_MATERIAL:
				if (mfMaterialFlags[i].iFlag == MATERIAL_FLAG_ANIMATED)
				{
					mCurrentMaterial->iAnimationFrame = 0;
					mCurrentMaterial->dAnimationTime = 0;
				}

				mCurrentMaterial->iFlags |= mfMaterialFlags[i].iFlag;
				break;
			case MATERIAL_FUNCTION_SKIN:
				mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].uiFlags |= mfMaterialFlags[i].iFlag;
				break;
			default:
				Con_Warning("Invalid context! (%s) (%s)\n", mCurrentMaterial->cName, mfMaterialFlags[i].ccName);
			}
		}
	}
}

typedef struct
{
	char	*cKey;

	void	(*Function)(Material_t *mCurrentMaterial, MaterialFunctionType_t mftContext, char *cArg);

	MaterialFunctionType_t	mftType;
} MaterialKey_t;

MaterialKey_t	mkMaterialFunctions[]=
{
	// Universal
	{ "flags", _Material_SetFlags, MATERIAL_FUNCTION_UNIVERSAL },
	{ "type", _Material_SetType, MATERIAL_FUNCTION_UNIVERSAL },

	// Material
	{ "animation_speed", _Material_SetAnimationSpeed, MATERIAL_FUNCTION_MATERIAL },
	{ "skin", _Material_AddSkin, MATERIAL_FUNCTION_MATERIAL },

	// Skin
	{ "map", _Material_AddTexture, MATERIAL_FUNCTION_SKIN },

	// Texture
	{ "scroll", _Material_SetTextureScroll, MATERIAL_FUNCTION_TEXTURE },
	{ "rotate", _Material_SetRotate, MATERIAL_FUNCTION_TEXTURE },

	{	0	}
};

void Material_CheckFunctions(Material_t *mNewMaterial)
{
	MaterialKey_t *mKey;

	// Find the related function.
	for (mKey = mkMaterialFunctions; mKey->cKey; mKey++)
		// Remain case sensitive.
		if (!Q_strcasecmp(mKey->cKey, cToken + 1))
		{
			/*	todo
				account for texture slots etc
			*/
			if ((mKey->mftType != MATERIAL_FUNCTION_UNIVERSAL) && (mftMaterialState != mKey->mftType))
				Sys_Error("Attempted to call a function within the wrong context! (%s) (%s) (%i)\n", cToken, mNewMaterial->cPath, iScriptLine);

			Script_GetToken(false);

			mKey->Function(mNewMaterial, mftMaterialState, cToken);
			return;
		}

	Con_Warning("Unknown function! (%s) (%s) (%i)\n", cToken, mNewMaterial->cPath, iScriptLine);
}

/*	Loads and parses material.
	Returns false on complete failure.
*/
Material_t *Material_Load(const char *ccPath)
{
    Material_t  *mNewMaterial;
	int			iMaterialVersion = 0;
	void        *cData;
	char		cPath[PLATFORM_MAX_PATH],
				cMaterialName[64] = { 0 };

	// Ensure that the given material names are correct!
	if (ccPath[0] == ' ')
		Sys_Error("Invalid material path! (%s)\n", ccPath);

	if (!bInitialized)
	{
		Con_Warning("Attempted to load material, before initialization! (%s)\n", ccPath);
		return NULL;
	}

	Con_DPrintf("Loading material: %s\n", ccPath);

	// Update the given path with the base path plus extension.
	sprintf(cPath,"%s%s.material",Global.cMaterialPath,ccPath);

	// Check if it's been cached already...
	mNewMaterial = Material_GetByPath(cPath);
	if(mNewMaterial)
		return mNewMaterial;
	
	cData = COM_LoadFile(cPath,0);
	if(!cData)
	{
		Con_Warning("Failed to load material! (%s) (%s)\n", cPath, ccPath);
		return NULL;
	}

	Script_StartTokenParsing((char*)cData);

	if(!Script_GetToken(true))
	{
		Con_Warning("Failed to get initial token! (%s) (%i)\n",ccPath,iScriptLine);
		return NULL;
	}
	else if (cToken[0] != '{')
	{
		if (!Q_strcmp(cToken, "material_version"))
		{
			Script_GetToken(false);

			iMaterialVersion = atoi(cToken);
		}
		else	// Probably a name...
		{
			// Copy over the given name.
			strncpy(cMaterialName, cToken, sizeof(cMaterialName));
			if (cMaterialName[0] == ' ')
				Sys_Error("Invalid material name!\n");

			// Check if it's been cached already...
			mNewMaterial = Material_GetByName(cMaterialName);
			if (mNewMaterial)
			{
				Con_Warning("Attempted to load duplicate material! (%s) (%s) vs (%s) (%s)\n",
					ccPath, cMaterialName,
					mNewMaterial->cPath, mNewMaterial->cName);

				Z_Free(cData);

				return mNewMaterial;
			}
		}

		Script_GetToken(true);

		if (cToken[0] != '{')
		{
			Con_Warning("Missing '{'! (%s) (%i)\n", ccPath, iScriptLine);

			goto MATERIAL_LOAD_ERROR;
		}
	}

	// Assume that the material hasn't been cached yet, so allocate a new copy of one.
	mNewMaterial = Material_Allocate();
	if (!mNewMaterial)
	{
		Con_Warning("Failed to allocate material! (%s)\n",ccPath);

		goto MATERIAL_LOAD_ERROR;
	}

	if (cMaterialName[0])
		// Copy the name over.
		strncpy(mNewMaterial->cName, cMaterialName, sizeof(mNewMaterial->cName));
#if 0	// This can end up causing unnecessary conflicts, don't bother.
	else
		// Otherwise just use the filename.
		ExtractFileBase(ccPath, mNewMaterial->cName);
#endif

	// Copy the path over.
	strncpy(mNewMaterial->cPath, ccPath, sizeof(mNewMaterial->cPath));

	while(true)
	{
		if(!Script_GetToken(true))
		{
			Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);

			goto MATERIAL_LOAD_ERROR;
		}

		mftMaterialState = MATERIAL_FUNCTION_MATERIAL;

		// End
		if (cToken[0] == '}')
			return mNewMaterial;
		// Start
		else if (cToken[0] == SCRIPT_SYMBOL_FUNCTION)
			Material_CheckFunctions(mNewMaterial);
	}

MATERIAL_LOAD_ERROR:
	Z_Free(cData);

	return NULL;
}

/**/

void Material_Shutdown(void)
{
	Con_Printf("Shutting down material system...\n");
}