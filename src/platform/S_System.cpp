#include "Local.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define SYS_LOG "^3[Sys]^7 "

// Disclaimer: This is the only file
// in the entire code base created
// with generative AI tools. (Claude)

static int s_ansiEnabled;

int S_AnsiEnabled( void )
{
	return s_ansiEnabled;
}

// ============================================================
//  platform: Windows
// ============================================================

#ifdef _WIN32

static void S_PrintSystemInfo( void )
{
	SYSTEM_INFO			si;
	MEMORYSTATUSEX		mem;
	HKEY				hKey;
	char				cpuName[256];
	DWORD				cpuNameSize;
	DWORD				cpuMHz;
	DWORD				mhzSize;

	// ---- CPU identity ----

	// GetSystemInfo fills a struct with basic hardware topology.
	// we don't pass anything in -- we hand it an empty struct
	// and Windows fills every field. no return value, can't fail.
	GetSystemInfo( &si );

	// the CPU marketing name ("AMD Ryzen 7 5700X 8-Core Processor")
	// isn't in SYSTEM_INFO. Windows stores it in the registry,
	// written there at boot from the CPUID brand string the CPU
	// reports about itself. we have to read it out manually.
	//
	// RegOpenKeyExA opens a registry key for reading.
	//   HKEY_LOCAL_MACHINE  - the root hive (machine-wide settings)
	//   the path string     - where the CPU info lives
	//   0                   - reserved, always 0
	//   KEY_READ            - we only want to read, not write
	//   &hKey               - receives the opened handle
	//
	// returns ERROR_SUCCESS if it worked. anything else means
	// the key doesn't exist or we don't have permission.
	// the A suffix means ANSI (char*), not wide (wchar_t*).

	cpuName[0] = '\0';
	cpuMHz = 0;

	if ( RegOpenKeyExA(
		HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0,
		KEY_READ,
		&hKey ) == ERROR_SUCCESS )
	{
		// RegQueryValueExA reads a single value from an open key.
		//   hKey                - the handle we just opened
		//   "ProcessorNameString" - the value name (like a filename inside the key)
		//   NULL                - reserved
		//   NULL                - we don't care about the value type (we know it's a string)
		//   (LPBYTE)cpuName    - destination buffer, cast to byte pointer because
		//                        the API is type-generic (it reads strings, ints, binary)
		//   &cpuNameSize       - in: buffer size. out: bytes written.
		//                        MUST be set before the call.

		cpuNameSize = sizeof( cpuName );
		RegQueryValueExA( hKey, "ProcessorNameString",
			NULL, NULL,
			(LPBYTE)cpuName, &cpuNameSize );

		// clock speed in MHz, stored as a DWORD (32-bit unsigned int).
		// same pattern: tell it the buffer size, it writes the value.
		// the cast to (LPBYTE)&cpuMHz works because a DWORD is 4 bytes
		// and the registry value is 4 bytes. size matches, no conversion.

		mhzSize = sizeof( cpuMHz );
		RegQueryValueExA( hKey, "~MHz",
			NULL, NULL,
			(LPBYTE)&cpuMHz, &mhzSize );

		// close the handle. every RegOpenKeyEx must have a RegCloseKey,
		// same as every fopen must have an fclose. leak the handle and
		// Windows keeps the key locked until the process dies.
		RegCloseKey( hKey );
	}

	// if the registry read failed, cpuName is still the empty string
	// we initialized above. print "unknown" instead of a blank line.
	Com_Printf( SYS_LOG "CPU: %s\n",
		cpuName[0] ? cpuName : "unknown" );
	Com_Printf( SYS_LOG "CPU clock: %lu MHz\n",
		cpuMHz );
	Com_Printf( SYS_LOG "logical cores: %lu\n",
		si.dwNumberOfProcessors );
	Com_Printf( SYS_LOG "page size: %lu bytes\n",
		si.dwPageSize );

	// ---- physical memory ----

	// GlobalMemoryStatusEx fills a MEMORYSTATUSEX struct.
	// the ONE gotcha: you MUST set dwLength before calling.
	// Windows uses this field to know which version of the struct
	// you compiled against (the struct has grown over Windows versions).
	// forget this line and the call fails silently -- returns TRUE
	// but every field is zero. extremely annoying to debug.

	mem.dwLength = sizeof( mem );
	GlobalMemoryStatusEx( &mem );

	// ullTotalPhys and ullAvailPhys are DWORDLONG (unsigned 64-bit).
	// they're in bytes. divide by (1024*1024) for megabytes.
	// the cast to (unsigned long long) matches the %llu format spec.
	// on MSVC, DWORDLONG is unsigned __int64, which is the same type,
	// but the cast makes the printf format agreement explicit.

	Com_Printf( SYS_LOG "RAM: %llu MB total, %llu MB available\n",
		(unsigned long long)( mem.ullTotalPhys / ( 1024 * 1024 ) ),
		(unsigned long long)( mem.ullAvailPhys / ( 1024 * 1024 ) ) );
	Com_Printf( SYS_LOG "memory load: %lu%%\n",
		mem.dwMemoryLoad );
}

