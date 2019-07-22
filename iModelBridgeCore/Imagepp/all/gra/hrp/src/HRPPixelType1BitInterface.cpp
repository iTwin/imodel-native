//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRPPixelType1bitInterface.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelType1BitInterface::HRPPixelType1BitInterface ()
    :m_IsForegroundStateDefined(false),
     m_ForegroundState(true)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelType1BitInterface::HRPPixelType1BitInterface (const HRPPixelType1BitInterface& pi_rObj)
    :m_IsForegroundStateDefined(pi_rObj.m_IsForegroundStateDefined),
     m_ForegroundState(pi_rObj.m_ForegroundState)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelType1BitInterface::~HRPPixelType1BitInterface ()
    {
    }
