/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MultiResolutionsCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MultiResolutionsCriteria.cpp,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class MultiResolutionsCriteria, MultiResolutionsBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "MultiResolutionsCriteria.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultiResolutionsCriteria::MultiResolutionsCriteria(CriteriaOperator criteriaOperator)
{
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiResolutionsCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    if (pRasterFile->GetPageDescriptor(0)->CountResolutions() > 1) {
        return (m_operator == CriteriaOperator::Equal);
    }
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiResolutionsCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator MultiResolutionsCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ MultiResolutionsCriteria::ToString()
{
    if (m_operator == CriteriaOperator::Equal) {
        return L"Multiple Resolutions = Yes";
    } else {
        return L"Multiple Resolutions = No";
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultiResolutionsBuilder::MultiResolutionsBuilder()
{
    // Nothing to do here for now
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType MultiResolutionsBuilder::GetFilterType()
{
    return FilterType::Boolean;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability MultiResolutionsBuilder::GetCapabilityType()
{
    return SupportedCapability::MultiResolutions;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ MultiResolutionsBuilder::GetName()
{
    return L"Multiple Resolutions";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ MultiResolutionsBuilder::GetOptions()
{
    return nullptr; // No option need to be supported
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ MultiResolutionsBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    MultiResolutionsCriteria^ criteria = gcnew MultiResolutionsCriteria(criteriaOperator);
    return criteria;
}
