#pragma once

typedef unsigned char byte_t; // unsigned 8-bit value

#define CVAR_ARCHIVE ( 1 << 0 ) // 1 - save to config
#define CVAR_ROM	 ( 1 << 1 ) // 2 - read only
#define CVAR_INIT	 ( 1 << 2 ) // 4 - only set from command line

struct cvar_t
{
	char	name[64];
	char	string[256];
	char	resetString[256];
	float	value;
	int		integer;
	int		flags;
};

typedef void ( *cmdFunction_t )( void );