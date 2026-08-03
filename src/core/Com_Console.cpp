#include "Local.h"
#include <stdio.h>
#include <cstdarg>
#include <termios.h>
#include <unistd.h>

#define COM_LOG		"^3[Console]^7 "

// As good practice, I am declaring every major system get started
// with a #define WHATEVER_LOG. Start of every single file.
// We'll then use that WHATEVER_LOG (if you want) in print calls.
// It's important to know what it will look like when the file
// is trying to talk to you for debug purposes.
// We want know *where* our error is coming from to be consistent in logs
// because I feel our console is gonna fill up with stuff pretty quickly.

static conColor_t con_colors[] = {
	{ '0',   0,   0,   0 },   // black
	{ '1', 255,  64,  64 },   // red
	{ '2',  64, 255,  64 },   // green
	{ '3', 255, 255,  64 },   // yellow
	{ '4',  64, 128, 255 },   // blue
	{ '5',  64, 255, 255 },   // cyan
	{ '6', 255,  64, 255 },   // magenta
	{ '7', 200, 200, 195 },   // default (warm off-white)
	{ 'g', 183, 110, 121 },   // rose gold
	{ 'd',  40,  40,  38 },   // dark warm gray
};

static const int con_numColors = sizeof ( con_colors ) / sizeof ( con_colors[0] );

// ============================================================
// raw mode terminal and command history
// ============================================================

#define CON_HISTORY_DEPTH	32

// special key identifiers from Con_ReadKey
#define CON_KEY_UP			256
#define CON_KEY_DOWN		257
#define CON_KEY_RIGHT		258
#define CON_KEY_LEFT		259

static struct termios	con_savedTerm;
static bool				con_rawMode;

static char		con_history[CON_HISTORY_DEPTH][MAX_CMD_LINE];
static int		con_histCount;		// total commands stored
static int		con_histBrowse;		// where we are while arrowing through history
static char		con_stash[MAX_CMD_LINE];	// preserves in-progress input before browsing

// ============================================================

static conColor_t *Com_ColorForCode( char code )
{
	for ( int i = 0; i < con_numColors; i++ )
	{
		if ( con_colors[i].code == code )
		{
			return &con_colors[i];
		}
	}

	return NULL;
}

void Com_Printf( const char* fmt, ... )
{
	static char		colored[MAX_PRINT_MSG * 3];
	char			msg[MAX_PRINT_MSG];
	va_list			argptr;
	const char		*p;
	char			*out;

	va_start( argptr, fmt );
	vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	if ( !S_AnsiEnabled() )
	{
		fputs( msg, stdout );
		return;
	}

	p = msg;
	out = colored;

	while ( *p )
	{
		if ( *p == '^'  && *( p + 1 ) ) 
		{
			// to use hex in console: ^#RRGGBB 
			if ( *( p + 1 ) == '#' )
			{
				byte_t r, g, b;

				if ( Com_ParseHexColor( p + 2, &r, &g, &b ) )
				{
					out += sprintf( out, "\033[38;2;%d;%d;%dm", r, g, b );
					p += 8;

					continue;
				}

				*out++ = *p++; // parse failed, fall through to literal copy
			}
			else if ( *( p + 1 ) == 'r' )
			{ // Reset code ^r (goes back to white)
				out += sprintf( out, "\033[0m" );
				p += 2;

				continue;
			}
			else if ( *( p + 1 ) == '^' )
			{
				*out++ = '^';
				p += 2;

				continue;
			}
			else 
			{
				conColor_t *c = Com_ColorForCode( *( p + 1 ) );

				if ( c)
				{
					out += sprintf( out, "\033[38;2;%d;%d;%dm", c->r, c->g, c->b );
					p += 2;

					continue;
				}

				*out++ = *p++;
			}
		}

		// default: copy one character
		*out++ = *p++;
	}

	*out = '\0';
	fputs ( colored , stdout );
}

void Com_Error( const char* fmt, ... ) 
{ // Same thing, but with some string adjustment
  // Note: This WILL stop the program, so use with intent.
	char		msg[MAX_PRINT_MSG];
	va_list		argptr;

	va_start( argptr, fmt );
	vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end ( argptr );

	Com_Printf( "^1FATAL: %s^7\n", msg );
	Con_Shutdown();
	exit(1);

	// About this exit(1) for now 7/27/26:
	// When SEGO has a rendered game with a menu system,
	// we'll want to split error pathing by severity:
	// ERR_DROP tear down gameplay, flush memory,
	// longjmp back to the main loop
	// otherwise severity says exit 1
}

// ============================================================
// Con_Init / Con_Shutdown
// ============================================================

void Con_Init( void )
{
	con_histCount = 0;
	con_histBrowse = 0;
	con_stash[0] = '\0';

	tcgetattr( STDIN_FILENO, &con_savedTerm );

	struct termios working = con_savedTerm;
	working.c_lflag &= ~( ICANON | ECHO );
	working.c_cc[VMIN] = 1;
	working.c_cc[VTIME] = 0;
	tcsetattr( STDIN_FILENO, TCSANOW, &working );
	con_rawMode = true;

	atexit( Con_Shutdown ); // ensure we restore terminal state on exit or else terminal gets broken too
}

