//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageView.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"

BEGIN_IMAGEPP_NAMESPACE

class HRAClearOptions;

/** -----------------------------------------------------------------------------
    This class represents a view of another raster. It is the base for
    logical (non-destructive) transformations on rasters.

    By default, all method implementations work directly with the source
    object, as if there was no wrapper class. Some operations as logically
    disabled instead of being applied to the source.
    -----------------------------------------------------------------------------
*/
class HRAImageView : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAImageViewId)

public:

    // Primary methods

    virtual         ~HRAImageView();

    // Overriden methods

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    virtual HRARasterEditor*
    CreateEditor   (HFCAccessMode pi_Mode);

    virtual HRARasterEditor*
    CreateEditor   (const HVEShape& pi_rShape,
                    HFCAccessMode   pi_Mode);

    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual HRARasterIterator*
    CreateIterator (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual HGF2DExtent
    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual HFCPtr<HVEShape>
    GetEffectiveShape () const;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;

    virtual bool   HasSinglePixelType() const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);

    virtual void    Rotate(double               pi_Angle,
                           const HGF2DLocation& pi_rOrigin);

    virtual void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin);

    virtual void    SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);

    virtual HGF2DExtent
    GetExtent() const;

    virtual const HVEShape&
    GetShape    () const;
    virtual void    SetShape    (const HVEShape& pi_rShape);

    // Source
    IMAGEPP_EXPORT const HFCPtr<HRARaster>&
    GetSource() const;

    // LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    //Context Methods
    virtual void               SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext> GetContext();

    virtual void               InvalidateRaster();

protected:

    // Primary methods
    HRAImageView();
    HRAImageView(const HFCPtr<HRARaster>& pi_pSource);
    HRAImageView(const HRAImageView& pi_rImageView);

    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) override;

private:
    
    HFCPtr<HRARaster> m_pSource;
    };

END_IMAGEPP_NAMESPACE