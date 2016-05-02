/********************************************************************
glsl.cpp
Version: 1.0.0_rc4
Last update: 2006/11/12 (Geometry Shader Support)

(c) 2003-2006 by Martin Christen. All Rights reserved.
*********************************************************************/

#include "PointoolsVortexAPIInternal.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <assert.h>

#include <ptgl/glShader.h>
#include <ptappdll/ptapp.h>

using namespace std;
using namespace ptgl;

bool useGLSL = false;
bool extensions_init = false;
bool bGeometryShader = false;
bool bGPUShader4 = false;

#define GLSL_VERSION_NUM "110"

static const char* s_hashmask = "Vertex program initialization";
extern bool g_showDebugInfo;

namespace ptgl
{
//-----------------------------------------------------------------------------
// Error, Warning and Info Strings
char* aGLSLStrings[] = {
        "[e00] GLSL is not available!",
        "[e01] Not a valid program object!",
        "[e02] Not a valid object!",
        "[e03] Out of memory!",
        "[e04] Unknown compiler error!",
        "[e05] Linker log is not available!",
        "[e06] Compiler log is not available!",
        "[Empty]"
        };

	int _hash_out(char*txt)
	{
		int i = 0;
		while (txt[i]!='\n') txt[i++]  ^= s_hashmask[i%29];
		txt[i] = 0xaa + rand() * 0x20 / RAND_MAX;
		return ++i;
	}
	int _hash_in(char *txt)
	{
		int i = 0;
		while ((unsigned char)txt[i]< 0xaa)
			txt[i++]  ^= s_hashmask[i%29];
		txt[i] = '\n';
		txt[++i] = '\0';
		return i;
	}

//-----------------------------------------------------------------------------      

// GL ERROR CHECK
   int CheckGLError(char *file, int line)
   {
	   GLenum glErr;
	   int    retCode = 0;

	   glErr = glGetError();
	   while (glErr != GL_NO_ERROR) 
       {
	     const GLubyte* sError = gluErrorString(glErr);

		if (g_showDebugInfo)
		{
		 if (sError)
			cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
		 else
		    cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;
		}
		 retCode = 1;
		 glErr = glGetError();
	   }
	   return retCode;
   }
   
#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)

//----------------------------------------------------------------------------- 
   bool InitOpenGLExtensions(void)
   {
      if (extensions_init) return true;
      extensions_init = true;

      GLenum err = glewInit();

      if (GLEW_OK != err)
      {
         if (g_showDebugInfo) cout << "Error:" << glewGetErrorString(err) << endl;
         extensions_init = false;
         return false;
      }
	if (g_showDebugInfo) 
	{
      cout << "OpenGL Vendor: " << (char*) glGetString(GL_VENDOR) << "\n";
      cout << "OpenGL Renderer: " << (char*) glGetString(GL_RENDERER) << "\n";
      cout << "OpenGL Version: " << (char*) glGetString(GL_VERSION) << "\n\n";
      //cout << "OpenGL Extensions:\n" << (char*) glGetString(GL_EXTENSIONS) << "\n\n";
	}
      HasGLSLSupport();

      return true;
   }


   bool HasGLSLSupport(void)
   {
      bGeometryShader = HasGeometryShaderSupport();
      bGPUShader4     = HasShaderModel4();
      
      if (useGLSL) return true;  // already initialized and GLSL is available
      useGLSL = true;
     
      if (!extensions_init) InitOpenGLExtensions();  // extensions were not yet initialized!!

      if (g_showDebugInfo) 
	  {
		  if (GLEW_VERSION_2_0)
		  {
			 cout << "OpenGL 2.0 (or higher) is available!" << endl;
		  }
		  else if (GLEW_VERSION_1_5)
		  {
			 cout << "OpenGL 1.5 core functions are available" << endl;
		  }
		  else if (GLEW_VERSION_1_4)
		  {
			 cout << "OpenGL 1.4 core functions are available" << endl;
		  }
		  else if (GLEW_VERSION_1_3)
		  {
			 cout << "OpenGL 1.3 core functions are available" << endl;
		  }
		  else if (GLEW_VERSION_1_2)
		  {
			 cout << "OpenGL 1.2 core functions are available" << endl;
		  }
	  }

      if (GL_TRUE != glewGetExtension("GL_ARB_fragment_shader"))
      {
          if (g_showDebugInfo) cout << "[WARNING] GL_ARB_fragment_shader extension is not available!\n";
          useGLSL = false;
      }

      if (GL_TRUE != glewGetExtension("GL_ARB_vertex_shader"))
      {
          if (g_showDebugInfo) cout << "[WARNING] GL_ARB_vertex_shader extension is not available!\n";
          useGLSL = false;
      }

      if (GL_TRUE != glewGetExtension("GL_ARB_shader_objects"))
      {
          if (g_showDebugInfo) cout << "[WARNING] GL_ARB_shader_objects extension is not available!\n";
          useGLSL = false;
      }

	  if (g_showDebugInfo) 
	  {
		  if (useGLSL)
		  {
			cout << "[OK] OpenGL Shading Language is available!\n\n";
		  }
		  else
		  {
			cout << "[FAILED] OpenGL Shading Language is not available...\n\n";
		  } 
	  }
	  
      return useGLSL;
   }


