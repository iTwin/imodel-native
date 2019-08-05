//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRADEMRaster.h>
#include <ImagePP/all/h/HFCGrid.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRAReferenceToStoredRaster.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HRPDEMFilter.h>
#include <ImagePP/all/h/HRATransaction.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>


HPM_REGISTER_CLASS(HRADEMRaster, HRAImageView)

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster()
    :   HRAImageView()
    {
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster(const HFCPtr<HRAStoredRaster>& pi_pSource,
                           double                        pi_PixelSizeX,
                           double                        pi_PixelSizeY,
                           HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                           HFCPtr<HRPDEMFilter> const&    pi_Filter)
    :   HRAImageView(reinterpret_cast<HFCPtr<HRARaster>const&> (pi_pSource)),
        m_pFilter(new HRPDEMFilter(*pi_Filter)),        // Must keep our own copy because we "SetFor" for this specific source.
        m_pSourceStoredRaster(pi_pSource)
    {
    m_pFilterOp = HRAImageOpDEMFilter::CreateDEMFilter(m_pFilter->GetStyle(), m_pFilter->GetUpperRangeValues(), pi_PixelSizeX, pi_PixelSizeY, *pi_pOrientationTransfo);
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetHillShadingSettings(m_pFilter->GetHillShadingSettings());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetClipToEndValue(m_pFilter->GetClipToEndValues());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetVerticalExaggeration(m_pFilter->GetVerticalExaggeration());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetDefaultRGBA(m_pFilter->GetDefaultColor());
    }

HRADEMRaster::HRADEMRaster(const HFCPtr<HRAReferenceToStoredRaster>& pi_pSource,
                           double                        pi_PixelSizeX,
                           double                        pi_PixelSizeY,
                           HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                           HFCPtr<HRPDEMFilter> const&    pi_Filter)
    :   HRAImageView(reinterpret_cast<HFCPtr<HRARaster>const&> (pi_pSource)),
        m_pFilter(new HRPDEMFilter(*pi_Filter))     // Must keep our own copy because we "SetFor" for this specific source.
    {
    HPRECONDITION(pi_pSource->GetSource()->IsCompatibleWith(HRAStoredRaster::CLASS_ID));

    m_pFilterOp = HRAImageOpDEMFilter::CreateDEMFilter(m_pFilter->GetStyle(), m_pFilter->GetUpperRangeValues(), pi_PixelSizeX, pi_PixelSizeY, *pi_pOrientationTransfo);
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetHillShadingSettings(m_pFilter->GetHillShadingSettings());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetClipToEndValue(m_pFilter->GetClipToEndValues());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetVerticalExaggeration(m_pFilter->GetVerticalExaggeration());
    ((HRAImageOpDEMFilter*)m_pFilterOp.get())->SetDefaultRGBA(m_pFilter->GetDefaultColor());

    
    m_pSourceStoredRaster = reinterpret_cast<HFCPtr<HRAStoredRaster>const&>(pi_pSource->GetSource());
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster(const HRADEMRaster& pi_rObject)
    :   HRAImageView(pi_rObject),
        m_pSourceStoredRaster(pi_rObject.m_pSourceStoredRaster),
        m_pFilter(new HRPDEMFilter(*pi_rObject.m_pFilter))
    {
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRADEMRaster::~HRADEMRaster()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRADEMRaster::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HRADEMRaster(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HRADEMRaster::Clone () const
    {
    return new HRADEMRaster(*this);
    }

//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
bool HRADEMRaster::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id) const
    {
    return GetPixelType()->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRADEMRaster::GetPixelType() const
    {
    return m_pFilter->GetOutputPixelType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRADEMRaster::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    HRACopyToOptions newOptions(options);

    imageNode.AddImageOp(m_pFilterOp, true/*atFront*/);
    
    return GetSource()->BuildCopyToContext(imageNode, newOptions);
    }
