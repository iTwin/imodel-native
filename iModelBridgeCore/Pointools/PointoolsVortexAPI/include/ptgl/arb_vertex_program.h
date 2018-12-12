/*
	ARB Vertex Program Extension header file.
	By Nutty
*/

#ifndef _ARB_VERTEX_PROGRAM_H_
#define _ARB_VERTEX_PROGRAM_H_

#include "ptgl.h"

#if defined _WIN32

#include <windows.h>

#else

#define APIENTRY __stdcall

#endif

#include <GL/gl.h>
namespace ptgl
{

//Prototype of initialization function.
bool PTGL_API InitARBVertexProgram(void);

}
//Extension string.
const char *const ARB_VERTEX_PROGRAM_STRING = "GL_ARB_vertex_program";


//Tokens.

//Accepted by the <cap> parameter of Disable, Enable, and IsEnabled, by the
//<pname> parameter of GetBooleanv, GetIntegerv, GetGLfloatv, and GetGLdoublev,
//and by the <target> parameter of ProgramStringARB, BindProgramARB,
//ProgramEnvParameter4[df][v]ARB, ProgramLocalParameter4[df][v]ARB,
//GetProgramEnvParameter[df]vARB, GetProgramLocalParameter[df]vARB,
//GetProgramivARB, and GetProgramStringARB.

#define GL_VERTEX_PROGRAM_ARB                              0x8620

//Accepted by the <cap> parameter of Disable, Enable, and IsEnabled, and by
//the <pname> parameter of GetBooleanv, GetIntegerv, GetGLfloatv, and GetGLdoublev:

#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB                   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB                     0x8643
#define GL_COLOR_SUM_ARB                                   0x8458

//Accepted by the <format> parameter of ProgramStringARB:

#define GL_PROGRAM_FORMAT_ASCII_ARB                        0x8875

//Accepted by the <pname> parameter of GetVertexAttrib[dfi]vARB:

#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                    0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                  0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                    0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB              0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB                       0x8626

//Accepted by the <pname> parameter of GetVertexAttribPointervARB:

#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                 0x8645

//Accepted by the <pname> parameter of GetProgramivARB:

#define GL_PROGRAM_LENGTH_ARB                              0x8627
#define GL_PROGRAM_FORMAT_ARB                              0x8876
#define GL_PROGRAM_BINDING_ARB                             0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB                        0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                    0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB             0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB                         0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB                     0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                  0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB              0x88A7
#define GL_PROGRAM_PARAMETERS_ARB                          0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB                      0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB                   0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB               0x88AB
#define GL_PROGRAM_ATTRIBS_ARB                             0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB                         0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB                      0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                  0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB                   0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB               0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB            0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB        0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                  0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                 0x88B6

//Accepted by the <pname> parameter of GetProgramStringARB:

#define GL_PROGRAM_STRING_ARB                              0x8628

//Accepted by the <pname> parameter of GetBooleanv, GetIntegerv,
//GetGLfloatv, and GetGLdoublev:

#define GL_PROGRAM_ERROR_POSITION_ARB                      0x864B
#define GL_CURRENT_MATRIX_ARB                              0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                    0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                  0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB                          0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB                        0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB              0x862E

//Accepted by the <name> parameter of GetString:

#define GL_PROGRAM_ERROR_STRING_ARB                        0x8874

//Accepted by the <mode> parameter of MatrixMode:

