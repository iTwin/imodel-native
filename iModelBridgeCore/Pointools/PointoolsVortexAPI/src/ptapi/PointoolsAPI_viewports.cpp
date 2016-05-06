#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL
#include <pt/timer.h>
#include <ptapi/PointoolsVortexAPI.h>
#include <math/matrix_math.h>
#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <pt/viewparams.h>
#include <ptengine/engine.h>
#include <ptengine/renderContext.h>
 
#include <diagnostics/diagnostics.h>

#include <utility/ptstr.h>
#include <ptcloud2/pod.h>

#include <pt/trace.h>

using namespace pointsengine;

RenderContext			*g_currentRenderContext=0;
pt::ViewParams			*g_currentViewParams=0;
PTuint					g_currentViewport = 0;

extern PTuint			g_frame;
extern int				setLastErrorCode( int );
extern pcloud::Scene*	sceneFromHandle(PThandle handle);

//-----------------------------------------------------------------------------
// Viewport setup and view
//-----------------------------------------------------------------------------
struct PTViewport
{
	PTViewport( int64_t context_id )
	{
		contextid = context_id;
		prevcontextid = 0;
		wcscpy(name, L"viewport");
		inUse = false;
		storeFrame = -1;
		initialized = false;
		enabled = true;
		pointsCap = -1;
	}
	virtual ~PTViewport()					{}

	virtual PTbool	isValid()				{ return false; } 
	virtual PTbool	makeCurrent() const		{ return false; }

	virtual PTvoid	storeView()				{ storeFrame = g_frame; }
	virtual PTvoid	restoreViewport()		{};
	virtual PTvoid	restoreProjection()		{};
	virtual PTvoid	restoreEye()			{};
	virtual PTvoid	restoreView()			{}; 
	virtual const	pt::String &type()		{ static pt::String sw("sw"); return sw; }

	virtual RenderContext *getRenderContext()
	{
		return RenderContextManager::instance()->getRenderContext( RenderEnvOther, contextid );
	}
	ContextID getContextID(void)
	{
		return contextid;
	}
	void setContextID(ContextID newContextID)
	{
		// Save the previous context id so it can be cleaned up properly later
		// once a new one has been set and any operations still using the old one have completed 
		if (contextid && (prevcontextid != contextid))
			prevcontextid = contextid;
		contextid = newContextID;
	}
	void destroyPreviousContext(void)
	{
		// Only destroy a previous context if a new one has been set
		if (prevcontextid && contextid && (prevcontextid != contextid))
		{
			RenderContextManager::instance()->destroyRenderContext(prevcontextid);
			prevcontextid = 0;
		}
	}

	pt::ViewParams				viewParams;
#ifdef HAVE_OPENGL
	ptgl::Viewstore				vstore;
#endif
	wchar_t						name[64];
	bool						inUse;
	PTint						pointsCap;
	PTint						storeFrame;
	bool						initialized;
	bool						enabled;

	VisibilityEngine::LoadedShortfallMap	pntsShortfallCurrent;
	VisibilityEngine::LoadedShortfallMap	pntsShortfallOnDraw;

private:
	ContextID					contextid;
	ContextID					prevcontextid;
};

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
// OpenGL Viewport abstraction
//-----------------------------------------------------------------------------
struct PTViewportGL : public PTViewport
{
	PTViewportGL() : PTViewport(0)
	{
		rc = 0;
		dc = 0;
	}
	PTvoid storeView() 
	{ 
		PTViewport::storeView();

		glGetIntegerv(GL_VIEWPORT, viewParams.viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, viewParams.eye_matrix);
		glGetDoublev(GL_PROJECTION_MATRIX, viewParams.proj_matrix);
		glGetDoublev(GL_DEPTH_RANGE, viewParams.depth_range);

		rc = wglGetCurrentContext();
		dc = wglGetCurrentDC();
	}
	PTvoid restoreViewport() 
	{ 
		glViewport(viewParams.viewport[0], 
			viewParams.viewport[1], 
			viewParams.viewport[2], 
			viewParams.viewport[3]); 
	}
	PTvoid restoreProjection() 
	{ 
		glMatrixMode(GL_PROJECTION);	
		glLoadMatrixd(viewParams.proj_matrix); 
	}
	PTvoid restoreEye()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixd(viewParams.eye_matrix);
	}
	PTvoid restoreView()
	{
		restoreViewport();
		restoreProjection();
		restoreEye();
	}
	const pt::String &type() { static pt::String t("gl"); return t; }
	PTbool makeCurrent() const
	{
		if (rc && dc) return wglMakeCurrent(dc, rc) ? true : false;
		else return false;
	}
	
	virtual RenderContext *getRenderContext()
	{
		RenderContext* contextRet = NULL;

		HGLRC current_rc = wglGetCurrentContext();

		if (rc && rc != current_rc)
		{
			RenderContext *context = RenderContextManager::instance()->
				getRenderContext( RenderEnvOpenGL, getContextID() );

			if (context)
			{
				setContextID((ContextID)current_rc);
				RenderContext *new_context = RenderContextManager::instance()->
					getRenderContext( RenderEnvOpenGL, getContextID() );
				
				if (new_context == context) 
				{
					contextRet = context;
				}
				else if (new_context)
				{
					// copy settings
					(*new_context->settings()) = (*context->settings());					
					rc = current_rc;
					contextRet = new_context;
				}
			}
		}
		if (!rc)
		{
			rc = current_rc;
			dc = wglGetCurrentDC();
			setContextID((ContextID)rc);
		}
		if (rc && dc) 
			contextRet = RenderContextManager::instance()->getRenderContext( RenderEnvOpenGL, getContextID() );
	
		// Destroy the previous context (if the context has changed a previous one will have been saved)
		destroyPreviousContext();

		return contextRet;
	}
	PTbool isValid() { return inUse && rc != 0 ? true : false; }

	HGLRC rc;
	HDC dc;
};
//-----------------------------------------------------------------------------
// Bitmap viewport
//-----------------------------------------------------------------------------
struct PTViewportBMPGL : public PTViewportGL
{
	PTViewportBMPGL()
	{
		bmp = 0;
		bmpData = 0;
		renderContext = 0;
		memset(&bmpInfo, 0, sizeof(BITMAPINFO) );
	}
	~PTViewportBMPGL()
	{
		RenderContextManager::instance()->destroyRenderContext( (ContextID)rc );
	}
	bool createRenderContext()	// must have a context already
	{
		assert( rc );
		if (!rc) return false;

		// need to force a fixed func pipeline
		renderContext = RenderContextManager::instance()->getOpenGLContext( (ContextID)rc, true );
		
		return renderContext ? true : false;
	}
	RenderContext *getRenderContext()
	{
		return renderContext;
	}
	const pt::String &type() { static pt::String t("glbitmap"); return t; }

