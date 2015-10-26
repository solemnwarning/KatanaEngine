// map.c

#include "bsp5.h"

// just for statistics
int			nummapbrushfaces;

int			nummapbrushes;
mbrush_t	mapbrushes[BSP_MAX_BRUSHES];

int			nummapplanes;
plane_t		mapplanes[BSP_MAX_PLANES];

int			nummiptex;
char		miptex[BSP_MAX_TEXINFO][128]; // LordHavoc: was [16]

int mapversion = 0;

/*	Returns a global plane number and the side that will be the front
*/
int	FindPlane( plane_t *dplane, int *side )
{
	int			i;
	plane_t		*dp, pl;
	vec_t		dot;

	pl = *dplane;
	NormalizePlane( &pl );

	if( DotProduct( pl.normal, dplane->normal ) > 0 )
		*side = 0;
	else
		*side = 1;

	for( i = 0, dp = mapplanes; i < nummapplanes; i++, dp++ )
		if( DotProduct( dp->normal, pl.normal ) > 1.0 - ANGLE_EPSILON && fabs( dp->dist - pl.dist ) < DIST_EPSILON )
			return i; // regular match

	if( nummapplanes == BSP_MAX_PLANES )
		Error( "FindPlane: nummapplanes == BSP_MAX_PLANES" );

	dot = VectorLength( dplane->normal );
	if( dot < 1.0 - ANGLE_EPSILON || dot > 1.0 + ANGLE_EPSILON )
		Error( "FindPlane: normalization error (%f %f %f, length %f)", dplane->normal[0], dplane->normal[1], dplane->normal[2], dot );

	mapplanes[nummapplanes] = pl;

	return nummapplanes++;
}

int FindMiptex( char *name )
{
	int		i;

	for( i = 0; i < nummiptex; i++ )
		if( !strcmp( name, miptex[i] ) )
			return i;

	if( nummiptex == BSP_MAX_TEXINFO )
		Error ("nummiptex == BSP_MAX_TEXINFO");

	p_strcpy(miptex[i], name);

	return nummiptex++;
}

/*	Returns a global texinfo number
*/
int	FindTexinfo( BSPTextureInfo_t *t )
{
	unsigned int		i;
	int					j;
	BSPTextureInfo_t	*tex;

	// Set the special flag for any "special" cases.
	if(/*(miptex[t->iMipTex][0] == '*' && !waterlightmap) || */!Q_strncasecmp (miptex[t->iMipTex],"sky",3))
		t->iFlags |= BSP_TEXTURE_SPECIAL;

	tex = texinfo;
	for( i = 0; i < numtexinfo; i++, tex++ )
	{
		if( t->iMipTex != tex->iMipTex )
			continue;
		if( t->iFlags != tex->iFlags )
			continue;

		for( j = 0; j < 8; j++ )
			if( t->v[0][j] != tex->v[0][j] )
				break;

		if( j != 8 )
			continue;

		return i;
	}

	// allocate a new texture
	if( numtexinfo == BSP_MAX_TEXINFO )
		Error( "numtexinfo == BSP_MAX_TEXINFO" );

	texinfo[i] = *t;

	return numtexinfo++;
}

//============================================================================


typedef enum brushtype_e
{
	BRUSHTYPE_QUAKE,
	BRUSHTYPE_PATCHDEF2,
	BRUSHTYPE_BRUSHDEF3,
	BRUSHTYPE_PATCHDEF3
}
brushtype_t;

