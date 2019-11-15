/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
