#include "Local.h"

int Com_HexDigit( char c )
{
	if ( c >= '0' && c <= '9' ) return c - '0';
	if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;

	return -1;
}

bool Com_ParseHexColor( const char *hex, byte_t *r, byte_t *g, byte_t *b )
{
	int i;

	for ( i = 0; i < 6; i++ ) {
		if ( Com_HexDigit( hex[i] ) < 0 ) {
			return false;
		}
	}

	*r = ( Com_HexDigit( hex[0] ) << 4 ) | Com_HexDigit( hex[1] );
	*g = ( Com_HexDigit( hex[2] ) << 4 ) | Com_HexDigit( hex[3] );
	*b = ( Com_HexDigit( hex[4] ) << 4 ) | Com_HexDigit( hex[5] );

	return true;
}
