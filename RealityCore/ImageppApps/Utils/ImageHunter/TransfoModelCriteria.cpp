/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/TransfoModelCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/TransfoModelCriteria.cpp,v 1.3 2011/06/02 19:38:28 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class TransfoModelCriteria, TransfoModelBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "TransfoModelCriteria.h"
#include "EnumOption.h"
#include "EnumOptionComparer.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModelCriteria::TransfoModelCriteria(HCLASS_ID id, String^ label, CriteriaOperator criteriaOperator)
{
    m_classID = id;
    m_label = label;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransfoModelCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    if (pRasterFile->GetPageDescriptor(0)->HasTransfoModel())
    {
        // if ANY has been chosen
        if (m_classID == -1)
        {
            return (m_operator == CriteriaOperator::Equal);
        }
        HCLASS_ID id = pRasterFile->GetPageDescriptor(0)->GetTransfoModel()->GetClassID();
        if (id == m_classID)
        {
            return (m_operator == CriteriaOperator::Equal);
        }
        return (m_operator == CriteriaOperator::NotEqual);
    } else {
        return (m_operator == CriteriaOperator::NotEqual);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransfoModelCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator TransfoModelCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ TransfoModelCriteria::ToString()
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
TransfoModelBuilder::TransfoModelBuilder(Hashtable^ supportedTypes)
{
    m_options = gcnew ArrayList();
    m_options->Add(gcnew EnumOption(-1, L"--- Any ---"));
    IDictionaryEnumerator^ itr = supportedTypes->GetEnumerator();
    while (itr->MoveNext())
    {
        m_options->Add(gcnew EnumOption(static_cast<int>(itr->Key), static_cast<String^>(itr->Value)));
    }
    m_options->Sort(gcnew EnumOptionComparer());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType TransfoModelBuilder::GetFilterType()
{
    return FilterType::Enum;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability TransfoModelBuilder::GetCapabilityType()
{
    return SupportedCapability::TransfoModel;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ TransfoModelBuilder::GetName()
{
    return L"Transformation Model";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ TransfoModelBuilder::GetOptions()
{
    return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ TransfoModelBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    EnumOption^ transfoModel = static_cast<EnumOption^>(args[0]);
    TransfoModelCriteria^ criteria = gcnew TransfoModelCriteria(transfoModel->GetID(), transfoModel->GetLabel(), criteriaOperator);
    return criteria;
}
