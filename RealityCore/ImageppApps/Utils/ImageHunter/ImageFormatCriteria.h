/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageFormatCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ImageFormatCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ImageFormatCriteria, ImageFormatBuilder
//-----------------------------------------------------------------------------
// Allows to search for particular image formats.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// ImageFormatCriteria class
//-----------------------------------------------------------------------------
ref class ImageFormatCriteria : public ICriteria
{
public:
    ImageFormatCriteria(HCLASS_ID classID, System::String^ imageFormat, CriteriaOperator criteriaOperator);
    
    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();   
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;
private:
    HCLASS_ID                   m_classID;
    System::String^             m_imageFormat;
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// ImageFormatBuilder class
//-----------------------------------------------------------------------------
ref class ImageFormatBuilder : public ICriteriaBuilder
{
public:
    ImageFormatBuilder(System::Collections::Hashtable^ supportedFormats);
    
    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
private:
    System::Collections::ArrayList^             m_options;
};