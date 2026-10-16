#pragma once

#include "SG_Shared.h"

// UI Module & Interface - UI_Public.h
//
//	Our UI is a DLL. It reaches the engine through a uiImport_t ONLY.
//	The engine reaches the UI through a uiExport_t ONLY.
//
//	Hold a pointer only to memory that will out live you. Otherwise, copy.
//	The engine reads the draw list after Refresh returns. Holds copies only.
//	Register assets in Init, and use handles in Refresh.
//	Change anything in this file, you must bump UI_API_VERSION.
//	The UI includes this file and nothing from the engine or SDL.
//
//	Draw order is list order. Commands draw in the order they were added.
//  Later commands land on top.

#define UI_API_VERSION		1
#define UI_GETAPI_NAME		"UI_GetAPI"

#ifdef _WIN32
#define UI_EXPORT			extern "C" __declspec ( dllexport )
#else
#define UI_EXPORT			extern "C" __attribute__ ( ( visibility ( "default" ) ) )
#endif

#define UI_MAX_DRAWCMDS		2048
#define UI_MAX_DRAWTEXT		16384

enum uiDrawType_t : int
{
	UIDRAW_RECT,
	UIDRAW_PIC,
	UIDRAW_TEXT
};

// RECT ( x y w h , color )
// PIC	( x y w h, color tints, data = shader handle )
// TEXT ( x, y = origin ; h = glyph height ; w = clip width or 0 , data = a textPool offset )
struct uiDrawCmd_t
{
	uiDrawType_t	type;			// 4 bytes
	int				flags;			// 4 bytes
	int				data;			// 4 bytes
	float			x, y, w, h;		// 16 bytes
	byte_t			color[4];		// 4 bytes	= 32 bytes
};

// uiDrawCmd_t must stay at 32 bytes.
// if the size has to change, you update the assert on purpose
// bump the version, too. 
static_assert( sizeof( uiDrawCmd_t ) == 32, "Check uiDrawCmd_t; is it 32 bytes?");

struct alignas( 64 ) uiDrawList_t
{
	uiDrawCmd_t		cmds[UI_MAX_DRAWCMDS];		// 64 KB
	char			textPool[UI_MAX_DRAWTEXT];	// 16 KB
	int				numCmds;					// 4 bytes
	int				textUsed;					// 4 bytes = 80 KB
};

// UI Frames: the UI only knows the time the engine has given it.
struct uiFrame_t
{
	int				width;
	int				height;
	int				time; // MSec
};

// This function pointer soup is what the engine gives the UI.
struct uiImport_t
{
	// Takes a string and uses it immediately
	void			(*Printf)( const char *fmt, ... );

	// Com_Error in the engine is [[noreturn]],
	// but we don't inherit this behavior in a func ptr.
	// In UI Code, always use a return after a ui.Error
	// or you will see warnings about missing return values
	void			(*Error)( const char *fmt, ... );

	// Returns a cvar_t * pointing into cvar_vars[MAX_CVARS]
	cvar_t			*(*Cvar_Create)( const char *name, const char *value, int flags );

	void			(*Cvar_Set)( const char *name, const char *value );
	void			(*Cmd_Execute)( const char *text );

	// The buffer goes through S_Malloc by the engine, 
	// but owned here by the UI.
	// The UI must free every file before Shutdown returns. 
	// If it doesn't, each reload leaks..
	long			(*FS_ReadFile)( const char *path, void **buffer );
	void			(*FS_FreeFile)( void *buffer );

	// RegisterShader and Snd_Register map a name to an integer handle.
	// Call them in Init only.
	// Registration means file lookup and hashing.
	int				(*RegisterShader)( const char *name );
	int				(*Snd_Register)( const char *name );
	void			(*Snd_Play)( int handle );
};

// This is what the UI gives the engine back.
struct uiExport_t
{
	// Called on load and before unload.
	// Shutdown must leave nothing behind.
	void			(*Init)( void );
	void			(*Shutdown)( void );
	
	// "key" is our local keycode, not an SDL scancode.
	// "down" is 1 for press and 0 for release.
	void			(*KeyEvent)( int key, int down );
	void			(*Refresh)( const uiFrame_t *frame, uiDrawList_t *out );
	
	// bool
	int				(*IsFullScreen)( void );
};

// The engine fills a uiImport_t and calls UI_GetAPI( UI_API_VERSION, &imports, &exports )
// DLL always returns its own UI_API_VERSION
// Fills exports only if the engine's version matches its own.
// Engine compares the returned number to its own; a mismatch prints both.
// A mismatch will then unload the DLL and keep running without a UI.
// The console will still be open in CLI if this happens. 
typedef int ( *UI_GetAPI_t )( int apiVersion, const uiImport_t *imports, uiExport_t *exports );