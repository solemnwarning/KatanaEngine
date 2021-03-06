/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2016 OldTimes Software

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

#include "engine_base.h"

#include "video.h"
#include "video_shader.h"

extern ConsoleVariable_t r_drawflat, gl_fullbrights, r_lerpmodels, r_lerpmove; //johnfitz

bool	bShading;		//johnfitz -- if false, disable vertex shading for various reasons (fullbright, r_lightmap, showtris, etc)

MathVector3f_t	alias_lightcolour, oldlightcolour;
MathVector3f_t	lightposition, oldlightposition;

void Alias_SetupLighting(ClientEntity_t *entity)
{
	Math_VectorSet(1.0f, alias_lightcolour);

	if(!bShading || r_showtris.bValue)
		return;

	if (!(entity->effects & EF_FULLBRIGHT))
	{
		Math_VectorSet(0, alias_lightcolour);

		// Check to see if we can grab the light source, for directional information.
		DynamicLight_t *dlight = Light_GetDynamic(entity->origin, true);
		if (dlight)
		{
			MathVector3f_t distance;

			// Approximate light intensity based on distance.
			Math_VectorSubtract(entity->origin, dlight->origin, distance);
			Math_VectorCopy(dlight->color, alias_lightcolour);
			Math_VectorSubtractValue(alias_lightcolour, (dlight->radius - Math_Length(distance)) / 1000.0f, alias_lightcolour);
			Math_VectorScale(alias_lightcolour, 1.0f / 200.0f, alias_lightcolour);
			//Math_VectorDivide(alias_lightcolour, 0.3f, alias_lightcolour);


#if 0	// TODO: DO THIS ON A PER ENTITY BASIS, RATHER THAN PER MESH!!!!
			VideoShader_SetVariable3f(iLightPositionUniform, dlLightSource->origin[0], dlLightSource->origin[1], dlLightSource->origin[2]);
			VideoShader_SetVariable3f(iLightColourUniform, alias_lightcolour[0], alias_lightcolour[1], alias_lightcolour[2]);
#endif
		}
		else
		{
			Math_MVToVector(Light_GetSample(entity->origin), alias_lightcolour);
			Math_VectorScale(alias_lightcolour, 1.0f / 200.0f, alias_lightcolour);

#if 0	// TODO: DO THIS ON A PER ENTITY BASIS, RATHER THAN PER MESH!!!!
			VideoShader_SetVariable3f(iLightPositionUniform, entity->origin[0], entity->origin[1], entity->origin[2]);
			VideoShader_SetVariable3f(iLightColourUniform, alias_lightcolour[0], alias_lightcolour[1], alias_lightcolour[2]);
#endif
		}
	}

	// Minimum light value on players (8)
	if (entity > cl_entities && entity <= cl_entities + cl.maxclients)
	{
		float mod = 24.0f - (alias_lightcolour[0] + alias_lightcolour[1] + alias_lightcolour[2]);
		if (mod > 0.0f)
			Math_VectorAddValue(alias_lightcolour, mod / 3.0f, alias_lightcolour);
	}
}

void Alias_DrawFrame(MD2_t *mModel, ClientEntity_t *entity, lerpdata_t lLerpData)
{
	VideoObjectVertex_t	*voModel;
	MD2TriangleVertex_t	*verts1, *verts2;
	MD2Frame_t			*frame1, *frame2;
	float				ilerp, fAlpha = 1.0f;
	unsigned int		uiVerts = 0;
	int					*order, count;

	ilerp = 1.0f - lLerpData.blend;
	if (bShading && !r_showtris.bValue)
		fAlpha	= ENTALPHA_DECODE(entity->alpha);

	//new version by muff - fixes bug, easier to read, faster (well slightly)
	frame1 = (MD2Frame_t*)((uint8_t*)mModel + mModel->ofs_frames + (mModel->framesize*entity->draw_lastpose));
	frame2 = (MD2Frame_t*)((uint8_t*)mModel + mModel->ofs_frames + (mModel->framesize*entity->draw_pose));

#if 0
	if ((entity->scale != 1.0f) && (entity->scale > 0.1f))
		VideoShader_SetVariablef(iScaleUniform, entity->scale);
	else
		VideoShader_SetVariablef(iScaleUniform, 1.0f);
#endif

	verts1 = &frame1->verts[0];
	verts2 = &frame2->verts[0];

	order = (int*)((uint8_t*)mModel + mModel->ofs_glcmds);

	// TODO: Stupid stupid stupid temporary shit until we do this properly.
	voModel = (VideoObjectVertex_t*)malloc(sizeof(VideoObjectVertex_t) * mModel->num_xyz);
	if (!voModel)
		Sys_Error("Failed to allocate data for model frame!\n");
	memset(voModel, 0, sizeof(voModel));

	for (;;)
	{
		count = *(order++);
		if (!count)
			break;

		uiVerts = 0;

		do
		{
			Video_ObjectTexture(&voModel[uiVerts], 0, ((float*)order)[0], ((float*)order)[1]);
			Video_ObjectVertex(&voModel[uiVerts],
				(verts1[order[2]].v[0] * frame1->scale[0] + frame1->translate[0])*ilerp +
				(verts2[order[2]].v[0] * frame2->scale[0] + frame2->translate[0])*lLerpData.blend,
				(verts1[order[2]].v[1] * frame1->scale[1] + frame1->translate[1])*ilerp +
				(verts2[order[2]].v[1] * frame2->scale[1] + frame2->translate[1])*lLerpData.blend,
				(verts1[order[2]].v[2] * frame1->scale[2] + frame1->translate[2])*ilerp +
				(verts2[order[2]].v[2] * frame2->scale[2] + frame2->translate[2])*lLerpData.blend);
			Video_ObjectNormal(&voModel[uiVerts],
				entity->model->object.vertices[uiVerts].mvNormal[0],
				entity->model->object.vertices[uiVerts].mvNormal[1],
				entity->model->object.vertices[uiVerts].mvNormal[2]);
			Video_ObjectColour(&voModel[uiVerts], alias_lightcolour[0], alias_lightcolour[1], alias_lightcolour[2], fAlpha);

			uiVerts++;

			order += 3;
		} while (--count);

		Video_DrawObject(voModel, VIDEO_PRIMITIVE_TRIANGLE_STRIP, uiVerts, entity->model->mAssignedMaterials, entity->skinnum);
	}

	free(voModel);
}

