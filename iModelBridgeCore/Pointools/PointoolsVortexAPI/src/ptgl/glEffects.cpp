/*--------------------------------------------------------------------------*/ 
/*	Pointools glEffects class implementation								*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*--------------------------------------------------------------------------*/ 
#include <gl/glew.h>
#include <ptgl/glExtensions.h>
#include <ptgl/glEffects.h>
#include <ptgl/glState.h>
//#include <ptgl/arb_vertex_program.h>
#include <boost/filesystem/path.hpp>
#include <ptappdll/ptapp.h>
#include <math/matrix_math.h>
#include <vector>
#include <fstream>


using namespace ptgl;

static GLuint _effects[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static bool _initialized[] =  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static bool _needsVP[]= { 0, 1, 1, 1, 1 };
static bool _needsMT[]= { 0, 0, 0, 1, 1 };
/*cel shader*/ 
static GLubyte celShadeTex[] = {120, 120, 150, 150, 150, 160, 160, 255, 255, 255, 255, 255, 255, 255, 255, 255};
static GLuint celShaderTexID = 0;

#define PTGL_ALLOW_COLOR		0x08
#define PTGL_ALLOW_LIGHTING		0x01
#define PTGL_ALLOW_TEX1D		0x02
#define PTGL_ALLOW_TEX2D		0x04
#define PTGL_ALLOW_TRANSPARENCY	0x10
#define PTGL_ALLOW_TEXTURE_BIND	0x20
#define PTGL_CULL_BACKFACE		0x40

namespace ptgl_effects
{
	EffectMode *_currentEffectsMode = 0;
	GLint _maxTexelUnits = 0;
	GLint _maxMatrices = 0;
	bool _hasVP = false;
	bool _hasMT = false;
	
	PFNGLACTIVETEXTUREARBPROC			glActiveTextureARB			= NULL;
}
using namespace ptgl_effects;

//
// construction
//
EffectMode::EffectMode(int e, bool backfacecull)
{
	_effect = e;
	_currentEffectsMode = this;
	_use = initializeEffect();
	_flags = PTGL_ALLOW_COLOR | PTGL_ALLOW_LIGHTING | PTGL_ALLOW_TEX1D | PTGL_ALLOW_TEX2D | PTGL_ALLOW_TRANSPARENCY | PTGL_ALLOW_TEXTURE_BIND;
	_backfacecull = backfacecull;

	if (_use) setupEffect();

}
//
// destruction
//
EffectMode::~EffectMode()
{
	if (_use) cleanupEffect();
	_currentEffectsMode = 0;
}
//
// instance
//
const EffectMode *EffectMode::current()
{
	return _currentEffectsMode;
}
//
// begin effect
//
void EffectMode::setupEffect()
{
	mmatrix4 mat;
	mat.loadGLmodel();
	mat.invert();
	pt::vector3 lightpos(10,10,10);
	pt::vector3 lightcol(1,1,1);
	mat.vec3_multiply_mat4(lightpos);

	switch(_effect)
	{
		case PTGL_NO_EFFECT:
			if (!_backfacecull)
			{
				glPolygonMode(GL_FRONT, GL_FILL);
				glPolygonMode(GL_BACK, GL_FILL);
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}

		_flags = 255;
		break;

   // Enable culling

   // Set to line.
		case PTGL_OUTLINE:
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_1D);         

			RenderStateSet::overrideEnable(ptgl::RS_VertexProgram);
			RenderStateSet::overrideDisable(ptgl::RS_Lighting);
			RenderStateSet::overrideEnable(ptgl::RS_Texture1D);
			RenderStateSet::overrideDisable(ptgl::RS_Texture2D);
			RenderStateSet::overrideSet(ptgl::RS_Texture1DClamp);
	
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _effects[_effect]);
			glEnable(GL_VERTEX_PROGRAM_ARB);

			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glPolygonMode(GL_BACK, GL_LINE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, lightpos.x, lightpos.y, lightpos.z, 1.0);
			
			glBindTexture(GL_TEXTURE_1D, celShaderTexID);
			_flags = PTGL_ALLOW_TRANSPARENCY;
			glLineWidth(4.0f);
			break;

		case PTGL_CELSHADE: 
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_1D);           
			
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _effects[_effect]);
			glEnable(GL_VERTEX_PROGRAM_ARB);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			RenderStateSet::overrideEnable(ptgl::RS_VertexProgram);
			RenderStateSet::overrideDisable(ptgl::RS_Lighting);
			RenderStateSet::overrideEnable(ptgl::RS_Texture1D);
			RenderStateSet::overrideDisable(ptgl::RS_Texture2D);
			RenderStateSet::overrideSet(ptgl::RS_Texture1DClamp);

			glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, lightpos.x, lightpos.y, lightpos.z, 1.0);
			glBindTexture(GL_TEXTURE_1D, celShaderTexID);
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_NONE);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
						
			_flags = PTGL_ALLOW_COLOR | PTGL_ALLOW_TRANSPARENCY;
			break;
		case PTGL_NO_NORMAL_TRANSFORM: 
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _effects[_effect]);
			glEnable(GL_VERTEX_PROGRAM_ARB);						
			_flags = 255;

			break;
	}	
}
//
// push Transform for no normal transform
//
void EffectMode::pushTransform()
{
	if (!glActiveTextureARB)
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
	}
	/*load basic inverted modelview into user matrix 0*/ 
	/*this is used by vertex program as modelview for normal transform */ 
	/* avoiding any other matrices push on after*/ 
	mmatrix4d mvp;
	mvp.loadGLmodel();
	mvp.invert();
	
	glMatrixMode(GL_MATRIX1_ARB);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd((double*)(&mvp));
	glMatrixMode(GL_MODELVIEW);
}

