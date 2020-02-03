/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/BlockTypeCriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/BlockTypeCriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : BlockTypeCriteria, BlockTypeBuilder
//-----------------------------------------------------------------------------
// Allows to search for different Block Type inside images.
//-----------------------------------------------------------------------------
#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// BlockType Enum
//
// .NET does not like HRFBlockType enum so it is cloned here.
// The values must be in the same order as in HRFTypes.h.
//-----------------------------------------------------------------------------
enum class BlockType : int
{
    LINE,
    TILE,
    STRIP,
    IMAGE,
    AUTO_DETECT
};

//-----------------------------------------------------------------------------
// BlockTypeCriteria class
//-----------------------------------------------------------------------------
ref class BlockTypeCriteria : public ICriteria
{
public:
    BlockTypeCriteria(BlockType blockType, System::UInt64 width, System::UInt64 height, 
                      System::String^ label, CriteriaOperator criteriaOperator);

    virtual bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    virtual bool                isValidWithAtLeastOne();
    virtual CriteriaOperator    GetCriteriaOperator();
    virtual System::String^     ToString() override;

private:
    HRFBlockType::Block         m_blockType;
    System::String^             m_label;
    System::UInt64              m_width;
    System::UInt64              m_height;
    CriteriaOperator            m_operator;
};

//-----------------------------------------------------------------------------
// BlockTypeBuilder class
//-----------------------------------------------------------------------------
ref class BlockTypeBuilder : public ICriteriaBuilder
{
public:
    BlockTypeBuilder(System::Collections::Hashtable^ supportedFormats);

    virtual FilterType                          GetFilterType();
    virtual SupportedCapability                 GetCapabilityType();
    virtual System::String^                     GetName();
    virtual System::Collections::ArrayList^     GetOptions();
    virtual ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
    static HRFBlockType::Block                  ConvertBlockType(BlockType btype);

private:
    System::Collections::ArrayList^ m_options;
};