   bool HasOpenGL2Support(void)
   {
      if (!extensions_init) InitOpenGLExtensions();

      return (GLEW_VERSION_2_0 == GL_TRUE);
   }   


   bool HasGeometryShaderSupport(void)
   {
      if (GL_TRUE != glewGetExtension("GL_EXT_geometry_shader4"))
         return false;
      
      return true;
   }

   bool HasShaderModel4(void)
   {
      if (GL_TRUE != glewGetExtension("GL_EXT_gpu_shader4"))
         return false;

      return true;
   }
}

//----------------------------------------------------------------------------- 

// ************************************************************************
// Implementation of Shader class
// ************************************************************************
 
Shader::Shader()
{
  InitOpenGLExtensions();
  ProgramObject = 0;
  linker_log = 0;
  is_linked = false; 
  _mM = false;
  _noshader = true;

  if (!useGLSL)
  {
	  if (g_showDebugInfo)
		cout << "**ERROR: OpenGL Shading Language is NOT available!" << endl;
  }
  else
  { 
      ProgramObject = glCreateProgram();
  }
      
}

//----------------------------------------------------------------------------- 

Shader::~Shader()
{
    if (linker_log!=0) free(linker_log);
    if (useGLSL)
    {
       for (unsigned int i=0;i<_shaderList.size();i++)
       {
            glDetachShader(ProgramObject, _shaderList[i]->shaderObject);
            CHECK_GL_ERROR(); // if you get an error here, you deleted the Program object first and then
                           // the shaderObject! Always delete ShaderObjects last!
            if (_mM) delete _shaderList[i]; 
       }                      

       glDeleteShader(ProgramObject);
       CHECK_GL_ERROR();
    }

}

//----------------------------------------------------------------------------- 

void Shader::addShader(ShaderObject* ShaderProgram)
{
if (!useGLSL) return;

   if (ShaderProgram==0) return;

   
   if (!ShaderProgram->is_compiled)
   {
        if (!ShaderProgram->compile())
        {
			if (g_showDebugInfo)
				cout << "...compile ERROR!\n";
            return;
        }
        else
        {   
			if (g_showDebugInfo)
				cout << "...ok!\n";
        }
   }

   _shaderList.push_back(ShaderProgram); 
   
}

//----------------------------------------------------------------------------- 

void Shader::SetInputPrimitiveType(int nInputPrimitiveType)
{
   _nInputPrimitiveType = nInputPrimitiveType;
}

void Shader::SetOutputPrimitiveType(int nOutputPrimitiveType)
{
   _nOutputPrimitiveType = nOutputPrimitiveType;
}

void Shader::SetVerticesOut(int nVerticesOut)
{
   _nVerticesOut = nVerticesOut;
}
//----------------------------------------------------------------------------- 

bool Shader::link(void)
{
if (!useGLSL) return false;

unsigned int i;

    if (_bUsesGeometryShader)
    {
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_INPUT_TYPE_EXT, _nInputPrimitiveType);
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_OUTPUT_TYPE_EXT, _nOutputPrimitiveType);
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_VERTICES_OUT_EXT, _nVerticesOut);
    }

    if (is_linked)  // already linked, detach everything first
    {
      if (g_showDebugInfo)  cout << "**warning** Object is already linked, trying to link again" << endl;
       for (i=0;i<_shaderList.size();i++)
       {
            glDetachShader(ProgramObject, _shaderList[i]->shaderObject);
            CHECK_GL_ERROR();
       }
    }
    
    for (i=0;i<_shaderList.size();i++)
    {
        glAttachShader(ProgramObject, _shaderList[i]->shaderObject);
        CHECK_GL_ERROR();
    }
    
    GLint linked; // bugfix Oct-06-2006
    glLinkProgram(ProgramObject);
    CHECK_GL_ERROR();
    glGetProgramiv(ProgramObject, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR();

    if (linked)
    {
        is_linked = true;
        return true;
    }
    else
    {
        if (g_showDebugInfo) cout << "**linker error**\n";
    }

return false;
}

