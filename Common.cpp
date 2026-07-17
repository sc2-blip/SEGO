#include "Local.h"

void Com_Frame( void )
{
	if ( S_SignalCaught() )
	{
		Com_Quit();
	}

	const char *line = S_ConsoleInput();

	if ( line )
	{
		Cmd_Execute( line );
	}
}

void Com_Quit( void )
{
	// Com_ConsoleShutdown();
	S_MemShutdown(); // audits what no one freed 
	exit( 0 ); 
}

void Com_Init( int argc, char **argv )
{
	S_InitSignals();
	S_InitConsoleAnsi();


	S_MemInit();
	Cmd_Init();
	Cvar_Init();

	Cvar_Create( "name", "Player", CVAR_ARCHIVE );
	Cvar_Create( "version", "SEGO 0.0.1", CVAR_ROM );

	S_SystemInit();

	Com_StartupArgs( argc, argv );

	Com_Printf( "^gSEGO initialized^7\n" );
}
