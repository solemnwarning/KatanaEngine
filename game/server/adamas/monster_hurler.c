/*	Copyright (C) 2014 OldTimes Software / Mark Elsworth Sowden
*/

#include "server_monster.h"

#include "server_effects.h"

/*
	Small little blob thing that throws itself around.
*/

#define	HURLER_MAX_HEALTH	250
#define	HURLER_MAX_DELAY	7.0f;

double dHurlerSpawnDelay = HURLER_MAX_DELAY;

int	iHurlers = 0;

void Hurler_Pain(ServerEntity_t *eHurler,ServerEntity_t *eOther);
void Hurler_Think(ServerEntity_t *eHurler);
void Hurler_Touch(ServerEntity_t *eHurler,ServerEntity_t *eOther);
void Hurler_Die(ServerEntity_t *eHurler,ServerEntity_t *eOther);

void Hurler_Spawn(ServerEntity_t *eHurler)
{
	Server_PrecacheModel(HURLER_MODEL_BODY);

	Entity_SetModel(eHurler,HURLER_MODEL_BODY);

	eHurler->Monster.iType = MONSTER_HURLER;
	eHurler->Monster.Pain = Hurler_Pain;
	eHurler->Monster.Frame = Hurler_Think;

	Entity_SetKilledFunction(eHurler, Hurler_Die);

	eHurler->Physics.fMass		= 0.5f;
	eHurler->Physics.iSolid		= SOLID_SLIDEBOX;
	eHurler->Physics.fFriction	= 10.0f;
	eHurler->Physics.fGravity	= SERVER_GRAVITY;

	eHurler->v.bTakeDamage		= true;
	eHurler->v.movetype			= MOVETYPE_BOUNCE;
	eHurler->v.iHealth			= HURLER_MAX_HEALTH;
	eHurler->local.iMaxHealth = HURLER_MAX_HEALTH;
	eHurler->v.netname			= "Hurler";
	eHurler->v.TouchFunction	= Hurler_Touch;

	Monster_SetState(eHurler, MONSTER_STATE_AWAKE);
	Monster_SetThink(eHurler, MONSTER_THINK_IDLE);

	// Add to the count of Hurlers currently in the game. ~hogsy
	iHurlers++;

//	DropToFloor(eHurler);
}

