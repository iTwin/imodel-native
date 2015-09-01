//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType1bitInterface.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


/*---------------------------------------------------------------------------------**//**
* This class is an interface for all binary pixeltype.
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType1BitInterface
    {
public:
    IMAGEPP_EXPORT HRPPixelType1BitInterface();
    IMAGEPP_EXPORT HRPPixelType1BitInterface(const HRPPixelType1BitInterface& pi_rObj);
    IMAGEPP_EXPORT virtual ~HRPPixelType1BitInterface();

    bool IsForegroundStateDefined() const {return m_IsForegroundStateDefined;}
    void UndefineForegroundState() { m_IsForegroundStateDefined = false;}

    bool GetForegroundState() const
        {
        HPRECONDITION(IsForegroundStateDefined());  // You should not call this if the foreground state is not defined
        return m_ForegroundState;
        }
    void SetForegroundState(bool pi_ForegroundState)
        {
        m_IsForegroundStateDefined = true;
        m_ForegroundState = pi_ForegroundState;
        }

private:
    bool   m_IsForegroundStateDefined;
    bool   m_ForegroundState;
    };
END_IMAGEPP_NAMESPACE

