/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERMISC__
#define __SERVERMISC__

#include "server_main.h"

/*	Spawn point types.
*/
enum
{
	INFO_PLAYER_START,
	INFO_PLAYER_COOP,
	INFO_PLAYER_DEATHMATCH,
	INFO_PLAYER_CTF,

	// Companions
	INFO_PLAYER_MIKIKO,
	INFO_PLAYER_SUPERFLY
};

void Bot_Spawn(ServerEntity_t *ent);

void CTF_FlagSpawn(ServerEntity_t *eFlag);

void Area_PlayerSpawn(ServerEntity_t *eArea);
void Area_BreakableSpawn(ServerEntity_t *eArea);
void Area_ButtonSpawn(ServerEntity_t *eArea);
void Area_ChangeLevel(ServerEntity_t *eArea);
void Area_ClimbSpawn(ServerEntity_t *eArea);
void Area_DoorSpawn(ServerEntity_t *eArea);
void Area_NoclipSpawn(ServerEntity_t *eArea);
void Area_PlatSpawn(ServerEntity_t *eArea);
void Area_PushSpawn(ServerEntity_t *eArea);
void Area_PushableSpawn(ServerEntity_t *eArea);
void Area_PlatformSpawn(ServerEntity_t *eArea);
void Area_RotateSpawn(ServerEntity_t *eArea);
void Area_TriggerSpawn(ServerEntity_t *eArea);
void Area_WallSpawn(ServerEntity_t *eArea);
void Area_DetailSpawn(ServerEntity_t *eArea);
void Area_KillSpawn(ServerEntity_t *area);

void Point_AmbientSpawn(ServerEntity_t *eEntity);
void Point_DecorationSpawn(ServerEntity_t *eDecoration);
void Point_DynamicLight(ServerEntity_t *ent);
void Point_DamageSpawn(ServerEntity_t *ent);
void Point_EffectSpawn(ServerEntity_t *eFlare);
void Point_FlareSpawn(ServerEntity_t *eFlare);
void Point_MessageSpawn(ServerEntity_t *eEntity);
void Point_LightstyleSpawn(ServerEntity_t *eEntity);
void Point_LogicSpawn(ServerEntity_t *eEntity);
void Point_VehicleSpawn(ServerEntity_t *eVehicle);
void Point_MonsterSpawn(ServerEntity_t *eMonster);
void Point_MultiTriggerSpawn(ServerEntity_t *eEntity);
void Point_NullSpawn(ServerEntity_t *eEntity);
void Point_SkyCameraSpawn(ServerEntity_t *entity);
void Point_ParticleSpawn(ServerEntity_t *ent);
void Point_PropSpawn(ServerEntity_t *eEntity);
void Point_SoundSpawn(ServerEntity_t *eEntity);
void Point_Start(ServerEntity_t *ent);
void Point_ShakeSpawn(ServerEntity_t *ent);
void Point_TeleportSpawn(ServerEntity_t *ent);
void Point_TimedTriggerSpawn(ServerEntity_t *eEntity);
void Point_WaypointSpawn(ServerEntity_t *eEntity);
void Point_LightSpawn(ServerEntity_t *eLight);
void Point_SpriteSpawn(ServerEntity_t *seSprite);
void Point_ExplodeSpawn(ServerEntity_t *ePoint);

// Weapons
void WEAPON_StickThink(ServerEntity_t *ent);

void ThrowGib(MathVector3f_t origin, MathVector3f_t velocity,char *model,float damage,bool bleed);
void Item_Respawn(ServerEntity_t *ent);

void Client_RelinkEntities(entity_t *ent,int i,double dTime);

#endif
