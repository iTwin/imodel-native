//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPWellKnownText.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Type : HFCWellKnownText
//-----------------------------------------------------------------------------
#pragma once


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct HCPWKT
    {
private:
    std::auto_ptr<WString>          m_wktP;
public:
    _HDLLg explicit                 HCPWKT                             ();

    _HDLLg explicit                 HCPWKT                             (WCharCP                    pi_wkt);
    _HDLLg explicit                 HCPWKT                             (const char*                 pi_wkt);

    _HDLLg                          ~HCPWKT                            ();

    _HDLLg                          HCPWKT                             (const HCPWKT&               rhs);
    _HDLLg HCPWKT&                  operator=                          (const HCPWKT&               rhs);

    _HDLLg HCPWKT&                  operator=                          (WCharCP                     pi_wkt);
    _HDLLg HCPWKT&                  operator=                          (const char*                 pi_wkt);

    _HDLLg bool                     IsEmpty                            () const;
    _HDLLg WCharCP                  GetCStr                            () const;
    };