void Con_Shutdown( void )
{
	// TODO: if con_rawMode, restore original terminal state:
	//       tcsetattr( STDIN_FILENO, TCSANOW, &con_savedTerm )
	// TODO: con_rawMode = false

	// if this doesn't fire on every exit path,
	// the user's shell is stuck in raw mode after we quit.
	// they'd have to type 'reset' blind to fix it.
	if ( con_rawMode ) 
	{
		tcsetattr( STDIN_FILENO, TCSANOW, &con_savedTerm );
		con_rawMode = false;
	}
}

// ============================================================
// Con_ReadKey
// reads one logical keypress, decoding escape sequences
// ============================================================

static int Con_ReadKey( void )
{
	char ch;
	if ( read( STDIN_FILENO, &ch, 1 ) != 1 )
	{
		return 0; // error or EOF
	}

	if ( ch == 0x1b ) 
	{
		read( STDIN_FILENO, &ch, 1 );
		if ( ch == '[' )
		{
			read( STDIN_FILENO, &ch, 1 );
			switch ( ch )
			{
				case 'A': return CON_KEY_UP;
				case 'B': return CON_KEY_DOWN;
				case 'C': return CON_KEY_RIGHT;
				case 'D': return CON_KEY_LEFT;
				default: return 0;
			}
		}
	}

	if ( ch == 0x7f || ch == 0x08 )
	{
		return 0x7f; // backspace
	}

	return ch;
}

// ============================================================
// command history
// ============================================================

static void Con_HistoryAdd( const char *line )
{
	if ( line[0] == '\0' )
	{
		return; // do not store empty lines
	}

	S_strncpyz( con_history[con_histCount % CON_HISTORY_DEPTH ], line, sizeof( con_history[0] ) );
	con_histCount++;
	con_histBrowse = con_histCount; // reset browsing to present
}

// clears the visible input line then redraws prompt + new text
static void Con_RedrawLine( const char *line, int len )
{
	write( STDOUT_FILENO, "\r\033[2K", 5 ); // return + clear line
	Com_Printf( "^g]^3 " );
	fflush( stdout ); // this has to go here otherwise the prompt doesn't show up before the line is printed
	if ( len > 0 )
	{
		write( STDOUT_FILENO, line, len );
	}

}

// ============================================================
// S_ConsoleInput (replaces fgets version)
// ============================================================

const char *S_ConsoleInput( void ) 
{
	static char	line[MAX_CMD_LINE];
	int			cursor;
	int			key;
	bool		browsing;

	Com_Printf( "^g]^3 " );
	fflush( stdout );

	cursor = 0;
	line[0] = '\0';
	browsing = false;

	for ( ;; )
	{
		key = Con_ReadKey();

		if ( key == '\n' || key == '\r' )
		{
			Com_Printf( "\n" );
			if ( line[0] != '\0' )
			{
				Con_HistoryAdd( line );
			}

			con_histBrowse = con_histCount; // reset browsing to present
			return line;
		}

		if ( key == 0x7f || key == 0x08 )
		{
			// backspace 
			if ( cursor > 0 )
			{
				cursor--;
				line[cursor] = '\0';
				write( STDOUT_FILENO, "\b \b", 3 );
			}
			continue;
		}

		if ( key == CON_KEY_UP )
		{
			if (con_histCount == 0) continue;

			if ( !browsing )
			{
				S_strncpyz( con_stash, line, sizeof( con_stash ) );
				con_histBrowse = con_histCount;
				browsing = true;
			}
			if ( con_histBrowse > 0 && ( con_histBrowse > con_histCount - CON_HISTORY_DEPTH ) ) 
			{
				con_histBrowse--;
				S_strncpyz( line, con_history[con_histBrowse % CON_HISTORY_DEPTH], sizeof( line ) );
				cursor = strlen( line );
				Con_RedrawLine( line, cursor );
			}

			continue;
		}

		if ( key == CON_KEY_DOWN )
		{
			if ( !browsing ) continue; 

			con_histBrowse++;

			if ( con_histBrowse >= con_histCount ) 
			{
				S_strncpyz( line, con_stash, sizeof( line ) );
				browsing = false;
				cursor = strlen( line );
				Con_RedrawLine( line, cursor );
			}
			else
			{
				S_strncpyz( line, con_history[con_histBrowse % CON_HISTORY_DEPTH], sizeof( line ) );
				cursor = strlen( line );
				Con_RedrawLine( line, cursor );
			}

			continue;
		}

		// printable character
		if ( key >= 32 && key < 256 && cursor < MAX_CMD_LINE - 1 )
		{
			char c = (char)key; // cast to char for clarity & portability
			line[cursor++] = c;
			line[cursor] = '\0';
			write( STDOUT_FILENO, &c, 1 );
		}
	}
}

void Com_StartupArgs( int argc, char **argv )
{
	char	line[MAX_CMD_LINE];
	int		i, len;

	i = 1;
	while ( i < argc )
	{
		// a command (from path) starts at a +
		// skip anything before the first one
		if ( argv[i][0] != '+' )
		{
			i++;
			continue;
		}

		// start the line with the command name (+ removed)
		S_strncpyz( line, argv[i] + 1, sizeof ( line ) );
		i++;

		while ( i < argc && argv[i][0] != '+')
		{
			len = strlen( line );
			// rebuild the space the shell ate between +
			S_strncpyz( line + len, " ", sizeof( line ) - len );
			len = strlen( line );
			S_strncpyz( line + len, argv[i], sizeof( line ) - len);
			i++;
		}

		Cmd_Execute( line );
	}
}