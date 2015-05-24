/*	Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __SHAREDTEXTURE__
#define	__SHAREDTEXTURE__

#include "SharedModel.h"

enum srcformat
{
	SRC_INDEXED,
	SRC_LIGHTMAP,
	SRC_RGBA
};

typedef struct gltexture_s 
{
	//managed by texture manager
	unsigned int		texnum;
	struct gltexture_s	*next;
	model_t				*owner;
	//managed by image loading
	char				name[64];
	unsigned int		width;					// Size of image as it exists in opengl
	unsigned int		height;					// Size of image as it exists in opengl
	unsigned int		flags;
	char				source_file[MAX_QPATH]; // Relative filepath to data source, or "" if source is in memory
	unsigned int		source_offset;			// Byte offset into file, or memory address
	enum srcformat		source_format;			// Format of pixel data (indexed, lightmap, or rgba)
	unsigned int		source_width;			// Size of image in source data
	unsigned int		source_height;			// Size of image in source data
	unsigned short		source_crc;				// Generated by source data before modifications
	char				shirt;					// 0-13 shirt color, or -1 if never colormapped
	char				pants;					// 0-13 pants color, or -1 if never colormapped
	//used for rendering
	int					visframe; //matches r_framecount if texture was bound this frame
} gltexture_t;

#endif