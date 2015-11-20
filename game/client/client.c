/*	Copyright (C) 2011-2015 OldTimes Software

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

#include "client_main.h"

#include "client_effects.h"

/*
	Only functions that are called by the engine should
	exist in here.
*/

/*	Initialize any client-side specific variables.
*/
void Client_Initialize(void)
{
}

/*	Crappy the sticky renderer loop!
	This lets us draw images and text to the screen... Forcefully.
*/
void Client_Draw(void)
{
}

/*	Parse temporary entity types, useful for particles and other small crap.
*/
void Client_ParseTemporaryEntity(void)
{
	int	type;

	// Set iType, so if the type is missing we can mention it below.
	type = Engine.ReadByte();

	switch (type)
	{
	case CTE_EXPLOSION:
		{
			int				i;
			MathVector3f_t	position;

			for(i = 0; i < 3; i++)
				position[i] = Engine.ReadCoord();

			Effect_Explosion(position);
		}
		break;
	case CTE_BLOODSPRAY:
		{
			int			i,j;
			char		cBlood[12];
			vec3_t		vPosition;
			Particle_t	*pBloodSpray;

			for(i = 0; i < 3; i++)
				vPosition[i] = Engine.ReadCoord();

			for(i = 0; i < 24; i++)
			{
				pBloodSpray = Engine.Client_AllocateParticle();
				if(!pBloodSpray)
					return;

				// Keep the textures random.
				PARTICLE_BLOOD(cBlood);

				Math_VectorSet(1.0f,pBloodSpray->vColour);
				Math_VectorCopy(vPosition,pBloodSpray->vOrigin);

				pBloodSpray->lifetime = (float)(Client.time + 5.0);
				pBloodSpray->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
				pBloodSpray->fRamp		= (float)(rand()&3);
				pBloodSpray->fScale		= (float)(rand()%8+1);
				pBloodSpray->iMaterial	= Engine.Client_GetEffect(cBlood);
				
				for(j = 0; j < 3; j++)
					pBloodSpray->vVelocity[j] = (float)((rand()%512)-256);
			}
		}
		break;
	case CTE_PARTICLE_FIELD:
		break;
	default:
		Engine.Con_Warning("Unknown temporary entity type! (%i)\n", type);
	}
}

