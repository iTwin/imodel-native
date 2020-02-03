/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/CodecCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/CodecCriteria.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class CodecCriteria, CodecBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CodecCriteria.h"
#include "EnumOption.h"
#include "EnumOptionComparer.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CodecCriteria::CodecCriteria(HCLASS_ID id, String^ label, CriteriaOperator criteriaOperator)
{
    m_classID = id;
    m_label = label;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool CodecCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    if (m_classID == pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID())
        return (m_operator == CriteriaOperator::Equal);
        
    return (m_operator == CriteriaOperator::NotEqual);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool CodecCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator CodecCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ CodecCriteria::ToString()
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CodecBuilder::CodecBuilder(Hashtable^ supportedFormats)
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
FilterType CodecBuilder::GetFilterType()
{
	return FilterType::Enum;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability CodecBuilder::GetCapabilityType()
{
    return SupportedCapability::Codec;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ CodecBuilder::GetName()
{
	return L"Codec";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ CodecBuilder::GetOptions()
{
	return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ CodecBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
	EnumOption^ codecType = static_cast<EnumOption^>(args[0]);
    CodecCriteria^ criteria = gcnew CodecCriteria(codecType->GetID(), codecType->GetLabel(), criteriaOperator);
    return criteria;
}