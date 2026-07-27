#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#define	MAX_PRINT_MSG	4096

#define MAX_CMD_ARGS	16
#define MAX_CMD_LINE	1024
#define MAX_CMDS		64

#define CVAR_ARCHIVE ( 1 << 0 ) // 1 - save to config
#define CVAR_ROM	 ( 1 << 1 ) // 2 - read only
#define CVAR_INIT	 ( 1 << 2 ) // 4 - only set from command line

typedef unsigned char byte_t; // unsigned 8-bit value

struct conColor_t
{
	char	code;
	byte_t	r, g, b;
};

struct S_MemHeader {
	size_t size;
};

struct cvar_t
{
	char	name[64];
	char	string[256];
	char	resetString[256];
	float	value;
	int		integer;
	int		flags;
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

// Console
[[noreturn]] void	Com_Error( const char *fmt, ... );
const char			*S_ConsoleInput( void );
void				Com_Printf( const char *fmt, ... );
void				Com_Frame ( void );

// Common
[[noreturn]] void	Com_Quit( void );
void				Com_StartupArgs( int argc, char **argv );
void				Com_Init( int argc, char **argv );

// Commands
typedef void ( *cmdFunction_t )( void );
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


// util
void	S_strncpyz( char *dest, const char *src, size_t size );
int		S_stricmp( const char *s1, const char *s2 );
void	S_SystemInit( void );
int		S_AnsiEnabled( void );
void	S_InitConsoleAnsi( void );

// Math helper functions
int		Com_HexDigit( char c );
bool	Com_ParseHexColor( const char *hex, byte_t *r, byte_t *g, byte_t *b );
