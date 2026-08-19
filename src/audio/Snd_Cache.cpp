#include "Local.h"
#include <AL/al.h>

#define CACHE_LOG "^3[Audio Cache]^7 "

static sndBuffer_t	s_sounds[MAX_SOUNDS];
static int			s_numSounds;
static int			s_soundHash[SOUND_HASH_SIZE];

// Snd_HashName
// Takes a sound name, returns an index into s_soundHash
// The end goal: spread different strings across 0 .. SOUND_HASH_SIZE - 1
// Walk each char, shift the running hash, put the char in
// Then mask to table size (SOUND_HASH_SIZE - 1) since it's a power of two
// Case-insensitive, lower each char before putting it in
static int Snd_HashName( const char *name )
{
	int hash = 0;

	// TODO: accumulate a hash from each character of name 
	// tolower() each char so "Sound/Sound_File.wav" and "sound/sound_file.wav" hash the same

	return hash & ( SOUND_HASH_SIZE - 1 );
}

// Snd_FindSound
// given a name, walk the hash chain and return the index into s_sounds
// return -1 if not found
static int Snd_FindSound( const char *name ) 
{
	int hash = Snd_HashName( name );

	// TODO: s_soundHash[hash] is the head of the chain
	// it's an index into s_sounds[], or -1 if the chain is empty
	// walk the chain, compare names with S_stricmp
	// folow hashNext to the next link
	// return the index when you find a match. -1 if the chain ends

	return -1;
}

// Snd_RegisterSound 
// the main entry point: give it a virtual path, get back a handle
// if already cached, return the existing hasndle instantly
// if not, decode the file, upload to openAL, store everything, return new handle
int Snd_RegisterSound( const char *name )
{
	int existing = Snd_FindSound( name );
	if ( existing >= 0 )
	{
		return existing;
	}

	if ( s_numSounds >= MAX_SOUNDS )
	{
		Com_Printf( CACHE_LOG "Sound cache is full, cannot register %s\n", name );
		return -1;
	}

	// claim the next slot
	int handle = s_numSounds;
	sndBuffer_t *buf = &s_sounds[handle];

	// TODO: copy name into buf_name S_strncpyz

	// TODO: decode the sound file
	// declare a sndPcm_t, call Snd_decode with the name
	// if it fails, return -1

	// TODO: upload the decoded PCM data to OpenAL
	// refer to Cmd_PlaySnd
	// 	figure out the AL format from channel count
	// 	compute dataSize (samples * channels * sizeof( short ))
	//  alGenBuffers into buf->alBuffer
	// 	alBufferData with the decoded PCM data
	//  check alGetError
	// then store rate, channels, samples, from the PCM into the buf struct
	// then free the PCM data. openAL made it's own copy

	// TODO: insert into the hash chain
	// get the hash index for this name
	// buf->hashNext = s_soundHash[hash]; (point to the old head)
	// s_soundHash[hash] = handle; 		  (this is the new head)
	// think about why this order matters..

	s_numSounds++;
	Com_Printf( CACHE_LOG "Registered sound %s as handle %d\n", name, handle );
	
	return handle;
}

// Snd_GetBuffer 
// turns a handle into a pointer to the cached buffer struct
// returns NULL if the handle is invalid
sndBuffer_t *Snd_GetBuffer( int handle )
{
	// TODO: bounds check handle against 0 and s_numSounds
	// return pointer to s_sounds[handle], or NULL

	return NULL;
}

void Snd_CacheInit( void )
{
	s_numSounds = 0;

	// tODO: set every entry in s_soundHash to -1, meaning empty chain
	// memset ( s_soundHash, ???, sizeof( s_soundHash ) );
	// think about what -1 looks like as bytes in two's complement:
	// 0xFF, 0xFF, 0xFF, 0xFF for a 32-bit int

	memset( s_sounds, 0, sizeof( s_sounds ) );

	Com_Printf( CACHE_LOG "Sound cache initialized\n" );
}

void Snd_CacheShutdown( void )
{
	// TODO: walk s_sounds from 0 to s_numSounds - 1
	// for each slot that has a valid alBuffer (nonzero):
	//	alDeleteBuffers( 1, &buf->alBuffer )
	//	zero the alBuffer field

	s_numSounds = 0;
	Com_Printf( CACHE_LOG "Shutting down..\n" );
}