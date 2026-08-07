#include <cstring>
#include "Local.h"

// Safe string copy

void S_strncpyz( char *dest, const char *src, size_t size ) 
{ 
	if ( !size ) return;

	strncpy( dest, src, size ); // Eventually we'll have to stop using this
	dest[size - 1] = '\0';
}

// Safe string compare (case-insensitive, no length limit)
// S_stricmp returns 0 when the strings *match*
// so !S_stricmp ( a , b ) reads as *no difference* between a & b

int S_stricmp( const char *s1, const char *s2 )
{
	/*if ( !s1 || !s2 ) {
		return ( s1 == s2 ) ? 0 : ( s1 ? 1 : -1 );
	}*/ // check for null

	// typically locals are declared in the top
	// but I compiled this in C++17 and not C89
	int	c1, c2;

	do 
	{
		c1 = *s1++;
		c2 = *s2++;

		// strictly ascii
		if ( c1 != c2 ) 
		{ // to upper 
			if ( c1 >= 'a' && c1 <= 'z' ) // str 1
			{
				c1 -= ( 'a' - 'A' ); // 0x61 - 0x41 = 32 
			}
			if ( c2 >= 'a' && c2 <= 'z' ) // str 2 
			{
				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) 
			{
				return c1 < c2 ? -1 : 1; // actual to upper of the chars
			}
		}
	} while ( c1 ); // cannot be made to stop early or run past the end

	return 0;
}

const char *Com_FormatDuration( float sec )
{
	static char buf[32];
	int s = (int)sec;
	int h = s / 3600;
	int m = ( s - h * 3600 ) / 60;
	s = s - h * 3600 - m * 60;

	if ( h > 0 )
	{
		snprintf( buf, sizeof( buf ), "%dh %dm %ds", h, m, s );
	}
	else if ( m > 0 )
	{
		snprintf( buf, sizeof( buf ), "%dm %ds", m, s );
	}
	else
	{
		snprintf( buf, sizeof( buf ), "%ds", s );
	}

	return buf;
}