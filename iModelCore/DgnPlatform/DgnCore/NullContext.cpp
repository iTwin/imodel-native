/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/NullContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem*         NullContext::_DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 index) 
    {
    stroker._StrokeForCache (dh, *this);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void NullContext::_AllocateScanCriteria ()
    {
    if (!m_setupScan)
        return;

    T_Super::_AllocateScanCriteria ();
    }