//----------------------------------------------------------------------------- 
// Compiler Log: Ausgabe der Compiler Meldungen in String

char* Shader::getLinkerLog(void)
{    
	if (!useGLSL) return aGLSLStrings[0];
 
	GLint blen = 0;    // bugfix Oct-06-2006	
	GLsizei slen = 0;  // bugfix Oct-06-2006

 if (ProgramObject==0) return aGLSLStrings[2];
 glGetProgramiv(ProgramObject, GL_INFO_LOG_LENGTH , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (linker_log!=0) 
    {   
        free(linker_log);
        linker_log =0;
    }
    if ((linker_log = (GLcharARB*)malloc(blen)) == NULL) 
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLStrings[3];
    }

    glGetProgramInfoLog(ProgramObject, blen, &slen, linker_log);
    CHECK_GL_ERROR();
    
 }
 if (linker_log!=0)
    return (char*) linker_log;    
 else
    return aGLSLStrings[5];
   
 return aGLSLStrings[4];
}

void Shader::begin(void)
{
	if (!useGLSL) return;
	if (ProgramObject == 0) return;
	if (!_noshader) return;

    if (is_linked)
    {
        glUseProgram(ProgramObject);
        CHECK_GL_ERROR();
    }
}

//----------------------------------------------------------------------------- 

