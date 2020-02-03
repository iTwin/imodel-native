/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ScanlineOrientationCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ScanlineOrientationCriteria.h,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ScanlineOrientationCriteria, ScanlineOrientationBuilder
//-----------------------------------------------------------------------------
// Allows to search by scanline orientation.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// ScanlineOrientationCriteria class
//-----------------------------------------------------------------------------
ref class ScanlineOrientationCriteria : public ICriteria
{
public:
    ScanlineOrientationCriteria(HRFScanlineOrientation::Scanline SLOType, 
                                System::String^ label, CriteriaOperator criteriaOperator);
    
    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    HRFScanlineOrientation::Scanline    m_ScanlineOrientation;
    CriteriaOperator                    m_operator;
    System::String^                     m_label;
};

//-----------------------------------------------------------------------------
// ScanlineOrientationBuilder class
//-----------------------------------------------------------------------------
ref class ScanlineOrientationBuilder : public ICriteriaBuilder
{
public:
    ScanlineOrientationBuilder(System::Collections::Hashtable^ supportedFormats);

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);

private:
    System::Collections::ArrayList^             m_options;
};