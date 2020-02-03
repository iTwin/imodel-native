/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/HistogramCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/HistogramCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : HistogramCriteria, HistogramBuilder
//-----------------------------------------------------------------------------
// Allows to search for images that has/has not a histogram.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// HistogramCriteria class
//-----------------------------------------------------------------------------
ref class HistogramCriteria : public ICriteria
{
public:
    HistogramCriteria(CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;
private:
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// HistogramBuilder class
//-----------------------------------------------------------------------------
ref class HistogramBuilder : public ICriteriaBuilder
{
public:
    HistogramBuilder();
    
    virtual FilterType                      GetFilterType();
    virtual SupportedCapability             GetCapabilityType();
    virtual System::String^                 GetName();
    virtual System::Collections::ArrayList^ GetOptions();
    virtual ICriteria^                      BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};