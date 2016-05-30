#pragma once

#include <ptengine/colourRamps.h>

#define PTRES_TEXID		int64_t
#define LPVOIDSHADER	void*

namespace pointsengine
{
/** Utility class - useful until we have a proper Rendering API abstraction */ 
struct Texture1D
{
	const ubyte *m_imgData;
	int			m_height;
	int			m_width;
	PTRES_TEXID	m_texid;
};
/** Manages resources needed for rendering without graphics API awareness */
/**
 * Does not create resources in the graphics api
 */
class RenderResourceManager
{
private:
	RenderResourceManager();
	void							createDefaultResources();

public:
	static RenderResourceManager	*instance();	// single instance only

	char*							getShaderScript( const char* name );
	ColourGradientManager			*gradientManager();

	void							releaseShaderScript( char *script );

private:
	typedef std::map<pt::String, pt::String> ShaderScriptMap;

	ColourGradientManager			m_gradients;
};


/** Adapts resource for use in graphics API */
/**
 * Each supported graphics API (ie GL/DX) needs an implementation in order to
 * create resource objects in that API
 * Can only be created by FActory, hence private constructor
 */
class ResourceAdaptorI
{
public:
    virtual                    ~ResourceAdaptorI() = 0;
	virtual		Texture1D		*createGradientTexture( int gradientID ) const=0;
	virtual		Texture1D		*createGradientTexture( const char* name) const=0;
	virtual		LPVOIDSHADER	createShaderObj( const char* name, const char*precompiler=0 ) const=0;
};
}