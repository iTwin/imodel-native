//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP/all/h/HGF2DTransfoModelAdapter.h>
#include <ImagePP/all/h/HCPGCoordModel.h>

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

private:

    virtual bool _IsConvertDirectThreadSafe() const override { return false; }
    virtual bool _IsConvertInverseThreadSafe() const override { return false; }
    virtual bool                        _CanBeRepresentedByAMatrix() const  override { return false; }
    virtual HFCMatrix<3, 3>             _GetMatrix() const override;

    virtual bool                        _PreservesLinearity() const override { return false; }
    virtual bool                        _PreservesParallelism() const  override { return false; }
    virtual bool                        _PreservesShape() const  override { return false; }
    virtual bool                        _PreservesDirection() const  override { return false; }

    virtual bool                        _IsStretchable(double pi_AngleTolerance) const  override { return false; }

    virtual void                        _GetStretchParams(double*           po_pScaleFactorX,
                                                         double*           po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const override;
    // HGF2DTransfoModel interface

    // Conversion interface
    virtual StatusInt     _ConvertDirect(double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertDirect(double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t     pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt     _ConvertDirect(double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt     _ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    virtual StatusInt     _ConvertInverse(double*   pio_pXInOut,
                                                        double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertInverse(double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t     pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    virtual StatusInt     _ConvertInverse(double    pi_XIn,
                                                        double    pi_YIn,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    virtual StatusInt     _ConvertInverse(size_t    pi_NumLoc,
                                                        double*   pio_aXInOut,
                                                        double*   pio_aYInOut) const override;

    virtual HGF2DTransfoModel*          _Clone() const override;


    virtual void     _Reverse() override;
    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override { return nullptr; };

    // Private methods
    void                        Copy (const HCPGCoordContiguousModelAdapter& pi_rObj);

    double m_MinXLongDirect;
    double m_MinXLongInverse;
    };

END_IMAGEPP_NAMESPACE
