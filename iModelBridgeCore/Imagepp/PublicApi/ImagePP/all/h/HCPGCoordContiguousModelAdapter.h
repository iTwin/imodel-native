//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordContiguousModelAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>
#include <Imagepp/all/h/HCPGCoordModel.h>

BEGIN_IMAGEPP_NAMESPACE

// ----------------------------------------------------------------------------
//  HCPGCoordContiguousModelAdapter
// ----------------------------------------------------------------------------
class HCPGCoordContiguousModelAdapter : public HGF2DTransfoModelAdapter
    {

public:
    // Primary methods
    IMAGEPP_EXPORT                       HCPGCoordContiguousModelAdapter(const HCPGCoordModel&  pi_ExactTransfoModel,
                                                                         double pi_MinXLongDirect, double pi_MinXLongInverse);


    IMAGEPP_EXPORT                       HCPGCoordContiguousModelAdapter(const HCPGCoordContiguousModelAdapter& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HCPGCoordContiguousModelAdapter();
    IMAGEPP_EXPORT HCPGCoordContiguousModelAdapter&       operator=(const HCPGCoordContiguousModelAdapter& pi_rObj);

    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}
    virtual bool                        CanBeRepresentedByAMatrix() const {return false;}
    virtual HFCMatrix<3, 3>             GetMatrix() const;

    virtual bool                        PreservesLinearity() const {return false;}
    virtual bool                        PreservesParallelism() const {return false;}
    virtual bool                        PreservesShape() const {return false;}
    virtual bool                        PreservesDirection() const {return false;}

    virtual bool                        IsStretchable   (double pi_AngleTolerance = 0) const {return false;}

    virtual void                        GetStretchParams(double*           po_pScaleFactorX,
                                                         double*           po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const;
    // HGF2DTransfoModel interface

    // Conversion interface
    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double*   pio_pXInOut,
                                                      double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double    pi_YIn,
                                                      double    pi_XInStart,
                                                      size_t     pi_NumLoc,
                                                      double    pi_XInStep,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double    pi_XIn,
                                                      double    pi_YIn,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double*   pio_pXInOut,
                                                        double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t     pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double    pi_XIn,
                                                        double    pi_YIn,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    virtual HGF2DTransfoModel*          Clone () const override;


    IMAGEPP_EXPORT virtual void     Reverse() override;

private:
    // Private methods
    void                        Copy (const HCPGCoordContiguousModelAdapter& pi_rObj);

    double m_MinXLongDirect;
    double m_MinXLongInverse;
    };

END_IMAGEPP_NAMESPACE