void Hurler_Think(ServerEntity_t *eHurler)
{
	switch (eHurler->Monster.think)
	{
	case MONSTER_THINK_IDLE:
		Monster_Jump(eHurler,70.0f);

		if(rand()%50 == 0)
			eHurler->v.angles[1] = (float)(rand()%360);

		// Add some random movement.
		if(rand()%120 == 0)
		{
			int	iResult = rand()%3;

			if(iResult == 0)
				eHurler->v.velocity[0] += 30.0f;
			else if(iResult == 1)
				eHurler->v.velocity[0] -= 30.0f;

			iResult = rand()%3;
			if(iResult == 0)
				eHurler->v.velocity[1] += 30.0f;
			else if(iResult == 1)
				eHurler->v.velocity[1] -= 30.0f;
		}

		if(!eHurler->Monster.eEnemy)
		{
			eHurler->Monster.eTarget = Monster_GetTarget(eHurler);
			if(!eHurler->Monster.eTarget)
				return;

			eHurler->Monster.eEnemy = Monster_GetEnemy(eHurler);
			if (eHurler->Monster.eEnemy)
				Monster_SetThink(eHurler, MONSTER_THINK_PURSUING);
		}
		break;
	case MONSTER_THINK_ATTACKING:
		// Throw us in the direction of the enemy. ~hogsy
		if (Entity_IsOnGround(eHurler))
		{
			MathVector3f_t	vOrigin;

			Monster_Jump(eHurler,150.0f);

			Math_VectorSubtract(eHurler->Monster.eEnemy->v.origin,eHurler->v.origin,vOrigin);
			Math_VectorDivide(vOrigin,5.0f,vOrigin);
			Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);
		}

		if(MONSTER_GetRange(eHurler,eHurler->Monster.eEnemy->v.origin) > MONSTER_RANGE_MELEE)
			Monster_SetThink(eHurler, MONSTER_THINK_PURSUING);
		break;
	case MONSTER_THINK_PURSUING:
		if(!eHurler->Monster.eEnemy || eHurler->Monster.eEnemy->v.iHealth <= 0)
		{
			Monster_SetThink(eHurler, MONSTER_THINK_IDLE);
			return;
		}

		if(dHurlerSpawnDelay < Server.dTime)
		{
			trace_t			tSpawnTraceCheck;
			MathVector3f_t	vNewOrigin;

			Math_VectorCopy(eHurler->v.origin,vNewOrigin);
			// Bump it up a lil' ~hogsy
			vNewOrigin[2] += 60.0f;
				
			tSpawnTraceCheck = Engine.Server_Move(eHurler->v.origin,eHurler->v.mins,eHurler->v.maxs,vNewOrigin,MOVE_NOMONSTERS,eHurler);
			if(!tSpawnTraceCheck.bAllSolid || tSpawnTraceCheck.bStartSolid)
			{
				ServerEntity_t *eNewHurler = Entity_Spawn();

				Math_VectorCopy(vNewOrigin,eNewHurler->v.origin);

				Hurler_Spawn(eNewHurler);

				if (!Entity_DropToFloor(eNewHurler))
				{
					Entity_Remove(eNewHurler);
					iHurlers--;
				}
				else
					// Remember to update monster count... Ugh ~hogsy
					Server.iMonsters++;
			}

			dHurlerSpawnDelay = Server.dTime+HURLER_MAX_DELAY;
		}

		// Throw us in the direction of the enemy. ~hogsy
		if(eHurler->v.flags & FL_ONGROUND)
		{
			vec3_t	vOrigin;

			Monster_Jump(eHurler,85.0f);

			Math_VectorSubtract(eHurler->Monster.eEnemy->v.origin,eHurler->v.origin,vOrigin);
			Math_VectorDivide(vOrigin,2.0f,vOrigin);
			Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);
		}

		{
			vec3_t	vAngles;

			// Update angles. ~hogsy
			Math_VectorSubtract(eHurler->v.origin,eHurler->Monster.eEnemy->v.origin,vAngles);
			Math_VectorNormalize(vAngles);
			Math_VectorInverse(vAngles);
			Math_MVToVector(Math_VectorToAngles(vAngles),eHurler->v.angles);

			eHurler->v.angles[0] = 0;
		}
		break;
	}
}

void Hurler_Touch(ServerEntity_t *eHurler,ServerEntity_t *eOther)
{
	if(Entity_IsPlayer(eOther))
	{
		if (eHurler->Monster.think != MONSTER_THINK_ATTACKING)
		{
			Monster_SetThink(eHurler, MONSTER_THINK_ATTACKING);
			return;
		}

		{
			vec3_t vOrigin;

			Entity_Damage(eOther, eHurler, 15, DAMAGE_TYPE_NONE);

			// Throw it back. ~hogsy
			Math_VectorSubtract(eHurler->Monster.eEnemy->v.origin,eHurler->v.origin,vOrigin);
			Math_VectorScale(vOrigin,4.0f,vOrigin);
			Math_VectorInverse(vOrigin);
			Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);

			Monster_SetThink(eHurler, MONSTER_THINK_PURSUING);
		}
	}
}

void Hurler_Pain(ServerEntity_t *eHurler,ServerEntity_t *eOther)
{
	vec3_t	vOrigin;
	char	cBlood[6];

	ServerEffect_BloodSpray(eHurler->v.origin);

	// Throw it back. ~hogsy
	Math_VectorSubtract(eOther->v.origin,eHurler->v.origin,vOrigin);
	Math_VectorScale(vOrigin,2.0f,vOrigin);
	Math_VectorInverse(vOrigin);
	Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);
	Math_MVToVector(Math_VectorToAngles(vOrigin),eHurler->v.angles);

	Monster_SetThink(eHurler, MONSTER_THINK_PURSUING);
}

void Hurler_Die(ServerEntity_t *eHurler,ServerEntity_t *eOther)
{
	Entity_Remove(eHurler);
}