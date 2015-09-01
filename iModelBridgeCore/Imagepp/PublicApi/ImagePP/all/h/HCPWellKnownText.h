//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPWellKnownText.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Type : HFCWellKnownText
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct HCPWKT
    {
private:
    std::unique_ptr<WString>          m_wktP;
public:
    IMAGEPP_EXPORT explicit                 HCPWKT                             ();

    IMAGEPP_EXPORT explicit                 HCPWKT                             (WCharCP                    pi_wkt);
    IMAGEPP_EXPORT explicit                 HCPWKT                             (const char*                 pi_wkt);

    IMAGEPP_EXPORT                          ~HCPWKT                            ();

    IMAGEPP_EXPORT                          HCPWKT                             (const HCPWKT&               rhs);
    IMAGEPP_EXPORT HCPWKT&                  operator=                          (const HCPWKT&               rhs);

    IMAGEPP_EXPORT HCPWKT&                  operator=                          (WCharCP                     pi_wkt);
    IMAGEPP_EXPORT HCPWKT&                  operator=                          (const char*                 pi_wkt);

    IMAGEPP_EXPORT bool                     IsEmpty                            () const;
    IMAGEPP_EXPORT WCharCP                  GetCStr                            () const;
    };

END_IMAGEPP_NAMESPACE