/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/BlockTypeCriteria.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/BlockTypeCriteria.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class BlockTypeCriteria, BlockTypeBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "BlockTypeCriteria.h"
#include "EnumOption.h"
#include "EnumOptionComparer.h"

//Conflict with bentley UInt32 using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BlockTypeCriteria::BlockTypeCriteria(BlockType blockType, System::UInt64 width, System::UInt64 height, System::String^ label, CriteriaOperator criteriaOperator)
{
    m_blockType = BlockTypeBuilder::ConvertBlockType(blockType);
    m_width = width;
    m_height = height;
    m_label = label;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool BlockTypeCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    HRFBlockType::Block blockType = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType().m_BlockType;
    HFCPtr<HRFResolutionDescriptor> resolution = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0);
    bool isValid = false;

    if (m_blockType == blockType)
    {
        isValid = (m_operator != CriteriaOperator::NotEqual);
    } 
    else 
    {
        isValid = (m_operator == CriteriaOperator::NotEqual);
    }

    if (!isValid)
    {
        return false;
    } 
    else 
    {
        switch (m_operator)
        {
        case CriteriaOperator::Equal:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width == resolution->GetBlockWidth() && m_height == resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width == resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height == resolution->GetBlockHeight());
                }
            }
            break;
        case CriteriaOperator::GreaterThan:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width < resolution->GetBlockWidth() && m_height < resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width < resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height < resolution->GetBlockHeight());
                }
            }
            break;
        case CriteriaOperator::GreaterThanOrEqual:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width <= resolution->GetBlockWidth() && m_height <= resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width <= resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height <= resolution->GetBlockHeight());
                }
            }
            break;
        case CriteriaOperator::LessThan:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width > resolution->GetBlockWidth() && m_height > resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width > resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height > resolution->GetBlockHeight());
                }
            }
            break;
        case CriteriaOperator::LessThanOrEqual:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width >= resolution->GetBlockWidth() && m_height >= resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width >= resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height >= resolution->GetBlockHeight());
                }
            }
            break;
        case CriteriaOperator::NotEqual:
            if (m_width != 0 && m_height != 0)
            {
                isValid = (m_width != resolution->GetBlockWidth() && m_height != resolution->GetBlockHeight());
            } 
            else 
            {
                if (m_width !=0) {
                    isValid = (m_width != resolution->GetBlockWidth());
                } else if (m_height !=0) {
                    isValid = (m_height != resolution->GetBlockHeight());
                }
            }
            break;
        }
    }

    return isValid;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool BlockTypeCriteria::isValidWithAtLeastOne()
{
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator BlockTypeCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BlockTypeBuilder::BlockTypeBuilder(Hashtable^ supportedFormats)
{
    m_options = gcnew ArrayList();
    IDictionaryEnumerator^ itr = supportedFormats->GetEnumerator();
    while (itr->MoveNext())
    {
        BlockType myBT = BlockType::TILE;
        myBT = static_cast<BlockType>((int)itr->Key);
        HRFBlockType::Block hrfBlockType = ConvertBlockType(myBT);

        System::String^ label = gcnew System::String(HUTClassIDDescriptor::GetInstance()->GetClassLabelBlockType(hrfBlockType).c_str());
        label = label->Trim();
        m_options->Add(gcnew EnumOption(static_cast<int>(itr->Key),label));
    }
    m_options->Sort(gcnew EnumOptionComparer());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType BlockTypeBuilder::GetFilterType()
{
    return FilterType::EnumWithFields;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ BlockTypeCriteria::ToString()
{
    System::String^ op;
    System::String^ description = L"Type ";

    switch (m_operator)
    {
    case CriteriaOperator::Equal:
        op = L"==";
        break;
    case CriteriaOperator::NotEqual:
        op = L"!=";
        break;
    case CriteriaOperator::GreaterThan:
        op = L">";
        break;
    case CriteriaOperator::GreaterThanOrEqual:
        op = L">=";
        break;
    case CriteriaOperator::LessThan:
        op = L"<";
        break;
    case CriteriaOperator::LessThanOrEqual:
        op = L"<=";
        break;
    }

    description += op + L" " + m_label;
    
    if (m_width != 0 && m_height != 0)
    {
        description += L" (" + m_width.ToString() + L" x " + m_height.ToString() + ")";
    } 
    else 
    {
        if (m_width != 0)
        {
            description += L" (Width: " + m_width.ToString() + L")";
        } else if (m_height != 0) {
            description += L" (Height: " + m_height.ToString() + L")";
        }
    }

    return  description;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability BlockTypeBuilder::GetCapabilityType()
{
    return SupportedCapability::BlockType;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ BlockTypeBuilder::GetName()
{
	return L"Block Type";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ BlockTypeBuilder::GetOptions()
{
	return m_options;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ BlockTypeBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
	EnumOption^ blockType = static_cast<EnumOption^>(args[0]);
    System::UInt64 width = 0;
    System::UInt64 height = 0;
    System::String^ strWidth = static_cast<System::String^>(args[1]);
    System::String^ strHeight = static_cast<System::String^>(args[2]);

    if (strWidth != L"")
    {
        width = int::Parse(strWidth);
    }
    if (strHeight != L"")
    {
        height = int::Parse(strHeight);
    }

    BlockTypeCriteria^ criteria = gcnew BlockTypeCriteria(static_cast<BlockType>(blockType->GetID()), width, height, blockType->GetLabel(), criteriaOperator);
    return criteria;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HRFBlockType::Block BlockTypeBuilder::ConvertBlockType(BlockType btype)
{
    HRFBlockType::Block hrfBlockType = HRFBlockType::TILE;
    switch (btype)
    {
    case BlockType::AUTO_DETECT:
        hrfBlockType = HRFBlockType::AUTO_DETECT;
        break;
    case BlockType::IMAGE:
        hrfBlockType = HRFBlockType::IMAGE;
        break;
    case BlockType::LINE:
        hrfBlockType = HRFBlockType::LINE;
        break;
    case BlockType::STRIP:
        hrfBlockType = HRFBlockType::STRIP;
        break;
    case BlockType::TILE:
        hrfBlockType = HRFBlockType::TILE;
        break;
    default:
        BeAssert(!"BlockTypeBuilder::ConvertBlockType -> Unknown block type");
    }

    return hrfBlockType;
}