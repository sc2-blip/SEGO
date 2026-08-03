#include "Local.h"

#define CVAR_LOG "^3[CVar]^7 "

#define MAX_CVARS		256
#define MAX_CVAR_STRING 256

static cvar_t	cvar_vars[MAX_CVARS];
static int		cvar_numVars;


// Internal helpers
static void Cvar_Update( cvar_t *cv )
{
	cv->value = (float)atof( cv->string );
	cv->integer = atoi( cv->string );
}

// Core C-Var functions
cvar_t *Cvar_Find( const char *name )
{
	for ( int i = 0; i < cvar_numVars; i++ )
	{
		if ( !S_stricmp( cvar_vars[i].name, name ) )
		{
			return &cvar_vars[i];
		}
	}

	return NULL;
}

cvar_t *Cvar_Create( const char *name, const char *value, int flags )
{
	cvar_t *cv = Cvar_Find ( name );

	if ( cv ) 
	{
		return cv;
	}

	if ( cvar_numVars >= MAX_CVARS )
	{
		Com_Error( CVAR_LOG "too many cvars: count %i", cvar_numVars );
	}

	cv = &cvar_vars[cvar_numVars];

	S_strncpyz( cv->name, name, sizeof( cv->name ) );
	S_strncpyz( cv->string, value, sizeof( cv->string ) );
	S_strncpyz( cv->resetString, value, sizeof( cv->resetString ) );
	cv->flags = flags;

	Cvar_Update( cv );

	cvar_numVars++;

	return cv;
}

void Cvar_Set( const char *name, const char *value )
{
	cvar_t *cv = Cvar_Find( name );

	if (!cv) 
	{
		Com_Printf(CVAR_LOG "WARN: could not set cvar \"%s\", not found\n", name);
		return;
	}

	// TODO: flag checks
	// if cv->flags & CVAR_ROM warn "read-only cvar" and return

	S_strncpyz( cv->string, value, sizeof( cv->string ) );
	Cvar_Update( cv );
}

const char *Cvar_GetString( const char *name )
{
	cvar_t *cv = Cvar_Find( name );

	if ( !cv )
	{
		return "";
	}

	return cv->string;
}

float Cvar_GetValue( const char *name )
{
	cvar_t *cv = Cvar_Find( name );

	if ( !cv )
	{
		return 0.0f;
	}

	return cv->value;
}

int Cvar_GetInteger( const char *name )
{
	cvar_t *cv = Cvar_Find( name );

	if ( !cv )
	{
		return 0;
	}

	return cv->integer;
}

bool Cvar_Command( void )
{
	cvar_t *cv = Cvar_Find( Cmd_Argv( 0 ) );

	if (!cv)
	{
		return false;
	}

	if (Cmd_Argc() >= 2)
	{
		Cvar_Set( Cmd_Argv( 0 ), Cmd_Argv( 1 ) );
	} else 
	{
		Com_Printf(CVAR_LOG "%s is \"%s\" - default \"%s\"\n", cv->name, cv->string, cv->resetString );
	}

	return true;
}

// built-in c-var commands 
static void Cmd_Set ( void )
{
	// set <name> <value>
	if ( Cmd_Argc() < 3 )
	{
		Com_Printf( CVAR_LOG "usage: set <name> <value>\n" );
		return;
	}

	Cvar_Set ( Cmd_Argv(1), Cmd_Argv(2) );
}

static void Cmd_CvarList( void ) 
{
	for ( int i = 0; i < cvar_numVars; i++ )
	{
		Com_Printf( "  %s = \"%s\"\n", cvar_vars[i].name, cvar_vars[i].string );
	}
	Com_Printf( CVAR_LOG "%i cvars\n", cvar_numVars );
}


// Init
void Cvar_Init( void )
{
	cvar_numVars = 0;
	Cmd_Create( "set", Cmd_Set );
	Cmd_Create( "cvarlist", Cmd_CvarList );
	Com_Printf( CVAR_LOG "cvars initialized\n" );
}