//
//
//
void EffectMode::popTransform()
{
	glMatrixMode(GL_MATRIX1_ARB);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
//
// end effect
//
void EffectMode::cleanupEffect()
{
	switch(_effect)
	{
		case PTGL_OUTLINE:
			glLineWidth(1.0f);
		case PTGL_CELSHADE: 
			glDisable(GL_CULL_FACE);
			glDisable(GL_TEXTURE_1D);
			ptgl::RenderStateSet::resetOverrides();
		case PTGL_NO_NORMAL_TRANSFORM:
			glDisable(GL_VERTEX_PROGRAM_ARB);
		break;
	}
}
void  EffectMode::initializeExtensions()
{
	_hasVP = ptgl::InitARBVertexProgram();
	glGetIntegerv(GL_MAX_PROGRAM_MATRICES_ARB, &_maxMatrices);
	_hasMT = Ext::isSupported("GL_ARB_multitexture");
	if (_hasMT) 
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
	}
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &_maxTexelUnits);

}
//
// init
//
bool EffectMode::initializeEffect()
{
	if (!_effect) return false;
	static bool initvp = false;
	if (!initvp)
	{
		initvp = true;
		_hasVP = ptgl::InitARBVertexProgram();
		glGetIntegerv(GL_MAX_PROGRAM_MATRICES_ARB, &_maxMatrices);
	}
	if (_needsVP && !_hasVP) return false;
	static bool initmt = false;
	if (!initmt)
	{
		_hasMT = Ext::isSupported("GL_ARB_multitexture");
		if (_hasMT) 
		{
			glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &_maxTexelUnits);
		}
		else
			printf("WARNING: No multi-texture unit support\n");
		initmt = true;
	}
	if (_needsMT && !_hasMT) return false;

	char *shader = 0;

	if (!_initialized[_effect])
	{		
		switch(_effect)
		{
		case 0: break;
			//cel shader
		case PTGL_OUTLINE:
		case PTGL_CELSHADE: 
			shader = loadShader("celshader.vsh");

			_initialized[PTGL_OUTLINE] = true;
			_initialized[PTGL_CELSHADE] = true;

			if (shader)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_VERTEX_PROGRAM_ARB);
				glGenProgramsARB(1, &_effects[PTGL_CELSHADE]);
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _effects[PTGL_CELSHADE]);
				glProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shader), shader);

				_effects[PTGL_OUTLINE] =_effects[PTGL_CELSHADE];
				/*check for errors*/ 
				if (GL_INVALID_OPERATION == glGetError())
				{
					GLint errPos;
					glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
					const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
					fprintf( stderr, "error at position: %d\n%s\n", errPos, errString);
				}


				glGenTextures(1, &celShaderTexID);
				glBindTexture(GL_TEXTURE_1D, celShaderTexID);
				glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, celShadeTex);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

				delete [] shader;
			}	
			else
			{
				printf("Vertex shader file not found");	
				return false;
			}
			break;
		case PTGL_NO_NORMAL_TRANSFORM:
			shader = loadShader("pointlight.vsh");
			printf("compiling point-light vertex program\n");
			_initialized[PTGL_NO_NORMAL_TRANSFORM] = true;

			if (shader)
			{
				glEnable(GL_VERTEX_PROGRAM_ARB);
				glGenProgramsARB(1, &_effects[PTGL_NO_NORMAL_TRANSFORM]);
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _effects[PTGL_NO_NORMAL_TRANSFORM]);
				glProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shader), shader);

				/*check for errors*/ 
				//if (GL_INVALID_OPERATION == glGetError())
				{
					GLint errPos;
					glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
					const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
					if (errString)
					{
						fprintf( stderr, "error at position: %d\n%s\n", errPos, errString);
						fprintf( stderr, "%s", &shader[errPos]);
					}
				}
				//else printf("\nvertex program compiled\n");
	
				delete [] shader;
			}	
			else
			{
				printf("WARNING: Vertex shader file not found");	
				return false;
			}
			break;
			//
		}
	}
	return true;
}
//
// load shader from file
//
char* EffectMode::loadShader(const char *filename)
{
	char filepath[260];

	sprintf(filepath, "%s\\modules\\shaders\\%s", ptapp::apppathA(), filename);

	std::ifstream inptest, input;

	inptest.open(filepath);
	if(!inptest.is_open())    
	{ 
		//input.close();
		sprintf(filepath, "%s\\shaders\\%s", ptapp::apppathA(), filename);
		
		inptest.open(filepath);
		if (!inptest.is_open()) 
			return NULL; 
	}
	inptest.close();

	input.open( filepath );

	std::vector<char *> buffer;

	while(!input.eof())
	{
		char *buff = new char[128];
		input.getline(buff, 128, '\n');
		buffer.push_back(buff);
	}

	input.close();

	char *vpstr = new char[buffer.size() * 50];
	vpstr[0] = '\0';

	for (unsigned int i=0; i<buffer.size(); i++)
	{
		strcat(vpstr, buffer[i]);
		strcat(vpstr, "\n");
		delete [] buffer[i];
	}
	return vpstr;
}
//
// Capabilities
//
const char *EffectMode::capabilities()
{
	static char _cap[256];
	_cap[0] = '\0';

	initializeExtensions();

	//lists extensions
	strcat(_cap, 
		"Relevent OpenGL Capabilities:\n"
		"===========================\n"
		"Extensions:\n");
	if (_hasMT)  strcat(_cap, "  GL_ARB_multitexture\n");
	if (_hasVP)  strcat(_cap, "  GL_ARB_vertex_program\n");
	if (ptgl::Ext::isSupported("GL_EXT_compiled_vertex_array"))
		strcat(_cap, "  GL_EXT_compiled_vertex_array\n");
	if (ptgl::Ext::isSupported("GL_NV_texture_shader"))
		strcat(_cap, "  GL_NV_texture_shader\n");

	strcat(_cap, "\n");
	strcat(_cap, "Limits:\n");
	sprintf(_cap, "%s  max texel units = %d\n  max program matrices = %d\n", _cap, _maxTexelUnits, _maxMatrices);
					
	return _cap;
}
