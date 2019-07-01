/******************************************************************************
glsl.h
Version: 1.0.0_rc4
Last update: 2006/11/12 

* New in RC4:
   > Support for Geometry Shader 
   > Shader Model 4: Support for unsigned integers
     (Vertex Attributes are currently unsupported, but easy to integrate if
      you actually need them)
   > retrieve variable index with GetUniformLocation
   > All setUniformXXX can use index
   > Added GetProgramObject in class Shader to return the OpenGL Program Object

#TODO:
  Support all vertex attribute types
  Support glGetActiveAttrib
  Support glGetAttribLocation
  Support New Matrix Types (OpenGL 2.1)
  Make SetUniformXXX / VertexXXX calls template-based

Important Note:
Make sure to check extension "GL_EXT_geometry_shader4" before using Geometry
shaders!

(c) 2003-2006 by Martin Christen. All Rights reserved.
******************************************************************************/

#ifndef A_GLSL_H
#define A_GLSL_H

//! \defgroup GLSL libglsl

#include <vector>
#include <iostream>
//#define GLEW_STATIC 
#include <GL/glew.h>
#include <ptgl/ptgl.h>

namespace ptgl
{
   class ShaderManager;

   //! Shader Object holds the Shader Source, the OpenGL "Shader Object" and provides some methods to load shaders.
   /*!
         \ingroup GLSL
         \bug This class should be an interface. (pure virtual)
   */
   class PTGL_API ShaderObject
   {
      friend class Shader;

   public:
                  ShaderObject();
      virtual     ~ShaderObject();
        
      int         load(const char* filename, const char*preprocessor=0); //!< \brief Loads a shader file. \param filename The name of the ASCII file containing the shader. \return Teturns 0 if everything is ok. -1: File not found, -2: Empty File, -3: no memory
      void        loadFromMemory(const char* program);      //!< \brief Load program from null-terminated char array. \param program Address of the memory containing the shader program.
       
      bool        compile(void);                            //!< compile program
      char*       getCompilerLog(void);                     //!< get compiler messages
      GLint       getAttribLocation(char* attribName);      //!< \brief Retrieve attribute location. \return Returns attribute location. \param attribName Specify attribute name.  
    
   protected:
       int        program_type;  //!< The program type. 1=Vertex Program, 2=Fragment Program, 3=Geometry Progam, 0=none

       GLuint     shaderObject;  //!< Shader Object
       GLubyte*   shaderSource;  //!< ASCII Source-Code
       
       GLcharARB* compiler_log;
       
       bool       is_compiled;   //!< true if compiled
       bool       _memalloc;     //!< true if memory for shader source was allocated
   };

//-----------------------------------------------------------------------------

   //! \ingroup GLSL
   class aVertexShader : public ShaderObject
   {
   public:
                  aVertexShader();  //!< Constructor for Vertex Shader
      virtual     ~aVertexShader(); 
   };

//-----------------------------------------------------------------------------

   //! \ingroup GLSL
   class aFragmentShader : public ShaderObject
   {
   public:
                  aFragmentShader(); //!< Constructor for Fragment Shader
      virtual     ~aFragmentShader();
    
   };

//-----------------------------------------------------------------------------

   //! \ingroup GLSL
   class aGeometryShader : public ShaderObject
   {
   public:
                   aGeometryShader(); //!< Constructor for Geometry Shader
      virtual     ~aGeometryShader();
   };

//-----------------------------------------------------------------------------

   //! \ingroup GLSL
   class PTGL_API Shader
   {
      friend class ShaderManager;

   public:
                 Shader();                  
      virtual    ~Shader();
      void       addShader(ShaderObject* ShaderProgram); //!< add a Vertex or Fragment Program \param ShaderProgram The shader object.
      
      //!< Returns the OpenGL Program Object (only needed if you want to control everything yourself) \return The OpenGL Program Object
      GLuint     GetProgramObject(){return ProgramObject;}

