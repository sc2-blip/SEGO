#include "Local.h"
#include <AL/al.h>
#include <AL/alc.h>

#define SND_LOG "^3[Audio]^7 "

#define SND_BAR_WIDTH   30

static ALCdevice    *snd_device;
static ALCcontext   *snd_context;
// Test buffer and source for audio playback testing
static ALuint       snd_testBuffer;
static ALuint       snd_testSource;
static float        snd_testDuration;
static char		    snd_testName[MAX_CMD_LINE]; // Currently playing sound's name

// TODO post-it note: 24-bit support later down the line

static ALint Snd_SourceState( ALuint source ) 
{ // returns AL_PLAYING, AL_PAUSED, AL_STOPPED, or AL_INITIAL
    ALint state;
    alGetSourcei( source, AL_SOURCE_STATE, &state );
    return state;
}

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
            Com_Printf( SND_LOG "Snd_ALFormat: Channels readout invalid\n" );
            return 0;
    }
}

static void Cmd_PlaySnd( void )
{
    if ( Cmd_Argc() < 2 )
    {
        Com_Printf( SND_LOG "Usage: snd_play <filepath>\n" );
        return;
    }

    if ( snd_testSource )
    {
        alDeleteSources( 1, &snd_testSource );
        snd_testSource = 0;
    }

    int handle = Snd_RegisterSound( Cmd_Argv( 1 ) );
    if ( handle < 0 )
    {
        Com_Printf( SND_LOG "Failed to load sound: %s\n", Cmd_Argv( 1 ) );
        return;
    }

    sndBuffer_t *buf = Snd_GetBuffer( handle );

    snd_testDuration = (float)buf->samples / (float)buf->rate;
    S_strncpyz( snd_testName, Cmd_Argv( 1 ), sizeof( snd_testName ) );

    Com_Printf( SND_LOG "%s\n", snd_testName );
    Com_Printf(
        SND_LOG "16-bit %g kHz %s\n",
        buf->rate / 1000.0f,
        buf->channels == 1 ? "mono" : "stereo"
    );

    alGenSources( 1, &snd_testSource );
    alSourcei( snd_testSource, AL_BUFFER, buf->alBuffer );
    alSourcePlay( snd_testSource );

    ALenum err = alGetError();
    if ( err != AL_NO_ERROR )
    {
        Com_Printf( SND_LOG "Source error: 0x%x\n", err );
        alDeleteSources( 1, &snd_testSource );
        snd_testSource = 0;
        return;
    }

    if ( Snd_SourceState( snd_testSource ) == AL_PLAYING )
    {
        Com_Printf( SND_LOG "%s\n", Com_FormatDuration( snd_testDuration ) );
    }
    else
    {
        Com_Printf( SND_LOG "Failed to play sound: %s\n", snd_testName );
    }
}

static void Cmd_PositionSnd( void )
{
    if ( !snd_testSource ) 
    {
        Com_Printf( SND_LOG "Nothing playing!\n" );
        return;
    }

    if ( snd_testDuration <= 0.0f )
    {
        Com_Printf( SND_LOG "No duration info\n" );
        return;
    }

    float curSec; // once again, working in seconds here 
    // Com_FormatDuration( float *seconds* )
    alGetSourcef( snd_testSource, AL_SEC_OFFSET, &curSec ); // copies into curSec for us
    
    float ratio = curSec / snd_testDuration; 
    // watch for out of bounds
    if ( ratio < 0.0f ) ratio = 0.0f;
    if ( ratio > 1.0f ) ratio = 1.0f;

    int filled = (int)(ratio * SND_BAR_WIDTH); // spaces to be filled out of 30

    char bar[SND_BAR_WIDTH + 1]; // build the progress bar

    for ( int i = 0; i < SND_BAR_WIDTH; i++ )
    {
        bar[i] = ( i < filled ) ? '=' : '-';
    }

    bar[SND_BAR_WIDTH] = '\0'; // cant forget you 

    // and for the console, finally:
    Com_Printf( SND_LOG "\"%s\"\n", snd_testName );
    const char *cur = Com_FormatDuration( curSec );
    Com_Printf( SND_LOG "%s [%s] ", cur, bar );
    Com_Printf( "%s\n", Com_FormatDuration( snd_testDuration ) );
}

static void Cmd_PauseSnd( void )
{
    if ( snd_testSource && Snd_SourceState( snd_testSource ) == AL_PLAYING )
    {
        alSourcePause( snd_testSource );
        Com_Printf( SND_LOG "Paused sound\n" );
    }
}

static void Cmd_ResumeSnd( void )
{
    if ( 
        snd_testSource && 
        (Snd_SourceState( snd_testSource ) == AL_PAUSED ||
         Snd_SourceState( snd_testSource ) == AL_STOPPED) 
    )
    {
        alSourcePlay( snd_testSource );
        Com_Printf( SND_LOG "Resumed sound\n" );
    }
}

static void Cmd_ReplaySnd( void )
{
    if ( snd_testSource )
    {
        alSourceRewind( snd_testSource );
        alSourcePlay( snd_testSource );
        Com_Printf( SND_LOG "Replaying: %s\n", snd_testName );
    }
}

static void Cmd_StopSnd( void )
{
    if ( snd_testSource )
    {
        // keep the buffer if we want to replay
        alSourceStop( snd_testSource );
        Com_Printf( SND_LOG "Stopped sound\n" );
    }
}

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

    Snd_CacheInit();

    Cmd_Create( "snd_play", Cmd_PlaySnd );
    Cmd_Create( "snd_stop", Cmd_StopSnd );
    Cmd_Create( "snd_pause", Cmd_PauseSnd );
    Cmd_Create( "snd_resume", Cmd_ResumeSnd );
    Cmd_Create( "snd_pos", Cmd_PositionSnd );
    Cmd_Create( "snd_replay", Cmd_ReplaySnd);

    Cmd_Create( "snd_position", Cmd_PositionSnd ); // alias for convenience
}

void Snd_Shutdown( void ) 
{
    
    if ( snd_testSource ) // Clean up
    {
        alDeleteSources( 1, &snd_testSource );
        snd_testSource = 0;
    }

    /* if ( snd_testBuffer ) // Clean up
    {
        alDeleteBuffers( 1, &snd_testBuffer );
        snd_testBuffer = 0;
    } */

    Snd_CacheShutdown(); 

    alcMakeContextCurrent( NULL ); // Clean up context

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