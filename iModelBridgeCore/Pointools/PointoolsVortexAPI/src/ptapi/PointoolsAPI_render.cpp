#include <pt/os.h>
#define POINTOOLS_API_BUILD_DLL

#include <gl/glew.h>
#include <ptapi/PointoolsVortexAPI.h>

#include <ptgl/glInvariant.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/PointsPager.h>
#include <ptengine/engine.h>

#include <pt/debugassert.h>
#include <ptappdll/ptapp.h>

#include <pt/timestamp.h>

using namespace pt;
using namespace pointsengine;

extern PTvoid _ptBeginViewportDraw();
extern PTvoid _ptEndViewportDraw();
extern pt::ViewParams *g_currentViewParams;
extern TCHAR g_lastError[1024];
extern int setLastErrorCode( int );

PTuint 		g_frame = 0;
PTenum 		g_units = PT_METERS;
PTdouble 	g_unitScale = 1.0;

ptgl::Camera 	g_camera;
ptgl::Light 	g_light;

namespace
{
//-----------------------------------------------------------------------------

PTdouble g_conv [] = { 
	1.0,
	10,
	100,
	1000,
	3.280839895,
	39.37007874 ,
	3.2808333333333333333333333333333
};
static PTfloat _framerate = 30.0f;
PTenum g_drawmodeOverride = PT_DRAW_MODE_DEFAULT;

//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
PTint	PTAPI ptNumRamps() 
{ 
	return theRenderEngine().colourRampsManager().numColourRamps();
}
//-----------------------------------------------------------------------------
const PTstr PTAPI ptRampInfo( PTint ramp, PTenum *type ) 
{ 
	if (!type)
	{
		setLastErrorCode( PTV_VOID_POINTER );
		return 0;
	}	

	if ( ptNumRamps() <= ramp)
	{
		*type = PT_INTENSITY_RAMP_TYPE;
		return L"* missing *";
	}
	const ColourRamp *CRamp = theRenderEngine().colourRampsManager().getRampByIndex( ramp );
	if (!CRamp) return L"";

	else
	{
		*type = 0;
		if (CRamp->m_use & ColourRampIntensity) *type |= PT_INTENSITY_RAMP_TYPE;
		if (CRamp->m_use & ColourRampPlane)		*type |= PT_PLANE_RAMP_TYPE;
		return CRamp->m_name.c_wstr();
	}

//	return (const PTstr)&_rampPaths[ramp].c_str()[1];
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptSetHostUnits(PTenum units)
{
	g_units = units;
	g_unitScale = g_conv[units - PT_METERS];
}
//-----------------------------------------------------------------------------
PTenum PTAPI ptGetHostUnits()
{
	return g_units;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptDynamicFrameRate(PTfloat fps)
{
	_framerate = fps;
}
//-----------------------------------------------------------------------------
PTfloat PTAPI ptGetDynamicFrameRate()
{
	return _framerate;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptStaticOptimizer(PTfloat opt)
{
	theVisibilityEngine().optimizerStrength(opt);
}
//-----------------------------------------------------------------------------
PTfloat PTAPI ptGetStaticOptimizer() 
{
	return theVisibilityEngine().optimizerStrength();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptDrawInteractiveGL() 
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
} ;
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptOverrideDrawMode(PTenum drawmode)
{
	g_drawmodeOverride = drawmode; 
}
//-------------------------------------------------------------------------------
// DrawGL - draw the point clouds in OpenGL
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptDrawGL(PTbool dynamic)
{
	ptDrawSceneGL(0, dynamic);
}
//-----------------------------------------------------------------------------
PTuint	PTAPI ptKbLoaded( PTbool reset )
{
	return (PTuint)pointsengine::thePointsPager().KBytesLoaded(reset);
}
//-----------------------------------------------------------------------------
PTuint	PTAPI ptWeightedPtsLoaded( PTbool reset )
{
	return (PTuint)pointsengine::thePointsPager().pointsLoadedMetric(reset);
}
//-----------------------------------------------------------------------------

extern pcloud::Scene* sceneFromHandle(PThandle);

//-----------------------------------------------------------------------------
PTvoid PTAPI ptDrawSceneGL(PThandle scene, PTbool dynamic)
{
	bool compatibility = false;

	if (g_drawmodeOverride != PT_DRAW_MODE_DEFAULT)
	{
		dynamic = g_drawmodeOverride & PT_DRAW_MODE_INTERACTIVE ?
			true : false;
		compatibility = g_drawmodeOverride & PT_DRAW_MODE_COMPATIBILITY ?
			true : false;
	}

	//theRenderEngine().setDiagnosticDisplay( 1 );

	if (ptIsCurrentViewportEnabled())
	{
		pcloud::Scene *pScene = scene ? sceneFromHandle(scene) : 0;
		_ptBeginViewportDraw();

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
		ptgl::Invariant inv;

		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND); 
		glDisable(GL_NORMALIZE);
		glDisable(GL_TEXTURE_2D);

		if (ptIsInitialized())
		{
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			
			glScaled(g_unitScale, g_unitScale, g_unitScale);

			if (g_currentViewParams)
				theVisibilityEngine().setViewParameters( *g_currentViewParams );
			
			glEnable(GL_LIGHT0);
			
			if (!g_camera.getLight())
			{
				g_camera.setLight(&g_light);
			}
			g_light.setupGL();

			theRenderEngine().setLight(&g_light); 
			theRenderEngine().setCamera(&g_camera);

			int ms = dynamic ? ((1000 / _framerate))/ptNumScenes() : 0;

			theVisibilityEngine().computeVisibility();
			if (compatibility) theRenderEngine().useVertexArrays(false);

			theRenderEngine().renderGL( ms, false, pScene);

			if (theRenderEngine().initialisationFailed())
			{
				_tcscpy(g_lastError, theRenderEngine().getErrorMessage().c_tstr());
			}
			if (compatibility)	theRenderEngine().useVertexArrays(true);

			/* reset the load metrics */ 
			pointsengine::thePointsPager().KBytesLoaded(true);
			pointsengine::thePointsPager().pointsLoadedMetric(true);

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
		_ptEndViewportDraw();
		glPopAttrib();
		glPopClientAttrib();
	}
	g_frame++;
}