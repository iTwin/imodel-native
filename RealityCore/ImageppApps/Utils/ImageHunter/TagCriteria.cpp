/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/TagCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/TagCriteria.cpp,v 1.3 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class TagCriteria, TagBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "TagCriteria.h"
#include "EnumOption.h"
#include "HuntTools.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TagCriteria::TagCriteria(System::String^ tagName, System::String^ value, CriteriaOperator criteriaOperator)
    {
    m_tagName = tagName;
    m_value = value;
    m_operator = criteriaOperator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile) 
    {
    HPMAttributeSet tags = pRasterFile->GetPageDescriptor(0)->GetTags();
    
    for (HPMAttributeSet::HPMASiterator TagIterator = tags.begin(); TagIterator != tags.end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> tag = *TagIterator;
        System::String^ label = gcnew System::String(tag->GetName().c_str());

        if (System::String::Compare(label, m_tagName, true) == 0)
            {
            // When no value has been entered, we only check the tag existence
            if (m_value == L"")
                return (m_operator == CriteriaOperator::Equal); 

            System::String^ dataString = gcnew System::String(tag->GetDataAsString().c_str());

            switch (m_operator)
                {
                case CriteriaOperator::GreaterThanOrEqual:
                case CriteriaOperator::GreaterThan:
                case CriteriaOperator::Equal:
                case CriteriaOperator::LessThan:
                case CriteriaOperator::LessThanOrEqual:
                    return System::String::Compare(m_value, dataString, true) == 0;
                case CriteriaOperator::NotEqual:
                    return System::String::Compare(m_value, dataString, true) != 0;
                    break;
                default:
                    return false;
                    break;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagCriteria::isValidWithAtLeastOne()
{
    return false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator TagCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ TagCriteria::ToString()
{
    System::String^ op;
    System::String^ value = L"---ANY---";

    switch (m_operator)
    {
    case CriteriaOperator::Equal:
        op = L"==";
        break;
    case CriteriaOperator::NotEqual:
        op = L"!=";
        break;
    }

    if (m_value != L"")
    {
        value = m_value;
    }

    return L"Tag " + op + L" (" + m_tagName + L"," + value + L")";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TagBuilder::TagBuilder(System::Collections::Hashtable^ supportedTags)
{
    m_options = gcnew System::Collections::ArrayList();
    System::Collections::IDictionaryEnumerator^ itr = supportedTags->GetEnumerator();
    while (itr->MoveNext())
    {
        System::String^ label = static_cast<System::String^>(itr->Value);
        label = label->Trim();
        m_options->Add(gcnew EnumOption(static_cast<int>(itr->Key),label));
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType TagBuilder::GetFilterType()
{
    return FilterType::KeyValue;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability TagBuilder::GetCapabilityType()
{
    return SupportedCapability::Tag;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ TagBuilder::GetName()
{
    return L"Tag";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Collections::ArrayList^ TagBuilder::GetOptions()
{
    return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ TagBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    System::String^tag = static_cast<System::String^>(args[0]);
    System::String^ value = static_cast<System::String^>(args[1]);
    TagCriteria^ criteria = gcnew TagCriteria(tag, value, criteriaOperator);
    return criteria;
}
