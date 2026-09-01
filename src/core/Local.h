#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cctype>

#include "SG_Shared.h"

#define	MAX_PRINT_MSG	4096

#define MAX_CMD_ARGS	16
#define MAX_CMD_LINE	1024
#define MAX_CMDS		64

struct conColor_t
{
	char	code;
	byte_t	r, g, b;
};

struct S_MemHeader {
	size_t size;
};

// System
void	S_InitSignals( void );
int		S_SignalCaught( void );

// Memory
void    S_MemInit( void );
void*	S_Malloc( size_t size );
void	S_Free( void *ptr );
void	S_MemInfo( void );
void	S_MemShutdown ( void );
void* 	S_ReAlloc( void *ptr, size_t size );

// Console
[[noreturn]] void	Com_Error( const char *fmt, ... );
const char			*S_ConsoleInput( void );
void				Com_Printf( const char *fmt, ... );
void				Com_Frame ( void );
void		        Con_Init( void );
void 				Con_Shutdown( void );	

// Audio
// Audio decode
struct sndPcm_t 
{
	void	*data;
	int 	samples;
	int 	rate;
	int		channels;
};

// Audio cache
#define MAX_SOUNDS		256
#define MAX_SOUNDPATH	256

struct sndBuffer_t
{
	char			name[MAX_SOUNDPATH];
	unsigned int	alBuffer;		// OpenAL buffer handle, 0 = empty slot
	int				rate;
	int				channels;
	int				samples;
	int				hashNext;		// next index in hash chain, -1 = end
};

void 		Snd_Init( void );
void		Snd_Shutdown( void );
int 		Snd_Decode( const char *virtualPath, sndPcm_t *out );
void 		Snd_FreePcm( sndPcm_t *pcm );
void		Snd_DecodeInit( void );
void		Snd_CacheInit( void );
void		Snd_CacheShutdown( void );
int			Snd_RegisterSound( const char *name );
sndBuffer_t	*Snd_GetBuffer( int handle );

// Common
[[noreturn]] void	Com_Quit( void );
void				Com_StartupArgs( int argc, char **argv );
void				Com_Init( int argc, char **argv );

// Commands
void		Cmd_Init( void );
void		Cmd_Create( const char *name, cmdFunction_t func);
void		Cmd_Execute( const char *text );
void		Cmd_TokenizeString( const char *text );
int			Cmd_Argc( void );
const char* Cmd_Argv( int index );

// C-Var
void		Cvar_Init( void );
cvar_t		*Cvar_Create( const char *name, const char *value, int flags );
cvar_t		*Cvar_Find( const char *name );
void		Cvar_Set( const char *name, const char *value);
const char	*Cvar_GetString( const char *name );
float		Cvar_GetValue ( const char *name );
int			Cvar_GetInteger( const char * name );
bool		Cvar_Command( void );

// Filesystem
void    FS_Init( void );
void    FS_Shutdown( void );
void    FS_AddSearchPath( const char *path );
long    FS_ReadFile( const char *virtualPath, void **buffer );
void    FS_FreeFile( void *buffer );
int     FS_FileExists( const char *virtualPath );

// util
const char *Com_FormatDuration( float sec );
void		S_strncpyz( char *dest, const char *src, size_t size );
int			S_stricmp( const char *s1, const char *s2 );
void		S_SystemInit( void );
int			S_AnsiEnabled( void );
void		S_InitConsoleAnsi( void );


// Math helper functions
int		Com_HexDigit( char c );
bool	Com_ParseHexColor( const char *hex, byte_t *r, byte_t *g, byte_t *b );