void ParseBrushFace (entity_t *ent, mbrush_t **brushpointer, brushtype_t brushtype)
{
	int					i,j,
						hltexdef,bpface,brushplane;
	vec_t				planepts[3][3], t1[3], t2[3], d, rotate, scale[2], vecs[2][4], ang, sinv, cosv, bp[2][3];
	mface_t				*f, *f2;
	plane_t				plane;
	BSPTextureInfo_t	tx;
	mbrush_t			*b;

	if (brushtype == BRUSHTYPE_PATCHDEF2 || brushtype == BRUSHTYPE_PATCHDEF3)
		return;
	// read the three point plane definition
	if (strcmp (token, "(") )
		Error ("parsing brush on line %d\n", scriptline);
	GetToken (false);
	planepts[0][0] = atof(token);
	GetToken (false);
	planepts[0][1] = atof(token);
	GetToken (false);
	planepts[0][2] = atof(token);
	GetToken (false);
	if (!strcmp(token, ")"))
	{
		brushplane = false;
		GetToken (false);
		if (strcmp(token, "("))
			Error("parsing brush on line %d\n", scriptline);
		GetToken (false);
		planepts[1][0] = atof(token);
		GetToken (false);
		planepts[1][1] = atof(token);
		GetToken (false);
		planepts[1][2] = atof(token);
		GetToken (false);
		if (strcmp(token, ")"))
			Error("parsing brush on line %d\n", scriptline);

		GetToken (false);
		if (strcmp(token, "("))
			Error("parsing brush on line %d\n", scriptline);
		GetToken (false);
		planepts[2][0] = atof(token);
		GetToken (false);
		planepts[2][1] = atof(token);
		GetToken (false);
		planepts[2][2] = atof(token);
		GetToken (false);
		if (strcmp(token, ")"))
			Error("parsing brush on line %d\n", scriptline);

		// convert points to a plane
		VectorSubtract(planepts[0], planepts[1], t1);
		VectorSubtract(planepts[2], planepts[1], t2);
		CrossProduct(t1, t2, plane.normal);
		VectorNormalize(plane.normal);
		plane.dist = DotProduct(planepts[1], plane.normal);
	}
	else
	{
		// oh, it's actually a 4 value plane
		brushplane = true;
		plane.normal[0] = planepts[0][0];
		plane.normal[1] = planepts[0][1];
		plane.normal[2] = planepts[0][2];
		plane.dist = -atof(token);
		GetToken (false);
		if (strcmp(token, ")"))
			Error("parsing brush on line %d\n", scriptline);
	}

	// read the texturedef
	memset (&tx, 0, sizeof(tx));
	GetToken (false);
	bpface = false;
	hltexdef = false;
	if (!strcmp(token, "("))
	{
		// brush primitives, utterly insane
		bpface = true;
		// (
		GetToken(false);
		// (
		GetToken(false);
		bp[0][0] = atof(token);
		GetToken(false);
		bp[0][1] = atof(token);
		GetToken(false);
		bp[0][2] = atof(token);
		GetToken(false);
		// )
		GetToken(false);
		// (
		GetToken(false);
		bp[1][0] = atof(token);
		GetToken(false);
		bp[1][1] = atof(token);
		GetToken(false);
		bp[1][2] = atof(token);
		GetToken(false);
		// )
		GetToken (false);
		GetToken (false);
		tx.iMipTex = FindMiptex (token);
		rotate = 0;
		scale[0] = 1;
		scale[1] = 1;
	}
	else
	{
		// if the texture name contains a / then this is a q2/q3 brushface
		// strip off the path, wads don't use a path on texture names
		tx.iMipTex = FindMiptex (token);
		GetToken (false);
		if (!strcmp(token, "["))
		{
			hltexdef = true;
			// S vector
			GetToken(false);
			vecs[0][0] = (vec_t)atof(token);
			GetToken(false);
			vecs[0][1] = (vec_t)atof(token);
			GetToken(false);
			vecs[0][2] = (vec_t)atof(token);
			GetToken(false);
			vecs[0][3] = (vec_t)atof(token);
			// ]
			GetToken(false);
			// [
			GetToken(false);
			// T vector
			GetToken(false);
			vecs[1][0] = (vec_t)atof(token);
			GetToken(false);
			vecs[1][1] = (vec_t)atof(token);
			GetToken(false);
			vecs[1][2] = (vec_t)atof(token);
			GetToken(false);
			vecs[1][3] = (vec_t)atof(token);
			// ]
			GetToken(false);

			// rotation (unused - implicit in tex coords)
			GetToken(false);
			rotate = 0;
		}
		else
		{
			vecs[0][3] = (vec_t)atof(token); // LordHavoc: float coords
			GetToken (false);
			vecs[1][3] = (vec_t)atof(token); // LordHavoc: float coords
			GetToken (false);
			rotate = atof(token); // LordHavoc: float coords
		}

		GetToken (false);
		scale[0] = (vec_t)atof(token); // LordHavoc: was already float coords
		GetToken (false);
		scale[1] = (vec_t)atof(token); // LordHavoc: was already float coords

		bp[0][0] = 1;
		bp[0][1] = 0;
		bp[0][2] = 0;
		bp[1][0] = 0;
		bp[1][1] = 1;
		bp[1][2] = 0;
	}

	// q3 .map properties, currently unused but parsed
	if (GetToken (false))
		if(GetToken (false))
			if (GetToken (false))
			{}

	// skip trailing info (the 3 q3 .map parameters for example)
	while (GetToken (false));

	if (DotProduct(plane.normal, plane.normal) < 0.1)
	{
		printf ("WARNING: brush plane with no normal on line %d\n", scriptline);
		return;
	}

	scale[0] = 1.0 / scale[0];
	scale[1] = 1.0 / scale[1];

	if (bpface)
	{
		// calculate proper texture vectors from GTKRadiant/Doom3 brushprimitives matrix
		float a, ac, as, bc, bs;
		a = -atan2(plane.normal[2], sqrt(plane.normal[0]*plane.normal[0]+plane.normal[1]*plane.normal[1]));
		ac = cos(a);
		as = sin(a);
		a = atan2(plane.normal[1], plane.normal[0]);
		bc = cos(a);
		bs = sin(a);
		vecs[0][0] = -bs;
		vecs[0][1] = bc;
		vecs[0][2] = 0;
		vecs[1][0] = -as*bc;
		vecs[1][1] = -as*bs;
		vecs[1][2] = -ac;
		tx.v[0][0] = bp[0][0]*vecs[0][0]+bp[0][1]*vecs[1][0];
		tx.v[0][1] = bp[0][0]*vecs[0][1]+bp[0][1]*vecs[1][1];
		tx.v[0][2] = bp[0][0]*vecs[0][2]+bp[0][1]*vecs[1][2];
		tx.v[0][3] = bp[0][0]*vecs[0][3]+bp[0][1]*vecs[1][3]+bp[0][2];
		tx.v[1][0] = bp[1][0]*vecs[0][0]+bp[1][1]*vecs[1][0];
		tx.v[1][1] = bp[1][0]*vecs[0][1]+bp[1][1]*vecs[1][1];
		tx.v[1][2] = bp[1][0]*vecs[0][2]+bp[1][1]*vecs[1][2];
		tx.v[1][3] = bp[1][0]*vecs[0][3]+bp[1][1]*vecs[1][3]+bp[1][2];
	}
	else if (hltexdef)
	{
		// HL texture vectors are almost ready to go
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 3; j++)
				tx.v[i][j] = vecs[i][j] * scale[i];
			tx.v[i][3] = vecs[i][3] /*+ DotProduct(origin, tx.vecs[i])*/;
// Sajt: ripped the commented out bit from the HL compiler code, not really sure what it is exactly doing
// 'origin': origin set on bmodel by origin brush or origin key
		}
	}
	else
	{
		// fake proper texture vectors from QuakeEd style

		// texture rotation around the plane normal
		if(rotate ==  0)
		{
			sinv = 0;
			cosv = 1;
		}
		else if (rotate == 90) {sinv = 1;cosv = 0;}
		else if (rotate == 180) {sinv = 0;cosv = -1;}
		else if (rotate == 270) {sinv = -1;cosv = 0;}
		else {ang = rotate * (Q_PI / 180);sinv = sin(ang);cosv = cos(ang);}

		if (fabs(plane.normal[2]) < fabs(plane.normal[0]))
		{
			if (fabs(plane.normal[0]) < fabs(plane.normal[1]))
			{
				// Y primary
				VectorSet4(tx.v[0],  cosv*scale[0],  0,  sinv*scale[0], vecs[0][3]);
				VectorSet4(tx.v[1],  sinv*scale[1],  0, -cosv*scale[1], vecs[1][3]);
			}
			else
			{
				// X primary
				VectorSet4(tx.v[0],  0,  cosv*scale[0],  sinv*scale[0], vecs[0][3]);
				VectorSet4(tx.v[1],  0,  sinv*scale[1], -cosv*scale[1], vecs[1][3]);
			}
		}
		else if (fabs(plane.normal[2]) < fabs(plane.normal[1]))
		{
			// Y primary
			VectorSet4(tx.v[0],  cosv*scale[0],  0,  sinv*scale[0], vecs[0][3]);
			VectorSet4(tx.v[1],  sinv*scale[1],  0, -cosv*scale[1], vecs[1][3]);
		}
		else
		{
			// Z primary
			VectorSet4(tx.v[0],  cosv*scale[0],  sinv*scale[0], 0, vecs[0][3]);
			VectorSet4(tx.v[1],  sinv*scale[1], -cosv*scale[1], 0, vecs[1][3]);
		}
		//printf("plane + rotate scale = texture vectors:\n(%f %f %f %f) + [%f %f %f] =\n[%f %f %f %f] [%f %f %f %f]\n", plane.normal[0], plane.normal[1], plane.normal[2], plane.dist, rotate, scale[0], scale[1], tx.vecs[0][0], tx.vecs[0][1], tx.vecs[0][2], tx.vecs[0][3], tx.vecs[1][0], tx.vecs[1][1], tx.vecs[1][2], tx.vecs[1][3]);
	}

	b = *brushpointer;
	if (b)
	{
		if (brushplane)
		{
			for (f2 = b->faces ; f2 ;f2=f2->next)
				if (VectorCompare(plane.normal, f2->plane.normal) && fabs(plane.dist - f2->plane.dist) < ON_EPSILON)
					break;
			if (f2)
			{
				printf ("WARNING: brush with duplicate plane (%g %g %g %g, .map file line number %d)\n", plane.normal[0], plane.normal[1], plane.normal[2], plane.dist, scriptline);
				return;
			}
		}
		else
		{
			// if the three points are all on a previous plane, it is a
			// duplicate plane
			for (f2 = b->faces ; f2 ; f2=f2->next)
			{
				for (i = 0;i < 3;i++)
				{
					d = DotProduct(planepts[i],f2->plane.normal) - f2->plane.dist;
					if (d < -ON_EPSILON || d > ON_EPSILON)
						break;
				}
				if (i==3)
					break;
			}
			if (f2)
			{
				printf ("WARNING: brush with duplicate plane (first point is at %g %g %g, .map file line number %d)\n", planepts[0][0], planepts[0][1], planepts[0][2], scriptline);
				return;
			}
		}
	}
	else
	{
		b = &mapbrushes[nummapbrushes];
		nummapbrushes++;
		b->next = ent->brushes;
		ent->brushes = b;
		*brushpointer = b;
	}

	f = qmalloc(sizeof(mface_t));
	f->next = b->faces;
	b->faces = f;
	f->plane = plane;
	f->texinfo = FindTexinfo (&tx);
	nummapbrushfaces++;
}

