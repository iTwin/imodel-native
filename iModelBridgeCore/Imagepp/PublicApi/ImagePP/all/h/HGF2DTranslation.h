//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTranslation.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DTranslation
//-----------------------------------------------------------------------------
// Description of 2D similitude transformation model.
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class encapsulates a coordinate transformation model of type translation.
    An object of this class expresses the bi-directional translation transformations
    to apply to coordinates. A translation transformation performs a translation.
    Coordinates can be transformed directly or inverse.


    -----------------------------------------------------------------------------
*/
class HGF2DTranslation : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DTranslationId, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT                   HGF2DTranslation();
    IMAGEPP_EXPORT                   HGF2DTranslation(const HGF2DDisplacement& pi_rTranslation);
    IMAGEPP_EXPORT                   HGF2DTranslation(const HGF2DTranslation& pi_rObj);
    IMAGEPP_EXPORT virtual           ~HGF2DTranslation ();
    HGF2DTranslation&
    operator= (const HGF2DTranslation& pi_rObj);


    // Conversion interface
    virtual bool                    IsConvertDirectThreadSafe() const override;
    virtual bool                    IsConvertInverseThreadSafe() const override;

    virtual StatusInt               ConvertDirect(double*   pio_pXInOut,
                                                  double*   pio_pYInOut) const override;

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
                                                   size_t    pi_NumLoc,
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

    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>    
                                    ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;

    virtual bool                    CanBeRepresentedByAMatrix() const;

    virtual HFCMatrix<3, 3>         GetMatrix() const;
    virtual HFCMatrix<3, 3>&        GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    virtual HFCPtr<HGF2DTransfoModel>    
                                    CreateSimplifiedModel() const;

    // Geometric properties
    virtual bool                    PreservesLinearity() const;
    virtual bool                    PreservesParallelism() const;
    virtual bool                    PreservesShape() const;
    virtual bool                    PreservesDirection() const;

    // Model definition
    void                            SetTranslation (const HGF2DDisplacement& pi_rTranslation);
    HGF2DDisplacement               GetTranslation () const;

    // Operations
    virtual void                    Reverse ();


protected:

    virtual void                    Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
private:

    // Primary attributes
    double m_XTranslation;
    double m_YTranslation;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    bool           m_ConversionRequired;
    double         m_PreparedDirectA1;
    double         m_PreparedInverseA1;
    double         m_PreparedDirectA2;
    double         m_PreparedInverseA2;

    // Private methods
    void            Copy (const HGF2DTranslation& pi_rObj);

    };

END_IMAGEPP_NAMESPACE
#include "hgf2dTranslation.hpp"
