#include "Local.h"
#include <AL/al.h>
#include <AL/alc.h>

#define SND_LOG "^3[Audio]^7 "

static ALCdevice    *snd_device;
static ALCcontext   *snd_context;

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