      bool       link(void);                        //!< Link all Shaders
      char*      getLinkerLog(void);                //!< Get Linker Messages \return char pointer to linker messages. Memory of this string is available until you link again or destroy this class.

      void       begin();                           //!< use Shader. OpenGL calls will go through vertex, geometry and/or fragment shaders.
      void       end();                             //!< Stop using this shader. OpenGL calls will go through regular pipeline.
      
      // Geometry Shader: Input Type, Output and Number of Vertices out
      void       SetInputPrimitiveType(int nInputPrimitiveType);   //!< Set the input primitive type for the geometry shader
      void       SetOutputPrimitiveType(int nOutputPrimitiveType); //!< Set the output primitive type for the geometry shader
      void       SetVerticesOut(int nVerticesOut);                 //!< Set the maximal number of vertices the geometry shader can output
     
      GLint       GetUniformLocation(const GLcharARB *name);  //!< Retrieve Location (index) of a Uniform Variable

      // Submitting Uniform Variables. You can set varname to 0 and specifiy index retrieved with GetUniformLocation (best performance)
      bool       setUniform1f(GLcharARB* varname, GLfloat v0, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform2f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform3f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform4f(GLcharARB* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.

      bool       setUniform1i(GLcharARB* varname, GLint v0, GLint index = -1);  //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform2i(GLcharARB* varname, GLint v0, GLint v1, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform3i(GLcharARB* varname, GLint v0, GLint v1, GLint v2, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform4i(GLcharARB* varname, GLint v0, GLint v1, GLint v2, GLint v3, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.

      // Note: unsigned integers require GL_EXT_gpu_shader4 (for example GeForce 8800)
      bool       setUniform1ui(GLcharARB* varname, GLuint v0, GLint index = -1); //!< Specify value of uniform unsigned integer variable. Only works if GL_EXT_gpu_shader4 is available. \param varname The name of the uniform variable.
      bool       setUniform2ui(GLcharARB* varname, GLuint v0, GLuint v1, GLint index = -1); //!< Specify value of uniform unsigned integer variable. Only works if GL_EXT_gpu_shader4 is available. \param varname The name of the uniform variable.
      bool       setUniform3ui(GLcharARB* varname, GLuint v0, GLuint v1, GLuint v2, GLint index = -1); //!< Specify value of uniform unsigned integer variable. Only works if GL_EXT_gpu_shader4 is available. \param varname The name of the uniform variable.
      bool       setUniform4ui(GLcharARB* varname, GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLint index = -1); //!< Specify value of uniform unsigned integer variable. Only works if GL_EXT_gpu_shader4 is available. \param varname The name of the uniform variable.

      // Arrays
      bool       setUniform1fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform2fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform3fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform4fv(GLcharARB* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      
      bool       setUniform1iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform2iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform3iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform4iv(GLcharARB* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      
      bool       setUniform1uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform2uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform3uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform4uiv(GLcharARB* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      
      bool       setUniformMatrix2fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 2x2 matrix. \param varname The name of the uniform variable.
      bool       setUniformMatrix3fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 3x3 matrix. \param varname The name of the uniform variable.
      bool       setUniformMatrix4fv(GLcharARB* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 4x4 matrix. \param varname The name of the uniform variable.
 
      // Receive Uniform variables:
      void       getUniformfv(GLcharARB* varname, GLfloat* values, GLint index = -1); //!< Receive value of uniform variable. \param varname The name of the uniform variable.
      void       getUniformiv(GLcharARB* varname, GLint* values, GLint index = -1); //!< Receive value of uniform variable. \param varname The name of the uniform variable.
      void       getUniformuiv(GLcharARB* varname, GLuint* values, GLint index = -1); //!< Receive value of uniform variable. Requires GL_EXT_gpu_shader4 \param varname The name of the uniform variable.

      /*! Bind Vertex Attribute Location
      Warning: NVidia implementation is different than the GLSL standard:
      GLSL attempts to eliminate aliasing of vertex attributes but this is 
      integral to NVIDIA�s hardware approach and necessary for maintaining 
      compatibility with existing OpenGL applications that NVIDIA customers rely on.
      NVIDIA�s GLSL implementation therefore does not allow built-in vertex attributes
      to collide with a generic vertex attributes that is assigned to a particular vertex 
      attribute index with glBindAttribLocation. For example, you should not use gl_Normal 
      (a built-in vertex attribute) and also use glBindAttribLocation to bind a generic 
      vertex attribute named "whatever" to vertex attribute index 2 because gl_Normal aliases to index 2.
      \verbatim
      gl_Vertex                0
      gl_Normal                2
      gl_Color                 3
      gl_SecondaryColor        4
      gl_FogCoord              5
      gl_MultiTexCoord0        8
      gl_MultiTexCoord1        9
      gl_MultiTexCoord2       10
      gl_MultiTexCoord3       11
      gl_MultiTexCoord4       12
      gl_MultiTexCoord5       13
      gl_MultiTexCoord6       14
      gl_MultiTexCoord7       15
      \endverbatim

      \param index Index of the variable
      \param name Name of the attribute.
      */
      //! This method simply calls glBindAttribLocation for the current ProgramObject.
      void      BindAttribLocation(GLint index, GLchar* name);

      //GLfloat
      bool      setVertexAttrib1f(GLuint index, GLfloat v0); //!< Specify value of attribute.
      bool      setVertexAttrib2f(GLuint index, GLfloat v0, GLfloat v1); //!< Specify value of attribute.
      bool      setVertexAttrib3f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2); //!< Specify value of attribute.
      bool      setVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); //!< Specify value of attribute.

      //GLdouble
      bool      setVertexAttrib1d(GLuint index, GLdouble v0); //!< Specify value of attribute.
      bool      setVertexAttrib2d(GLuint index, GLdouble v0, GLdouble v1); //!< Specify value of attribute.
      bool      setVertexAttrib3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2); //!< Specify value of attribute.
      bool      setVertexAttrib4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3); //!< Specify value of attribute.

      //GLshort
      bool      setVertexAttrib1s(GLuint index, GLshort v0); //!< Specify value of attribute.
      bool      setVertexAttrib2s(GLuint index, GLshort v0, GLshort v1); //!< Specify value of attribute.
      bool      setVertexAttrib3s(GLuint index, GLshort v0, GLshort v1, GLshort v2); //!< Specify value of attribute.
      bool      setVertexAttrib4s(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3); //!< Specify value of attribute.

      // Normalized Byte (for example for RGBA colors)
      bool      setVertexAttribNormalizedByte(GLuint index, GLbyte v0, GLbyte v1, GLbyte v2, GLbyte v3); //!< Specify value of attribute. Values will be normalized.

      //GLint (Requires GL_EXT_gpu_shader4)
      bool      setVertexAttrib1i(GLuint index, GLint v0); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool      setVertexAttrib2i(GLuint index, GLint v0, GLint v1); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool      setVertexAttrib3i(GLuint index, GLint v0, GLint v1, GLint v2); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool      setVertexAttrib4i(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.

      //GLuint (Requires GL_EXT_gpu_shader4)
      bool      setVertexAttrib1ui(GLuint index, GLuint v0); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4. \param v0 value of the first component
      bool      setVertexAttrib2ui(GLuint index, GLuint v0, GLuint v1); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool      setVertexAttrib3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool      setVertexAttrib4ui(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.

	  //Vertex Pointer
	  
	  bool		setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,	GLsizei stride,	const GLvoid * pointer);	

      //! Enable this Shader:
	   void        enable(void) //!< Enables Shader (Shader is enabled by default)
	   {
	      _noshader = true;
	   }

      //! Disable this Shader:
	   void        disable(void) //!< Disables Shader.
	   {
	      _noshader = false;
	   } 
     
   protected:
      void        manageMemory(void){_mM = true;}
      void        UsesGeometryShader(bool bYesNo){ _bUsesGeometryShader = bYesNo;}

   private:      
      GLuint      ProgramObject;                      // GLProgramObject
      

      GLcharARB*  linker_log;
      bool        is_linked;
      std::vector<ShaderObject*> _shaderList;       // List of all Shader Programs

      bool        _mM;
      bool        _noshader;

      bool        _bUsesGeometryShader;

      int         _nInputPrimitiveType;
      int         _nOutputPrimitiveType;
      int         _nVerticesOut;
        
   };

//-----------------------------------------------------------------------------
// To simplify the process loading/compiling/linking shaders I created this
// high level interface to simplify setup of a vertex/fragment shader.
   //! \ingroup GLSL
   class PTGL_API ShaderManager
   {
   public:
       ShaderManager();
       virtual ~ShaderManager();

       // Regular GLSL (Vertex+Fragment Shader)
       Shader* loadfromFile(const char* vertexFile, const char* fragmentFile, const char*vertexPP=0, const char*fragPP=0);    //!< load vertex/fragment shader from file. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part. \param vertexFile Vertex Shader File. \param fragmentFile Fragment Shader File.
       Shader* loadfromMemory(const char* vertexMem, const char* fragmentMem); //!< load vertex/fragment shader from memory. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part.
       
       // With Geometry Shader (Vertex+Geomentry+Fragment Shader)
       Shader* loadfromFile(char* vertexFile, char* geometryFile, char* fragmentFile); //!< load vertex/geometry/fragment shader from file. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part. \param vertexFile Vertex Shader File. \param geometryFile Geometry Shader File \param fragmentFile Fragment Shader File.
       Shader* loadfromMemory(const char* vertexMem, const char* geometryMem, const char* fragmentMem); //!< load vertex/geometry/fragment shader from memory. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part.
       
       void      SetInputPrimitiveType(int nInputPrimitiveType);    //!< Set the input primitive type for the geometry shader \param nInputPrimitiveType Input Primitive Type, for example GL_TRIANGLES
       void      SetOutputPrimitiveType(int nOutputPrimitiveType);  //!< Set the output primitive type for the geometry shader \param nOutputPrimitiveType Output Primitive Type, for example GL_TRIANGLE_STRIP
       void      SetVerticesOut(int nVerticesOut);                  //!< Set the maximal number of vertices the geometry shader can output \param nVerticesOut Maximal number of output vertices. It is possible to output less vertices!
  
       bool      free(Shader* o); //!< Remove the shader and free the memory occupied by this shader.

   private:
       std::vector<Shader*>  _shaderObjectList; 
       int                     _nInputPrimitiveType;
       int                     _nOutputPrimitiveType;
       int                     _nVerticesOut;
   };

//-----------------------------------------------------------------------------
// Global functions to initialize OpenGL Extensions and check for GLSL and 
// OpenGL2. Also functions to check if Shader Model 4 is available and if
// Geometry Shaders are supported.
bool InitOpenGLExtensions(void); //!< Initialize OpenGL Extensions (using glew) \ingroup GLSL
bool HasGLSLSupport(void);       //!< Returns true if OpenGL Shading Language is supported. (This function will return a GLSL version number in a future release) \ingroup GLSL  
bool HasOpenGL2Support(void);    //!< Returns true if OpenGL 2.0 is supported. This function is deprecated and shouldn't be used anymore. \ingroup GLSL \deprecated
bool HasGeometryShaderSupport(void); //!< Returns true if Geometry Shaders are supported. \ingroup GLSL
bool HasShaderModel4(void); //!< Returns true if Shader Model 4 is supported. \ingroup GLSL
};

#endif // A_GLSL_H

