//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DIdentity.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DIdentity
//-----------------------------------------------------------------------------
// Description of 2D transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert
    This class encapsulates a 2D-transformation model. An object of this class
    expresses the bidirectionnal transformations to apply to coordinates.
    Coordinates can be transformed directly or inverse.
    The inverse or direct labels can be seen as simple labels to two bi-directional
    doors that lead to a black box, which is the transformation model, itself.

    To each input/output channel is assigned distance units. If the units are not
    the same, unit conversion will be performed.

    The present class implements an identity model. Even the model being described
    is neutral, unit conversion if they are required will be performed.

    -----------------------------------------------------------------------------
*/
class HGF2DIdentity : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DIdentityId, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT                   HGF2DIdentity();
    IMAGEPP_EXPORT                   HGF2DIdentity(const HGF2DIdentity& pi_rObj);
    IMAGEPP_EXPORT virtual           ~HGF2DIdentity();
    HGF2DIdentity&                  operator=(const HGF2DIdentity& pi_rObj);

    // Conversion interface
    virtual bool                    IsConvertDirectThreadSafe()  const override;
    virtual bool                    IsConvertInverseThreadSafe() const override;

    virtual StatusInt               ConvertDirect(double*   pio_pXInOut,
                                                double*   pio_pYInOut) const;

    virtual StatusInt               ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t    pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_aXOut,
                                                  double*   po_aYOut) const override;

    virtual StatusInt               ConvertDirect(double    pi_XIn,
                                                  double    pi_YIn,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

    virtual StatusInt               ConvertDirect(size_t    pi_NumLoc,
                                                  double*   pio_aXInOut,
                                                  double*   pio_aYInOut) const override;

    virtual StatusInt               ConvertInverse(double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

    virtual StatusInt               ConvertInverse(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t     pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_aXOut,
                                                   double*   po_aYOut) const override;

    virtual StatusInt               ConvertInverse(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               ConvertInverse(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                    IsIdentity() const;
    virtual bool                    IsStretchable(double pi_AngleTolerance = 0) const;
    virtual void                    GetStretchParams(double*  po_pScaleFactorX,
                                                     double*  po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*      Clone() const override;

    virtual HFCPtr<HGF2DTransfoModel>    
                                    ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;
    virtual bool                    CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>         GetMatrix() const;
    virtual HFCMatrix<3, 3>&        GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;


    // Geometric properties
    virtual bool                    PreservesLinearity() const;
    virtual bool                    PreservesParallelism() const;
    virtual bool                    PreservesShape() const;
    virtual bool                    PreservesDirection() const;


    // Operations
    virtual void                    Reverse ();


protected:

    virtual void                    Prepare ();

private:

    };
END_IMAGEPP_NAMESPACE