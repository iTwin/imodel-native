//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelType1BitInterface.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelType1BitInterface.h>

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