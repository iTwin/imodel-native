//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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


protected:
    // Conversion interface
    virtual bool                    _IsConvertDirectThreadSafe()  const override;
    virtual bool                    _IsConvertInverseThreadSafe() const override;

    virtual StatusInt               _ConvertDirect(double*   pio_pXInOut,
                                                double*   pio_pYInOut) const override;

    virtual StatusInt               _ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t    pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_aXOut,
                                                  double*   po_aYOut) const override;

    virtual StatusInt               _ConvertDirect(double    pi_XIn,
                                                  double    pi_YIn,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

    virtual StatusInt               _ConvertDirect(size_t    pi_NumLoc,
                                                  double*   pio_aXInOut,
                                                  double*   pio_aYInOut) const override;

    virtual StatusInt               _ConvertInverse(double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

    virtual StatusInt               _ConvertInverse(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t     pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_aXOut,
                                                   double*   po_aYOut) const override;

    virtual StatusInt               _ConvertInverse(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               _ConvertInverse(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                    _IsIdentity() const override;
    virtual bool                    _IsStretchable(double pi_AngleTolerance) const override;
    virtual void                    _GetStretchParams(double*  po_pScaleFactorX,
                                                     double*  po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel*      _Clone() const override;

    virtual HFCPtr<HGF2DTransfoModel> _ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const override;
    virtual bool                    _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>         _GetMatrix() const override;
    
    // Geometric properties
    virtual bool                    _PreservesLinearity() const override;
    virtual bool                    _PreservesParallelism() const override;
    virtual bool                    _PreservesShape() const override;
    virtual bool                    _PreservesDirection() const override;

    // Operations
    virtual void                    _Reverse () override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override { return nullptr; };

    virtual void                    _Prepare () override;

private:

    };
END_IMAGEPP_NAMESPACE
