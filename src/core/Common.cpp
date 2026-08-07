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
	FS_Init();

	Snd_Init();
	Snd_DecodeInit();

	Com_StartupArgs( argc, argv );

	Com_Printf( "^gSEGO initialized^7\n" );
}

void Com_Quit( void )
{
	Con_Shutdown();
	Snd_Shutdown();
	FS_Shutdown();
	S_MemShutdown(); // audits what no one freed
	exit( 0 );
}
