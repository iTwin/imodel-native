/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/NullContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr NullContext::_DrawCached (IStrokeForCache& stroker) 
    {
    stroker._StrokeForCache (*this);

    return nullptr;
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
