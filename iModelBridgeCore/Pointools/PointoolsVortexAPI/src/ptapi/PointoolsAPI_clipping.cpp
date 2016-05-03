#include "PointoolsVortexAPIInternal.h"
#include <ptapi/PointoolsVortexAPI.h>
#include <ptengine/renderengine.h>
#include <ptengine/engine.h>
#include <ptengine/clipManager.h>
#include <string.h>



/* clipping options */ 
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptEnableClipping( void )
{
	pointsengine::ClipManager::instance().enableClipping();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptDisableClipping( void )
{
	pointsengine::ClipManager::instance().disableClipping();
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetClipStyle( PTuint style )
{
	return pointsengine::ClipManager::instance().setClipStyle(style);
}
//-----------------------------------------------------------------------------
PTuint	PTAPI ptGetNumClippingPlanes( void )
{
	return pointsengine::ClipManager::instance().getNumClippingPlanes();
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptIsClippingPlaneEnabled( PTuint id )
{
	return pointsengine::ClipManager::instance().isClippingPlaneEnabled(id);
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptEnableClippingPlane( PTuint id )
{
	return pointsengine::ClipManager::instance().enableClippingPlane(id);
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptDisableClippingPlane( PTuint id )
{
	return pointsengine::ClipManager::instance().disableClippingPlane(id);
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetClippingPlaneParameters( PTuint id, PTdouble a, PTdouble b, PTdouble c, PTdouble d )
{
	return pointsengine::ClipManager::instance().setClippingPlaneParameters(id, a, b, c, d);
}
