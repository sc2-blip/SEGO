#include "Local.h"
#include <cstdlib>	

#define CMD_LOG "^3[Cmd]^7 "

// maximum 16 arguments a command usage
// maximum 1024 characters in a single argument
// maximum 64 commands we can hold in our registry

struct cmd_entry_t // a command
{
	char			name[64]; // 64 chars in length maximum
	cmdFunction_t	func;
};

static cmd_entry_t	cmd_commands[MAX_CMDS]; // where we'll put em
static int			cmd_numCommands; // and how many

static int			cmd_argc; // how many args 
static char			cmd_argv[MAX_CMD_ARGS][MAX_CMD_LINE]; // what are the args

int Cmd_Argc( void ) 
{
	return cmd_argc;
}

const char* Cmd_Argv( int index ) 
{
	if ( index < 0 || index >= cmd_argc )
	{
		return "";
	}
	
	return cmd_argv[index];
}

void Cmd_TokenizeString( const char* text )
{
	const char *in;
	char	   *out;
	int			len;

	cmd_argc = 0;

	if ( !text )
	{
		return;
	}

	in = text;

	while (1)
	{
		while ( *in && *in <= ' ' )
		{ // increment walker 
			in++; 
		}

		if ( !*in )
		{ // no in 
			return;
		}

		if ( cmd_argc >= MAX_CMD_ARGS ) // Cmd_Argc() >= MAX_CMD_ARGS
		{ 
			return;
		}

		out = cmd_argv[cmd_argc];
		len = 0;

		while ( *in > ' ' ) // while we HAVE *in and it isn't a whitespace character
		{ //		   1024
			if ( len < MAX_CMD_LINE - 1 ) 
			{
				out[len++] = *in;
			}
			in++;
		}

		out[len] = '\0';
		cmd_argc++;
	}
}

void Cmd_Create( const char* name, cmdFunction_t func ) {
	if (cmd_numCommands >= MAX_CMDS)
		Com_Error( CMD_LOG "too many commands, count: %i", 
			cmd_numCommands );

	S_strncpyz( cmd_commands[cmd_numCommands].name, 
		name, 
		sizeof( cmd_commands[cmd_numCommands].name )
	); 
	
	cmd_commands[cmd_numCommands].func = func; 

	cmd_numCommands++; //increment

	Com_Printf( // may want to consider gating this behind a debug/dev mode later 
		CMD_LOG "command registered: %s (%i)\n",
		name,
		cmd_numCommands
	);
}

void Cmd_Execute( const char* text ) 
{
	Cmd_TokenizeString( text ); // only thing I need to do with text in this fn

	if ( cmd_argc == 0 ) // Cmd_Argc() == 0 ?
		return; // quietly do nothing

	for ( int i = 0; i < cmd_numCommands; i++ ) // count up to the NUMBER of CMDS and not the capacity (64)
	{
		if ( !S_stricmp( cmd_commands[i].name, Cmd_Argv( 0 ) ) ) // if returns 0
		{ 
			cmd_commands[i].func();
			return;
		}
	}

	if ( Cvar_Command() )
	{
		return;
	}

	Com_Printf( CMD_LOG "unknown command: %s\n", Cmd_Argv( 0 ) ); 
}

// built-in commands
// should probably *always* define our functions here
// even if they're a shell of a function from another file 
// that has an obvious call back/visual which it creates
// on it's own

static void Cmd_Quit( void ) 
{
	Com_Quit();
}

static void Cmd_MemInfo( void )
{
	S_MemInfo(); // prints on its own
}

static void Cmd_CmdList( void ) // sure, but I'd like a proper formatted "help" command later
{
	for ( int i = 0; i < cmd_numCommands; i++ )
	{
		Com_Printf( "%s\n", cmd_commands[i].name);
	}
	Com_Printf( CMD_LOG "%i commands\n", cmd_numCommands );
}

static void Cmd_Echo( void )
{
	for (int i = 1; i < Cmd_Argc(); i++ )
	{
		Com_Printf( "%s ", Cmd_Argv( i ));
	}

	Com_Printf( "\n" );
}

static void Cmd_Exec( void )
{
	char	filename[MAX_CMD_LINE];
	char	path[MAX_CMD_LINE];
	char	line[MAX_CMD_LINE];
	FILE	*f;
	long	size;
	char	*buf;
	char	*text;
	int		len;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( CMD_LOG "usage: exec <filename>\n" );
	}

	S_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ));

	// auto append .cfg if not there in string
	if ( !strstr( filename, "cfg" ) )
	{
		len = strlen( filename );
		S_strncpyz( filename + len, ".cfg", sizeof( filename ) - len );
	}

	// build path to cfg relative to engine root
	snprintf( path, sizeof( path ), "cfg/%s", filename );

	// open and measure
	f = fopen( path, "rb" );

	if ( !f )
	{
		Com_Printf( CMD_LOG "couldn't exec %s\n", path );
	}

	fseek( f, 0, SEEK_END );
	size = ftell( f );
	fseek( f, 0, SEEK_SET);

	if ( size <= 0 )
	{
		fclose( f );
		return;
	}

	// load the whole file into tracked buffer
	buf = ( char * )S_Malloc( size + 1 );
	fread( buf, 1, size, f);
	buf[size] = '\0';
	fclose( f );

	Com_Printf( CMD_LOG "execing %s (%ld bytes)\n", path, size );

	// walk buffer line by line, same char-walk pattern as toenizer
	text = buf;

	while ( *text )
	{
		len = 0;

		while ( *text && *text != '\n' && *text != '\r' )
		{
			if ( len < MAX_CMD_LINE - 1 )
			{
				line[len++] = *text;
			}
			text++;
		}

		line[len] = '\0';

		// skip line endings (\r\n, \n, \r all handled)
		if ( *text == '\r' ) text++;
		if ( *text == '\n' ) text++;

		// execute non-empty lines
		if ( len > 0 )
		{
			Cmd_Execute( line );
		}
	}

	S_Free( buf );
}

static void Cmd_xColorTest( void ) 
{
	if ( Cmd_Argc() < 2 )
		return;

	if ( Cmd_Argv( 1 )[0] != '#' )
		return;

	Com_Printf( CMD_LOG "^%s hello world!^7\n", Cmd_Argv( 1 ) );
}

/*static void Cmd_KillSysTest( void )
{
	// https://tenor.com/bOhks.gif
	Com_Error( " System killed per user request (killsystest) ");
}*/

void Cmd_Init( void )
{
	cmd_numCommands = 0;	// reset so re-init on reload doesn't append duplicates
	Cmd_Create( "quit", Cmd_Quit );
	Cmd_Create( "meminfo", Cmd_MemInfo );
	Cmd_Create( "cmdlist", Cmd_CmdList );
	Cmd_Create( "echo", Cmd_Echo );
	Cmd_Create( "exec", Cmd_Exec );
	//Cmd_Create( "xcolortest", Cmd_xColorTest );
	//Cmd_Create( "killsystest", Cmd_KillSysTest );
	Com_Printf( CMD_LOG "commands initialized\n" );
}


