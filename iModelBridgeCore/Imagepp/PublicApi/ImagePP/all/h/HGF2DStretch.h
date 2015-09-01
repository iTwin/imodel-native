//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DStretch.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    HGF2DStretch&                       operator= (const HGF2DStretch& pi_rObj);


    // Conversion interface
    virtual bool                        IsConvertDirectThreadSafe()  const override;
    virtual bool                        IsConvertInverseThreadSafe() const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect (double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect (double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_aXOut,
                                                       double*   po_aYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect (double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse (double*   pio_pXInOut,
                                                         double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse (double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t    pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_aXOut,
                                                        double*   po_aYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt      ConvertInverse (double    pi_XIn,
                                                         double    pi_YIn,
                                                         double*   po_pXOut,
                                                         double*   po_pYOut) const override;

   IMAGEPP_EXPORT virtual StatusInt       ConvertInverse(size_t    pi_NumLoc,
                                                        double*   pio_aXInOut,
                                                        double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                        IsIdentity () const;
    virtual bool                        IsStretchable (double pi_AngleTolerance = 0) const;
    virtual void                        GetStretchParams (double*           po_pScaleFactorX,
                                                          double*           po_pScaleFactorY,
                                                          HGF2DDisplacement* po_pDisplacement) const;


    virtual HGF2DTransfoModel*          Clone () const override;

    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>
                                        ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;
    virtual bool                        CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>             GetMatrix() const;
    virtual HFCMatrix<3, 3>&            GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    virtual HFCPtr<HGF2DTransfoModel>   CreateSimplifiedModel() const;

    // Model definition
    void                                SetTranslation (const HGF2DDisplacement& pi_rTranslation);
    HGF2DDisplacement                   GetTranslation () const;
    IMAGEPP_EXPORT void                  SetXScaling  (double pi_Scale);
    IMAGEPP_EXPORT void                  SetYScaling  (double pi_Scale);
    double                              GetXScaling  () const;
    double                              GetYScaling  () const;

    // High level model definition
    IMAGEPP_EXPORT void                  AddTranslation (const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT void                  AddAnisotropicScaling(double pi_ScalingFactorX,
                                                              double pi_ScalingFactorY,
                                                              double pi_XCenter=0.0,
                                                              double pi_YCenter=0.0);
    void                                AddIsotropicScaling(double pi_ScalingFactor,
                                                            double pi_XCenter=0.0,
                                                            double pi_YCenter=0.0);

    // Geometric properties
    virtual bool                        PreservesLinearity() const;
    virtual bool                        PreservesParallelism() const;
    virtual bool                        PreservesShape() const;
    virtual bool                        PreservesDirection() const;

    // Operations
    virtual void                        Reverse ();


protected:

    virtual void                        Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>   ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
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
