#ifndef ENGCOMMON_H
#define ENGCOMMON_H

#if !defined BYTE_DEFINED
typedef unsigned char 		byte;
#define BYTE_DEFINED 1
#endif

#if __cplusplus
extern "C" {
#endif

	char *va(const char *format, ...);

#if __cplusplus
};
#endif

#include "shared_common.h"

//============================================================================

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;

void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((uint8_t *)l - (int)&(((t *)0)->m)))

//============================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)

extern	bool    bigendien;

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);
void MSG_WriteAngle16 (sizebuf_t *sb, float f); //johnfitz

extern	int		msg_readcount;
extern	bool	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
float MSG_ReadAngle16 (void); //johnfitz

//============================================================================

int	Q_atoi (char *str);
float Q_atof (char *str);

//============================================================================

extern	char	com_token[1024];
extern	bool	com_eof;

extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (char *parm);
void COM_Init(void);
void COM_InitArgv (int argc, char **argv);

char *FileSystem_SkipPath(char *pathname);
void COM_StripExtension (char *in, char *out);
void COM_FileBase (const char *in, char *out);
void COM_DefaultExtension (char *path, char *extension);
char *COM_FileExtension(char *in);

char *va(const char *format, ...);
// does a varargs printf into a temp buffer


//============================================================================

#ifdef __cplusplus
extern "C" {
#endif

	extern int com_filesize;
	struct cache_user_s;

	extern char com_gamedir[PLATFORM_MAX_PATH];

	void FileSystem_WriteFile(const char *ccFileName, void *data, int len);
	int COM_OpenFile(const char *filename, int *hndl);
	int COM_FOpenFile(const char *filename, FILE **file);
	void COM_CloseFile(int h);

	uint8_t *COM_LoadTempFile(const char *path);
	uint8_t *COM_LoadHunkFile(char *path);
	void    *COM_LoadHeapFile(const char *path);

	/*
		File System
	*/

	void FileSystem_Initialize(void);
	void FileSystem_UpdatePath(char cPath[]);

#ifdef __cplusplus
}
#endif

#endif	// COMMON_H
