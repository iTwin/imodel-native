/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#define WIN32_LEAN_AND_MEAN

#include "Includes.h"

using namespace vortex;

bool	PTVFIT_API vortex::InitializeFeatureExtractAPI( const char *licenseCode )
{
	//todo: license

	return true;
}
void	PTVFIT_API vortex::GetFeatureExtractAPIVersion( int &major, int &minor )
{
	major = 1;
	minor = 0;
}


