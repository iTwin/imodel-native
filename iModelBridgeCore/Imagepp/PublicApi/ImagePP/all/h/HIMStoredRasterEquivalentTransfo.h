//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStoredRasterEquivalentTransfo.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


#include "HRARaster.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    This is a utility object. Its sole purpose is to decapsulate rasters in
    order to obtain the effective transformation model, as if the input
    raster was a simple HRAStoredRaster derivative. This is only possible if
    the following conditions are met:
     - There is an HRAStoredRaster derivative at the bottom of the raster chain.
     - The raster chain is of the following form:
         [R [ ... R]] o [RTR [... RTR]] o [R [... R]] o SR, where
         R = An HRARaster derivative that does not change the geometry
         RTR = an HRAReferenceToRaster object.
         SR = An HRAStoredRaster derivative.

    What this effectively means is that if there are logical rasters on top of
    the stored raster instance, they must be placed in the specified order. The
    HRAReferenceToRaster instances must all be grouped together, and these can
    be preceded and/or followed by raster instances that don't change the
    geometry.

    These HRARaster derivative classes are currently handled:
     - HRAImageView : No geometry change (equivalent to R in previous model)
     - HRAStoredRaster : SR in previous model
     - HRAReferenceToRaster : RTR in previous model.

    If there are any other HRARaster derivatives involved, this object will not
    be able to extract the transformation model. The EquivalentTransfoCanBeComputed()
    method must be used to verify this before calling any other method.

    The UsesLogicalTransformations() method checks if HRAReferenceToRaster
    instances are used. If there are none, the logical and physical coordinate
    systems can be extracted and used as if the raster was indeed a derivative
    of the HRAStoredRaster class. Otherwise, the shape transformation methods
    must be used.

    If new raster classes are created (that don't change the geometry and that
    aren't HRAImageView children), they must be taken into account in this
    utility object.
    -----------------------------------------------------------------------------
*/
class HIMStoredRasterEquivalentTransfo
    {
public:

    //:> Primary methods
    IMAGEPP_EXPORT HIMStoredRasterEquivalentTransfo(const HFCPtr<HRARaster>& pi_pRaster);

    IMAGEPP_EXPORT ~HIMStoredRasterEquivalentTransfo();


    IMAGEPP_EXPORT bool    EquivalentTransfoCanBeComputed() const;

    bool           UsesLogicalTransformations() const;

    HFCPtr<HGF2DTransfoModel>
    GetEquivalentTransfoModel() const;

    HFCPtr<HGF2DCoordSys>
    GetLogicalCoordSys() const;
    HFCPtr<HGF2DCoordSys>
    GetPhysicalCoordSys() const;

    IMAGEPP_EXPORT void     TransformLogicalShapeIntoPhysical(HVEShape& pio_rShape) const;
    void            TransformPhysicalShapeIntoLogical(HVEShape& pio_rShape) const;

protected:


private:

    ////////////////
    // Methods
    ////////////////

    //:> Disabled
    HIMStoredRasterEquivalentTransfo(const HIMStoredRasterEquivalentTransfo& pi_rObj);
    HIMStoredRasterEquivalentTransfo&
    operator=(const HIMStoredRasterEquivalentTransfo& pi_rObj);

    bool           CanDecapsulate(const HFCPtr<HRARaster>& pi_rpRaster) const;
    bool           IsAReference(const HFCPtr<HRARaster>& pi_rpRaster) const;
    bool           IsAStoredRaster(const HFCPtr<HRARaster>& pi_rpRaster) const;
    HFCPtr<HRARaster>
    Decapsulate(const HFCPtr<HRARaster>& pi_rpRaster) const;


    ////////////////
    // Attributes
    ////////////////

    bool           m_EquivalentIsComputed;
    bool           m_UsesLogicalTransformations;

    // The TransfoModel that is effectively used for this raster
    HFCPtr<HGF2DTransfoModel>
    m_pEquivalentTransfoModel;

    // The logical coordinate system
    HFCPtr<HGF2DCoordSys>
    m_pLogicalCoordSys;

    // The logical system of the source of the innermost Ref2Raster
    HFCPtr<HGF2DCoordSys>
    m_pLogicalCoordSysUnderReferences;

    // The internal physical coordinate system
    HFCPtr<HGF2DCoordSys>
    m_pPhysicalCoordSys;
    };

END_IMAGEPP_NAMESPACE