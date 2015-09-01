//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDMetaDataContainer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDMetaDataContainer.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDMetaDataContainer::HMDMetaDataContainer(HMDMetaDataContainer::Type pi_Type)
    {
    m_Type       = pi_Type;
    m_HasChanged = false;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDMetaDataContainer::~HMDMetaDataContainer()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Copy constructor
//-----------------------------------------------------------------------------
HMDMetaDataContainer::HMDMetaDataContainer(const HMDMetaDataContainer& pi_rObj)
    {
    m_Type       = pi_rObj.m_Type;
    m_HasChanged = pi_rObj.m_HasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// Get the type of the metadata contained in the container
//-----------------------------------------------------------------------------
HMDMetaDataContainer::Type HMDMetaDataContainer::GetType() const
    {
    return m_Type;
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the metadata have been changed
//-----------------------------------------------------------------------------
bool HMDMetaDataContainer::HasChanged() const
    {
    return m_HasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// Specify if the metadata have been changed or not
//-----------------------------------------------------------------------------
void HMDMetaDataContainer::SetModificationStatus(bool pi_HasChanged)
    {
    m_HasChanged = pi_HasChanged;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HMDDrawOptionsPDF::HMDDrawOptionsPDF()
    :HMDMetaDataContainer(HMD_PDF_DRAW_OPTIONS)
    {
    m_smoothText = m_smoothLineArt = m_smoothImage = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HMDDrawOptionsPDF::~HMDDrawOptionsPDF()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HMDMetaDataContainer> HMDDrawOptionsPDF::Clone() const
    {
    return new HMDDrawOptionsPDF(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HMDDrawOptionsPDF::HMDDrawOptionsPDF(const HMDDrawOptionsPDF& pi_rObj)
    : HMDMetaDataContainer(pi_rObj)
    {
    m_smoothText = pi_rObj.m_smoothText;
    m_smoothLineArt = pi_rObj.m_smoothLineArt;
    m_smoothImage = pi_rObj.m_smoothImage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HMDDrawOptionsPDF::GetSmoothText() const
    {
    return m_smoothText;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HMDDrawOptionsPDF::SetSmoothText(bool val)
    {
    m_smoothText = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HMDDrawOptionsPDF::GetSmoothLineArt() const
    {
    return m_smoothLineArt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HMDDrawOptionsPDF::SetSmoothLineArt(bool val)
    {
    m_smoothLineArt = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HMDDrawOptionsPDF::GetSmoothImage() const
    {
    return m_smoothImage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HMDDrawOptionsPDF::SetSmoothImage(bool val)
    {
    m_smoothImage = val;
    }