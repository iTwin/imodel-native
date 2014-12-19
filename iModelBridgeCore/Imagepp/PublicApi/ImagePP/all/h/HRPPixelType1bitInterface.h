//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType1bitInterface.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/h/HmrTypes.h>

/*---------------------------------------------------------------------------------**//**
* This class is an interface for all binary pixeltype.
+---------------+---------------+---------------+---------------+---------------+------*/
class HRPPixelType1BitInterface
    {
public:
    _HDLLg HRPPixelType1BitInterface();
    _HDLLg HRPPixelType1BitInterface(const HRPPixelType1BitInterface& pi_rObj);
    _HDLLg virtual ~HRPPixelType1BitInterface();

    _HDLLg bool   IsForegroundStateDefined() const;
    _HDLLg bool   GetForegroundState() const;
    _HDLLg void    SetForegroundState(bool pi_ForegroundState);

private:
    bool   m_IsForegroundStateDefined;
    bool   m_ForegroundState;
    };

#include "HRPPixelType1BitInterface.hpp"