	HBITMAP bmp;
	PTubyte *bmpData;
	BITMAPINFO	bmpInfo;
	RenderContext *renderContext;
};
#endif

//-----------------------------------------------------------------------------
// DirectX Viewport - unimplemented
//-----------------------------------------------------------------------------
struct PTViewportDX : public PTViewport
{

};
//-----------------------------------------------------------------------------
// Software viewport - unimplemented
//-----------------------------------------------------------------------------
struct PTViewportSW : public PTViewport
{

};

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
//	Make Current on construct + switch back on destruct
//-----------------------------------------------------------------------------
struct PTMakeCurrent
{
	PTMakeCurrent(const PTViewport *vp)
	{
		currentRC = wglGetCurrentContext();
		currentDC = wglGetCurrentDC();
		vp->makeCurrent();
	}
	~PTMakeCurrent()
	{
		wglMakeCurrent(currentDC, currentRC);
	}
	HDC currentDC;
	HGLRC currentRC;
};
#endif
//-----------------------------------------------------------------------------
namespace detail
{
#ifdef HAVE_OPENGL
	static HGLRC s_storeRC;
	static HDC s_storeDC;
#endif
	PTViewport* g_viewports[PT_MAX_VIEWPORTS];
	PTbool _flipMouseY = true;
	bool _initialised = false;

	static ptdg::LoadData1		s_loadMetric;
}
//-----------------------------------------------------------------------------
extern PTvoid _ptApplyShader(PTint);
extern PTvoid _ptApplyLight(PTint);

