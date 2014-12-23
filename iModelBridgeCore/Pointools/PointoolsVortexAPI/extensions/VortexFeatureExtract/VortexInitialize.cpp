#define WIN32_LEAN_AND_MEAN

#include "Includes.h"

using namespace vortex;

bool	PTVFIT_API vortex::InitializeFeatureExtractAPI( const char *licenseCode )
{
	extern bool LoadVortex(HMODULE hMod);

	HMODULE vortexDll = ::GetModuleHandleA("PointoolsVortexAPI.dll");

	if (LoadVortex(vortexDll))
	{
		//todo: version check
		return true; 
	}
	//todo: license

	return false;
}
void	PTVFIT_API vortex::GetFeatureExtractAPIVersion( int &major, int &minor )
{
	major = 1;
	minor = 0;
}


