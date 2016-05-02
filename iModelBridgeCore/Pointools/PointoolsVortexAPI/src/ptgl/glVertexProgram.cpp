#include "PointoolsVortexAPIInternal.h"
#include <ptgl/glvertexprogram.h>
#include <ptgl/glEffects.h>

#include <math/matrix_math.h>
#include <ptappdll/ptapp.h>

#include <iostream>
#include <fstream>
#include <set>

using namespace ptgl;

extern bool g_showDebugInfo;

namespace ptgl_vp
{
	GLint _maxTexelUnits = 0;
	GLint _maxMatrices = 0;
	bool _hasVP = false;
	bool _hasMT = false;

	PFNGLACTIVETEXTUREARBPROC			glActiveTextureARB			= NULL;

	bool _vpinuse = false;

	int _hash_out(char*txt)
	{
		static const char* mask = "Vertex program initialization";
		int i = 0;
		while (txt[i]!='\n') 
		{
			txt[i]  ^= mask[i%29];
			++i;
		}
		txt[i] = (char)0xaa;
		return ++i;
	}
	int _hash_in(char *txt)
	{
		static const char* mask = "Vertex program initialization";
		int i = 0;
		while ((unsigned char)txt[i] < 0xaa)
		{
			txt[i]  ^= mask[i%29];
			++i;
		}
		txt[i] = '\n';
		txt[++i] = '\0';
		return i;
	}
	typedef std::map<HGLRC, GLuint> RCIDmap;
}
using namespace ptgl_vp;