using namespace detail;
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptFlipMouseYCoords()		{ _flipMouseY = true; }
PTvoid	PTAPI ptDontFlipMouseYCoords()	{ _flipMouseY = false; }
//-----------------------------------------------------------------------------
// internal initialisation
//-----------------------------------------------------------------------------
PTvoid _ptInitialiseViewports() 
{
	if (!g_viewports[g_currentViewport])
	{
		g_viewports[g_currentViewport] = new PTViewport(g_currentViewport);
		g_currentViewParams = &g_viewports[g_currentViewport]->viewParams;
		g_currentRenderContext = g_viewports[g_currentViewport]->getRenderContext();
	}
	if (_initialised) return;
	_initialised = true;
	for (int i=0; i<PT_MAX_VIEWPORTS; i++)
	{
		if (i != g_currentViewport) g_viewports[i] = 0;
	}
	if (!g_viewports[0])
		g_viewports[0] = new PTViewport(0);
}
//-----------------------------------------------------------------------------
PTint nextAvailableViewportIndex()
{
	for (int i=0; i<PT_MAX_VIEWPORTS; i++)
	{
		if (!g_viewports[i]) return i;
	}	
	return -1;
}
//-----------------------------------------------------------------------------
PTvoid _ptAdjustMouseYVal( int &y)
{
	if (_flipMouseY)
		y = g_viewports[g_currentViewport]->viewParams.viewport[3] - y;	
}
//-------------------------------------------------------------------------------
// ptMakeVPContextCurrent
//-------------------------------------------------------------------------------
PTvoid _ptMakeVPContextCurrent()
{
#ifdef HAVE_OPENGL
	s_storeRC = wglGetCurrentContext();
	s_storeDC = wglGetCurrentDC();
#endif
	g_viewports[g_currentViewport]->makeCurrent();
}
//-------------------------------------------------------------------------------
// ptRestoreContext
//-------------------------------------------------------------------------------
PTvoid _ptRestoreContext()
{
#ifdef HAVE_OPENGL
    wglMakeCurrent(s_storeDC, s_storeRC);
#else
    assert(!"OpenGL is not available...");
#endif
}
//
//-----------------------------------------------------------------------------
PTint64	PTAPI ptPtsLoadedInViewportSinceDraw( PThandle forScene )
{
	PTViewport *currentVP = g_viewports[g_currentViewport] ;

	if (!currentVP) return 0;

	const pcloud::Scene *sc = forScene ? sceneFromHandle( forScene ) : 0;

	const VisibilityEngine::LoadedShortfallMap &shortfall = g_viewports[g_currentViewport]->pntsShortfallOnDraw;
	VisibilityEngine::LoadedShortfallMap &currShortfall = g_viewports[g_currentViewport]->pntsShortfallCurrent;

	theVisibilityEngine().computeCurrentPntsShortfall( sc, currShortfall  );

	int64_t currShortfallNum = 0;
	int64_t lastShortfallNum = 0;

	if (sc)
	{
		VisibilityEngine::LoadedShortfallMap::const_iterator i = shortfall.find( sc );
		if (i != shortfall.end()) lastShortfallNum = i->second;

		i = currShortfall.find( sc );
		if (i != currShortfall.end()) currShortfallNum = i->second;
	}
	else
	{
		VisibilityEngine::LoadedShortfallMap::const_iterator i = shortfall.begin();

		while (i != shortfall.end())
		{
			lastShortfallNum += i->second;
			++i;
		}		
		i = currShortfall.begin();

		while (i != currShortfall.end())
		{
			currShortfallNum += i->second;
			++i;
		}
	}
	int64_t shortfallNum  = lastShortfallNum - currShortfallNum;
	
	s_loadMetric.m_ptsLoadedSinceDraw = shortfallNum;


	return (shortfallNum < 0) ? 0 : shortfallNum;
}
//-----------------------------------------------------------------------------
// GLOBAL - used for stats output only
int64_t g_lastPtsToLoadInViewportValue = 0;
//-----------------------------------------------------------------------------
PTint64	PTAPI ptPtsToLoadInViewport( PThandle forScene, PTbool reCompute )
{
	int64_t pntsToLoad = 0;

	if (forScene)
	{
		const pcloud::Scene *sc = sceneFromHandle( forScene );
		if (sc)
		{
			// get last draw status
			VisibilityEngine::LoadedShortfallMap &shortfall = g_viewports[g_currentViewport]->pntsShortfallOnDraw;
			VisibilityEngine::LoadedShortfallMap &currShortfall = g_viewports[g_currentViewport]->pntsShortfallCurrent;

			VisibilityEngine::LoadedShortfallMap::const_iterator i = shortfall.find( sc );
			
			bool notFound = false;
			if (i == shortfall.end()) notFound = true;	// needs computing now

			if (notFound || reCompute)
			{
				//TODO: MUST UPDATE VIS ENGINE TO USE THIS VIEWPORT
				theVisibilityEngine().computeCurrentPntsShortfall( sc, currShortfall  );
				i = currShortfall.find( sc );
				if (i != currShortfall.end())
				{
					// copy value into draw shortfall, not accurate, but better than 0
					if (notFound)
						shortfall.insert( VisibilityEngine::LoadedShortfallMap::value_type( sc, i->second ));

					pntsToLoad = i->second;
				}
			} 
			else
			{
				pntsToLoad = i->second;
			}
		}
	}
	else
	{
		VisibilityEngine::LoadedShortfallMap &shortfall = reCompute ?
			g_viewports[g_currentViewport]->pntsShortfallCurrent :
			g_viewports[g_currentViewport]->pntsShortfallOnDraw;

		if (reCompute)
		{
			//TODO: MUST UPDATE VIS ENGINE TO USE THIS VIEWPORT
			theVisibilityEngine().computeCurrentPntsShortfall( 0, shortfall  );
		}

		VisibilityEngine::LoadedShortfallMap::const_iterator i = shortfall.begin();

		while (i != shortfall.end())
		{
			pntsToLoad += i->second;
			++i;
		}
	}
	s_loadMetric.m_ptsShortfall = pntsToLoad; 
	g_lastPtsToLoadInViewportValue = pntsToLoad;

	return pntsToLoad;
}
//-------------------------------------------------------------------------------
// _ptSavePntsLoadedData | saves points deficet on last draw
//-------------------------------------------------------------------------------
PTvoid _ptSavePntsLoadedData( int viewport )
{
	if (g_viewports[viewport])
	{
		theVisibilityEngine().getLastFramePntsShortfall( 
			g_viewports[viewport]->pntsShortfallOnDraw );
	}
}
//-------------------------------------------------------------------------------
PTvoid PTAPI ptEndDrawFrameMetrics()
{
	_ptSavePntsLoadedData( g_currentViewport );
	
	ptdg::Time::stamp( s_loadMetric );
	ptdg::Diagnostics::instance()->addMetric( s_loadMetric );

	s_loadMetric.m_ptsShortfall = 0;
	s_loadMetric.m_ptsLoadedSinceDraw = 0;
}
//-------------------------------------------------------------------------------
PTvoid PTAPI ptStartDrawFrameMetrics()
{
	//does nothing for now
}
//-------------------------------------------------------------------------------
// Prepares the viewport for drawing : this function is not exported
//-------------------------------------------------------------------------------
PTvoid _ptBeginViewportDraw()
{
	_ptInitialiseViewports();

	g_currentRenderContext = g_viewports[g_currentViewport]->getRenderContext();

	_ptApplyShader(g_currentViewport);
#ifdef HAVE_OPENGL
	_ptApplyLight(g_currentViewport);
#endif

	theVisibilityEngine().pointsBudget( g_viewports[g_currentViewport]->pointsCap );
#ifdef HAVE_OPENGL
    g_viewports[g_currentViewport]->vstore.store();
#endif
}
//-------------------------------------------------------------------------------
// Prepares the viewport for drawing : this function is not exported
//-------------------------------------------------------------------------------
PTvoid _ptEndViewportDraw()
{
	/* only do this for static view, not dynamic */ 
	_ptSavePntsLoadedData(g_currentViewport);
    theVisibilityEngine().pointsBudget( -1 );
}
//-------------------------------------------------------------------------------
// find the least recently used viewport
//-------------------------------------------------------------------------------
PTint _ptLeastRecentlyUsedViewport()
{
	PTint min = static_cast<PTint>(1e9);
	PTint viewport = -1;
	for (int i=0; i<PT_MAX_VIEWPORTS; i++)
	{
		PTViewport *vp = g_viewports[i];
		if ( vp && vp->storeFrame < min)
		{
			min = vp->storeFrame;
			viewport = i;
		}	
	}
	return viewport;
}

