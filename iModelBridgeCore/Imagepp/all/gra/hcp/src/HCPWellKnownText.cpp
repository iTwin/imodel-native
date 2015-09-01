//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPWellKnownText.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCPWellKnownText.h>

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT::HCPWKT ()
    :   m_wktP(new WString)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT::HCPWKT (WCharCP pi_wkt)
    :   m_wktP(new WString(pi_wkt))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT::HCPWKT (const char* pi_wkt)
    :   m_wktP(new WString())
    {
    BeStringUtilities::CurrentLocaleCharToWChar(*m_wktP,pi_wkt);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT::~HCPWKT ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT::HCPWKT (const HCPWKT& rhs)
    :   m_wktP(new WString(*rhs.m_wktP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT& HCPWKT::operator= (const HCPWKT& rhs)
    {
    *m_wktP = *rhs.m_wktP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT& HCPWKT::operator= (WCharCP pi_wkt)
    {
    *m_wktP = pi_wkt;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HCPWKT& HCPWKT::operator= (const char* pi_wkt)
    {
    BeStringUtilities::CurrentLocaleCharToWChar(*m_wktP,pi_wkt);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HCPWKT::IsEmpty () const
    {
    return m_wktP->empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP HCPWKT::GetCStr () const
    {
    return m_wktP->c_str();
    }