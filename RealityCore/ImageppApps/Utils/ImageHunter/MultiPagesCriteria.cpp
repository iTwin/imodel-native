/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MultiPagesCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MultiPagesCriteria.cpp,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class MultiPagesCriteria, MultiPagesBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "MultiPagesCriteria.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultiPagesCriteria::MultiPagesCriteria(CriteriaOperator criteriaOperator)
{
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiPagesCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    if (pRasterFile->CountPages() > 1) {
        return (m_operator == CriteriaOperator::Equal);
    }
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiPagesCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator MultiPagesCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ MultiPagesCriteria::ToString()
{
    if (m_operator == CriteriaOperator::Equal) {
        return L"Multiple Pages = Yes";
    } else {
        return L"Multiple Pages = No";
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultiPagesBuilder::MultiPagesBuilder()
{
    // Nothing to do here for now
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType MultiPagesBuilder::GetFilterType()
{
    return FilterType::Boolean;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability MultiPagesBuilder::GetCapabilityType()
{
    return SupportedCapability::MultiPages;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ MultiPagesBuilder::GetName()
{
    return L"Multiple Pages";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ MultiPagesBuilder::GetOptions()
{
    return nullptr; // No option need to be supported
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ MultiPagesBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    MultiPagesCriteria^ criteria = gcnew MultiPagesCriteria(criteriaOperator);
    return criteria;
}
