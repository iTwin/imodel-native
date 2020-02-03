/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MultiResolutionsCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MultiResolutionsCriteria.h,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : MultiResolutionsCriteria, MultiResolutionsBuilder
//-----------------------------------------------------------------------------
// Allows to search for images with multiple resolutions.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// MultiResolutionsCriteria class
//-----------------------------------------------------------------------------
ref class MultiResolutionsCriteria : public ICriteria
{
public:
    MultiResolutionsCriteria(CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// MultiResolutionsBuilder class
//-----------------------------------------------------------------------------
ref class MultiResolutionsBuilder : public ICriteriaBuilder
{
public:
    MultiResolutionsBuilder();

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};