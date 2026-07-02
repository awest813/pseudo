/* Host-check stub of kos-ports libAL (ALdc) <AL/alc.h>. */
#ifndef PSEUDO_HOSTCHECK_ALC_H
#define PSEUDO_HOSTCHECK_ALC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALCdevice  ALCdevice;
typedef struct ALCcontext ALCcontext;

ALCdevice  *alcOpenDevice(const char *devicename);
void        alcCloseDevice(ALCdevice *device);
ALCcontext *alcCreateContext(ALCdevice *device, const int *attrlist);
void        alcMakeContextCurrent(ALCcontext *context);

#ifdef __cplusplus
}
#endif

#endif
