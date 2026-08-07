#include "Local.h"

#define DR_FLAC_IMPLEMENTATION
#include "../extern/dr_flac.h"

#define DEC_LOG "^3[Audio Decode]^7 "

// Snd_DecodeFLAC
// takes raw file bytes already in memory, decoes to interleaved signed 16-bit PCM
// Returns void fileData, long fileSize, sndPcm_t *out
// DRFLAC_API drflac_int16* drflac_open_memory_and_read_pcm_frames_s16(
//  const void* data, 
//  size_t dataSize, 
//  unsigned int* channels, 
//  unsigned int* sampleRate, 
//  drflac_uint64* totalPCMFrameCount, 
//  const drflac_allocation_callbacks* pAllocationCallbacks
// );
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

    int result = Snd_DecodeFLAC( buf, size, out );

    FS_FreeFile( buf ); 

    return result; // 0 = success, -1 = failure
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
        drflac_free( pcm->data, NULL );
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
    Cmd_Create( "decodetest", Cmd_DecodeTest );
    Com_Printf( DEC_LOG "Initialized audio decoder\n" );
}
