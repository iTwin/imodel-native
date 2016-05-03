/*--------------------------------------------------------------------------*/ 
/*	Pointools glEffects class definition and implementation					*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLS_GL_EFFECTS_INTERFACE
#define POINTOOLS_GL_EFFECTS_INTERFACE

#include <pt/geomTypes.h>
#include <ptgl/glCamera.h>
#include <ptgl/ptgl.h>

namespace ptgl
{

#define PTGL_NO_EFFECT			 0
#define PTGL_CELSHADE			 1
#define PTGL_OUTLINE			 2
#define PTGL_CULL_BOX			 3
#define PTGL_NO_NORMAL_TRANSFORM 4

class PTGL_API EffectMode
{	
public:
	EffectMode(int e, bool backfacecull = true);
	~EffectMode();
	void cleanupEffect();

	inline bool allowLighting() const { return _flags & 0x01 ? true : false; }
	inline bool allowTexture2D() const { return _flags & 0x02 ? true : false; }
	inline bool allowTexture1D() const { return _flags & 0x04 ? true : false; }
	inline bool allowTextureBind() const { return _flags & 0x20 ? true : false; }
	inline bool allowColour() const { return _flags & 0x08 ? true : false;; }
	inline bool allowTransparency() const { return _flags & 0x10 ? true : false; }

	static void pushTransform();
	static void popTransform();
	static const EffectMode *current();

	static const char* capabilities();

	static void initializeExtensions();

private:
	bool initializeEffect();
	void setupEffect();

	static char* loadShader(const char *filename);
	int _effect;
	bool _use;
	GLuint _flags;
	bool _backfacecull;
};

}
#endif