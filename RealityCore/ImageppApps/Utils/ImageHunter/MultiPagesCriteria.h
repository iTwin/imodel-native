/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MultiPagesCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MultiPagesCriteria.h,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : MultiPagesCriteria, MultiPagesBuilder
//-----------------------------------------------------------------------------
// Allows to search for images with multiple pages.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// MultiplePagesCriteria class
//-----------------------------------------------------------------------------
ref class MultiPagesCriteria : public ICriteria
{
public:
    MultiPagesCriteria(CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// MultiPagesBuilder class
//-----------------------------------------------------------------------------
ref class MultiPagesBuilder : public ICriteriaBuilder
{
public:
    MultiPagesBuilder();

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};