/*****************************************************************************/
/**
* @brief
* @param index
* @param name
* @return PTint PTAPI
*/
/*****************************************************************************/
PTint PTAPI _ptAddViewportDeduceContext(PTint index, const PTstr name)
{
#ifdef HAVE_OPENGL
    if (wglGetCurrentContext())
	{
		return ptAddViewport(index, name, PT_GL_VIEWPORT);
	}
	else 
#endif
        return ptAddViewport(index, name, PT_SW_VIEWPORT);

	//TODO: DirectX
}
//-------------------------------------------------------------------------------
// Add a viewport
//-------------------------------------------------------------------------------
PTint	PTAPI ptAddViewport(PTint index, const PTstr name, PTenum contextType)
{
	PTTRACE_FUNC_P3(index, name, contextType)

	if (index >= PT_MAX_VIEWPORTS)
	{
		/* find least recently used */ 
		PTint vp = _ptLeastRecentlyUsedViewport();
		delete g_viewports[vp];
		g_viewports[vp] = 0;

		return ptAddViewport(vp, name, contextType);
	}
	// check if name exists, if so return index
	if (index == -1)
	{
		index = ptViewportIndexFromName( name );	// get index from name
	}
	if (index == -1) 
	{
		index = nextAvailableViewportIndex();		// get new index
	}
	if (index == -1)
	{
		index = _ptLeastRecentlyUsedViewport();
	}
	if (index == -1) 
	{
		return -1;
	}
	
	if (g_viewports[index]) 
	{
		switch ( contextType )
		{
		case PT_GL_VIEWPORT:
			if (g_viewports[index]->type() == pt::String("gl")) return index;
			break;
		case PT_SW_VIEWPORT:
			if (g_viewports[index]->type() == pt::String("sw")) return index;
			break;
		}
		delete g_viewports[index];
		g_viewports[index] = 0;
	}

	try 
	{
		switch( contextType )
		{
		case PT_GL_VIEWPORT:
#ifdef HAVE_OPENGL
            g_viewports[index] = new PTViewportGL;
#else
            g_viewports[index] = new PTViewport(0);	// No support for OpenGL
#endif
			break;
		case PT_DX_VIEWPORT:
			g_viewports[index] = new PTViewport(0);	// not implemented yet 	//TODO: DirectX
			break;
		case PT_SW_VIEWPORT:
			g_viewports[index] = new PTViewport(index);
			break;
		}
	}
	catch (std::bad_alloc)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return PT_MAX_VIEWPORTS;
	}
	if (!name)
		ptstr::copy( g_viewports[index]->name, L"Viewport", sizeof((g_viewports[0]->name)) / sizeof(wchar_t));
	else
		ptstr::copy(g_viewports[index]->name, name, sizeof((g_viewports[0]->name)) / sizeof(wchar_t));

	return index;
}
//-------------------------------------------------------------------------------
// Remove a viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptRemoveViewport(PTint index)
{
	PTTRACE_FUNC_P1(index)

	int next = -1;
	/* check there is at least one viewport */ 
	for (int i=0; i<PT_MAX_VIEWPORTS; i++)
	{
		if (g_viewports[i]  && i != index)
		{
			next = i; break;
		}
	}

	if(index != -1 && g_viewports[index])
	{
															// Discard LODs associated with the viewport
		thePointsScene().removeLOD(index);
	}

	if (next != -1 && g_viewports[index])
	{
		RenderContextManager::instance()->destroyRenderContext(g_viewports[index]->getContextID());
		delete g_viewports[index];
		g_viewports[index] = 0;
		
		if (g_currentViewport == index)
			ptSetViewport( next );
	}
}
//-------------------------------------------------------------------------------
// Enable a viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptEnableViewport(PTint index)
{
	PTTRACE_FUNC_P1(index)

	_ptInitialiseViewports();

	if (g_viewports[index])
	{
		g_viewports[index]->enabled = true;
	}
}
//-------------------------------------------------------------------------------
// Disable a viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptDisableViewport(PTint index)
{
	PTTRACE_FUNC_P1(index)

	if (g_viewports[index])
	{
		g_viewports[index]->enabled = false;
															// Discard LODs associated with the viewport
		thePointsScene().removeLOD(index);
	}
}


