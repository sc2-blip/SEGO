#include "Local.h"
#include <AL/al.h>

#define CACHE_LOG "^3[Audio Cache]^7 "

#define SOUND_HASH_SIZE 64

static sndBuffer_t	s_sounds[MAX_SOUNDS];
static int			s_numSounds;
static int			s_soundHash[SOUND_HASH_SIZE];

// FIXME: This should really live in Snd_Local.h or something..
static ALenum Snd_ALFormat( int channels ) 
{ // returns AL_FORMAT_MONO16 or AL_FORMAT_STEREO16 based on channels 

    switch ( channels )
    {
        case 1:
            return AL_FORMAT_MONO16;
        case 2:
            return AL_FORMAT_STEREO16;
        default:
            Com_Printf( CACHE_LOG "Snd_ALFormat: Channels readout invalid\n" );
            return 0;
    }
}

// Snd_HashName
// Takes a sound name, returns an index into s_soundHash
// The end goal: spread different strings across 0 .. SOUND_HASH_SIZE - 1
// Walk each char, shift the running hash, put the char in
// Then mask to table size (SOUND_HASH_SIZE - 1) since it's a power of two
// Case-insensitive, lower each char before putting it in
static int Snd_HashName( const char *name )
{
	int hash = 0;

	for ( const char *p = name; *p; p++ )
	{
		hash = ( hash << 5 ) + hash + tolower( *p ); 
	}

	return hash & ( SOUND_HASH_SIZE - 1 ); 
}

// Snd_FindSound
// given a name, walk the hash chain and return the index into s_sounds
// return -1 if not found
static int Snd_FindSound( const char *name ) 
{
	int hash = Snd_HashName( name );

	for ( int i = s_soundHash[hash]; i != -1; i = s_sounds[i].hashNext )
	{
		if ( !S_stricmp( name, s_sounds[i].name ) )
		{
			return i;
		}
	}

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

	S_strncpyz( buf->name, name, sizeof( buf->name ) );

	sndPcm_t pcm;
	if ( Snd_Decode( name, &pcm ) != 0 ) // if the result was anything OTHER than success 
	{
		Com_Printf( CACHE_LOG "Failed to decode sound %s\n", name );
		return -1;
	}

	ALenum format = Snd_ALFormat( pcm.channels );
	if ( format == 0 )
	{
		Com_Printf( CACHE_LOG "Failed to determine AL format for sound %s\n", name );
		Snd_FreePcm( &pcm );
		return -1;
	}

	int dataSize = pcm.samples * pcm.channels * sizeof( short ); // size in bytes
	// alternatively: drflac_int16 exists

	alGenBuffers( 1, &buf->alBuffer );
	alBufferData( buf->alBuffer, format, pcm.data, dataSize, pcm.rate );

	ALenum err = alGetError();
	if ( err != AL_NO_ERROR )
	{
		Com_Printf( CACHE_LOG "Failed to buffer audio data for sound %s: %s\n", name, alGetString( err ) );

		alDeleteBuffers( 1, &buf->alBuffer );
		buf->alBuffer = 0; // Zero the handle to avoid useless reference
		Snd_FreePcm( &pcm );

		return -1;
	}

	buf->rate = pcm.rate;
	buf->channels = pcm.channels;
	buf->samples = pcm.samples;

	int hash = Snd_HashName( name );
	buf->hashNext = s_soundHash[hash];
	s_soundHash[hash] = handle;

	s_numSounds++;
	Com_Printf( CACHE_LOG "Registered sound %s as handle %d\n", name, handle );
	
	return handle;
}

// Snd_GetBuffer 
// turns a handle into a pointer to the cached buffer struct
// returns NULL if the handle is invalid
sndBuffer_t *Snd_GetBuffer( int handle )
{
	if ( handle >= 0 && handle <= s_numSounds )
	{
		return &s_sounds[handle];
	}
	
	Com_Printf( CACHE_LOG "Snd_GetBuffer WARN: Handle out of bounds\n" );
	return NULL;
}

void Snd_CacheInit( void )
{
	s_numSounds = 0;

	memset( s_soundHash, 0xFF, sizeof( s_soundHash ) ); // memset sequential by default, fills with 0xFF -1
	memset( s_sounds, 0, sizeof( s_sounds ) );

	Com_Printf( CACHE_LOG "Sound cache initialized\n" );
}

void Snd_CacheShutdown( void )
{
	for ( int i = 0; i < s_numSounds; i++ )
	{
		if ( s_sounds[i].alBuffer != 0 )
		{
			alDeleteBuffers( 1, &s_sounds[i].alBuffer );
			s_sounds[i].alBuffer = 0;
		}
	}

	s_numSounds = 0;
	Com_Printf( CACHE_LOG "Shutting down..\n" );
}