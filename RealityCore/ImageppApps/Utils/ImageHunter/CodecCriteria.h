/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/CodecCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/CodecCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : CodecCriteria, CodecBuilder
//-----------------------------------------------------------------------------
// Allows to search for different compression codec used in the rasters.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// CodecCriteria class
//-----------------------------------------------------------------------------
ref class CodecCriteria : public ICriteria
{
public:
    CodecCriteria(HCLASS_ID id, System::String^ label, CriteriaOperator criteriaOperator);
	
    virtual bool             isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool             isValidWithAtLeastOne();
    virtual CriteriaOperator GetCriteriaOperator();
    virtual System::String^  ToString() override;
private:
    HCLASS_ID                m_classID;
    System::String^          m_label;
    CriteriaOperator         m_operator;
};

//-----------------------------------------------------------------------------
// CodecBuilder class
//-----------------------------------------------------------------------------
ref class CodecBuilder : public ICriteriaBuilder
{
public:
    CodecBuilder(System::Collections::Hashtable^ supportedFormats);

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);

private:
    System::Collections::ArrayList^ m_options;
};