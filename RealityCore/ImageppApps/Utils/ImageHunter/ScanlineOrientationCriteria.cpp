/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ScanlineOrientationCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ScanlineOrientationCriteria.cpp,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ScanlineOrientationCriteria, ScanlineOrientationBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ScanlineOrientationCriteria.h"
#include "EnumOption.h"
#include "EnumOptionComparer.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScanlineOrientationCriteria::ScanlineOrientationCriteria(HRFScanlineOrientation::Scanline SLOType, String^ label, CriteriaOperator criteriaOperator)
{
    m_ScanlineOrientation = SLOType;
    m_label = label;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanlineOrientationCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    HRFScanlineOrientation::Scanline sloType = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().m_ScanlineOrientation;
	
    if (m_ScanlineOrientation == sloType)
    {
        return (m_operator == CriteriaOperator::Equal);
    }
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanlineOrientationCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator ScanlineOrientationCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ScanlineOrientationCriteria::ToString()
{
    String^ op;
    switch (m_operator)
    {
    case CriteriaOperator::Equal:
        op = L"==";
        break;
    case CriteriaOperator::NotEqual:
        op = L"!=";
        break;
    }
    return L"Orientation " + op + L" " + m_label;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScanlineOrientationBuilder::ScanlineOrientationBuilder(Hashtable^ supportedFormats)
{
    m_options = gcnew ArrayList();
    IDictionaryEnumerator^ itr = supportedFormats->GetEnumerator();
    while (itr->MoveNext())
    {
        String^ label = static_cast<String^>(itr->Value);
        label = label->Trim();
        m_options->Add(gcnew EnumOption(static_cast<int>(itr->Key),label));
    }
    m_options->Sort(gcnew EnumOptionComparer());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType ScanlineOrientationBuilder::GetFilterType()
{
	return FilterType::Enum;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability ScanlineOrientationBuilder::GetCapabilityType()
{
    return SupportedCapability::ScanlineOrientation;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ScanlineOrientationBuilder::GetName()
{
	return L"Scanline Orientation";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ ScanlineOrientationBuilder::GetOptions()
{
	return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ ScanlineOrientationBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
	EnumOption^ SLOType = static_cast<EnumOption^>(args[0]);
    ScanlineOrientationCriteria^ criteria = gcnew ScanlineOrientationCriteria(static_cast<HRFScanlineOrientation::Scanline>(SLOType->GetID()), SLOType->GetLabel(), criteriaOperator);
    return criteria;
}

