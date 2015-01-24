/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "engine_videoshadow.h"	// [7/8/2013] TODO: Change to engine_video ~hogsy

/*
	Contains all basic drawing routines.
	TODO:
		Rename to engine_draw!
*/

#include "KatAlias.h"
#include "engine_editor.h"
#include "engine_console.h"
#include "engine_client.h"
#include "engine_video.h"	// [7/8/2013] TODO: Remove ~hogsy

cvar_t	cvDrawFlares	= {	"video_flares",			"1",					    true,   false,  "Toggles the rendering of environmental flares."	        },
		cvLitParticles	= {	"video_particles_lit",	"0",	                    true,   false,  "Sets whether or not particles are lit by dynamic lights."	},
		cvShadowPath	= {	"video_shadows_path",	"textures/engine/shadow",   false,  false,  "Changes the texture path used for blob shadows."	        };

gltexture_t	/**gRenderTarget,*/*gShadow;

extern vec3_t	lightspot;
extern entity_t *currententity;

void Shadow_Initialize(void)
{
	unsigned    int	iWidth,iHeight;
	byte            *bShadowData;

	Cvar_RegisterVariable(&cvLitParticles,NULL);
	Cvar_RegisterVariable(&cvDrawFlares,NULL);
	Cvar_RegisterVariable(&cvShadowPath,NULL);

	flares		= (flare_t*)Hunk_AllocName(MAX_FLARES*sizeof(flare_t),"flares");

	bShadowData = Image_LoadImage(cvShadowPath.string,&iWidth,&iHeight);
	if(bShadowData)
		gShadow	= TexMgr_LoadImage(NULL,cvShadowPath.string,iWidth,iHeight,SRC_RGBA,bShadowData,cvShadowPath.string,0,TEXPREF_ALPHA|TEXPREF_PERSIST);
	else
		Con_Warning("Failed to load %s!\n",cvShadowPath.string);
}

void R_DrawString(int x,int y,char *msg)
{
	if(y <= -8)
		return;

	while(*msg)
	{
		Draw_Character(x,y,*msg);
		msg++;
		x += 8;
	}
}

/*	Draw multiple shadow types.
	TODO:
		Fade out blob shadow based on distance from ground.
		Shadow maps.
*/
void Draw_Shadow(entity_t *ent)
{
	lerpdata_t		lerpdata;
	float			fShadowMatrix[16] =
	{	1,		0,		0,		0,
		0,		1,		0,		0,
		0,		0,		0,		0,
		0,		0,		0.1f,	1	};
	float			lheight,fShadowScale[2],
					fShadowAlpha = 0;

	if(ENTALPHA_DECODE(ent->alpha) <= 0)
		return;

#if 0
	fShadowScale[0] = ent->model->maxs[0]-15;
	fShadowScale[1] = ent->model->maxs[1]-15;
#else
	fShadowScale[0] = fShadowScale[1] = 20.0f;
#endif

	if(ent == &cl.viewent || R_CullModelForEntity(ent) || (!fShadowScale[0] || !fShadowScale[1]))
		return;

	// Allow us to cast shadows from entities that use bmodels. ~hogsy
	// TODO: Add a flag for this, rather than checking a string everytime... ~hogsy
	if(!strstr(ent->model->name,".") || !strstr(ent->model->name,"/"))
		return;

	Light_GetSample(ent->origin);

	lheight = ent->origin[2]-lightspot[2];

	{
		VideoObject_t voShadow[4]=
		{
			{	{	-fShadowScale[0],	fShadowScale[1],	0	},	{	{	0,		0,		}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	fShadowScale[0],	fShadowScale[1],	0	},	{	{	1.0f,	0		}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	fShadowScale[0],	-fShadowScale[1],	0	},	{	{	1.0f,	1.0f	}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	-fShadowScale[0],	-fShadowScale[1],	0	},	{	{	0,		1.0f	}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	}
		};

		Video_ResetCapabilities(false);

		Video_SetTexture(gShadow);

		glPushMatrix();

		Video_EnableCapabilities(VIDEO_BLEND);

		Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_FALSE);

		glTranslatef(ent->origin[0],ent->origin[1],ent->origin[2]);
		glTranslatef(0,0,-lheight+0.1f);

		Video_DrawFill(voShadow,NULL);

		glTranslatef(0,0,lheight+0.1);
		glPopMatrix();

		Video_ResetCapabilities(true);
	}

	if((r_shadows.value >= 2) && (ent->model->mType == MODEL_TYPE_MD2))
	{
		MD2_t			*pmd2;
		DynamicLight_t	*dlLight;
		vec3_t			vDistance;

		pmd2 = (MD2_t*)Mod_Extradata(ent->model);

		Alias_SetupFrame(pmd2,&lerpdata);

		bShading = false;

		fShadowMatrix[8] =
		fShadowMatrix[9] = 0;

		dlLight = Light_GetDynamic(ent->origin);
		if(!dlLight)
			return;

		Math_VectorSubtract(ent->origin,dlLight->origin,vDistance);

		fShadowAlpha = (dlLight->radius-Math_Length(vDistance))/100.0f;
		if(fShadowAlpha <= 0)
			return;

		fShadowMatrix[8] = vDistance[0]/100.0f;
		fShadowMatrix[9] = vDistance[1]/100.0f;

		glPushMatrix();

		Video_ResetCapabilities(false);

		Video_EnableCapabilities(VIDEO_BLEND|VIDEO_STENCIL_TEST);
		Video_DisableCapabilities(VIDEO_TEXTURE_2D);
		Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_FALSE);

		glStencilFunc(GL_EQUAL,1,2);
		Video_SetColour(0, 0, 0, fShadowAlpha);
		glTranslatef(ent->origin[0],ent->origin[1],ent->origin[2]);
		glTranslatef(0,0,-lheight);
		glMultMatrixf(fShadowMatrix);
		glTranslatef(0,0,lheight);
		glRotatef(ent->angles[1],0,0,1);
		glRotatef(ent->angles[0],0,1,0);
		glRotatef(ent->angles[2],1,0,0);
		glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);

		Alias_DrawFrame(pmd2, ent, lerpdata);

		glPopMatrix();

		Video_ResetCapabilities(true);
	}
}