void ParseBrush (entity_t *ent)
{
	brushtype_t	brushtype;
	mbrush_t	*b;

	b = NULL;

	brushtype = BRUSHTYPE_QUAKE;
	for (;;)
	{
		if (!GetToken (true))
			break;
		if (!strcmp (token, "patchDef2"))
		{
			brushtype = BRUSHTYPE_PATCHDEF2;
			printf("patches not supported, skipping patch on line %d\n", scriptline);
		}
		else if (!strcmp (token, "brushDef3"))
			brushtype = BRUSHTYPE_BRUSHDEF3;
		else if (!strcmp (token, "patchDef3"))
		{
			brushtype = BRUSHTYPE_PATCHDEF3;
			printf("patches not supported, skipping patch on line %d\n", scriptline);
		}
		else if (!strcmp (token, "{") && brushtype != BRUSHTYPE_QUAKE)
		{
			for (;;)
			{
				if (!GetToken(true))
					Error("parsing brush on line %d\n", scriptline);
				if (!strcmp (token, "}"))
					break;
				ParseBrushFace(ent, &b, brushtype);
			}
		}
		else if (!strcmp (token, "}"))
			break;
		else if (!strcmp (token, "("))
			ParseBrushFace(ent, &b, BRUSHTYPE_QUAKE);
		else
			Error("parsing brush on line %d, unknown token \"%s\"\n", scriptline, token);
	}
}