void S_InitConsoleAnsi( void )
{
	HANDLE	hOut;
	DWORD	mode;

	s_ansiEnabled = 0;

	hOut = GetStdHandle( STD_OUTPUT_HANDLE );

	if ( hOut == INVALID_HANDLE_VALUE ) {
		return;
	}

	if ( !GetConsoleMode( hOut, &mode ) ) {
		return;
	}

	if ( SetConsoleMode( hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING ) )
	{
		s_ansiEnabled = 1;
	}
}

void S_SystemInit( void )
{
	const char *ver = Cvar_GetString( "version" );
	char	title[256];
	snprintf( title, sizeof( title ), "%s - %s", ver, __DATE__ );
	SetConsoleTitleA( title );

	Cmd_Create( "systeminfo", S_PrintSystemInfo );
	S_PrintSystemInfo();
}

// ============================================================
//  platform: Linux / POSIX
// ============================================================

#else

// grab a value from a /proc key-value file
// looks for a line starting with key, returns the text after ':'
static void S_ReadProcValue( const char *path, const char *key, char *out, size_t outSize )
{
	FILE		*f;
	char		line[256];
	size_t		keyLen, len;
	const char	*val;

	out[0] = '\0';

	f = fopen( path, "r" );
	if ( !f )
		return;

	keyLen = strlen( key );

	while ( fgets( line, sizeof( line ), f ) )
	{
		if ( strncmp( line, key, keyLen ) )
			continue;

		val = strchr( line, ':' );
		if ( !val )
			break;

		val++;
		while ( *val == ' ' || *val == '\t' )
			val++;

		S_strncpyz( out, val, outSize );

		len = strlen( out );
		if ( len && out[len - 1] == '\n' )
			out[len - 1] = '\0';

		break;
	}

	fclose( f );
}

static void S_PrintSystemInfo( void )
{
	char	buf[256];
	long	memTotal, memAvail;

	// /proc/cpuinfo has one block per logical core, first match is fine
	S_ReadProcValue( "/proc/cpuinfo", "model name", buf, sizeof( buf ) );
	Com_Printf( SYS_LOG "CPU: %s\n",
		buf[0] ? buf : "unknown" );

	S_ReadProcValue( "/proc/cpuinfo", "cpu MHz", buf, sizeof( buf ) );
	Com_Printf( SYS_LOG "CPU clock: %d MHz\n",
		(int)atof( buf ) );

	Com_Printf( SYS_LOG "logical cores: %ld\n",
		sysconf( _SC_NPROCESSORS_ONLN ) );
	Com_Printf( SYS_LOG "page size: %ld bytes\n",
		sysconf( _SC_PAGESIZE ) );

	// /proc/meminfo reports in kB
	S_ReadProcValue( "/proc/meminfo", "MemTotal", buf, sizeof( buf ) );
	memTotal = atol( buf ) / 1024;

	S_ReadProcValue( "/proc/meminfo", "MemAvailable", buf, sizeof( buf ) );
	memAvail = atol( buf ) / 1024;

	Com_Printf( SYS_LOG "RAM: %ld MB total, %ld MB available\n",
		memTotal, memAvail );

	if ( memTotal > 0 )
	{
		Com_Printf( SYS_LOG "memory load: %ld%%\n",
			( ( memTotal - memAvail ) * 100 ) / memTotal );
	}
}

void S_InitConsoleAnsi( void )
{
	// unix terminals speak ANSI natively, just check it's a real tty
	s_ansiEnabled = isatty( STDOUT_FILENO );
}

void S_SystemInit( void )
{
	const char	*ver = Cvar_GetString( "version" );
	char		title[256];

	snprintf( title, sizeof( title ), "%s - %s", ver, __DATE__ );

	// xterm/VTE/kitty/alacritty all honor OSC 0 for window title
	printf( "\033]0;%s\007", title );
	fflush( stdout );

	Cmd_Create( "systeminfo", S_PrintSystemInfo );
	S_PrintSystemInfo();
}

#endif
