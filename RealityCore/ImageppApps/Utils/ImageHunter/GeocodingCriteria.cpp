/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/GeocodingCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/GeocodingCriteria.cpp,v 1.3 2011/06/02 19:38:27 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class GeocodingCriteria, GeocodingBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "GeocodingCriteria.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GeocodingCriteria::GeocodingCriteria(CriteriaOperator criteriaOperator)
{
    m_operator = criteriaOperator;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeocodingCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
	if (pRasterFile->GetPageDescriptor(0)->GetCapabilities()->HasCapabilityOfType(HRFGeocodingCapability::CLASS_ID))
    {
        return (m_operator == CriteriaOperator::Equal);
    } else {
        return (m_operator == CriteriaOperator::NotEqual);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeocodingCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator GeocodingCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ GeocodingCriteria::ToString()
{
    if (m_operator == CriteriaOperator::Equal) {
        return L"Geocoding = Yes";
    } else {
        return L"Geocoding = No";
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GeocodingBuilder::GeocodingBuilder()
{
    // Nothing to do here for now
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType GeocodingBuilder::GetFilterType()
{
    return FilterType::Boolean;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability GeocodingBuilder::GetCapabilityType()
{
    return SupportedCapability::Geocoding;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ GeocodingBuilder::GetName()
{
    return L"Geocoding";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ GeocodingBuilder::GetOptions()
{
    return nullptr; // No option need to be supported
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ GeocodingBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    GeocodingCriteria^ criteria = gcnew GeocodingCriteria(criteriaOperator);
    return criteria;
}
