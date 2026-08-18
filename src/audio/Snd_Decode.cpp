#include "Local.h"

#define DR_FLAC_IMPLEMENTATION
#include "../extern/dr_flac.h"

#define DR_WAV_IMPLEMENTATION
#include "../extern/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "../extern/dr_mp3.h"

#define DEC_LOG "^3[Audio Decode]^7 "

// TO-DO things to consider for the rest of our sound module:
// Use data streaming and my own pAllocationCallbacks (S_Malloc, S_ReAlloc (NEW), S_Free)
// to manage memory for data streaming. Will also double as a more
// robust memory alloc system on the side.
// Lastly, 24-bit support later down the line

//Snd_FileExtension
// returns pointer to the extension after the dot or "" if none
static const char *Snd_FileExtension( const char *path )
{
    const char *dot = NULL;
    const char *p   = path;

    while ( *p )
    {
        if ( *p == '.' )
            dot = p;

        p++;
    }

    return dot ? dot + 1 : "";
}

static int Snd_DecodeFLAC( void *fileData, long fileSize, sndPcm_t *out )
{
    unsigned int channels;
    unsigned int rate;
    drflac_uint64 totalFrames;

    drflac_int16* pcmData = drflac_open_memory_and_read_pcm_frames_s16(
        fileData,
        fileSize,
        &channels,
        &rate,
        &totalFrames,
        NULL
    );

    if ( pcmData == NULL )
    {
        Com_Printf( DEC_LOG "Failed to decode FLAC data" );
        return -1;
    }

    // TODO populate out->data, out->samples, out->rate, out->channels
    out->data = pcmData;
    out->samples = (int)totalFrames;
    out->rate = rate;
    out->channels = channels;

    return 0;
}

static int Snd_DecodeWAV( void *fileData, long fileSize, sndPcm_t *out )
{
    unsigned int channels;
    unsigned int rate;
    drwav_uint64 totalFrames;

    drwav_int16* pcmData = drwav_open_memory_and_read_pcm_frames_s16(
        fileData,
        fileSize,
        &channels,
        &rate,
        &totalFrames,
        NULL
    );
    
    if ( pcmData == NULL )
    {
        Com_Printf( DEC_LOG "Failed to decode WAV data" );
        return -1;
    }

    out->data = pcmData;
    out->samples = (int)totalFrames;
    out->rate = rate;
    out->channels = channels;

    return 0;
}

static int Snd_DecodeMP3( void *fileData, long fileSize, sndPcm_t *out )
{
    drmp3_config cfg;
    drmp3_uint64 totalFrames;

    drmp3_int16 *pcmData = drmp3_open_memory_and_read_pcm_frames_s16(
        fileData,
        fileSize,
        &cfg,
        &totalFrames,
        NULL
    )

    if ( pcmData == NULL )
    {
        Com_Printf( DEC_LOG "Failed to decode MP3 data\n" );
        return -1;
    }

    out->data = pcmData;
    out->samples = (int)totalFrames;
    out->rate = cfg.sampleRate;
    out->channels = cfg.channels;

    return 0;
}

int Snd_Decode( const char *virtualPath, sndPcm_t *out )
{
    // Snd_DecodeFLAC - static function from dr_flac.h
    // Snd_Decode - OUR function, for the engine to call, 
    // which will read the file from "VFS" and pass it to Snd_DecodeFLAC

    void *buf = NULL;
    long size = FS_ReadFile( virtualPath, &buf );

    if ( size <= 0 )
    {
        Com_Printf( DEC_LOG "Failed to read sound file: %s\n", virtualPath );
        return -1;
    }

    //int result = Snd_DecodeFLAC( buf, size, out );
    const char *ext = Snd_FileExtension( virtualPath );
    int result = -1;

    if ( !S_stricmp( ext, "flac" ) )
        result = Snd_DecodeFLAC( buf, size, out );
    else if ( !S_stricmp( ext, "wav") )
        result = Snd_DecodeWAV( buf, size, out );
    else if ( !S_stricmp( ext, "mp3" ) )
        result = Snd_DecodeMP3( buf, size, out );
    else
        Com_Printf( DEC_LOG "Unknown audio format: .%s\n", ext );
    

    FS_FreeFile( buf ); 

    return result;
}

void Snd_FreePcm( sndPcm_t *pcm )
{
    /* data fields to zero after free
    void	*data;
	int     samples;
	int 	rate;
	int		channels;
    */
    if ( pcm->data )
    {
        //drflac_free( pcm->data, NULL );
        free( pcm->data );

        pcm->data = NULL;
        pcm->samples = 0;
        pcm->rate = 0;
        pcm->channels = 0;
    }
}

// -- audio commands --

static void Cmd_DecodeTest( void )
{
    if ( Cmd_Argc() < 2 )
    {
        Com_Printf( DEC_LOG "Usage: decodetest <filepath>\n" );
        return;
    }

    const char *filepath = Cmd_Argv( 1 );

    sndPcm_t pcm;
    int result = Snd_Decode( filepath, &pcm );

    if ( result != 0 )
    {
        Com_Printf( DEC_LOG "Failed to decode audio file: %s\n", filepath );
        return;
    }

    float durSec = (float)pcm.samples / (float)pcm.rate; // calculate duration in seconds

    Com_Printf( DEC_LOG "Rate: %i\n", pcm.rate );
    Com_Printf( DEC_LOG "Channels: %i\n", pcm.channels );
    Com_Printf( DEC_LOG "Sample Count: %i\n", pcm.samples );
    Com_Printf( DEC_LOG "Duration: %s\n", Com_FormatDuration( durSec ) );
    
    Snd_FreePcm( &pcm );
}

void Snd_DecodeInit( void )
{
    Cmd_Create( "snd_decodetest", Cmd_DecodeTest );
    Com_Printf( DEC_LOG "Initialized audio decoder\n" );
}