//-------------------------------------------------------------------------------
// Is Enabled
//-------------------------------------------------------------------------------
PTbool	PTAPI ptIsViewportEnabled(PTint index)
{
	_ptInitialiseViewports();

	return g_viewports[index] ? g_viewports[index]->enabled : false;
}
//-------------------------------------------------------------------------------
// Is Enabled
//-------------------------------------------------------------------------------
PTbool	PTAPI ptIsCurrentViewportEnabled()
{
	_ptInitialiseViewports();

	return g_viewports[g_currentViewport]->enabled;
}
//-------------------------------------------------------------------------------
// Set the current Viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewport(PTint index) 
{
	PTTRACE_FUNC_P1(index)

	if (index > PT_MAX_VIEWPORTS-1) return;

	g_currentViewport = index;

	_ptInitialiseViewports();
	g_currentViewParams = &g_viewports[g_currentViewport]->viewParams;
	g_currentRenderContext = g_viewports[g_currentViewport]->getRenderContext();

    theVisibilityEngine().pointsBudget( g_viewports[g_currentViewport]->pointsCap );

	/* if this is a bitmap viewport we are managing the GL context so must switch here */ 
	if (g_viewports[g_currentViewport]->type() == pt::String("glbitmap"))
	{
		g_viewports[g_currentViewport]->makeCurrent();
	}
}
//-------------------------------------------------------------------------------
// get the viewports index from its name
//-------------------------------------------------------------------------------
PTint	PTAPI ptViewportIndexFromName( const PTstr name )
{
	int i;
	for (i=0;i<PT_MAX_VIEWPORTS; i++)
	{
		if (g_viewports[i] && wcscmp(g_viewports[i]->name, name)==0)
		{
			return i;
		}
	}
	return -1;
}
//-------------------------------------------------------------------------------
// Set the current Viewport
//-------------------------------------------------------------------------------
PTint	PTAPI ptSetViewportByName(const PTstr name)
{
	PTTRACE_FUNC_P1(name)

	_ptInitialiseViewports();

	if (g_viewports[g_currentViewport] && 
		g_currentViewport != PT_MAX_VIEWPORTS && 
		wcscmp(g_viewports[g_currentViewport]->name, name)==0)
		return g_currentViewport;

	g_currentViewport = PT_MAX_VIEWPORTS;
	int i;
	for (i=0;i<PT_MAX_VIEWPORTS; i++)
	{
		if (g_viewports[i] && wcscmp(g_viewports[i]->name, name)==0)
		{
			ptSetViewport(i);
			break;
		}
	}
	if (g_currentViewport == PT_MAX_VIEWPORTS) 
	{	
		for (i=0; i<PT_MAX_VIEWPORTS; i++)
		{
			if (!g_viewports[i])
			{
				_ptAddViewportDeduceContext(i, name);
				ptSetViewport(i);
				break;
			}
		}
	}
	if (g_currentViewport == PT_MAX_VIEWPORTS)
		ptSetViewport( _ptAddViewportDeduceContext(PT_MAX_VIEWPORTS, name) );

	return g_currentViewport;
}

