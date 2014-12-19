//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DStretch.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DStretch
//-----------------------------------------------------------------------------
// Description of 2D stretch transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"

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
    HDECLARE_CLASS_ID(1043, HGF2DTransfoModel)


public:

    // Primary methods
    _HDLLg                 HGF2DStretch();
    _HDLLg                 HGF2DStretch (const HGF2DDisplacement& pi_rTranslation,
                                         double                  pi_ScalingX,
                                         double                  pi_ScalingY);
    _HDLLg                 HGF2DStretch(const HGF2DStretch& pi_rObj);
    _HDLLg virtual         ~HGF2DStretch();
    HGF2DStretch&
    operator= (const HGF2DStretch& pi_rObj);


    // Conversion interface
    _HDLLg virtual void    ConvertDirect (double*   pio_pXInOut,
                                          double*   pio_pYInOut) const;

    _HDLLg virtual void    ConvertDirect (double    pi_YIn,
                                          double    pi_XInStart,
                                          size_t     pi_NumLoc,
                                          double    pi_XInStep,
                                          double*   po_aXOut,
                                          double*   po_aYOut) const;

    _HDLLg virtual void    ConvertDirect (double    pi_XIn,
                                          double    pi_YIn,
                                          double*   po_pXOut,
                                          double*   po_pYOut) const;

    _HDLLg virtual void    ConvertInverse (double*   pio_pXInOut,
                                           double*   pio_pYInOut) const;

    _HDLLg virtual void    ConvertInverse (double    pi_YIn,
                                           double    pi_XInStart,
                                           size_t     pi_NumLoc,
                                           double    pi_XInStep,
                                           double*   po_aXOut,
                                           double*   po_aYOut) const;

    _HDLLg virtual void    ConvertInverse (double    pi_XIn,
                                           double    pi_YIn,
                                           double*   po_pXOut,
                                           double*   po_pYOut) const;



    // Miscalenious
    virtual bool   IsIdentity () const;
    virtual bool   IsStretchable (double pi_AngleTolerance = 0) const;
    virtual void    GetStretchParams (double*           po_pScaleFactorX,
                                      double*           po_pScaleFactorY,
                                      HGF2DDisplacement* po_pDisplacement) const;


    virtual HGF2DTransfoModel* Clone () const override;

    _HDLLg virtual HFCPtr<HGF2DTransfoModel>
    ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;
    virtual bool   CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>
    GetMatrix() const;
    virtual HFCMatrix<3, 3>&
    GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    virtual HFCPtr<HGF2DTransfoModel>
    CreateSimplifiedModel() const;

    // Model definition
    void            SetTranslation (const HGF2DDisplacement& pi_rTranslation);
    HGF2DDisplacement
    GetTranslation () const;
    _HDLLg void            SetXScaling  (double pi_Scale);
    _HDLLg void            SetYScaling  (double pi_Scale);
    double         GetXScaling  () const;
    double         GetYScaling  () const;

    // High level model definition
    _HDLLg void            AddTranslation (const HGF2DDisplacement& pi_rTranslation);
    _HDLLg void            AddAnisotropicScaling(double pi_ScalingFactorX,
                                                 double pi_ScalingFactorY,
                                                 double pi_XCenter=0.0,
                                                 double pi_YCenter=0.0);
    void            AddIsotropicScaling(double pi_ScalingFactor,
                                        double pi_XCenter=0.0,
                                        double pi_YCenter=0.0);

    // Geometric properties
    virtual bool   PreservesLinearity() const;
    virtual bool   PreservesParallelism() const;
    virtual bool   PreservesShape() const;
    virtual bool   PreservesDirection() const;

    // Operations
    virtual void    Reverse ();


protected:

    virtual void    Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
private:

    // Primary attributes
    double m_XTranslation;
    double m_YTranslation;

    double     m_ScaleX;
    double     m_ScaleY;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    double     m_PreparedDirectA1;
    double     m_PreparedInverseA1;
    double     m_PreparedDirectA2;
    double     m_PreparedInverseA2;

    // Private methods
    void            Copy (const HGF2DStretch& pi_rObj);

    };

#include "HGF2DStretch.hpp"
