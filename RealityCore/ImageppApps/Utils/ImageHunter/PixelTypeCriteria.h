/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/PixelTypeCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/PixelTypeCriteria.h,v 1.3 2011/06/02 19:38:28 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Class : PixelTypeCriteria, PixelTypeBuilder
//-----------------------------------------------------------------------------
// Allows to search for images with a particular pixel type.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"
#include "EnumOption.h"

//-----------------------------------------------------------------------------
// PixelTypeCriteria class
//-----------------------------------------------------------------------------
ref class PixelType : public EnumOption
{
public:
    PixelType(int id, System::String^ label)
    {
        m_id = id;
        m_label = label;
    };
    property System::UInt64 ClassKey;
};

//-----------------------------------------------------------------------------
// PixelTypeCriteria class
//-----------------------------------------------------------------------------
ref class PixelTypeCriteria : public ICriteria
{
public:
    PixelTypeCriteria(HCLASS_ID id, System::String^ label, CriteriaOperator criteriaOperator);
    
    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;
private:
    HCLASS_ID                   m_classID;
    System::String^             m_label;
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// PixelTypeBuilder class
//-----------------------------------------------------------------------------
ref class PixelTypeBuilder : public ICriteriaBuilder
{
public:
    PixelTypeBuilder(System::Collections::Hashtable^ supportedTypes);
    
    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
private:
    System::Collections::ArrayList^             m_options;
};


//-----------------------------------------------------------------------------
// PixelTypeComparer class
//-----------------------------------------------------------------------------
ref class PixelTypeComparer : public System::Collections::IComparer
{
public:
    virtual int Compare(Object^ x, Object^ y);
};