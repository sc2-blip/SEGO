#include "Local.h"

#define FS_LOG "^3[FS]^7 "

#define MAX_OSPATH			256
#define MAX_SEARCH_PATHS	8

struct fsSearchPath_t
{
	char	path[MAX_OSPATH];
};

static fsSearchPath_t	fs_searchPaths[MAX_SEARCH_PATHS];
static int				fs_numSearchPaths;

// ---- internal helpers ----

// FS_BuildOSPath
// glues a search path and a virtual path into a real OS path
// example: base="base" virtual="textures/gold.tga"
// result: "base/textures/gold.tga"
static void FS_BuildOSPath( const char *base, const char *virtualPath, char *out, size_t outSize )
{
	snprintf( out, outSize, "%s/%s", base, virtualPath );
}

// ---- public API ----

// FS_AddSearchPath
// pushes a directory onto the search stack
// last added = highest priority = searched first
void FS_AddSearchPath( const char *path )
{
	assert( path != NULL );

	if ( fs_numSearchPaths >= MAX_SEARCH_PATHS )
	{
	    // throw fatal and quit
	    Com_Error( FS_LOG "fs_numSearchPaths %d larger than MAX_SEARCH_PATHS (8)", fs_numSearchPaths );
	}

	S_strncpyz( fs_searchPaths[fs_numSearchPaths].path, path, sizeof( fs_searchPaths[fs_numSearchPaths].path ) );
	fs_numSearchPaths++;

	Com_Printf( FS_LOG "Added search path: %s\n", path );
}

// FS_FileExists
// walks search paths top-down (highest priority first)
// returns 1 if the file can be opened, 0 if not
// does not leave anything open
int FS_FileExists( const char *virtualPath )
{
	for ( int i = fs_numSearchPaths - 1; i >= 0; i-- )
	{
		char osPath[MAX_OSPATH]; // local buffer for the OS path
		FS_BuildOSPath( fs_searchPaths[i].path, virtualPath, osPath, sizeof( osPath ) );
		FILE *f = fopen( osPath, "rb" );
		if ( f )
		{
			fclose( f );
			return 1;
		} 
	}

	return 0;
}

// FS_ReadFile
// the main file loader
// searches for virtualPath, reads the whole file into S_Malloc'd memory
// returns the file size in bytes, or -1 if not found
// *buffer receives the allocation -- caller MUST call FS_FreeFile on it
// the buffer is null-terminated so text files can be used as strings
long FS_ReadFile( const char *virtualPath, void **buffer )
{
	assert( buffer != NULL );
	*buffer = NULL;

	for ( int i = fs_numSearchPaths - 1; i >= 0; i-- )
	{
		char osPath[MAX_OSPATH]; // local buffer for the OS path
		FS_BuildOSPath( fs_searchPaths[i].path, virtualPath, osPath, sizeof( osPath ) );
		FILE *f = fopen( osPath, "rb" );
		if ( f )
		{
			fseek( f, 0, SEEK_END );
			long size = ftell( f );
			fseek( f, 0, SEEK_SET );

			*buffer = S_Malloc( size + 1 ); // if this fails it calls Com_Error
			size_t readSize = fread( *buffer, 1, size, f );
			((char *)*buffer)[readSize] = '\0';
			fclose( f );

			return readSize;
 
		}
	}

	// Com_Printf( FS_LOG "File not found: %s\n", virtualPath );
	return -1; // not found 
}

// FS_FreeFile
// frees a buffer that FS_ReadFile returned
void FS_FreeFile( void *buffer )
{
	assert( buffer != NULL );
	S_Free( buffer );
}

// ---- commands ----

// Cmd_Path
// prints the current search path stack so you can see the priority order
static void Cmd_Path( void )
{
	for (int i = 0; i < fs_numSearchPaths; i++)
	{
		Com_Printf( FS_LOG "%d: %s\n", i, fs_searchPaths[i].path );
	}
	Com_Printf( FS_LOG "Total search paths: %d\n", fs_numSearchPaths );
}

// Cmd_ReadTest
// usage: readtest <filepath>
// tries FS_ReadFile, prints the size if it worked, then frees
// a disposable command for you to verify the pipeline works
static void Cmd_ReadTest( void )
{
	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( FS_LOG "Usage: readtest <filepath>\n" );
		return;
	}

	const char *filepath = Cmd_Argv( 1 );
	void *buf = NULL;
	long size = FS_ReadFile( filepath, &buf );
	if ( size > 0 )
	{
		Com_Printf( FS_LOG "Read file: %s, size: %ld bytes\n", filepath, size );
		FS_FreeFile( buf );
	}
	else
	{
		Com_Printf( FS_LOG "File not found: %s\n", filepath );
	}
}

// ---- init / shutdown ----

void FS_Init( void )
{
	fs_numSearchPaths = 0;

	FS_AddSearchPath( "rsc" );
	Cmd_Create( "path", Cmd_Path );
	Cmd_Create( "readtest", Cmd_ReadTest );

	Com_Printf( FS_LOG "VFS initialized with %d search paths\n", fs_numSearchPaths );
}

void FS_Shutdown( void )
{
	fs_numSearchPaths = 0;
	Com_Printf( FS_LOG "VFS shutdown\n" );
}
