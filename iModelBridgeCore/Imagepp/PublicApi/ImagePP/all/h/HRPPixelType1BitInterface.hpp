//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType1BitInterface.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool HRPPixelType1BitInterface::IsForegroundStateDefined () const
    {
    return m_IsForegroundStateDefined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool HRPPixelType1BitInterface::GetForegroundState () const
    {
    HPRECONDITION(IsForegroundStateDefined());  // You should not call this if the foreground state is not defined
    return m_ForegroundState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  02/2005
+---------------+---------------+---------------+---------------+---------------+------*/
inline void HRPPixelType1BitInterface::SetForegroundState (bool pi_ForegroundState)
    {
    m_IsForegroundStateDefined = true;
    m_ForegroundState = pi_ForegroundState;
    }

