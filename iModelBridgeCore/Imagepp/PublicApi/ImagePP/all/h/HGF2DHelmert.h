//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DHelmert.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DHelmert
//-----------------------------------------------------------------------------
// Description of 2D Helmert transformation model.
//-----------------------------------------------------------------------------

#pragma once


#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
#define HELMERT_MIN_NB_TIE_PTS 2

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a coordinate transformation model of type
    Helmert. An object of this class expresses the bi-directional
    Helmert transformations to apply to coordinates. A Helmert
    transformation performs, a rotation and a scaling (in either order) then
    afterwards applies a translation. Coordinates can therefore be transformed
    directly or inverse.
    -----------------------------------------------------------------------------
*/
class HGF2DHelmert : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DHelmertId, HGF2DTransfoModel)
public:

    // Primary methods
                                    HGF2DHelmert();
                                    HGF2DHelmert (const HGF2DDisplacement& pi_rTranslation,
                                                  double                   pi_rRotation);
                                    HGF2DHelmert (const HGF2DHelmert& pi_rObj);
                                    HGF2DHelmert(unsigned short pi_NumberOfPoints,
                                                 const double* const pi_pTiePoints);
    virtual                         ~HGF2DHelmert();
    HGF2DHelmert&                   operator= (const HGF2DHelmert& pi_rObj);


    // Conversion interface
    virtual bool                    IsConvertDirectThreadSafe()  const override;
    virtual bool                    IsConvertInverseThreadSafe() const override;

    virtual StatusInt               ConvertDirect (double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

    virtual StatusInt               ConvertDirect (double    pi_YIn,
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

    // Miscellaneous
    virtual bool                    IsIdentity() const;
    virtual bool                    IsStretchable(double pi_AngleTolerance = 0) const;
    virtual void                    GetStretchParams(double*            po_pScaleFactorX,
                                                     double*            po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const;
    static uint32_t                 GetMinimumNumberOfTiePoints();

    virtual HGF2DTransfoModel*      Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;
    virtual bool                    CanBeRepresentedByAMatrix() const;

    virtual HFCMatrix<3, 3>         GetMatrix() const;
    virtual HFCMatrix<3, 3>&        GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    virtual HFCPtr<HGF2DTransfoModel>    
                                    CreateSimplifiedModel() const;

    // Model definition
    void                            SetTranslation (const HGF2DDisplacement& pi_rTranslation);
    HGF2DDisplacement               GetTranslation () const;
    void                            SetRotation (double pi_Angle);
    double                          GetRotation () const;

    // High level model definition
    void                            AddTranslation (const HGF2DDisplacement& pi_rTranslation);
    void                            AddRotation(double pi_Angle,
                                                double pi_XCenter=0.0,
                                                double pi_YCenter=0.0);

    void                            SetByMatrixParameters(double pi_A0,
                                                          double pi_A1,
                                                          double pi_A2,
                                                          double pi_B0,
                                                          double pi_B1,
                                                          double pi_B2);

    // Geometric properties
    virtual bool                    PreservesLinearity() const;
    virtual bool                    PreservesParallelism() const;
    virtual bool                    PreservesShape() const;
    virtual bool                    PreservesDirection() const;

    // Operations
    virtual void                    Reverse();

protected:

    virtual void                    Prepare();
    virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
private:

    // Primary attributes

    // HGF2DDisplacement
    double  m_XTranslation;
    double  m_YTranslation;

    double  m_Rotation;

    // Acceleration attributes
    double m_XTranslationPrime;
    double m_YTranslationPrime;
    double m_XTranslationInverse;
    double m_YTranslationInverse;
    double m_XTranslationInversePrime;
    double m_YTranslationInversePrime;

    double m_PreparedDirectA1;
    double m_PreparedDirectB1;
    double m_PreparedInverseA1;
    double m_PreparedInverseB1;
    double m_PreparedDirectA2;
    double m_PreparedDirectB2;
    double m_PreparedInverseA2;
    double m_PreparedInverseB2;


    // Private methods
    void            Copy (const HGF2DHelmert& pi_rObj);

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DHelmert.hpp"



              
