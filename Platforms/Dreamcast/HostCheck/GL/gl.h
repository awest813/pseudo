/* Host-check stub of kos-ports libGL (GLdc) <GL/gl.h>.
 *
 * Mirrors the subset of the real GLdc API surface that PSeudo uses, so
 * that compiling the emulator on a PC catches use of entry points GLdc
 * does not provide. Deliberately ABSENT, because GLdc lacks them:
 * glTexCoord2s, glVertex2s, glClipPlane, GL_COMBINE, GL_RGB_SCALE.
 */
#ifndef PSEUDO_HOSTCHECK_GL_H
#define PSEUDO_HOSTCHECK_GL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef void           GLvoid;
typedef signed char    GLbyte;
typedef short          GLshort;
typedef int            GLint;
typedef unsigned char  GLubyte;
typedef unsigned short GLushort;
typedef unsigned int   GLuint;
typedef int            GLsizei;
typedef float          GLfloat;
typedef float          GLclampf;
typedef double         GLdouble;
typedef double         GLclampd;

/* Primitive types */
#define GL_POINTS                       0x0000
#define GL_LINES                        0x0001
#define GL_LINE_LOOP                    0x0002
#define GL_LINE_STRIP                   0x0003
#define GL_TRIANGLES                    0x0004
#define GL_TRIANGLE_STRIP               0x0005
#define GL_TRIANGLE_FAN                 0x0006
#define GL_QUADS                        0x0007

/* Matrix modes */
#define GL_MODELVIEW                    0x1700
#define GL_PROJECTION                   0x1701
#define GL_TEXTURE                      0x1702

/* Blending */
#define GL_BLEND                        0x0BE2
#define GL_ZERO                         0x0
#define GL_ONE                          0x1
#define GL_SRC_COLOR                    0x0300
#define GL_ONE_MINUS_SRC_COLOR          0x0301
#define GL_SRC_ALPHA                    0x0302
#define GL_ONE_MINUS_SRC_ALPHA          0x0303
#define GL_DST_ALPHA                    0x0304
#define GL_ONE_MINUS_DST_ALPHA          0x0305
#define GL_DST_COLOR                    0x0306
#define GL_ONE_MINUS_DST_COLOR          0x0307

/* Texturing */
#define GL_TEXTURE_2D                   0x0DE1
#define GL_TEXTURE_WRAP_S               0x2802
#define GL_TEXTURE_WRAP_T               0x2803
#define GL_TEXTURE_MAG_FILTER           0x2800
#define GL_TEXTURE_MIN_FILTER           0x2801
#define GL_REPEAT                       0x2901
#define GL_NEAREST                      0x2600
#define GL_LINEAR                       0x2601
#define GL_TEXTURE_ENV                  0x2300
#define GL_TEXTURE_ENV_MODE             0x2200
#define GL_TEXTURE_ENV_COLOR            0x2201
#define GL_MODULATE                     0x2100
#define GL_DECAL                        0x2101
#define GL_REPLACE                      0x1E01

/* Clip planes: GLdc defines the constants but has no glClipPlane */
#define GL_CLIP_PLANE0                  0x3000
#define GL_CLIP_PLANE1                  0x3001
#define GL_CLIP_PLANE2                  0x3002
#define GL_CLIP_PLANE3                  0x3003
#define GL_CLIP_PLANE4                  0x3004
#define GL_CLIP_PLANE5                  0x3005

/* Buffers / formats */
#define GL_COLOR_BUFFER_BIT             0x00004000
#define GL_UNSIGNED_BYTE                0x1401
#define GL_UNSIGNED_SHORT               0x1403
#define GL_RGB                          0x1907
#define GL_RGBA                         0x1908
#define GL_UNSIGNED_SHORT_5_6_5         0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4       0x8033
#define GL_UNSIGNED_SHORT_1_5_5_5_REV   0x8366

void glBegin(GLenum mode);
void glEnd(void);
void glVertex2f(GLfloat x, GLfloat y);
void glTexCoord2f(GLfloat u, GLfloat v);
void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);
void glRecti(GLint x1, GLint y1, GLint x2, GLint y2);

void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glLineWidth(GLfloat width);

void glClear(GLbitfield mask);
void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glFlush(void);
void glFinish(void);

void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glOrtho(GLdouble left, GLdouble right,
             GLdouble bottom, GLdouble top,
             GLdouble znear, GLdouble zfar);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glGenTextures(GLsizei n, GLuint *textures);
void glDeleteTextures(GLsizei n, GLuint *textures);
void glBindTexture(GLenum target, GLuint texture);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexEnvi(GLenum target, GLenum pname, GLint param);
void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *pixels);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type, const GLvoid *pixels);

#ifdef __cplusplus
}
#endif

#endif