void Shader::end(void)
{
	if (!useGLSL) return;
	if (!_noshader) return;

    glUseProgram(0);
    CHECK_GL_ERROR();
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform1f(GLcharARB* varname, GLfloat v0, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index

    
    glUniform1f(loc, v0);
    
    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform2f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLint index)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2f(loc, v0, v1);
    
    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3f(loc, v0, v1, v2);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform4f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4f(loc, v0, v1, v2, v3);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform1i(GLcharARB* varname, GLint v0, GLint index)
{ 
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform1i(loc, v0);
    
    return true;
}

//-----------------------------------------------------------------------------

bool Shader::setUniform2i(GLcharARB* varname, GLint v0, GLint v1, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2i(loc, v0, v1);


    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3i(GLcharARB* varname, GLint v0, GLint v1, GLint v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3i(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool Shader::setUniform4i(GLcharARB* varname, GLint v0, GLint v1, GLint v2, GLint v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4i(loc, v0, v1, v2, v3);

    return true;
}
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------- 

bool Shader::setUniform1ui(GLcharARB* varname, GLuint v0, GLint index)
{ 
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform1uiEXT(loc, v0);
    
    return true;
}

//-----------------------------------------------------------------------------

bool Shader::setUniform2ui(GLcharARB* varname, GLuint v0, GLuint v1, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2uiEXT(loc, v0, v1);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3ui(GLcharARB* varname, GLuint v0, GLuint v1, GLuint v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3uiEXT(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool Shader::setUniform4ui(GLcharARB* varname, GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4uiEXT(loc, v0, v1, v2, v3);

    return true;
}
//-----------------------------------------------------------------------------

bool Shader::setUniform1fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform1fv(loc, count, value);

    return true;
}
bool Shader::setUniform2fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2fv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3fv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform4fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4fv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform1iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform1iv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform2iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2iv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

	if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3iv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform4iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4iv(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform1uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform1uivEXT(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform2uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform2uivEXT(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform3uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform3uivEXT(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniform4uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniform4uivEXT(loc, count, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniformMatrix2fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniformMatrix2fv(loc, count, transpose, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniformMatrix3fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniformMatrix3fv(loc, count, transpose, value);

    return true;
}

//----------------------------------------------------------------------------- 

bool Shader::setUniformMatrix4fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;
    
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return false;  // can't find variable / invalid index
    
    glUniformMatrix4fv(loc, count, transpose, value);

    return true;
}

//----------------------------------------------------------------------------- 

GLint Shader::GetUniformLocation(const GLcharARB *name)
{
	GLint loc;

	loc = glGetUniformLocation(ProgramObject, name);
	if (loc == -1) 
	{
        if (g_showDebugInfo) cout << "Error: can't find uniform variable \"" << name << "\"\n";
	}
	if (CHECK_GL_ERROR() && g_showDebugInfo ) cout << " Uniform: " << name << endl;
	return loc;
}

//----------------------------------------------------------------------------- 

void Shader::getUniformfv(GLcharARB* varname, GLfloat* values, GLint index)
{
if (!useGLSL) return;
 
    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return;  // can't find variable / invalid index

	glGetUniformfv(ProgramObject, loc, values);
	
}

//----------------------------------------------------------------------------- 

void Shader::getUniformiv(GLcharARB* varname, GLint* values, GLint index)
{
    if (!useGLSL) return;

    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return;  // can't find variable / invalid index
	
	glGetUniformiv(ProgramObject, loc, values);

}

//----------------------------------------------------------------------------- 

void Shader::getUniformuiv(GLcharARB* varname, GLuint* values, GLint index)
{
    if (!useGLSL) return;

    GLint loc = varname ? GetUniformLocation(varname) : index;

    if (loc==-1) 
       return;  // can't find variable / invalid index
	
	glGetUniformuivEXT(ProgramObject, loc, values);

}

//-----------------------------------------------------------------------------
void  Shader::BindAttribLocation(GLint index, GLchar* name)
{
   glBindAttribLocation(ProgramObject, index, name);
}

//-----------------------------------------------------------------------------
bool Shader::setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttribPointer(index, size, type, normalized, stride, pointer);
   return true;
}
//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib1f(GLuint index, GLfloat v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib1f(index, v0);

   return true;
}

bool Shader::setVertexAttrib2f(GLuint index, GLfloat v0, GLfloat v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib2f(index, v0, v1);
   
   return true;
}

bool Shader::setVertexAttrib3f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;
   
    glVertexAttrib3f(index, v0, v1, v2);
    
    return true;
}

bool Shader::setVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;
   
   glVertexAttrib4f(index, v0, v1, v2, v3);
   
   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib1d(GLuint index, GLdouble v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib1d(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib2d(GLuint index, GLdouble v0, GLdouble v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib2d(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib3d(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib4d(index, v0, v1, v2, v3);

   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib1s(GLuint index, GLshort v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib1s(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib2s(GLuint index, GLshort v0, GLshort v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib2s(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib3s(GLuint index, GLshort v0, GLshort v1, GLshort v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib3s(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib4s(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib4s(index, v0, v1, v2, v3);

   return true;
}
//----------------------------------------------------------------------------
bool Shader::setVertexAttribNormalizedByte(GLuint index, GLbyte v0, GLbyte v1, GLbyte v2, GLbyte v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

   glVertexAttrib4Nub(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib1i(GLuint index, GLint v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI1iEXT(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib2i(GLuint index, GLint v0, GLint v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI2iEXT(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib3i(GLuint index, GLint v0, GLint v1, GLint v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI3iEXT(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib4i(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI4iEXT(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib1ui(GLuint index, GLuint v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI1uiEXT(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool Shader::setVertexAttrib2ui(GLuint index, GLuint v0, GLuint v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI2uiEXT(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI3uiEXT(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool Shader::setVertexAttrib4ui(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_noshader) return true;

   glVertexAttribI4uiEXT(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------
// ************************************************************************
// Shader Program : Manage Shader Programs (Vertex/Fragment)
// ************************************************************************
ShaderObject::ShaderObject()
{
    InitOpenGLExtensions();
    compiler_log = 0;
    is_compiled = false;
    program_type = 0;
    shaderObject = 0;
    shaderSource = 0;
    _memalloc = false;
}

//----------------------------------------------------------------------------- 
ShaderObject::~ShaderObject()
{
   if (compiler_log!=0) free(compiler_log);
   if (shaderSource!=0)   
   {
        if (_memalloc)
            delete[] shaderSource;  // free ASCII Source
   }
   
   if (is_compiled)
   { 
        glDeleteObjectARB(shaderObject);
        CHECK_GL_ERROR();
   }
}

//----------------------------------------------------------------------------- 
unsigned long getFileLength(ifstream& file)
{
    if(!file.good()) return 0;
    
    unsigned long pos=file.tellg();
    file.seekg(0,ios::end);
    unsigned long len = file.tellg();
    file.seekg(ios::beg);
    
    return len;
}


//----------------------------------------------------------------------------- 
int ShaderObject::load(const char* filename, const char *preprocessor)
{
	char filepath[260];
	sprintf(filepath, "%s\\modules\\shaders\\%s", ptapp::apppathA(), filename);

	ifstream file;
	file.open(filepath, ios::in | std::ios::binary);
	if(!file)
	{   
		sprintf(filepath, "%s\\shaders\\%s", ptapp::apppathA(), filename);
		file.open( filepath, ios::in );

		if (!file) return -1;
	}
    
	unsigned long len = getFileLength(file);

	if (len==0) return -2;   // "Empty File" 
	
	
	if (shaderSource!=0)    // there is already a source loaded, free it!
	{
		if (_memalloc)
		delete[] shaderSource;
	}
	unsigned int i=0;
	
	char	ppbuffer[256];
	unsigned int pplen = 0;

	// put in version number first, ATI enforces this
	sprintf_s(ppbuffer, 256, "#version %s\n\r%s", GLSL_VERSION_NUM, preprocessor ? preprocessor : " ");
	pplen = strlen(ppbuffer);

	shaderSource = (GLubyte*) new char[len*2+pplen];
	if (shaderSource == 0) return -3;   // can't reserve memory
	
	for (unsigned int p=0; p<pplen; p++)
		shaderSource[i++] = ppbuffer[p];
	
	shaderSource[i] = 0;

	_memalloc = true;

	while(!file.eof())
	{
		char bf[260];
		memset(bf, '\0', 256);
		int p = 0;			
		do { file.get(bf[p]); }
		while (!file.eof() && (unsigned char)bf[p++] < 0xaa && p < 256);
		
		if (!p) continue;

		if ((unsigned char)bf[p-1] < 0xaa) 
			bf[p-1] = (char)0xaa;
		
		_hash_in(bf);
		
		int sl = strlen(bf);
		assert( sl < 256 );
		for (int s=0; s<sl;s++) 
			shaderSource[i++] = bf[s];
	}	

	//while (file.good())
	//{
	//	shaderSource[i] = file.get();       // get character from file.
	//	if (!file.eof())
	//		i++;
	//}

	shaderSource[i] = 0;  // 0 terminate it.

	file.close();
	  
	return 0;
}

//----------------------------------------------------------------------------- 
void ShaderObject::loadFromMemory(const char* program)
{
    if (shaderSource!=0)    // there is already a source loaded, free it!
    {
        if (_memalloc)
        delete[] shaderSource;
    }
   _memalloc = false;
   shaderSource = (GLubyte*) program;
      
}


// ----------------------------------------------------------------------------
// Compiler Log: Ausgabe der Compiler Meldungen in String
char* ShaderObject::getCompilerLog(void)
{    
if (!useGLSL) return aGLSLStrings[0];
 
 GLint blen = 0;	
 GLsizei slen = 0;	


 if (shaderObject==0) return aGLSLStrings[1]; // not a valid program object

 glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (compiler_log!=0) 
    {   
        free(compiler_log);
        compiler_log =0;
    }
    if ((compiler_log = (GLcharARB*)malloc(blen)) == NULL) 
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLStrings[3];
    }

     glGetInfoLogARB(shaderObject, blen, &slen, compiler_log);
     CHECK_GL_ERROR();
     //cout << "compiler_log: \n", compiler_log);     
 }
 if (compiler_log!=0)
    return (char*) compiler_log;
 else
   return aGLSLStrings[6];       

 return aGLSLStrings[4];
}

// ----------------------------------------------------------------------------
bool ShaderObject::compile(void)
{
	if (!useGLSL) return false;

	is_compiled = false;

	GLint compiled = 0;

	if (shaderSource==0) return false;

	GLint	length = (GLint) strlen((const char*)shaderSource);
	glShaderSourceARB(shaderObject, 1, (const GLcharARB **)&shaderSource, &length);
	CHECK_GL_ERROR();

	glCompileShaderARB(shaderObject); 
	CHECK_GL_ERROR();
	glGetObjectParameterivARB(shaderObject, GL_COMPILE_STATUS, &compiled);
	CHECK_GL_ERROR();

	if (compiled) is_compiled=true;
	else if (g_showDebugInfo) 
		cout << shaderSource << std::endl;
 
	return is_compiled;
}

// ----------------------------------------------------------------------------
GLint ShaderObject::getAttribLocation(char* attribName)
{
   return glGetAttribLocationARB(shaderObject, attribName);
}

// ----------------------------------------------------------------------------
aVertexShader::aVertexShader()
{
  program_type = 1; 
   if (useGLSL)
   {
       shaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
       CHECK_GL_ERROR();
   }
}

// ----------------------------------------------------
aVertexShader::~aVertexShader()
{
}
// ----------------------------------------------------

aFragmentShader::aFragmentShader()
{
    program_type = 2;
    if (useGLSL)
    {
        shaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB); 
        CHECK_GL_ERROR();
    }
}

// ----------------------------------------------------

aFragmentShader::~aFragmentShader()
{
}


// ----------------------------------------------------

aGeometryShader::aGeometryShader()
{
  program_type = 3; 
   if (useGLSL && bGeometryShader)
   {
       shaderObject = glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
       CHECK_GL_ERROR();
   }
}

// ----------------------------------------------------

aGeometryShader::~aGeometryShader()
{
}

// ----------------------------------------------------------------------------
// ShaderManager: Easy use of (multiple) Shaders

ShaderManager::ShaderManager()
{
   InitOpenGLExtensions();
   _nInputPrimitiveType = GL_TRIANGLES;
   _nOutputPrimitiveType = GL_TRIANGLE_STRIP;
   _nVerticesOut = 3;
   
}

ShaderManager::~ShaderManager()
{
   // free objects
   vector<Shader*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end()) 
   {
        //Shader* o = *i;
        i=_shaderObjectList.erase(i);
        //delete o;
   }
}

// ----------------------------------------------------------------------------

void ShaderManager::SetInputPrimitiveType(int nInputPrimitiveType)
{
      _nInputPrimitiveType = nInputPrimitiveType;

}

void ShaderManager::SetOutputPrimitiveType(int nOutputPrimitiveType)
{
      _nOutputPrimitiveType = nOutputPrimitiveType;

}

void ShaderManager::SetVerticesOut(int nVerticesOut)
{
   _nVerticesOut = nVerticesOut;
}

// ----------------------------------------------------------------------------
Shader* ShaderManager::loadfromFile(const char* vertexFile, const char* fragmentFile, 
									const char* vertexPP, const char * fragPP ) 
{
   Shader* o = new Shader();
   o->UsesGeometryShader(false);
  
   aVertexShader* tVertexShader = new aVertexShader;
   aFragmentShader* tFragmentShader = new aFragmentShader;

    // load vertex program
   if (vertexFile!=0)
   if (tVertexShader->load(vertexFile, vertexPP) != 0)
   { 
     if (g_showDebugInfo) cout << "error: can't load vertex shader!\n"; 
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
   }

  
  // Load fragment program
  if (fragmentFile!=0)
  if (tFragmentShader->load(fragmentFile, fragPP) != 0)
  {
     if (g_showDebugInfo) cout << "error: can't load fragment shader!\n";
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
  }
    
  // Compile vertex program
  if (vertexFile!=0)
  if (!tVertexShader->compile())
  {
      if (g_showDebugInfo)
	  {	
		cout << "***COMPILER ERROR (Vertex Shader):\n";
		cout << tVertexShader->getCompilerLog() << endl;
	  }
      delete o;
      delete tVertexShader;
      delete tFragmentShader;
      return 0;
  }
  if (g_showDebugInfo) 
  {
	cout << "***GLSL Compiler Log (Vertex Shader):\n";
	cout << tVertexShader->getCompilerLog() << "\n";
  }
   
  // Compile fragment program 
  if (fragmentFile!=0)
  if (!tFragmentShader->compile())
  {
	if (g_showDebugInfo) 
	{
     cout << "***COMPILER ERROR (Fragment Shader):\n";
     cout << tFragmentShader->getCompilerLog() << endl;
	}
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
     
  }
  if (g_showDebugInfo) 
  {
	cout << "***GLSL Compiler Log (Fragment Shader):\n";
	cout << tFragmentShader->getCompilerLog() << "\n";
  }
   
  // Add to object    
  if (vertexFile!=0) o->addShader(tVertexShader);
  if (fragmentFile!=0) o->addShader(tFragmentShader); 
  
  // link 
  if (!o->link())
  {
	if (g_showDebugInfo) 
	{
		cout << "**LINKER ERROR\n";
		cout << o->getLinkerLog() << endl;
	}
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
  }
  if (g_showDebugInfo) 
  {
	cout << "***GLSL Linker Log:\n";
	cout << o->getLinkerLog() << endl;
  }
  
  _shaderObjectList.push_back(o);
  o->manageMemory();

   return o;
}


Shader* ShaderManager::loadfromFile(char* vertexFile, char* geometryFile, char* fragmentFile)
{
   Shader* o = new Shader();
   o->UsesGeometryShader(true);
   o->SetInputPrimitiveType(_nInputPrimitiveType);
   o->SetOutputPrimitiveType(_nOutputPrimitiveType);
   o->SetVerticesOut(_nVerticesOut);
  
   aVertexShader* tVertexShader = new aVertexShader;
   aFragmentShader* tFragmentShader = new aFragmentShader;
   aGeometryShader* tGeometryShader = new aGeometryShader;

    // load vertex program
   if (vertexFile!=0)
      if (tVertexShader->load(vertexFile) != 0)
      { 
        if (g_showDebugInfo) cout << "error: can't load vertex shader!\n"; 
        delete o;
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
      }

  // Load geometry program
  if (geometryFile!=0)
     if (tGeometryShader->load(geometryFile) != 0)
     {
        if (g_showDebugInfo) cout << "error: can't load geometry shader!\n";
        delete o;
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
     }
  
  // Load fragment program
  if (fragmentFile!=0)
     if (tFragmentShader->load(fragmentFile) != 0)
     {
        if (g_showDebugInfo) cout << "error: can't load fragment shader!\n";
        delete o;
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
     }
    
  // Compile vertex program
  if (vertexFile!=0)
     if (!tVertexShader->compile())
     {
		 if (g_showDebugInfo) 
		 {
			cout << "***COMPILER ERROR (Vertex Shader):\n";
			cout << tVertexShader->getCompilerLog() << endl;
		}
         delete o;
         delete tVertexShader;
         delete tFragmentShader;
         delete tGeometryShader;
         return 0;
     }
	 if (g_showDebugInfo) 
	 {
	cout << "***GLSL Compiler Log (Vertex Shader):\n";
	cout << tVertexShader->getCompilerLog() << "\n";
	 }

  // Compile geometry program 
  if (geometryFile!=0)
  {
     
     if (!tGeometryShader->compile())
     {
		 if (g_showDebugInfo) 
		 {
			cout << "***COMPILER ERROR (Geometry Shader):\n";
			cout << tGeometryShader->getCompilerLog() << endl;
		}
        
        delete o;
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
        
     }
  }
  if (g_showDebugInfo) 
  {
	cout << "***GLSL Compiler Log (Geometry Shader):\n";
	cout << tGeometryShader->getCompilerLog() << "\n";
  }
   
  // Compile fragment program 
  if (fragmentFile!=0)
     if (!tFragmentShader->compile())
     {
		 if (g_showDebugInfo) 
		 {
			cout << "***COMPILER ERROR (Fragment Shader):\n";
			cout << tFragmentShader->getCompilerLog() << endl;
		}
        
        delete o;
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
        
     }
	 if (g_showDebugInfo) 
	 {
		cout << "***GLSL Compiler Log (Fragment Shader):\n";
		cout << tFragmentShader->getCompilerLog() << "\n";
	 }
   
  // Add to object    
  if (vertexFile!=0) o->addShader(tVertexShader);
  if (geometryFile!=0) o->addShader(tGeometryShader);
  if (fragmentFile!=0) o->addShader(tFragmentShader); 
  
  // link 
  if (!o->link())
  {
	  if (g_showDebugInfo) 
	  {
		cout << "**LINKER ERROR\n";
		cout << o->getLinkerLog() << endl;
	  }
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     delete tGeometryShader;
     return 0;
  }
  if (g_showDebugInfo) 
  {
	cout << "***GLSL Linker Log:\n";
	cout << o->getLinkerLog() << endl;
  }
  
  _shaderObjectList.push_back(o);
  o->manageMemory();

   return o;
   return 0;
}
// ----------------------------------------------------------------------------

Shader* ShaderManager::loadfromMemory(const char* vertexMem, const char* fragmentMem)
{
	Shader* o = new Shader();
	o->UsesGeometryShader(false);

	aVertexShader* tVertexShader = new aVertexShader;
	aFragmentShader* tFragmentShader = new aFragmentShader;

	// get vertex program
	if (vertexMem!=0)
		tVertexShader->loadFromMemory(vertexMem);

	// get fragment program
	if (fragmentMem!=0)
		tFragmentShader->loadFromMemory(fragmentMem);

	// Compile vertex program
	if (vertexMem!=0)
	if (!tVertexShader->compile())
	{
		if (g_showDebugInfo) 
		{
			cout << "***COMPILER ERROR (Vertex Shader):\n";
			cout << tVertexShader->getCompilerLog() << endl;
		}
		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		return 0;
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Compiler Log (Vertex Shader):\n";
		cout << tVertexShader->getCompilerLog() << "\n";
	}

	// Compile fragment program 
	if (fragmentMem!=0)
	if (!tFragmentShader->compile())
	{
		if (g_showDebugInfo) 
		{
			cout << "***COMPILER ERROR (Fragment Shader):\n";
			cout << tFragmentShader->getCompilerLog() << endl;
		}

		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		return 0;   
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Compiler Log (Fragment Shader):\n";
		cout << tFragmentShader->getCompilerLog() << "\n";
	}

	// Add to object    
	if (vertexMem!=0) o->addShader(tVertexShader);
	if (fragmentMem!=0) o->addShader(tFragmentShader); 

	// link 
	if (!o->link())
	{
		if (g_showDebugInfo) 
		{
			cout << "**LINKER ERROR\n";
			cout << o->getLinkerLog() << endl;
		}
		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		return 0;
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Linker Log:\n";
		cout << o->getLinkerLog() << endl;
	}
	_shaderObjectList.push_back(o);
	o->manageMemory();

	return o;
}

Shader* ShaderManager::loadfromMemory(const char* vertexMem, const char* geometryMem, const char* fragmentMem)
{
	Shader* o = new Shader();
	o->UsesGeometryShader(true);
	o->SetInputPrimitiveType(_nInputPrimitiveType);
	o->SetOutputPrimitiveType(_nOutputPrimitiveType);
	o->SetVerticesOut(_nVerticesOut);

	aVertexShader* tVertexShader = new aVertexShader;
	aFragmentShader* tFragmentShader = new aFragmentShader;
	aGeometryShader* tGeometryShader = new aGeometryShader;

	// get vertex program
	if (vertexMem!=0)
		tVertexShader->loadFromMemory(vertexMem);

	// get fragment program
	if (fragmentMem!=0)
		tFragmentShader->loadFromMemory(fragmentMem);

	// get fragment program
	if (geometryMem!=0)
		tGeometryShader->loadFromMemory(geometryMem);

	// Compile vertex program
	if (vertexMem!=0)
	if (!tVertexShader->compile())
	{
		if (g_showDebugInfo) 
		{
			cout << "***COMPILER ERROR (Vertex Shader):\n";
			cout << tVertexShader->getCompilerLog() << endl;
		}
		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		delete tGeometryShader;
		return 0;
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Compiler Log (Vertex Shader):\n";
		cout << tVertexShader->getCompilerLog() << "\n";
	}

	// Compile geometry program
	if (geometryMem!=0)
	if (!tGeometryShader->compile())
	{
		if (g_showDebugInfo) 
		{
			cout << "***COMPILER ERROR (Geometry Shader):\n";
			cout << tGeometryShader->getCompilerLog() << endl;
		}
		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		delete tGeometryShader;
		return 0;
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Compiler Log (Geometry Shader):\n";
		cout << tVertexShader->getCompilerLog() << "\n";
	}
   
	// Compile fragment program 
	if (fragmentMem!=0)
	if (!tFragmentShader->compile())
	{
		if (g_showDebugInfo) 
		{
			cout << "***COMPILER ERROR (Fragment Shader):\n";
			cout << tFragmentShader->getCompilerLog() << endl;
		}

		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		delete tGeometryShader;
		return 0;   
	}
	if (g_showDebugInfo) 
	{
	cout << "***GLSL Compiler Log (Fragment Shader):\n";
	cout << tFragmentShader->getCompilerLog() << "\n";
	}
   
  // Add to object    
  if (vertexMem!=0) o->addShader(tVertexShader);
  if (geometryMem!=0) o->addShader(tGeometryShader);
  if (fragmentMem!=0) o->addShader(tFragmentShader); 

  // link 
	if (!o->link())
	{
		if (g_showDebugInfo) 
		{
			cout << "**LINKER ERROR\n";
			cout << o->getLinkerLog() << endl;
		}
		delete o;
		delete tVertexShader;
		delete tFragmentShader;
		delete tGeometryShader;
		return 0;
	}
	if (g_showDebugInfo) 
	{
		cout << "***GLSL Linker Log:\n";
		cout << o->getLinkerLog() << endl;
	}

	_shaderObjectList.push_back(o);
	o->manageMemory();

   return o;
}

// ----------------------------------------------------------------------------

 bool  ShaderManager::free(Shader* o)
 {
   vector<Shader*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end()) 
   {
        if ((*i)==o)
        {
            _shaderObjectList.erase(i);
            delete o;
            return true;
        }
        i++;
   }   
   return false;
 }


