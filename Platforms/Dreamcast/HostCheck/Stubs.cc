/* No-op implementations backing the host-check stub headers, so the
 * Dreamcast build of PSeudo can be fully linked on a PC. */

#include <kos.h>
#include <dc/biosfont.h>
#include <GL/gl.h>
#include <GL/glkos.h>
#include <AL/al.h>
#include <AL/alc.h>

extern "C" {

/* --- KOS ------------------------------------------------------------- */

static struct maple_device *const no_device = 0;

maple_device_t *maple_enum_type(int, unsigned int)    { return no_device; }
void *maple_dev_status(maple_device_t *)              { return 0; }

/* Threads are not spawned on the host: the entry points run real
 * emulator loops that need a BIOS to make progress. */
kthread_t *thd_create(int, void *(*)(void *), void *) { return 0; }
int thd_join(kthread_t *, void **)                    { return 0; }
void thd_pass(void)                                   { }
void thd_sleep(int)                                   { }

/* A checkerboard glyph is enough for compile/link checking */
unsigned char *bfont_find_char(unsigned int) {
    static unsigned char glyph[36] = {
        0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55,
        0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55,
        0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55,
        0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55, 0xaa, 0xa5, 0x55,
    };
    return glyph;
}

/* --- GLdc ------------------------------------------------------------ */

void glKosInit(void)                                  { }
void glKosSwapBuffers(void)                           { }

void glBegin(GLenum)                                  { }
void glEnd(void)                                      { }
void glVertex2f(GLfloat, GLfloat)                     { }
void glTexCoord2f(GLfloat, GLfloat)                   { }
void glColor4ub(GLubyte, GLubyte, GLubyte, GLubyte)   { }
void glRecti(GLint, GLint, GLint, GLint)              { }

void glEnable(GLenum)                                 { }
void glDisable(GLenum)                                { }
void glBlendFunc(GLenum, GLenum)                      { }
void glLineWidth(GLfloat)                             { }

void glClear(GLbitfield)                              { }
void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) { }
void glFlush(void)                                    { }
void glFinish(void)                                   { }

void glMatrixMode(GLenum)                             { }
void glLoadIdentity(void)                             { }
void glScalef(GLfloat, GLfloat, GLfloat)              { }
void glOrtho(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) { }
void glViewport(GLint, GLint, GLsizei, GLsizei)       { }
void glScissor(GLint, GLint, GLsizei, GLsizei)        { }

void glGenTextures(GLsizei n, GLuint *textures) {
    static GLuint next = 1;
    for (GLsizei i = 0; i < n; i++) {
        textures[i] = next++;
    }
}
void glDeleteTextures(GLsizei, GLuint *)              { }
void glBindTexture(GLenum, GLuint)                    { }
void glTexParameteri(GLenum, GLenum, GLint)           { }
void glTexEnvi(GLenum, GLenum, GLint)                 { }
void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                  GLenum, GLenum, const GLvoid *)     { }
void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei,
                     GLenum, GLenum, const GLvoid *)  { }

/* --- ALdc ------------------------------------------------------------ */

static ALCdevice  *const no_al_device  = (ALCdevice *)0;
static ALCcontext *const no_al_context = (ALCcontext *)0;

ALCdevice  *alcOpenDevice(const char *)               { return no_al_device; }
void        alcCloseDevice(ALCdevice *)               { }
ALCcontext *alcCreateContext(ALCdevice *, const int *){ return no_al_context; }
void        alcMakeContextCurrent(ALCcontext *)       { }

void alGenSources(ALsizei n, ALuint *sources) {
    for (ALsizei i = 0; i < n; i++) {
        sources[i] = (ALuint)(i + 1);
    }
}
void alDeleteSources(ALsizei, const ALuint *)         { }
void alGenBuffers(ALsizei n, ALuint *buffers) {
    for (ALsizei i = 0; i < n; i++) {
        buffers[i] = (ALuint)(i + 1);
    }
}
void alDeleteBuffers(ALsizei, const ALuint *)         { }
void alBufferData(ALuint, ALenum, const ALvoid *, ALsizei, ALsizei) { }
void alSourceQueueBuffers(ALuint, ALsizei, const ALuint *) { }
void alSourceUnqueueBuffers(ALuint, ALsizei, ALuint *buffers) {
    if (buffers) {
        buffers[0] = 1;
    }
}
void alGetSourcei(ALuint, ALenum param, ALint *value) {
    if (value) {
        *value = (param == AL_SOURCE_STATE) ? AL_PLAYING : 0;
    }
}
void alSourcePlay(ALuint)                             { }

} /* extern "C" */
