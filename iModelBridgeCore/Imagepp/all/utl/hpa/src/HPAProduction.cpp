//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAProduction.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAProduction
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPAProduction.h>
#include <Imagepp/all/h/HPARule.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPAProduction::SetRule(HPARule* pi_pRule)
    {
    m_pRule = pi_pRule;
    Syntax::iterator itr = m_Syntax.begin();
    (*itr)->AddReferingProduction(this);
    if ((*itr) == pi_pRule)
        pi_pRule->SetLeftRecursive();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPAProduction::operator==(const HPAProduction& pi_rRight)
    {
    bool Result = (m_Syntax.size() == pi_rRight.m_Syntax.size());
    Syntax::iterator itrL = m_Syntax.begin();
    Syntax::const_iterator itrR = pi_rRight.m_Syntax.begin();
    while (Result && (itrL != m_Syntax.end()))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        }
    return Result;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPAProduction::operator!=(const HPAProduction& pi_rRight)
    {
    bool Result = (m_Syntax.size() == pi_rRight.m_Syntax.size());
    Syntax::iterator itrL = m_Syntax.begin();
    Syntax::const_iterator itrR = pi_rRight.m_Syntax.begin();
    while (Result && (itrL != m_Syntax.end()))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        }
    return !Result;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPAProduction::operator>(const HPAProduction& pi_rRight)
    {
    bool Result = (m_Syntax.size() <= pi_rRight.m_Syntax.size());
    Syntax::iterator itrL = m_Syntax.begin();
    Syntax::const_iterator itrR = pi_rRight.m_Syntax.begin();
    while (Result && (itrL != m_Syntax.end()))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        }
    return !Result;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPAProduction::operator<(const HPAProduction& pi_rRight)
    {
    bool Result = (m_Syntax.size() < pi_rRight.m_Syntax.size());
    Syntax::iterator itrL = m_Syntax.begin();
    Syntax::const_iterator itrR = pi_rRight.m_Syntax.begin();
    while (Result && (itrL != m_Syntax.end()))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        }
    return Result;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPAProduction::operator<=(const HPAProduction& pi_rRight)
    {
    bool Result = (m_Syntax.size() <= pi_rRight.m_Syntax.size());
    Syntax::iterator itrL = m_Syntax.begin();
    Syntax::const_iterator itrR = pi_rRight.m_Syntax.begin();
    while (Result && (itrL != m_Syntax.end()))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        }
    return Result;
    }
