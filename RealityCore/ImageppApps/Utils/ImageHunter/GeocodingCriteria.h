/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/GeocodingCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/GeocodingCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : GeocodingCriteria, GeocodingBuilder
//-----------------------------------------------------------------------------
// Allows to search for images that support the Geocoding Capability.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// GeocodingCriteria class
//-----------------------------------------------------------------------------
ref class GeocodingCriteria : public ICriteria
{
public:
    GeocodingCriteria(CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// GeocodingBuilder class
//-----------------------------------------------------------------------------
ref class GeocodingBuilder : public ICriteriaBuilder
{
public:
    GeocodingBuilder();

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};
