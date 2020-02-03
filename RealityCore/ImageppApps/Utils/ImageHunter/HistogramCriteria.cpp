/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/HistogramCriteria.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/HistogramCriteria.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class HistogramCriteria, HistogramBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "HistogramCriteria.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HistogramCriteria::HistogramCriteria(CriteriaOperator criteriaOperator)
{
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool HistogramCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    bool hasHistogram = false;
    for (uint32_t index(0); index < pRasterFile->CountPages(); ++index)
    {
        if (pRasterFile->GetPageDescriptor(index)->HasHistogram())
        {
            hasHistogram = true;
            break;
        }
    }

    if (m_operator == CriteriaOperator::Equal && hasHistogram)
    {
        return true;
    } else if (m_operator == CriteriaOperator::NotEqual && !hasHistogram) {
        return true;
    } else {
        return false;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool HistogramCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator HistogramCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ HistogramCriteria::ToString()
{
    if (m_operator == CriteriaOperator::Equal) {
        return L"Histogram = Yes";
    } else {
        return L"Histogram = No";
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HistogramBuilder::HistogramBuilder()
{
    // Nothing to do here for now
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType HistogramBuilder::GetFilterType()
{
    return FilterType::Boolean;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability HistogramBuilder::GetCapabilityType()
{
    return SupportedCapability::Histogram;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ HistogramBuilder::GetName()
{
    return L"Histogram";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ HistogramBuilder::GetOptions()
{
    return nullptr; // No option need to be supported
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ HistogramBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    HistogramCriteria^ criteria = gcnew HistogramCriteria(criteriaOperator);
    return criteria;
}