#define PTGL_VP_INVALID 784937465
#define PTGL_VP_FAILED 784937466
//--------------------------------------------------------------
// constructor
//--------------------------------------------------------------
VertexProgram::VertexProgram(const char*id, const char *vpath)
{
	_vfile = vpath;
	_active = false;
	_id = 0;
	/*dont initialize until context ready*/ 
}
//--------------------------------------------------------------
// destructor
//--------------------------------------------------------------
VertexProgram::~VertexProgram()
{
	/*delete object*/ 
	//glDeleteProgramsARB(1, &_id); ???
}
//--------------------------------------------------------------
// start vertex program
//--------------------------------------------------------------
bool VertexProgram::begin()
{
	HGLRC rc = wglGetCurrentContext();
	RCIDmap::iterator id = _rcid.find(rc);

	if (id == _rcid.end()) 
	{
		if  (!initialize()) return false;
	}
	else _id = id->second;

	if (_id == PTGL_VP_FAILED) return false;
	if (_active) return true;

	_active = true;

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _id);
	//glEnable(GL_VERTEX_PROGRAM_ARB);

	return true;
}
//--------------------------------------------------------------
// end vertex program
//--------------------------------------------------------------
void VertexProgram::end()
{
	if (!_active) return;
	_active = false;
	_id = 0;

	//glDisable(GL_VERTEX_PROGRAM_ARB);
}
//--------------------------------------------------------------
// Initialize Context
//--------------------------------------------------------------
bool VertexProgram::initializeContext()
{
	_hasVP = glewIsSupported("GL_ARB_vertex_program") != 0;
	_hasMT = glewIsSupported("GL_ARB_multitexture") != 0;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &_maxTexelUnits);	
	glGetIntegerv(GL_MAX_PROGRAM_MATRICES_ARB, &_maxMatrices);

	return true;
}
//--------------------------------------------------------------
// Initialize Vertex Program
//--------------------------------------------------------------
bool VertexProgram::initialize()
{
	if (_id == PTGL_VP_FAILED) return false;

	/* check if this context has been initialized */ 
	static bool initializeExt = false;
	if (!initializeExt)
	{
		initializeContext();
		initializeExt = true;
	}

	//if (!_hasVP) return false;

	char *script = loadScript();
	if (!script) 
	{	
		if (g_showDebugInfo) std::cout << "ERROR: Unable to open shader file " << _vfile << std::endl;	
		_id = PTGL_VP_FAILED;
	}
	else
		{
		glEnable(GL_VERTEX_PROGRAM_ARB);
		glGenProgramsARB(1, &_id);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _id);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(script), script);

		/*check for errors*/ 
		GLint errPos;
		glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
		if (errPos != -1)
		{
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			if (errString)
			{
				if (g_showDebugInfo) 
				{
					std::cout << "error at position: " << errPos << errString << std::endl;
					std::cout << &script[errPos] << std::endl;
				}
				_id = PTGL_VP_FAILED;
			}
		}
		delete [] script;
	}
	_rcid.insert(RCIDmap::value_type(wglGetCurrentContext(), _id));

	return _id != PTGL_VP_FAILED;
}
//--------------------------------------------------------------
// load Vertex Program
//--------------------------------------------------------------
char* VertexProgram::loadScript()
{
	char filepath[260];

	sprintf(filepath, "%s\\modules\\shaders\\%s", ptapp::apppathA(), _vfile);

	std::ifstream inptest, input;

	inptest.open(filepath);
	if(!inptest.is_open())    
	{ 
		//input.close();
		sprintf(filepath, "%s\\shaders\\%s", ptapp::apppathA(), _vfile);
		
		inptest.open(filepath);
		if (!inptest.is_open()) 
			return NULL; 
	}
	inptest.close();

	input.open( filepath );

	std::vector<char *> buffer;
	char c=0;
	input.get(c);
	bool hashed = false;

	if (c == '!')
	{
		bool first = true;
		
		while(!input.eof())
		{
			char *bf = new char[132];
			if (first) { bf[0] = '!'; ++bf; }

			input.getline(bf, 128, '\n');
			if (first) { --bf; first = false; }

			int l = strlen(bf);
			bf[l++] = '\n';
			bf[l] = '\0';
			
			if (bf[0] != '#') buffer.push_back(bf);
			else delete [] bf;
		}
		input.close();
	}
	else
	{
		/*open as binary*/ 
		std::ifstream input;
		input.open(filepath, std::ios::in | std::ios::binary);

		hashed = true;

		while(!input.eof())
		{
			char *bf = new char[132];
			memset(bf, '\0', 128);
			int i = 0;			
			do { input.get(bf[i]); }
			while (!input.eof() && (unsigned char)bf[i++] < 0xaa && i < 128);
			
			if ((unsigned char)bf[i-1] < 0xaa) bf[i] = (char)0xaa;
			
			_hash_in(bf);
			buffer.push_back(bf);
		}	
		input.close();
	}

	char *vpstr = new char[buffer.size() * 50];
	vpstr[0] = '\0';
	unsigned int i;
	for (i=0; i<buffer.size(); i++)
		strcat(vpstr, buffer[i]);
	
	if (!hashed) 
	{
		/*
		int len = strlen(vpstr);
	
		strcat(filepath, "-enc");
		std::ofstream hash;
		hash.open(filepath, std::ios::out | std::ios::binary);

		if (hash.is_open())
		{
			for (i=0;i<buffer.size();i++)
			{
				char buf[128];
				int llen = _hash_out(buffer[i]);
				for (int j=0; j<llen; j++)	hash.put(buffer[i][j]);
			}
			hash.close();
		}
		*/ 
	}
	for (i=0;i<buffer.size();i++) delete [] buffer[i];

	return vpstr;
}
//--------------------------------------------------------------------
// set Transform Matrix - all the transformations before the quantize
//--------------------------------------------------------------------
void VertexProgram::setEyeMatrix()
{
	if (!_active) return;
	
	mmatrix4d mvp;
	mvp.loadGLmodel();
	
	glMatrixMode(GL_MATRIX1_ARB);
	glLoadIdentity();
	//glPushMatrix();
	//glLoadIdentity();
	glMultMatrixd((double*)(&mvp));
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------------------------
// set Transform Matrix - all the transformations before the quantize
//--------------------------------------------------------------------
void VertexProgram::setTransformMatrix(const mmatrix4d &mat) const
{
	if (!_active) return;
	mmatrix4d m = mat;
	m.transpose();
	
	glMatrixMode(GL_MATRIX2_ARB);
	glLoadIdentity();
	//glPushMatrix();
	//glLoadIdentity();
	glMultMatrixd((double*)(&m));
	glMatrixMode(GL_MODELVIEW);
}
void VertexProgram::pushTransformMatrix(const mmatrix4d &mat) const
{
	if (!_active) return;
	mmatrix4d m = mat;
	m.transpose();
	
	glMatrixMode(GL_MATRIX2_ARB);
	glPushMatrix();
	glMultMatrixd((double*)(&m));
	glMatrixMode(GL_MODELVIEW);
}
void VertexProgram::popTransformMatrix() const
{
	if (!_active) return;
	
	glMatrixMode(GL_MATRIX2_ARB);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}








