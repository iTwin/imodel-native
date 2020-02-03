/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageFormatCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ImageFormatCriteria.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ImageFormatCriteria, ImageFormatBuilder
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ImageFormatCriteria.h"
#include "EnumOption.h"
#include "EnumOptionComparer.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageFormatCriteria::ImageFormatCriteria(HCLASS_ID classID, String^ imageFormat, CriteriaOperator criteriaOperator)
{
    m_classID = classID;
    m_imageFormat = imageFormat;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFormatCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    HCLASS_ID id = pRasterFile->GetClassID();
    if (id == m_classID)
    {
        return (m_operator == CriteriaOperator::Equal);
    }
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFormatCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator ImageFormatCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ImageFormatCriteria::ToString()
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
    return L"Format " + op + L" " + m_imageFormat;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageFormatBuilder::ImageFormatBuilder(Hashtable^ supportedFormats)
{
	m_options = gcnew ArrayList();
    IDictionaryEnumerator^ itr = supportedFormats->GetEnumerator();
    while (itr->MoveNext())
    {
        m_options->Add(gcnew EnumOption(static_cast<int>(itr->Key), static_cast<String^>(itr->Value)));
    }
    m_options->Sort(gcnew EnumOptionComparer());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType ImageFormatBuilder::GetFilterType()
{
	return FilterType::Enum;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability ImageFormatBuilder::GetCapabilityType()
{
    return SupportedCapability::FileFormat;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ImageFormatBuilder::GetName()
{
	return L"Image format";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ ImageFormatBuilder::GetOptions()
{
	return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ ImageFormatBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    EnumOption^ imageFormat = static_cast<EnumOption^>(args[0]);
    ImageFormatCriteria^ criteria = gcnew ImageFormatCriteria(imageFormat->GetID(), imageFormat->GetLabel(), criteriaOperator);
    return criteria;
}