#define GL_MATRIX0_ARB                                     0x88C0
#define GL_MATRIX1_ARB                                     0x88C1
#define GL_MATRIX2_ARB                                     0x88C2
#define GL_MATRIX3_ARB                                     0x88C3
#define GL_MATRIX4_ARB                                     0x88C4
#define GL_MATRIX5_ARB                                     0x88C5
#define GL_MATRIX6_ARB                                     0x88C6
#define GL_MATRIX7_ARB                                     0x88C7
#define GL_MATRIX8_ARB                                     0x88C8
#define GL_MATRIX9_ARB                                     0x88C9
#define GL_MATRIX10_ARB                                    0x88CA
#define GL_MATRIX11_ARB                                    0x88CB
#define GL_MATRIX12_ARB                                    0x88CC
#define GL_MATRIX13_ARB                                    0x88CD
#define GL_MATRIX14_ARB                                    0x88CE
#define GL_MATRIX15_ARB                                    0x88CF
#define GL_MATRIX16_ARB                                    0x88D0
#define GL_MATRIX17_ARB                                    0x88D1
#define GL_MATRIX18_ARB                                    0x88D2
#define GL_MATRIX19_ARB                                    0x88D3
#define GL_MATRIX20_ARB                                    0x88D4
#define GL_MATRIX21_ARB                                    0x88D5
#define GL_MATRIX22_ARB                                    0x88D6
#define GL_MATRIX23_ARB                                    0x88D7
#define GL_MATRIX24_ARB                                    0x88D8
#define GL_MATRIX25_ARB                                    0x88D9
#define GL_MATRIX26_ARB                                    0x88DA
#define GL_MATRIX27_ARB                                    0x88DB
#define GL_MATRIX28_ARB                                    0x88DC
#define GL_MATRIX29_ARB                                    0x88DD
#define GL_MATRIX30_ARB                                    0x88DE
#define GL_MATRIX31_ARB                                    0x88DF



//Function types.
typedef void (APIENTRY *PFNGLVERTEXATTRIB1SARB)(GLuint index, GLshort x);
typedef void (APIENTRY *PFNGLVERTEXATTRIB1FARB)(GLuint index, GLfloat x);
typedef void (APIENTRY *PFNGLVERTEXATTRIB1DARB)(GLuint index, GLdouble x);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2SARB)(GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2FARB)(GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2DARB)(GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3SARB)(GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3FARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3DARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4SARB)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4FARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4DARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUBARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);

typedef void (APIENTRY *PFNGLVERTEXATTRIB1SVARB)(GLuint index, const GLshort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB1FVARB)(GLuint index, const GLfloat *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB1DVARB)(GLuint index, const GLdouble *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2SVARB)(GLuint index, const GLshort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2FVARB)(GLuint index, const GLfloat *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB2DVARB)(GLuint index, const GLdouble *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3SVARB)(GLuint index, const GLshort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3FVARB)(GLuint index, const GLfloat *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB3DVARB)(GLuint index, const GLdouble *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4BVARB)(GLuint index, const GLbyte *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4SVARB)(GLuint index, const GLshort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4IVARB)(GLuint index, const GLint *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4UBVARB)(GLuint index, const GLubyte *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4USVARB)(GLuint index, const GLushort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4UIVARB)(GLuint index, const GLuint *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4FVARB)(GLuint index, const GLfloat *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4DVARB)(GLuint index, const GLdouble *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NBVARB)(GLuint index, const GLbyte *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NSVARB)(GLuint index, const GLshort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NIVARB)(GLuint index, const GLint *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUBVARB)(GLuint index, const GLubyte *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUSVARB)(GLuint index, const GLushort *v);
typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUIVARB)(GLuint index, const GLuint *v);

typedef void (APIENTRY *PFNGLVERTEXATTRIBPOINTERARB)(GLuint index, GLint size, GLenum type, bool normalized, GLsizei stride,  const void *pointer);
typedef void (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYARB)(GLuint index);
typedef void (APIENTRY *PFNGLDISABLEVERTEXATTRIBARRAYARB)(GLuint index);
typedef void (APIENTRY *PFNGLPROGRAMSTRINGARB)(GLenum target, GLenum format, GLsizei len, const void *string); 
typedef void (APIENTRY *PFNGLBINDPROGRAMARB)(GLenum target, GLuint program);
typedef void (APIENTRY *PFNGLDELETEPROGRAMSARB)(GLsizei n, const GLuint *programs);
typedef void (APIENTRY *PFNGLGENPROGRAMSARB)(GLsizei n, GLuint *programs);

typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4DARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4DVARB)(GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4FARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4FVARB)(GLenum target, GLuint index, const GLfloat *params);

typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DVARB)(GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FVARB)(GLenum target, GLuint index, const GLfloat *params);

typedef void (APIENTRY *PFNGLGETPROGRAMENVPARAMETERDVARB)(GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY *PFNGLGETPROGRAMENVPARAMETERFVARB)(GLenum target, GLuint index, GLfloat *params);

typedef void (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERDVARB)(GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERFVARB)(GLenum target, GLuint index, GLfloat *params);

typedef void (APIENTRY *PFNGLGETPROGRAMIVARB)(GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETPROGRAMSTRINGARB)(GLenum target, GLenum pname, void *string);

typedef void (APIENTRY *PFNGLGETVERTEXATTRIBDVARB)(GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY *PFNGLGETVERTEXATTRIBFVARB)(GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY *PFNGLGETVERTEXATTRIBIVARB)(GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETVERTEXATTRIBPOINTERVARB)(GLuint index, GLenum pname, void **pointer);
typedef GLboolean (APIENTRY *PFNGLISPROGRAMARB)(GLuint program);



//The fucntion pointers.

#if defined __cplusplus
extern "C" {
#endif


extern PTGL_API PFNGLVERTEXATTRIB1SARB glVertexAttrib1sARB;
extern PTGL_API PFNGLVERTEXATTRIB1FARB glVertexAttrib1fARB;
extern PTGL_API PFNGLVERTEXATTRIB1DARB glVertexAttrib1dARB;
extern PTGL_API PFNGLVERTEXATTRIB2SARB glVertexAttrib2sARB;
extern PTGL_API PFNGLVERTEXATTRIB2FARB glVertexAttrib2fARB;
extern PTGL_API PFNGLVERTEXATTRIB2DARB glVertexAttrib2dARB;
extern PTGL_API PFNGLVERTEXATTRIB3SARB glVertexAttrib3sARB;
extern PTGL_API PFNGLVERTEXATTRIB3FARB glVertexAttrib3fARB;
extern PTGL_API PFNGLVERTEXATTRIB3DARB glVertexAttrib3dARB;
extern PTGL_API PFNGLVERTEXATTRIB4SARB glVertexAttrib4sARB;
extern PTGL_API PFNGLVERTEXATTRIB4FARB glVertexAttrib4fARB;
extern PTGL_API PFNGLVERTEXATTRIB4DARB glVertexAttrib4dARB;
extern PTGL_API PFNGLVERTEXATTRIB4NUBARB glVertexAttrib4NubARB;

extern PTGL_API PFNGLVERTEXATTRIB1SVARB glVertexAttrib1svARB;
extern PTGL_API PFNGLVERTEXATTRIB1FVARB glVertexAttrib1fvARB;
extern PTGL_API PFNGLVERTEXATTRIB1DVARB glVertexAttrib1dvARB;
extern PTGL_API PFNGLVERTEXATTRIB2SVARB glVertexAttrib2svARB;
extern PTGL_API PFNGLVERTEXATTRIB2FVARB glVertexAttrib2fvARB;
extern PTGL_API PFNGLVERTEXATTRIB2DVARB glVertexAttrib2dvARB;
extern PTGL_API PFNGLVERTEXATTRIB3SVARB glVertexAttrib3svARB;
extern PTGL_API PFNGLVERTEXATTRIB3FVARB glVertexAttrib3fvARB;
extern PTGL_API PFNGLVERTEXATTRIB3DVARB glVertexAttrib3dvARB;
extern PTGL_API PFNGLVERTEXATTRIB4BVARB glVertexAttrib4bvARB;
extern PTGL_API PFNGLVERTEXATTRIB4SVARB glVertexAttrib4svARB;
extern PTGL_API PFNGLVERTEXATTRIB4IVARB glVertexAttrib4ivARB;
extern PTGL_API PFNGLVERTEXATTRIB4UBVARB glVertexAttrib4ubvARB;
extern PTGL_API PFNGLVERTEXATTRIB4USVARB glVertexAttrib4usvARB;
extern PTGL_API PFNGLVERTEXATTRIB4UIVARB glVertexAttrib4uivARB;
extern PTGL_API PFNGLVERTEXATTRIB4FVARB glVertexAttrib4fvARB;
extern PTGL_API PFNGLVERTEXATTRIB4DVARB glVertexAttrib4dvARB;
extern PTGL_API PFNGLVERTEXATTRIB4NBVARB glVertexAttrib4NbvARB;
extern PTGL_API PFNGLVERTEXATTRIB4NSVARB glVertexAttrib4NsvARB;
extern PTGL_API PFNGLVERTEXATTRIB4NIVARB glVertexAttrib4NivARB;
extern PTGL_API PFNGLVERTEXATTRIB4NUBVARB glVertexAttrib4NubvARB;
extern PTGL_API PFNGLVERTEXATTRIB4NUSVARB glVertexAttrib4NusvARB;
extern PTGL_API PFNGLVERTEXATTRIB4NUIVARB glVertexAttrib4NuivARB;

extern PTGL_API PFNGLVERTEXATTRIBPOINTERARB glVertexAttribPointerARB;
extern PTGL_API PFNGLENABLEVERTEXATTRIBARRAYARB glEnableVertexAttribArrayARB;
extern PTGL_API PFNGLDISABLEVERTEXATTRIBARRAYARB glDisableVertexAttribArrayARB;
extern PTGL_API PFNGLPROGRAMSTRINGARB glProgramStringARB;
extern PTGL_API PFNGLBINDPROGRAMARB glBindProgramARB;
extern PTGL_API PFNGLDELETEPROGRAMSARB glDeleteProgramsARB;
extern PTGL_API PFNGLGENPROGRAMSARB glGenProgramsARB;

extern PTGL_API PFNGLPROGRAMENVPARAMETER4DARB glProgramEnvParameter4dARB;
extern PTGL_API PFNGLPROGRAMENVPARAMETER4DVARB glProgramEnvParameter4dvARB;
extern PTGL_API PFNGLPROGRAMENVPARAMETER4FARB glProgramEnvParameter4fARB;
extern PTGL_API PFNGLPROGRAMENVPARAMETER4FVARB glProgramEnvParameter4fvARB;

extern PTGL_API PFNGLPROGRAMLOCALPARAMETER4DARB glProgramLocalParameter4dARB;
extern PTGL_API PFNGLPROGRAMLOCALPARAMETER4DVARB glProgramLocalParameter4dvARB;
extern PTGL_API PFNGLPROGRAMLOCALPARAMETER4FARB glProgramLocalParameter4fARB;
extern PTGL_API PFNGLPROGRAMLOCALPARAMETER4FVARB glProgramLocalParameter4fvARB;

extern PTGL_API PFNGLGETPROGRAMENVPARAMETERDVARB glGetProgramEnvParameterdvARB;
extern PTGL_API PFNGLGETPROGRAMENVPARAMETERFVARB glGetProgramEnvParameterfvARB;

extern PTGL_API PFNGLGETPROGRAMLOCALPARAMETERDVARB glGetProgramLocalParameterdvARB;
extern PTGL_API PFNGLGETPROGRAMLOCALPARAMETERFVARB glGetProgramLocalParameterfvARB;

extern PTGL_API PFNGLGETPROGRAMIVARB glGetProgramivARB;
extern PTGL_API PFNGLGETPROGRAMSTRINGARB glGetProgramStringARB;
extern PTGL_API PFNGLGETVERTEXATTRIBDVARB glGetVertexAttribdvARB;
extern PTGL_API PFNGLGETVERTEXATTRIBFVARB glGetVertexAttribfvARB;
extern PTGL_API PFNGLGETVERTEXATTRIBIVARB glGetVertexAttribivARB;
extern PTGL_API PFNGLGETVERTEXATTRIBPOINTERVARB glGetVertexAttribPointervARB;
extern PTGL_API PFNGLISPROGRAMARB glIsProgramARB;


#if defined __cplusplus
}
#endif



#endif	//_ARB_VERTEX_PROGRAM_H_