#ifdef HAVE_OPENGL
//-------------------------------------------------------------------------------
// destroy bitmap viewport
//-------------------------------------------------------------------------------
PTvoid PTAPI ptDestroyBitmapViewport(const PTstr name)
{
	PTint index = ptSetViewportByName(name);	
	if (index == PT_MAX_VIEWPORTS) return;
	
	PTViewportBMPGL *vp = (PTViewportBMPGL*)g_viewports[index];

	if (vp->bmp)
	{
		/* first destroy existing context */ 
		if (wglGetCurrentContext() == vp->rc)
			wglMakeCurrent(NULL, NULL);
		
		wglDeleteContext(vp->rc); 
		DeleteObject(vp->bmp);
		DeleteDC(vp->dc);
		vp->dc = 0;
		vp->rc = 0;
		vp->bmpData = 0;
		
		ptRemoveViewport(index);
	}
}
//-------------------------------------------------------------------------------
// capture context info
//-------------------------------------------------------------------------------
PTvoid* PTAPI ptCreateBitmapViewport(int w, int h, const PTstr name)
{
	PTint index = ptSetViewportByName(name);	// creates a new viewport if (name) doesn't exist
	if (index == PT_MAX_VIEWPORTS)
	{
		setLastErrorCode( PTV_MAXIMUM_VIEWPORTS_USED );	
		return NULL;
	}
	
	PTViewport *evp = g_viewports[index];
	const pt::ViewParams &vparams = evp->viewParams;

	if (vparams.viewport[pt::ViewParams::Width] == w
		&& vparams.viewport[pt::ViewParams::Height] == h
		&& evp->type() == pt::String("glbitmap"))
	{
		PTViewportBMPGL *nvp = static_cast<PTViewportBMPGL*>(evp);
		return nvp->bmp;
	} 
	ptRemoveViewport(index);	// remove it and replace it with a bitmap one

	PTViewportBMPGL *vp = new PTViewportBMPGL;

	wcsncpy(vp->name, name, 64);
	g_viewports[index] = vp;
	g_viewports[index]->viewParams.viewport[0] = 0;
	g_viewports[index]->viewParams.viewport[1] = 0;
	g_viewports[index]->viewParams.viewport[2] = w;
	g_viewports[index]->viewParams.viewport[3] = h;

	if (vp->bmp)
	{
		if (vp->bmpInfo.bmiHeader.biHeight == h && 
			vp->bmpInfo.bmiHeader.biWidth == w)
			return vp->bmp;

		/* first destroy existing context */ 
		if (wglGetCurrentContext() == vp->rc)
		{
			wglMakeCurrent(NULL, NULL);
		}
		wglDeleteContext(vp->rc);
		DeleteObject(vp->bmp);
		DeleteDC(vp->dc);
		vp->dc = 0;
		vp->rc = 0;
		vp->bmpData = 0;
	}
	vp->dc = CreateCompatibleDC(NULL);

	memset(&vp->bmpInfo, 0, sizeof(vp->bmpInfo));
	vp->bmpInfo.bmiHeader.biSize	= sizeof(BITMAPINFOHEADER);
	vp->bmpInfo.bmiHeader.biWidth	= w;
	vp->bmpInfo.bmiHeader.biHeight	= h;
	vp->bmpInfo.bmiHeader.biPlanes	= 1;
	vp->bmpInfo.bmiHeader.biBitCount = 24;
	vp->bmpInfo.bmiHeader.biCompression = BI_RGB;

	vp->bmp = CreateDIBSection(vp->dc, &vp->bmpInfo, DIB_RGB_COLORS, (void**)&vp->bmpData, vp->bmp, 0);

	SelectObject(vp->dc, vp->bmp); 

	PIXELFORMATDESCRIPTOR	pfd;
	int						pf;

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize		= sizeof(pfd);
	pfd.nVersion	= 1;
	pfd.dwFlags		= PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;
	pfd.iPixelType	= PFD_TYPE_RGBA;
	pfd.cColorBits	= 24;
	pfd.cRedBits	= 8;
	pfd.cGreenBits	= 8;
	pfd.cBlueBits	= 8; 
	pfd.cDepthBits	= 32;

	pf = ChoosePixelFormat(vp->dc, &pfd);
	SetPixelFormat(vp->dc, pf, &pfd);

	vp->rc = wglCreateContext(vp->dc);
	vp->createRenderContext();

	return vp->bmp;
}
#endif
//-------------------------------------------------------------------------------
// capture context info
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptCaptureViewportInfo(){}
//-------------------------------------------------------------------------------
// Store the view setup for rendering into later
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptStoreView()
{
	//g_viewports[g_currentViewport]->storeView(); /* remove from API */
}
//-------------------------------------------------------------------------------
// Get Current Viewport
//-------------------------------------------------------------------------------
PTint	PTAPI ptCurrentViewport() { return g_currentViewport; }

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static float screenToActual(PTint sx, PTint sy, PTdouble *actual, PTfloat *depth4x4=0)
{
	_ptAdjustMouseYVal(sy);

	pt::ViewParams &vstore = g_viewports[g_currentViewport]->viewParams;
	PTMakeCurrent mc(g_viewports[g_currentViewport]);

	static float pixels[16]; 

	// avoid boundary conditions
	if (sx < 3 || sy < 3 || sx >vstore.viewport[2]-3 || sy >vstore.viewport[3]-3)  
		return -1.0f;

	int i, px;
	bool found = false;
	static int check []  = 
		{	5, 6, 9, 10,
			0, 1, 2, 3,
			7, 11, 15, 14,
			13,12, 8, 4 };	// pixel order for checking

	int spx = sx - 2 ; 
	int spy = sy - 2 ;

	float* usedepth = depth4x4 ? depth4x4 : pixels;

	if (!depth4x4)
	{
		glReadBuffer(GL_FRONT);
		glReadPixels(spx, spy, 4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
	}
	for (i=0; i<16; i++)
	{
		if (usedepth[check[i]] < 1.0f && usedepth[check[i]] > 0.0001f) 
		{ 
			found = true; 
			break; 
		}
	}
	if (found)
	{
		px = check[i];

		if (px < 16 && px >= 0)
		{
			double a[3];
			double v[]= { sx,sy,usedepth[px] };
			vstore.unproject3v(a, v);
			for (i=0; i<3; i++) actual[i] = (float)a[i];
			return usedepth[px];
		}
	}
	return -1;
}
//-----------------------------------------------------------------------------
PTint	PTAPI ptFindNearestScreenPoint( PThandle scene, PTint sx, PTint sy, PTdouble *pnt )
{
	PTTRACE_FUNC_P3( scene, sx, sy )

	pt::vector3d act;
	float px = screenToActual(sx,sy, act);
	
	if (px >= 0 && ptFindNearestPoint(scene, act, pnt) >= 0)
	{
		pt::vector3d win;
		pt::vector3d vpnt(pnt);

		g_viewports[g_currentViewport]->viewParams.project3v(&vpnt.x, &win.x);
		return (int)win.dist(pt::vector3d(sx,sy,win.z));
	}
	else
	{
		/* at least return the screen unproject */ 
		pnt[0] = act[0];
		pnt[1] = act[1];
		pnt[2] = act[2];
	}	
	return -1;
}
//-----------------------------------------------------------------------------
PTint	PTAPI ptFindNearestScreenPointWDepth( PThandle scene, PTint sx, PTint sy, PTfloat *depth4x4, PTdouble *pnt )
{
	PTTRACE_FUNC_P3( scene, sx, sy )

	pt::vector3d act;
	float px = screenToActual(sx,sy, act, depth4x4 );
	
	if (px>=0 && ptFindNearestPoint(scene, act, pnt) >= 0)
	{
		pt::vector3d win;
		pt::vector3d vpnt(pnt);

		g_viewports[g_currentViewport]->viewParams.project3v(&vpnt.x, &win.x);
		return (int)win.dist(pt::vector3d(sx,sy,win.z));
	}
	return -1;
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptReadViewFromGL( void )
{
	PTTRACE_FUNC

	_ptInitialiseViewports();

	ptgl::Viewstore view(true);

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	memcpy(viewp.viewport, view.viewport, sizeof(view.viewport));
	memcpy(viewp.proj_matrix, view.proj_mat, sizeof(view.proj_mat));
	memcpy(viewp.eye_matrix, view.model_mat, sizeof(view.model_mat));
	memcpy(viewp.depth_range, view.depth_range, sizeof(view.depth_range));
	viewp.updatePipeline();

	return PT_TRUE;
}
#endif
//-----------------------------------------------------------------------------
PTbool	PTAPI ptReadViewFromDX( void )
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	return PT_FALSE;
}
//-----------------------------------------------------------------------------
//ptSetViewProjectionOrtho
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewProjectionOrtho( PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f )
{
	PTTRACE_FUNC_P6( l,r,b,t,n,f )

	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	/* the gl version */ 
	mmatrix4d ortho = mmatrix4d::projectGlOrtho(l,r,b,t,n,f);
	ortho.transpose();
	memcpy(viewp.proj_matrix, ortho.data(), sizeof(viewp.proj_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
//ptSetViewProjectionFrustum
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewProjectionFrustum( PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f )
{
	PTTRACE_FUNC_P6( l,r,b,t,n,f )

	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	/* the gl version */ 
	mmatrix4d pers = mmatrix4d::projectPerspectiveGlFrustum(l,r,b,t,n,f);
	pers.transpose();
	memcpy(viewp.proj_matrix, pers.data(), sizeof(viewp.proj_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
// set view projection matrix
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewProjectionMatrix( const PTdouble *matrix, bool row_major )
{
	PTTRACE_FUNC

	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	if (row_major)
		viewp._transposeMatrix( matrix, viewp.proj_matrix );
	else
		memcpy(viewp.proj_matrix, matrix, sizeof(viewp.proj_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
// set view eye perspectiveGL 
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewProjectionPerspective( PTenum type, PTdouble fov, PTdouble aspect, PTdouble n, PTdouble f)
{
	PTTRACE_FUNC_P5( type, fov, aspect, n, f )

	_ptInitialiseViewports();
	
	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	fov *= 0.017453292519943295769236907684886;

	/* the gl version */ 
	mmatrix4d pers;
	
	switch (type)
	{
		case PT_PROJ_PERSPECTIVE_GL: 
			pers = mmatrix4d::projectPerspectiveGl( fov, aspect, n, f );
			break;
		case PT_PROJ_PERSPECTIVE_DX:
			pers = mmatrix4d::projectPerspectiveD3D( fov, aspect, n, f );
			break;
		case PT_PROJ_PERSPECTIVE_BLINN:
			pers = mmatrix4d::projectPerspectiveBlinn( fov, aspect, n, f );
			break;
	}
	pers.transpose();
	memcpy(viewp.proj_matrix, pers.data(), sizeof(viewp.proj_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
// set view eye matrix
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewEyeLookAt( const PTdouble *eye, const PTdouble *target, const PTdouble *up )
{
	PTTRACE_FUNC 

	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	/* the gl version */ 
    pt::vector3d forward( target[0] - eye[0], target[1] - eye[1], target[2] - eye[2] );
	forward.normalize();

    /* Side = forward x up */
	pt::vector3d side( forward.cross(up) );
    side.normalize();

    /* Recompute up as: up = side x forward */
	pt::vector3d upv( side.cross(forward) );

	mmatrix4d m = mmatrix4d::identity();
	mmatrix4d t = mmatrix4d::identity();
	t.inv_translate(eye);
	
    m(0,0) = side.x;
    m(1,0) = side.y;
    m(2,0) = side.z;

    m(0,1) = upv.x;
    m(1,1) = upv.y;
    m(2,1) = upv.z;

    m(0,2) = -forward.x;
    m(1,2) = -forward.y;
    m(2,2) = -forward.z;

	t >>= m;
	t.transpose();
	memcpy(viewp.eye_matrix, t.data(), sizeof(viewp.eye_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
// set view eye matrix
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewEyeMatrix( PTdouble const* matrix, bool row_major )
{
	PTTRACE_FUNC 

	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	if (row_major)
		viewp._transposeMatrix( matrix, viewp.eye_matrix );
	else
		memcpy(viewp.eye_matrix, matrix, sizeof(viewp.eye_matrix));
	viewp.updatePipeline();
}
//-----------------------------------------------------------------------------
// set viewport size
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetViewportSize( PTint left, PTint bottom, PTuint width, PTuint height )
{
	PTTRACE_FUNC_P4( left, bottom, width, height )

	_ptInitialiseViewports();

	int *vp = g_viewports[g_currentViewport]->viewParams.viewport;
	vp[pt::ViewParams::Left] = left;
	vp[pt::ViewParams::Bottom] = bottom;
	vp[pt::ViewParams::Width] = width;
	vp[pt::ViewParams::Height] = height;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTvoid PTAPI ptGetViewEyeMatrix( PTdouble *matrix )
{
	PTTRACE_FUNC 

	_ptInitialiseViewports();
	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	memcpy(matrix, viewp.eye_matrix, sizeof(viewp.eye_matrix));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTvoid PTAPI ptGetViewProjectionMatrix( PTdouble *matrix )
{
	_ptInitialiseViewports();
	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	memcpy(matrix, viewp.proj_matrix, sizeof(viewp.proj_matrix));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTint  PTAPI ptGetViewportPointsBudget( void )
{
	_ptInitialiseViewports();

	return 	g_viewports[g_currentViewport]->pointsCap;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTvoid PTAPI ptSetViewportPointsBudget( PTint budget )
{
	_ptInitialiseViewports();

	if (budget == 0) 
		budget = -1;	// ie. uncapped

	g_viewports[g_currentViewport]->pointsCap = budget;
}
//-----------------------------------------------------------------------------
// unit tests
//-----------------------------------------------------------------------------
PTbool _compMatrices(const double *a, const double *b)
{
	for (int i=0; i< 16; i++)
	{
		if (fabs(a[i]-b[i]) > 1e-4) return false;
	}
	return true;
}
//-----------------------------------------------------------------------------
PTvoid _outputMatrix(const double *m)
{
	int i;
	for (i=0; i<15; i++) std::cout << m[i] << ", ";
	std::cout << m[15] << std::endl;
}

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
PTbool _ptUnitTestProjection()
{
	_ptInitialiseViewports();

	pt::ViewParams &viewp = g_viewports[g_currentViewport]->viewParams;

	/* test view params */ 
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "OpenGL matrix replication" << std::endl;
	
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	/* frustum */ 
	glFrustum(-5,5,-8,8,1,100);

	ptgl::Viewstore view(true);
	ptSetViewProjectionFrustum(-5,5,-8,8,1,100);

	bool res = _compMatrices( viewp.proj_matrix, view.proj_mat );
	std::cout << "glFrustum: " << (!res ? "failed" : "passed") << std::endl;
	if (!res)
	{
		_outputMatrix(viewp.proj_matrix);
		_outputMatrix(view.proj_mat);
	}

	/* glOrtho */ 
	glLoadIdentity();
	glOrtho(-5,5,-8,8,1,100);
	view.store();

	ptSetViewProjectionOrtho(-5,5,-8,8,1,100);
	res = _compMatrices( viewp.proj_matrix, view.proj_mat );
	std::cout << "glOrtho: " << (!res ? "failed" : "passed") << std::endl;
	if (!res)
	{
		_outputMatrix(viewp.proj_matrix);
		_outputMatrix(view.proj_mat);
	}

	/* gluPerspective */ 
	glLoadIdentity();
	gluPerspective(60, 1.33, 1.0, 50.0);
	view.store();

	ptSetViewProjectionPerspective(PT_PROJ_PERSPECTIVE_GL, 60, 1.33, 1.0, 50.0);
	res = _compMatrices( viewp.proj_matrix, view.proj_mat );
	std::cout << "gluPerspective: " << (!res ? "failed" : "passed") << std::endl;
	if (!res)
	{
		_outputMatrix(viewp.proj_matrix);
		_outputMatrix(view.proj_mat);
	}
	glPopMatrix();

	// look at
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	gluLookAt(10,200,-50, 2, -40, 10, 0, 0, 1);
	view.store();

	double eye[] = {10, 200, -50};
	double target[] = { 2,-40, 10};
	double up[] = { 0,0,1 };

	ptSetViewEyeLookAt(eye,target,up);

	res = _compMatrices( viewp.eye_matrix, view.model_mat );
	std::cout << "gluLookAt: " << (!res ? "failed" : "passed") << std::endl;
	if (!res)
	{
		_outputMatrix(viewp.eye_matrix);
		_outputMatrix(view.model_mat);
	}

	glPopMatrix();
	return true;
}
#endif