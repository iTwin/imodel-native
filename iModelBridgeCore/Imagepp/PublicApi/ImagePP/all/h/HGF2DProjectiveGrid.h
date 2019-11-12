//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DProjectiveGrid
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    The purpose of the grid adapter is to linearize a non-linear model in order
    to accelerate the conversion process by providing an approximation of a projective
    over a grid. The grid is positioned at integer number of steps in both
    X and Y directions. Although the purpose is similar to the
    projective adapter, it is more precise since a projective approximation
    is applicable for a small region of step by step area only. The projectives
    are side fitted and since they are based on the approximation of the
    four corner points of each region, there should be no discontinuities
    between adjoining projective areas.
    The projective adapter is valid for any coordinate value. If coordinates to
    convert fall in some area that has not been evaluated yet, the
    model will generate the projective approximation for this area.
    Although many different areas are approximated, only a limited amount
    of projectives can be kept.

    Since the adaptation is an approximation of the transformation model, an error may be
    introduced by adaptation using such a projective adapter. The error can
    be studied using the StudyPrecisionOver() method.

    Study of precision can be performed on the original area or on any other area.

    -----------------------------------------------------------------------------
*/
class HGF2DProjectiveGrid : public HGF2DTransfoModelAdapter
    {
    HDECLARE_CLASS_ID(HGF2DProjectiveId_Grid, HGF2DTransfoModelAdapter)

public:

    // Primary methods
    IMAGEPPTEST_EXPORT     HGF2DProjectiveGrid();

    IMAGEPPTEST_EXPORT     HGF2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                double                   pi_DirectStep,
                                                double                   pi_InverseStep);


                            HGF2DProjectiveGrid(const HGF2DProjectiveGrid& pi_rObj);
    IMAGEPPTEST_EXPORT virtual ~HGF2DProjectiveGrid();
    HGF2DProjectiveGrid&    operator=(const HGF2DProjectiveGrid& pi_rObj);

    double                  GetDirectStep() const;
    double                  GetInverseStep() const;

protected:

    virtual bool _IsConvertDirectThreadSafe() const override {return false;}
    virtual bool _IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt       _ConvertDirect(double*   pio_pXInOut,
                                          double*   pio_pYInOut) const override;

    virtual StatusInt       _ConvertDirect(double    pi_YIn,
                                          double    pi_XInStart,
                                          size_t    pi_NumLoc,
                                          double    pi_XInStep,
                                          double*   po_pXOut,
                                          double*   po_pYOut) const override;

    virtual StatusInt       _ConvertDirect(double    pi_XIn,
                                          double    pi_YIn,
                                          double*   po_pXOut,
                                          double*   po_pYOut) const override;

    virtual StatusInt       _ConvertDirect(size_t    pi_NumLoc,
                                          double*   pio_aXInOut,
                                          double*   pio_aYInOut) const override;

    virtual StatusInt       _ConvertInverse(double*   pio_pXInOut,
                                           double*   pio_pYInOut) const override;

    virtual StatusInt       _ConvertInverse(double    pi_YIn,
                                           double    pi_XInStart,
                                           size_t    pi_NumLoc,
                                           double    pi_XInStep,
                                           double*   po_pXOut,
                                           double*   po_pYOut) const override;

    virtual StatusInt        _ConvertInverse(double    pi_XIn,
                                            double    pi_YIn,
                                            double*   po_pXOut,
                                            double*   po_pYOut) const override;

    virtual StatusInt        _ConvertInverse(size_t    pi_NumLoc,
                                            double*   pio_aXInOut,
                                            double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool            _IsIdentity      () const override;
    virtual bool            _IsStretchable   (double pi_AngleTolerance) const override;
    virtual void            _GetStretchParams(double*            po_pScaleFactorX,
                                             double*            po_pScaleFactorY,
                                             HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel* 
        _Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>
        _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;


    // Model definition
    virtual bool            _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3> _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

    // Geometric properties
    virtual bool            _PreservesLinearity() const override;
    virtual bool            _PreservesParallelism() const override;
    virtual bool            _PreservesShape() const override;
    virtual bool            _PreservesDirection() const override;

    // Operations
    virtual void            _Reverse ()override;

    virtual void            _Prepare ()override;
    virtual HFCPtr<HGF2DTransfoModel>
                            _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;

    HGF2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfo,
                        const HGF2DTransfoModel& pi_rPreTransfo,
                        const HGF2DTransfoModel& pi_rPostTransfo,
                        double                   pi_DirectStep,
                        double                   pi_InverseStep);


private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HGF2DTransfoModelAdapter::ValidateInvariants();
        }
#endif

    // Private methods
    void                        Copy (const HGF2DProjectiveGrid& pi_rObj);
    HFCPtr<HGF2DTransfoModel>   CreateDirectModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const;
    HFCPtr<HGF2DTransfoModel>   CreateInverseModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const;
    const HFCPtr<HGF2DTransfoModel>&    
                                GetDirectModelFromCoordinate(double pi_X, double pi_Y) const;
    const HFCPtr<HGF2DTransfoModel>&    
                                GetInverseModelFromCoordinate(double pi_X, double pi_Y) const;
    void                        ClearDirectModels() const;
    void                        ClearInverseModels() const;

    // Primary attributes

    HFCPtr<HGF2DTransfoModel> m_pPreTransfoModel;
    HFCPtr<HGF2DTransfoModel> m_pPostTransfoModel;

    double                   m_DirectStep;
    double                   m_InverseStep;


    // Acceleration attributes
    uint32_t                            m_NumberOfRows;
    uint32_t                            m_NumberOfColumns;
    mutable bool                        m_DirectModelsPresent;
    mutable HFCPtr<HGF2DTransfoModel>*  m_pArrayOfDirectModels;
    mutable bool                        m_InverseModelsPresent;
    mutable HFCPtr<HGF2DTransfoModel>*  m_pArrayOfInverseModels;
    mutable HGF2DLiteExtent             m_CurrentDirectExtent;
    mutable HGF2DLiteExtent             m_CurrentInverseExtent;
    };
END_IMAGEPP_NAMESPACE
#include "HGF2DProjectiveGrid.hpp"
