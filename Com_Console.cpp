#include "Local.h"
#include <stdio.h>
#include <cstdarg>

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
	char		msg[MAX_PRINT_MSG];
	va_list		argptr;

	va_start( argptr, fmt );
	vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end ( argptr );

	Com_Printf( "^1FATAL: %s^7\n", msg );
	exit(1); //actually exit
}

const char *S_ConsoleInput( void ) 
{
	static char line[MAX_CMD_LINE];

	Com_Printf( "^g]^3 " );
	fflush( stdout );

	if ( !fgets( line, sizeof( line ), stdin ) )
	{
		// EOF or read error, ctrl+d or empty file stream
		//Com_Printf( COM_LOG "EOF or read error");
		Com_Quit();
	}

	return line;
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

