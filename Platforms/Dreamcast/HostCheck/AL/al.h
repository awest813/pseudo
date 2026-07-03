/* Host-check stub of kos-ports libAL (ALdc) <AL/al.h>. */
#ifndef PSEUDO_HOSTCHECK_AL_H
#define PSEUDO_HOSTCHECK_AL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int          ALint;
typedef unsigned int ALuint;
typedef int          ALsizei;
typedef int          ALenum;
typedef void         ALvoid;

#define AL_FORMAT_STEREO16      0x1103
#define AL_BUFFERS_PROCESSED    0x1016
#define AL_SOURCE_STATE         0x1010
#define AL_PLAYING              0x1012

void alGenSources(ALsizei n, ALuint *sources);
void alDeleteSources(ALsizei n, const ALuint *sources);
void alGenBuffers(ALsizei n, ALuint *buffers);
void alDeleteBuffers(ALsizei n, const ALuint *buffers);
void alBufferData(ALuint buffer, ALenum format, const ALvoid *data,
                  ALsizei size, ALsizei freq);
void alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint *buffers);
void alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint *buffers);
void alGetSourcei(ALuint source, ALenum param, ALint *value);
void alSourcePlay(ALuint source);

#ifdef __cplusplus
}
#endif

#endif
