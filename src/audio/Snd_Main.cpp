#include "Local.h"
#include <AL/al.h>
#include <AL/alc.h>

#define SND_LOG "^3[Audio]^7 "

static ALCdevice    *snd_device;
static ALCcontext   *snd_context;

// Test buffer and source for audio playback testing
static ALuint       snd_testBuffer;
static ALuint       snd_testSource;

void Snd_Init( void )
{
    snd_device = alcOpenDevice( NULL );
    if ( !snd_device )
    {
        Com_Error( SND_LOG "Failed to open OpenAL device" );
    }

    snd_context = alcCreateContext( snd_device, NULL );
    if ( !snd_context || !alcMakeContextCurrent( snd_context ) )
    {
        Com_Error( SND_LOG "Failed to create OpenAL context" );
    }

    const char *name = alcGetString( snd_device, ALC_DEVICE_SPECIFIER );
    Com_Printf( SND_LOG "OpenAL device: %s\n", name ? name : "Unknown" );
    Com_Printf( SND_LOG "OpenAL version: %s\n", alGetString( AL_VERSION ) );
    Com_Printf( SND_LOG "Initialized OpenAL audio system\n" );
}

void Snd_Shutdown( void ) 
{
    alcMakeContextCurrent( NULL );

    if ( snd_context ) 
    {
        alcDestroyContext( snd_context );
        snd_context = NULL;
    }

    if ( snd_device )
    {
        alcCloseDevice( snd_device );
        snd_device = NULL;
    }

    Com_Printf( SND_LOG "Shutdown OpenAL audio system\n" );
}

static ALenum Snd_ALFormat( int channels ) 
{
// TODO: return AL_FORMAT_MONO16 or AL_FORMAT_STEREO16 based on channels
//      if channels is neither 1 or 2, log a warning and return 0    
}

static void Cmd_PlaySnd( void )
{
    if ( Cmd_Argc() < 2 )
    {
        Com_Printf( SND_LOG "Usage: playsnd <filepath>\n" );
        return;
    }

    // TODO if a previous sound is still loaded, clean it up:
    // alDeleteSources( 1, &snd_testSource );
    // alDeleteBuffers( 1, &snd_testBuffer );
    // zero both handles

    // TODO Snd_Decode( Cmd_Argv( 1 ) ) in to a sndPcm_t 
    // exit on failure

    // TODO: figure out the AL format with Snd_ALFormat
    // exit if 0

    // TODO: Calculate data size in bytes 
    //       frames * channels * sizeof( drflac_int16 )

    // TODO: alGenBuffers( 1, &snd_testBuffer )
    //       alBufferData( snd_testBuffer, format, pcm.data, dataSize, pcm.rate )
    
    // TODO: Snd_FreePcm: OpenAL copied the data, we don't need ours anymore

    // TODO: alGenSources( 1, &snd_testSource )
    //       alSourcei( snd_testSource, AL_BUFFER, snd_testBuffer )
    //       alSourcePlay( snd_testSource )

    // TODO: print what's currently playing 
    
}
