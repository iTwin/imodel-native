/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageSizeCriteria.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ImageSizeCriteria.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ImageSizeCriteria, ImageSizeBuilder
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ImageSizeCriteria.h"

//using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSizeCriteria::ImageSizeCriteria(int width, int height, CriteriaOperator criteriaOperator)
{
    m_width = width;
    m_height = height;
    m_operator = criteriaOperator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageSizeCriteria::isRespected(HFCPtr<HRFRasterFile> pRasterFile)
{
    HFCPtr<HRFResolutionDescriptor> resolution = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0);
    bool isValid = false;

    switch (m_operator)
    {
    case CriteriaOperator::Equal:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width == resolution->GetWidth() && m_height == resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width == resolution->GetWidth());
            } else {
                isValid = (m_height == resolution->GetHeight());
            }
        }
        break;
    case CriteriaOperator::GreaterThan:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width < resolution->GetWidth() && m_height < resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width < resolution->GetWidth());
            } else {
                isValid = (m_height < resolution->GetHeight());
            }
        }
        break;
    case CriteriaOperator::GreaterThanOrEqual:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width <= resolution->GetWidth() && m_height <= resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width <= resolution->GetWidth());
            } else {
                isValid = (m_height <= resolution->GetHeight());
            }
        }
        break;
    case CriteriaOperator::LessThan:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width > resolution->GetWidth() && m_height > resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width > resolution->GetWidth());
            } else {
                isValid = (m_height > resolution->GetHeight());
            }
        }
        break;
    case CriteriaOperator::LessThanOrEqual:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width >= resolution->GetWidth() && m_height >= resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width >= resolution->GetWidth());
            } else {
                isValid = (m_height >= resolution->GetHeight());
            }
        }
        break;
    case CriteriaOperator::NotEqual:
        if (m_width != 0 && m_height != 0)
        {
            isValid = (m_width != resolution->GetWidth() && m_height != resolution->GetHeight());
        } else {
            if (m_width !=0) {
                isValid = (m_width != resolution->GetWidth());
            } else {
                isValid = (m_height != resolution->GetHeight());
            }
        }
        break;
    }

    return isValid;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageSizeCriteria::isValidWithAtLeastOne()
{
    return false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CriteriaOperator ImageSizeCriteria::GetCriteriaOperator()
{
    return m_operator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ ImageSizeCriteria::ToString()
{
    System::String^ op;
    System::String^ description = L"";

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
    
    if (m_width != 0 && m_height != 0)
    {
        description += m_width.ToString() + L" x " + m_height.ToString();
    } else {
        if (m_width != 0)
        {
            description += L"Width: " + m_width.ToString();
        } else {
            description += L"Height: " + m_height.ToString();
        }
    }

    return L"Size " + op + L" " + description;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSizeBuilder::ImageSizeBuilder()
{
    // Nothing to do here for now
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterType ImageSizeBuilder::GetFilterType()
{
    return FilterType::Size;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedCapability ImageSizeBuilder::GetCapabilityType()
{
    return SupportedCapability::ImageSize;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ ImageSizeBuilder::GetName()
{
    return L"Image Size";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ ImageSizeBuilder::GetOptions()
{
    return nullptr; // No option needed
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteria^ ImageSizeBuilder::BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args)
{
    int width = 0;
    int height = 0;
    System::String^ strWidth = static_cast<System::String^>(args[0]);
    System::String^ strHeight = static_cast<System::String^>(args[1]);
    
    if (strWidth != L"")
    {
        width = int::Parse(strWidth);
    }
    if (strHeight != L"")
    {
        height = int::Parse(strHeight);
    }
    ImageSizeCriteria^ criteria = gcnew ImageSizeCriteria(width, height, criteriaOperator);
    return criteria;
}