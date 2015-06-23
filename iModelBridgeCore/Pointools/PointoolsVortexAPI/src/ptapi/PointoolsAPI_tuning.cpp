#include <pt/os.h>
#include <commdlg.h>
#define POINTOOLS_API_BUILD_DLL

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsVortexAPI_ResultCodes.h>

#include <diagnostics/diagnosticCmds.h>
#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/engine.h>

#include <ptcloud2/pod.h>

#include <ptl/branch.h>
#include <ptl/project.h>

extern int setLastErrorCode( int );
using namespace pt;
using namespace pointsengine;
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetCacheSizeMb( PTuint mb )
{
	PointsPager::setCacheSizeMb( (int)mb );
}
//-----------------------------------------------------------------------------
PTuint	PTAPI ptGetCacheSizeMb()
{
	return (PTuint)PointsPager::getCacheSizeMb( );
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptAutoCacheSize()
{
	PointsPager::useAutoCacheSize();
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetLoadingPriorityBias( PTenum bias )
{
	switch (bias)
	{
		case PT_LOADING_BIAS_SCREEN:
			theVisibilityEngine().setBias( VisibilityEngine::BiasScreen ); break;

		case PT_LOADING_BIAS_NEAR:
			theVisibilityEngine().setBias( VisibilityEngine::BiasNear); break;

		case PT_LOADING_BIAS_FAR:
			theVisibilityEngine().setBias( VisibilityEngine::BiasFar ); break;

		case PT_LOADING_BIAS_POINT:
			theVisibilityEngine().setBias( VisibilityEngine::BiasPoint ); break;
		
		default:
			return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTenum	PTAPI ptGetLoadingPriorityBias()
{
	VisibilityEngine::VisibilityBias bias = theVisibilityEngine().getBias();

	switch (bias)
	{
		case VisibilityEngine::BiasScreen:
			return PT_LOADING_BIAS_SCREEN;

		case VisibilityEngine::BiasNear:
			return PT_LOADING_BIAS_NEAR; 

		case VisibilityEngine::BiasFar:
			return PT_LOADING_BIAS_FAR; 

		case VisibilityEngine::BiasPoint:
			return PT_LOADING_BIAS_POINT; 
	}
	return PT_LOADING_BIAS_SCREEN;
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetTuningParameterfv( PTenum param, const PTfloat *values )
{
	if ( param != PT_LOADING_BIAS_POINT) 
	{
		return setLastErrorCode( PTV_INVALID_OPTION );
	}

	theVisibilityEngine().setBiasPoint( values );
	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptGetTuningParameterfv( PTenum param, PTfloat *values )
{
	if ( param != PT_LOADING_BIAS_POINT) 
	{
		return setLastErrorCode( PTV_INVALID_OPTION );;
	}

	vector3 pnt ( theVisibilityEngine().getBiasPoint() );
	pnt.get(values);

	return setLastErrorCode( PTV_SUCCESS );;
}
//-----------------------------------------------------------------------------
PTres	PTAPI _ptDiagnostic( PTvoid *data )
{
	DiagnosticData *d=reinterpret_cast<DiagnosticData*>(data);

	if (d)
	{
		switch(d->in_cmd)
		{
			
		
		}
	}
	else return PTV_INVALID_PARAMETER;
}