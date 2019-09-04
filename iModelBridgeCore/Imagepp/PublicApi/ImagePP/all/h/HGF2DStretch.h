//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DStretch
//-----------------------------------------------------------------------------
// Description of 2D stretch transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a coordinate transformation model of type stretch.
    An object of this class expresses the bi-directional stretch transformations
    to apply to coordinates. A stretch transformation permits to perform a
    translation and scalings. The scaling is performed first, then the translation
    is applied. Coordinates can be transformed directly or inverse.
    -----------------------------------------------------------------------------
*/
class HGF2DStretch : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DStretchId, HGF2DTransfoModel)


public:

    // Primary methods
    IMAGEPP_EXPORT                       HGF2DStretch();
    IMAGEPP_EXPORT                       HGF2DStretch (const HGF2DDisplacement& pi_rTranslation,
                                                 double                  pi_ScalingX,
                                                 double                  pi_ScalingY);
    IMAGEPP_EXPORT                       HGF2DStretch(const HGF2DStretch& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HGF2DStretch();
    IMAGEPP_EXPORT HGF2DStretch&         operator= (const HGF2DStretch& pi_rObj);


    // Model definition
    IMAGEPP_EXPORT void                 SetTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT HGF2DDisplacement    GetTranslation() const;
    IMAGEPP_EXPORT void                 SetXScaling(double pi_Scale);
    IMAGEPP_EXPORT void                 SetYScaling(double pi_Scale);
    IMAGEPP_EXPORT double               GetXScaling() const;
    IMAGEPP_EXPORT double               GetYScaling() const;

    // High level model definition
    IMAGEPP_EXPORT void                 AddTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT void                 AddAnisotropicScaling(double pi_ScalingFactorX,
                                                               double pi_ScalingFactorY,
                                                               double pi_XCenter = 0.0,
                                                               double pi_YCenter = 0.0);
    IMAGEPP_EXPORT void                 AddIsotropicScaling(double pi_ScalingFactor,
                                                            double pi_XCenter = 0.0,
                                                            double pi_YCenter = 0.0);

protected:
    // Conversion interface
    virtual bool                        _IsConvertDirectThreadSafe()  const override;
    virtual bool                        _IsConvertInverseThreadSafe() const override;

    virtual StatusInt     _ConvertDirect (double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertDirect (double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_aXOut,
                                                       double*   po_aYOut) const override;

    virtual StatusInt     _ConvertDirect (double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt     _ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    virtual StatusInt     _ConvertInverse (double*   pio_pXInOut,
                                                         double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertInverse (double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t    pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_aXOut,
                                                        double*   po_aYOut) const override;

    virtual StatusInt      _ConvertInverse (double    pi_XIn,
                                                         double    pi_YIn,
                                                         double*   po_pXOut,
                                                         double*   po_pYOut) const override;

   virtual StatusInt       _ConvertInverse(size_t    pi_NumLoc,
                                                        double*   pio_aXInOut,
                                                        double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                        _IsIdentity () const override;
    virtual bool                        _IsStretchable (double pi_AngleTolerance) const override;
    virtual void                        _GetStretchParams (double*           po_pScaleFactorX,
                                                          double*           po_pScaleFactorY,
                                                          HGF2DDisplacement* po_pDisplacement) const override;


    virtual HGF2DTransfoModel*          _Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>   _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;
    virtual bool                        _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3>             _GetMatrix() const override;
    
    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

    

    // Geometric properties
    virtual bool                        _PreservesLinearity() const override;
    virtual bool                        _PreservesParallelism() const override;
    virtual bool                        _PreservesShape() const override;
    virtual bool                        _PreservesDirection() const override;

    // Operations
    virtual void                        _Reverse () override;

    virtual void                        _Prepare () override;
    virtual HFCPtr<HGF2DTransfoModel>   _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;
private:

    // Primary attributes
    double m_XTranslation;
    double m_YTranslation;

    double m_ScaleX;
    double m_ScaleY;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    double m_PreparedDirectA1;
    double m_PreparedInverseA1;
    double m_PreparedDirectA2;
    double m_PreparedInverseA2;

    // Private methods
    void            Copy (const HGF2DStretch& pi_rObj);

    };

END_IMAGEPP_NAMESPACE

#include "HGF2DStretch.hpp"