/*	Moves entity's brushes into tree.
*/
void MoveEntityBrushesIntoWorld( entity_t *ent )
{
	entity_t *world;
	mbrush_t *b, *next;

	world = &entities[0];
	if( ent == world )
		return;

	for( b = ent->brushes; b; b = next ) {
		next = b->next;
		b->next = world->brushes;
		world->brushes = b;
	}

	ent->brushes = NULL;
}

bool ParseEntity (void)
{
	epair_t		*e,*next;
	entity_t	*ent;

	if(!GetToken(true))
		return false;

	if(!strcmp(token, "Version"))
	{
		GetToken(true);
		mapversion = atof(token);
		GetToken(true);
	}

	if(strcmp(token,"{" ))
		Error("ParseEntity: { not found");

	if( num_entities == BSP_MAX_ENTITIES )
		Error( "num_entities == MAX_MAP_ENTITIES" );

	ent = &entities[num_entities++];
	ent->epairs = NULL;

	do
	{
		fflush( stdout );

		if( !GetToken( true ) )
			Error( "ParseEntity: EOF without closing brace" );

		if( !strcmp (token, "}") )
			break;
		else if( !strcmp( token, "{" ) )  {
			ParseBrush( ent );
		} else {
			e = ParseEpair ();
			e->next = ent->epairs;
			ent->epairs = e;
		}
	} while( 1 );

	if(!strcmp( ValueForKey(ent,"classname"),"func_group"))
	{
		MoveEntityBrushesIntoWorld( ent );

		for( e = ent->epairs; e; e = next ) {
			next = e->next;
			qfree( e );
		}

		num_entities--;
	}

	return true;
}

void LoadMapFile (char *filename)
{
	void	*buf;

	num_entities		= 0;
	nummapbrushfaces	= 0;
	nummapbrushes		= 0;
	nummapplanes		= 0;
	nummiptex			= 0;
	mapversion			= 0;

	qprintf("Loading map file (%s)...\n",filename);

	if(LoadFile(filename,&buf) == -1)
		Error("Failed to load level! (%s)",filename);

	StartTokenParsing((char*)buf);

	while(ParseEntity());

	qfree(buf);

	qprintf ("%5i faces\n", nummapbrushfaces);
	qprintf ("%5i brushes\n", nummapbrushes);
	qprintf ("%5i entities\n", num_entities);
	qprintf ("%5i textures\n", nummiptex);
	qprintf ("%5i texinfo\n", numtexinfo);
}