/*	Called by the engine.
*/
void Client_RelinkEntities(ClientEntity_t *entity,int i,double dTime)
{
	MathVector3f_t	f,r,u;
	DynamicLight_t	*dLight;

	// [6/5/2012] Keep client time updated (temp) ~hogsy
	// [5/6/2012] TODO: Move over to somewhere more appropriate please ~hogsy
	Client.time = dTime;

	if (entity->effects & EF_MOTION_FLOAT)
		entity->origin[2] += ((float)sin(Client.time*2.0f))*5.0f;

	if (entity->effects & EF_MOTION_ROTATE)
		entity->angles[YAW] = Math_AngleMod((float)(100.0*Client.time));

	if (entity->effects & EF_PARTICLE_SMOKE)
	{
		int j;

		if (rand()%5 == 0)
		{
			Particle_t	*pParticle;
			
			pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->iMaterial = Engine.Client_GetEffect(va("smoke%i", rand() % 4));
			pParticle->fRamp = (float)(rand() & 3);
			pParticle->fScale = ((float)(rand() % 15) * 2);
			pParticle->lifetime = (Client.time + (rand() % 5));
			pParticle->pBehaviour = PARTICLE_BEHAVIOUR_SMOKE;
			
			// [5/9/2012] TODO: Simplify this ~hogsy
			for(j = 0; j < 3; j++)
				pParticle->vOrigin[j] = entity->origin[j] + ((rand() & 8) - 5.0f);

			Math_VectorClear(pParticle->vVelocity);
		}
	}

	// [23/11/2013] Simple bloody particle effect, which is now done client-side! Yay! ~hogsy
	if (entity->effects & EF_PARTICLE_BLOOD)
	{
		char	cBlood[12];
		int		k,j;

		for(k = 0; k < 2; k++)
		{
			Particle_t *pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			// Keep the textures random.
			PARTICLE_BLOOD(cBlood);

			pParticle->iMaterial	= Engine.Client_GetEffect(cBlood);
			pParticle->fRamp		= (float)(rand()&4);
			pParticle->fScale		= (float)(rand()%2+2);
			pParticle->lifetime = (float)(Client.time + 0.3*(rand() % 5));
			pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_GRAVITY;
			
			for(j = 0; j < 3; j++)
				pParticle->vOrigin[j] = entity->origin[j] + ((rand() & 15) - 5.0f);

			pParticle->vVelocity[2] = 15.0f;
			//Math_VectorScale(eClient->angles,15.0f,pParticle->vel);
		}
	}

	if (entity->effects & EF_LIGHT_GREEN)
	{
		int k,j;

		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius = (float)(rand() % 20) * 10;
		dLight->color[RED] = 0;
		dLight->color[GREEN] = 255.0f;
		dLight->color[BLUE] = 0;
		dLight->minlight = 16.0f;
		dLight->die = (float)(Client.time + 0.01);
		dLight->lightmap		 = true;

		for(k = 0; k < 4; k++)
		{
			Particle_t *pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->iMaterial = Engine.Client_GetEffect("spark2");
			pParticle->fRamp = (float)(rand() & 3);
			pParticle->fScale = 1.5f + (float)(rand() % 15 / 10);
			pParticle->lifetime = (Client.time + (double)(rand() % 2));
			pParticle->pBehaviour = PARTICLE_BEHAVIOUR_SLOWGRAVITY;
			
			// [5/9/2012] TODO: Simplify this ~hogsy
			for(j = 0; j < 3; j++)
				pParticle->vOrigin[j] = entity->origin[j] + ((rand() & 15) - 5.0f);

			Math_VectorClear(pParticle->vVelocity);
		}
	}

	if (entity->effects & EF_LIGHT_BLUE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius			= 120.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= (Client.time+0.01);
		dLight->lightmap		= true;
	}

	if (entity->effects & EF_LIGHT_RED)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius			= 120.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 32.0f;
		dLight->die				= Client.time+0.01;
		dLight->lightmap		= true;
	}

	if (entity->effects & EF_MUZZLEFLASH)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->origin[2] += 16.0f;

		Math_AngleVectors(entity->angles, f, r, u);
		Math_VectorMA(dLight->origin,18,f,dLight->origin);
		
		dLight->radius			= 170.0f+(rand()&31);
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 50.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= Client.time+0.1;
		dLight->lightmap		= true;
	}

	if (entity->effects & EF_DIMLIGHT)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius			= 200.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= Client.time+0.01;
		dLight->lightmap		= true;
	}

	if (entity->effects & EF_BRIGHTLIGHT)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);
		// [22/4/2013] Simplified ~hogsy
		Math_VectorSet(255.0f,dLight->color);

		dLight->origin[2]		+= 16.0f;
		dLight->radius			= 300.0f+(rand()&31);
		dLight->minlight		= 32.0f;
		dLight->die				= Client.time + 0.001;
		dLight->lightmap		= true;
	}

	if (entity->effects & EF_GLOW_RED)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 16.0f;
		dLight->die				= Client.time + 0.01f;
		dLight->lightmap		= true;
	}

/*	if(ent->effects & EF_GLOW_GREEN)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(ent->origin,dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 16.0f;
		dLight->die				= Client.time+0.01f;
		dLight->lightmap		= true;
	}*/

	if (entity->effects & EF_GLOW_BLUE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 16.0f;
		dLight->die				= (Client.time+0.01);
		dLight->lightmap	= true;
	}

	if (entity->effects & EF_GLOW_WHITE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(entity->origin, dLight->origin);
		// [22/4/2013] Simplified ~hogsy
		Math_VectorSet(255.0f,dLight->color);

		dLight->radius		= (float)sin(Client.time*2.0f)*100.0f;
		dLight->minlight	= 16.0f;
		dLight->die			= (Client.time+0.01);
		dLight->lightmap	= true;
	}

	entity->bForceLink = false;
}