/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/TransfoModelCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/TransfoModelCriteria.h,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : TransfoModelCriteria, TransfoModelBuilder
//-----------------------------------------------------------------------------
// Allows to search for images that support a transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// TransfoModelCriteria class
//-----------------------------------------------------------------------------
ref class TransfoModelCriteria : public ICriteria
{
public:
    TransfoModelCriteria(HCLASS_ID id, System::String^ label, CriteriaOperator criteriaOperator);
    
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
// TrasnfoModelBuilder class
//-----------------------------------------------------------------------------
ref class TransfoModelBuilder : public ICriteriaBuilder
{
public:
    TransfoModelBuilder(System::Collections::Hashtable^ supportedTypes);

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);

private:
    System::Collections::ArrayList^             m_options;
};