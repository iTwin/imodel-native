#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL


#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/PointsExchanger.h>
#include <ptengine/VisibilityEngine.h>
#include <ptengine/engine.h>
 
#include <ptcloud2/pod.h>
#include <pt/project.h>

#include <ptgl/glcamera.h>
#include <ptgl/gltext.h>

#include <ptengine/queryScene.h> 

using namespace pt;
using namespace pcloud;
using namespace pointsengine;

PThandle	PTAPI ptCreateImageBuffer( PTint width, PTint height, PTubyte *buffer )
{
	return 0;
}
PTbool		PTAPI ptBindImageBuffer( PThandle )
{
	return false;
}
PTbool		PTAPI ptGeneratePolygonImage( PTdouble *fencePoints, PTuint numPoints, PTdouble thickness )
{
	return false;
}
 