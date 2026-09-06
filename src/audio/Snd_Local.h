#pragma once

#include "Local.h"
#include <AL/al.h>
#include <AL/alc.h>

// decode output
struct sndPcm_t
{
	void	*data;
	int		samples;
	int		rate;
	int		channels;
};

// buffer cache
#define MAX_SOUNDS		256
#define MAX_SOUNDPATH	256

struct sndBuffer_t
{
	char			name[MAX_SOUNDPATH];
	unsigned int	alBuffer;
	int				rate;
	int				channels;
	int				samples;
	int				hashNext;
};

inline ALenum Snd_ALFormat( int channels )
{
	switch ( channels )
	{
	case 1:		return AL_FORMAT_MONO16;
	case 2:		return AL_FORMAT_STEREO16;
	default:	return 0;
	
	}
}

void		Snd_Init( void );
void		Snd_Shutdown( void );
int			Snd_Decode( const char *virtualPath, sndPcm_t *out );
void		Snd_FreePcm( sndPcm_t *pcm );
void		Snd_DecodeInit( void );
void		Snd_CacheInit( void );
void		Snd_CacheShutdown( void );
int			Snd_RegisterSound( const char *name );
sndBuffer_t	*Snd_GetBuffer( int handle );