void Alias_SetupFrame(MD2_t *mModel,ClientEntity_t *ceCurrent,lerpdata_t *ldLerp)
{
	if ((ceCurrent->frame >= mModel->num_frames) || (ceCurrent->frame < 0))
	{
		Con_Warning("No such frame! (%d) (%s)\n", ceCurrent->frame, ceCurrent->model->name);

		ceCurrent->frame = 0;
	}

	if (r_lerpmodels.value >= 1 && ceCurrent->draw_lastmodel == ceCurrent->model)
	{
		if (ceCurrent->frame != ceCurrent->draw_pose)
		{
			ceCurrent->draw_lastpose = ceCurrent->draw_pose;
			ceCurrent->draw_pose = ceCurrent->frame;
			ceCurrent->draw_lerpstart = cl.time;

			ldLerp->blend = 0;
		}
		else
			ldLerp->blend = (cl.time - ceCurrent->draw_lerpstart)*20.0;
	}
	else
	{
		ceCurrent->draw_lastmodel = ceCurrent->model;
		ceCurrent->draw_lastpose = ceCurrent->draw_pose = ceCurrent->frame;
		ceCurrent->draw_lerpstart = cl.time;

		ldLerp->blend = 0;
	}

	if(ldLerp->blend > 1.0f)
		ldLerp->blend = 1.0f;
}

void Alias_SetupEntityTransform(ClientEntity_t *ceEntity, lerpdata_t *lerpdata)
{
	float			blend;
	int				i;
	MathVector3f_t	d;

	// if LERP_RESETMOVE, kill any lerps in progress
	if (ceEntity->lerpflags & LERP_RESETMOVE)
	{
		ceEntity->movelerpstart = 0;

		Math_VectorCopy(ceEntity->origin, ceEntity->previousorigin);
		Math_VectorCopy(ceEntity->origin, ceEntity->currentorigin);
		Math_VectorCopy(ceEntity->angles, ceEntity->previousangles);
		Math_VectorCopy(ceEntity->angles, ceEntity->currentangles);

		ceEntity->lerpflags -= LERP_RESETMOVE;
	}
	else if (!Math_VectorCompare(ceEntity->origin, ceEntity->currentorigin) || !Math_VectorCompare(ceEntity->angles, ceEntity->currentangles)) // origin/angles changed, start new lerp
	{
		ceEntity->movelerpstart = cl.time;

		Math_VectorCopy(ceEntity->currentorigin, ceEntity->previousorigin);
		Math_VectorCopy(ceEntity->origin, ceEntity->currentorigin);
		Math_VectorCopy(ceEntity->currentangles, ceEntity->previousangles);
		Math_VectorCopy(ceEntity->angles, ceEntity->currentangles);
	}

	//set up values
	if (r_lerpmove.value && ceEntity != &cl.viewent && ceEntity->lerpflags & LERP_MOVESTEP)
	{
		if (ceEntity->lerpflags & LERP_FINISH)
			blend = Math_Clamp(0, (cl.time - ceEntity->movelerpstart) / (ceEntity->lerpfinish - ceEntity->movelerpstart), 1);
		else
			blend = Math_Clamp(0, (cl.time - ceEntity->movelerpstart) / 0.1, 1);

		//translation
		Math_VectorSubtract(ceEntity->currentorigin, ceEntity->previousorigin, d);
		lerpdata->origin[0] = ceEntity->previousorigin[0] + d[0] * blend;
		lerpdata->origin[1] = ceEntity->previousorigin[1] + d[1] * blend;
		lerpdata->origin[2] = ceEntity->previousorigin[2] + d[2] * blend;

		//rotation
		Math_VectorSubtract(ceEntity->currentangles, ceEntity->previousangles, d);
		for(i = 0; i < 3; i++)
		{
			if (d[i] > 180.0f)
				d[i] -= 360.0f;
			if (d[i] < -180.0f)
				d[i] += 360.0f;
		}

		Math_VectorMA(ceEntity->previousangles, blend, d, lerpdata->angles);
	}
	else //don't lerp
	{
		Math_VectorCopy(ceEntity->origin, lerpdata->origin);
		Math_VectorCopy(ceEntity->angles, lerpdata->angles);
	}
}

/*	Draw the alias model.
*/
void Alias_Draw(ClientEntity_t *eEntity)
{
	lerpdata_t	lLerpData;
	MD2_t		*mModel;

	if (!cv_video_drawmodels.value || R_CullModelForEntity(eEntity))
		return;

	// Set defaults.
	bShading = true;

	mModel = (MD2_t*)Mod_Extradata(eEntity->model);

	rs_aliaspolys += mModel->numtris;

	Alias_SetupFrame(mModel,eEntity,&lLerpData);
	Alias_SetupEntityTransform(eEntity, &lLerpData);

	glPushMatrix();

	R_RotateForEntity(eEntity->origin,eEntity->angles);

	Alias_SetupLighting(eEntity);
	Alias_DrawFrame(mModel, eEntity, lLerpData);

	if(r_drawflat_cheatsafe)
		// Restore randomness
		srand((int)(cl.time*1000));

	glPopMatrix();
}
