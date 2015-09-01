//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageView.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAImageView.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HMDContext.h>
#include <Imagepp/all/h/HVEShape.h>

HPM_REGISTER_ABSTRACT_CLASS(HRAImageView, HRARaster)


/** -----------------------------------------------------------------------------
    protected
    constructor
    -----------------------------------------------------------------------------
*/
HRAImageView::HRAImageView()
    :   HRARaster()
    {
    }

/** -----------------------------------------------------------------------------
    protected
    constructor
    -----------------------------------------------------------------------------
*/
HRAImageView::HRAImageView(const HFCPtr<HRARaster>& pi_pSource)
    :   HRARaster(pi_pSource->GetCoordSys())
    {
    m_pSource = pi_pSource;

    LinkTo(m_pSource);
    }

/** -----------------------------------------------------------------------------
    protected
    copy constructor
    -----------------------------------------------------------------------------
*/
HRAImageView::HRAImageView(const HRAImageView& pi_rImageView)
    : HRARaster(pi_rImageView)
    {
    m_pSource = pi_rImageView.m_pSource;

    LinkTo(m_pSource);
    }

/** -----------------------------------------------------------------------------
    public
    destructor
    -----------------------------------------------------------------------------
*/
HRAImageView::~HRAImageView()
    {
    if(m_pSource != 0)
        UnlinkFrom(m_pSource);
    }

/** -----------------------------------------------------------------------------
    Check if our source contains pixels of the specified channel.
    -----------------------------------------------------------------------------
*/
bool HRAImageView::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id) const
    {
    return m_pSource->ContainsPixelsWithChannel(pi_Role, pi_Id);
    }

/** -----------------------------------------------------------------------------
    Apply the copy on our source raster.
    -----------------------------------------------------------------------------
*/
void HRAImageView::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    m_pSource->CopyFromLegacy(pi_pSrcRaster);
    }

/** -----------------------------------------------------------------------------
    Apply the copy on our source raster.
    -----------------------------------------------------------------------------
*/
void HRAImageView::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    m_pSource->CopyFromLegacy(pi_pSrcRaster, pi_rOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageView::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options)
    {
    return m_pSource->CopyFrom(srcRaster, options);
    }


/** -----------------------------------------------------------------------------
Clear
-----------------------------------------------------------------------------
*/
void HRAImageView::Clear()
    {
    m_pSource->Clear();
    }

/** -----------------------------------------------------------------------------
Clear
-----------------------------------------------------------------------------
*/
void HRAImageView::Clear(const HRAClearOptions& pi_rOptions)
    {
    m_pSource->Clear(pi_rOptions);
    }

/** -----------------------------------------------------------------------------
Set volatile layers
-----------------------------------------------------------------------------
*/
void HRAImageView::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    m_pSource->SetContext(pi_rpContext);
    }

/** -----------------------------------------------------------------------------
Get volatile layers
-----------------------------------------------------------------------------
*/
HFCPtr<HMDContext> HRAImageView::GetContext()
    {
    return m_pSource->GetContext();
    }

/** -----------------------------------------------------------------------------
Invalidate raster
-----------------------------------------------------------------------------
*/
void HRAImageView::InvalidateRaster()
    {
    m_pSource->InvalidateRaster();
    }

/** -----------------------------------------------------------------------------
    Create the editor on the source.
    -----------------------------------------------------------------------------
*/
HRARasterEditor* HRAImageView::CreateEditor   (HFCAccessMode pi_Mode)
    {
    return m_pSource->CreateEditor(pi_Mode);
    }

HRARasterEditor* HRAImageView::CreateEditor   (const HVEShape& pi_rShape,
                                               HFCAccessMode   pi_Mode)
    {
    return m_pSource->CreateEditor(pi_rShape, pi_Mode);
    }


HRARasterEditor* HRAImageView::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return m_pSource->CreateEditorUnShaped(pi_Mode);
    }


/** -----------------------------------------------------------------------------
    Create an iterator on the source raster.
    -----------------------------------------------------------------------------
*/
HRARasterIterator* HRAImageView::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    HASSERT(m_pSource != 0);

    // Iterate on source directly.
    return m_pSource->CreateIterator(pi_rOptions);
    }


/** -----------------------------------------------------------------------------
    Retrieve the source average pixel size.
    -----------------------------------------------------------------------------
*/
HGF2DExtent HRAImageView::GetAveragePixelSize () const
    {
    return m_pSource->GetAveragePixelSize();
    }

/** -----------------------------------------------------------------------------
    Retrieve the source pixel size range.
    -----------------------------------------------------------------------------
*/
void HRAImageView::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    m_pSource->GetPixelSizeRange(po_rMinimum, po_rMaximum);
    }

/** -----------------------------------------------------------------------------
    Retrieve the source effective shape.
    -----------------------------------------------------------------------------
*/
HFCPtr<HVEShape> HRAImageView::GetEffectiveShape () const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource->GetEffectiveShape();
    }

/** -----------------------------------------------------------------------------
    Retrieve the source extent
    -----------------------------------------------------------------------------
*/
HGF2DExtent HRAImageView::GetExtent() const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource->GetExtent();
    }

/** -----------------------------------------------------------------------------
    Retrieve the source shape.
    -----------------------------------------------------------------------------
*/
const HVEShape& HRAImageView::GetShape() const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource->GetShape();
    }

/** -----------------------------------------------------------------------------
    The SetShape operation is disabled by default.
    -----------------------------------------------------------------------------
*/
void HRAImageView::SetShape(const HVEShape& pi_rShape)
    {
    }

/** -----------------------------------------------------------------------------
    Retrieve the source pixel type.
    -----------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HRAImageView::GetPixelType() const
    {
    return m_pSource->GetPixelType();
    }


/** -----------------------------------------------------------------------------
    Retrieve the source raster
    -----------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HRAImageView::GetSource() const
    {
    return m_pSource;
    }

/** -----------------------------------------------------------------------------
    Test on the source
    -----------------------------------------------------------------------------
*/
bool HRAImageView::HasSinglePixelType() const
    {
    return m_pSource->HasSinglePixelType();
    }

/** -----------------------------------------------------------------------------
    We really don't want the CoordSys to be changed, so we override
    SetCoordSys instead of SetCoordSysImplementation.
    -----------------------------------------------------------------------------
*/
void HRAImageView::SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    }

/** -----------------------------------------------------------------------------
    Disabled by default
    -----------------------------------------------------------------------------
*/
void HRAImageView::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    }


/** -----------------------------------------------------------------------------
    Disabled by default
    -----------------------------------------------------------------------------
*/
void HRAImageView::Rotate(double               pi_Angle,
                          const HGF2DLocation& pi_rOrigin)
    {
    }


/** -----------------------------------------------------------------------------
    Disabled by default
    -----------------------------------------------------------------------------
*/
void HRAImageView::Scale(double pi_ScaleFactorX,
                         double pi_ScaleFactorY,
                         const HGF2DLocation& pi_rOrigin)
    {
    }


/** -----------------------------------------------------------------------------
    Test if the source supports the LookAhead mechanism
    -----------------------------------------------------------------------------
*/
bool HRAImageView::HasLookAhead() const
    {
    HPRECONDITION(m_pSource != 0);

    return (m_pSource->HasLookAhead());
    }


/** -----------------------------------------------------------------------------
    Pass the lookahead request to the source raster
    -----------------------------------------------------------------------------
*/
void HRAImageView::SetLookAhead(const HVEShape& pi_rShape,
                                uint32_t        pi_ConsumerID,
                                bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    m_pSource->SetLookAhead(pi_rShape, pi_ConsumerID, pi_Async);
    }



