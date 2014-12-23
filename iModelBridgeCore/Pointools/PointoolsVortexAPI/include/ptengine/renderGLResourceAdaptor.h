#pragma once
#include <ptengine/renderResourceManager.h>

namespace pointsengine
{
	class OpenGLResourceAdaptor : public ResourceAdaptorI
	{
	public:
		OpenGLResourceAdaptor();
	
		Texture1D		*createGradientTexture( int gradientID ) const;
		Texture1D		*createGradientTexture( const char* name) const;
		LPVOIDSHADER	createShaderObj( const char* name, const char *precompiler=0 ) const;

	private:
		void			setupGradientTexture( Texture1D *tex ) const;
	};
}