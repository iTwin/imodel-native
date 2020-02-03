/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/TagCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/TagCriteria.h,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : TagCriteria, TagBuilder
//-----------------------------------------------------------------------------
// Allows to search for tags found inside images.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// TagCriteria class
//-----------------------------------------------------------------------------
ref class TagCriteria : public ICriteria
{
public:
    TagCriteria(System::String^ tagName, System::String^ value, CriteriaOperator criteriaOperator); 

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual                     CriteriaOperator GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    System::String^     m_tagName;
    System::String^     m_value;
    CriteriaOperator    m_operator;
};

//-----------------------------------------------------------------------------
// TagBuilder class
//-----------------------------------------------------------------------------
ref class TagBuilder : public ICriteriaBuilder
{
public:
    TagBuilder(System::Collections::Hashtable^ supportedTags);

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
private:
    System::Collections::ArrayList^             m_options;
};
