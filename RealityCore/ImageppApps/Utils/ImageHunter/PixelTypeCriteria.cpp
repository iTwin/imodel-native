/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/PixelTypeCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/PixelTypeCriteria.cpp,v 1.4 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class PixelTypeCriteria, PixelTypeBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PixelTypeCriteria.h"

using namespace System; 
using namespace System::Collections;

//-----------------------------------------------------------------------------
// PixelTypeCriteria Methods
//-----------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PixelTypeCriteria::PixelTypeCriteria(HCLASS_ID id, String^ label, CriteriaOperator criteriaOperator)
{
    m_classID = id;
    m_label = label;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PixelTypeCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    HCLASS_ID id = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetClassID();
    if (id == m_classID)
    {
        return (m_operator == CriteriaOperator::Equal);
    }
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PixelTypeCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator PixelTypeCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ PixelTypeCriteria::ToString()
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
    return L"Type " + op + L" " + m_label;
}

//-----------------------------------------------------------------------------
// PixelTypeBuilder Methods
//-----------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PixelTypeBuilder::PixelTypeBuilder(Hashtable^ supportedTypes)
{
    m_options = gcnew ArrayList();
    IDictionaryEnumerator^ itr = supportedTypes->GetEnumerator();
    while (itr->MoveNext())
    {
        HCLASS_ID pixelClassID = (HCLASS_ID) (itr->Key);
        
        PixelType^ pixelType = gcnew PixelType(pixelClassID, static_cast<String^>(itr->Value));
        m_options->Add(static_cast<Object^>(pixelType));
    }
    // Sorting pixel types by complexity
    m_options->Sort(gcnew PixelTypeComparer());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType PixelTypeBuilder::GetFilterType()
{
	return FilterType::Enum;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability PixelTypeBuilder::GetCapabilityType()
{
    return SupportedCapability::PixelType;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ PixelTypeBuilder::GetName()
{
	return L"Pixel Type";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ PixelTypeBuilder::GetOptions()
{
	return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ PixelTypeBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    EnumOption^ pixelType = static_cast<EnumOption^>(args[0]);
    PixelTypeCriteria^ criteria = gcnew PixelTypeCriteria(pixelType->GetID(), pixelType->GetLabel(), criteriaOperator);
    return criteria;
}

//-----------------------------------------------------------------------------
// PixelTypeComparer Methods
//-----------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int PixelTypeComparer::Compare(Object^ x, Object^ y)
{
    int comparisonResult = 0;

    PixelType^ pixelTypeX = static_cast<PixelType^>(x);
    PixelType^ pixelTypeY = static_cast<PixelType^>(y);

    // Creating HRPPixelType objects to get Raw data bits.
    // The PixelType are compared by the complexity (more data bits means more complex).
    HFCPtr<HRPPixelType> pHRPPixelX = HRPPixelTypeFactory::GetInstance()->Create(pixelTypeX->GetID());
    HFCPtr<HRPPixelType> pHRPPixelY = HRPPixelTypeFactory::GetInstance()->Create(pixelTypeY->GetID());
    System::UInt32 bitsX = pHRPPixelX->CountPixelRawDataBits();
    System::UInt32 bitsY = pHRPPixelY->CountPixelRawDataBits();

    System::Int16 valueBitsX = 0;
    System::Int16 valueBitsY = 0;
    System::Int16 indexBitsX = 0;
    System::Int16 indexBitsY = 0;

    // Bits comparison
    if (bitsX < bitsY)
    {
        comparisonResult = -1;
    }
    else if (bitsX > bitsY)
    {
        comparisonResult = 1;
    }
    else
    {
        indexBitsX = pHRPPixelX->CountIndexBits();
        indexBitsY = pHRPPixelY->CountIndexBits();
        if (indexBitsX == indexBitsY)
        {
            valueBitsX = pHRPPixelX->CountValueBits();
            valueBitsY = pHRPPixelY->CountValueBits();

            if (valueBitsX == valueBitsY)
                // When Raw data bits are equals, we compare the label
                comparisonResult = String::Compare(pixelTypeX->GetLabel(), pixelTypeY->GetLabel());
            else if (valueBitsX < valueBitsY)
                comparisonResult = -1;
            else
                comparisonResult = 1;
        }
        else
        {
            if (indexBitsX < indexBitsY)
                comparisonResult = -1;
            else
                comparisonResult = 1;
        }
    }

    return comparisonResult;
}