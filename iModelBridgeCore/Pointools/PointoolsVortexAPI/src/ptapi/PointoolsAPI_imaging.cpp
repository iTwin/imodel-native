/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL


#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/pointsScene.h>
#include <ptengine/pointspager.h>
#include <ptengine/renderengine.h>
#include <ptengine/pointsexchanger.h>
#include <ptengine/visibilityengine.h>
#include <ptengine/engine.h>
 
#include <ptcloud2/pod.h>
#include <pt/project.h>

#ifdef HAVE_OPENGL
#include <ptgl/glCamera.h>
#include <ptgl/gltext.h>
#endif

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
 
