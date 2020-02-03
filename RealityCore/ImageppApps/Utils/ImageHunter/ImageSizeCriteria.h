/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageSizeCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ImageSizeCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ImageSizeCriteria, ImageSizeBuilder
//-----------------------------------------------------------------------------
// Allows to search for images with a particular resolution
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// ImageSizeCriteria class
//-----------------------------------------------------------------------------
ref class ImageSizeCriteria : public ICriteria
{
public:
    ImageSizeCriteria(int width, int height, CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;
private:
    System::UInt64              m_width;
    System::UInt64              m_height;
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// ImageSizeBuilder class
//-----------------------------------------------------------------------------
ref class ImageSizeBuilder : ICriteriaBuilder
{
public:
    ImageSizeBuilder();

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};