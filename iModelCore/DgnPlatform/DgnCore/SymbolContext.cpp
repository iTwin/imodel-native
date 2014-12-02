/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SymbolContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolContext::SymbolContext (ViewContextR seedContext) : m_seedContext (seedContext)
    {
    if (SUCCESS != Attach (seedContext.GetViewport (), seedContext.GetDrawPurpose ()))
        SetDgnProject (seedContext.GetDgnProject ()); // so "by level" stuff will work in the symbol.

    m_currDisplayParams = *m_seedContext.GetCurrentDisplayParams ();
    m_elemMatSymb       = *m_seedContext.GetElemMatSymb ();
    m_filterLOD         = FILTER_LOD_Off;
    m_ignoreViewRange   = true;
    m_parentRangeResult = RangeResult::Inside; // This will cause inhibit all range testing.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            SymbolContext::_SetupOutputs ()
    {
    m_IViewDraw   = m_viewport->GetIViewDraw ();
    m_IDrawGeom   = m_viewport->GetICachedDraw ();
    m_ICachedDraw = m_viewport->GetICachedDraw ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            SymbolContext::_Detach ()
    {
    // Preserve current QVis state, i.e. don't call PopAll, ActivateOverrideMatSymb, ResynchColorMap, etc.
    m_IViewDraw   = NULL;
    m_IDrawGeom   = NULL;
    m_ICachedDraw = NULL;
    m_viewport    = NULL;

    T_Super::_Detach ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
QvElemP         SymbolContext::DrawSymbolForCache (IDisplaySymbol* symbol, QvCacheR symbolCache)
    {
    ViewFlags   viewFlags;
    ViewFlagsCP viewFlagsP = m_seedContext.GetViewFlags ();

    if (NULL != viewFlagsP)
        {
        // Make sure that we don't try to draw with a line style because of level symbology...
        viewFlags = *viewFlagsP;
        viewFlags.lev_symb = false;
        viewFlagsP = &viewFlags;
        }

    m_ICachedDraw->BeginCacheElement (Is3dView (), &symbolCache, viewFlagsP);

    m_creatingCacheElem = true;

    symbol->_Draw (*this);

    m_creatingCacheElem = false;

    QvElemP qvElem = m_ICachedDraw->EndCacheElement ();

    if (NULL == qvElem)
        return NULL;

    T_HOST.GetGraphicsAdmin()._SaveQvElemForSymbol (symbol, qvElem); // save the qvelem in case we encounter this symbol again
    
    return qvElem;
    }
