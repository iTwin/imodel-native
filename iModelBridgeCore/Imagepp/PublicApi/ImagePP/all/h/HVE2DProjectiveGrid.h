//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DProjectiveGrid.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DProjectiveGrid
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DQuadrilaterMesh;


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
class HVE2DProjectiveGrid : public HGF2DTransfoModelAdapter
    {
    HDECLARE_CLASS_ID(HVE2DProjectiveGridId, HGF2DTransfoModelAdapter)

public:

    // Primary methods
                                HVE2DProjectiveGrid();

                                HVE2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                    double                   pi_DirectStep,
                                                    double                   pi_InverseStep);


                                HVE2DProjectiveGrid(const HVE2DProjectiveGrid& pi_rObj);
    virtual                     ~HVE2DProjectiveGrid();
    HVE2DProjectiveGrid&        operator=(const HVE2DProjectiveGrid& pi_rObj);

    double                      GetDirectStep() const;
    double                      GetInverseStep() const;

    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt           ConvertDirect(double*   pio_pXInOut,
                                              double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertDirect(double    pi_YIn,
                                              double    pi_XInStart,
                                              size_t    pi_NumLoc,
                                              double    pi_XInStep,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;

    virtual StatusInt           ConvertDirect(double    pi_XIn,
                                              double    pi_YIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;

    virtual StatusInt           ConvertDirect(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const override;

    virtual StatusInt           ConvertInverse(double*   pio_pXInOut,
                                               double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertInverse(double    pi_YIn,
                                               double    pi_XInStart,
                                               size_t    pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           ConvertInverse(double    pi_XIn,
                                               double    pi_YIn,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           ConvertInverse(size_t    pi_NumLoc,
                                               double*   pio_aXInOut,
                                               double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                IsIdentity      () const;
    virtual bool                IsStretchable   (double pi_AngleTolerance = 0) const;
    virtual void                GetStretchParams(double*           po_pScaleFactorX,
                                                 double*           po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*  Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


    // Model definition
    virtual bool                CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>     GetMatrix() const;

    virtual HFCPtr<HGF2DTransfoModel>
                                CreateSimplifiedModel() const;

    // Geometric properties
    virtual bool                PreservesLinearity() const;
    virtual bool                PreservesParallelism() const;
    virtual bool                PreservesShape() const;
    virtual bool                PreservesDirection() const;

    // Operations
    virtual void                Reverse ();

protected:

    virtual void                Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;

    HVE2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfo,
                        const HGF2DTransfoModel& pi_rPreTransfo,
                        const HGF2DTransfoModel& pi_rPostTransfo,
                        double                  pi_DirectStep,
                        double                  pi_InverseStep);


private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HGF2DTransfoModelAdapter::ValidateInvariants();
        }
#endif

    // Private methods
    void                                Copy (const HVE2DProjectiveGrid& pi_rObj);
    HFCPtr<HGF2DTransfoModel>           CreateDirectModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const;
    HFCPtr<HGF2DTransfoModel>           CreateInverseModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const;
    const HFCPtr<HGF2DTransfoModel>&    GetDirectModelFromCoordinate(double pi_X, double pi_Y) const;
    const HFCPtr<HGF2DTransfoModel>&    GetInverseModelFromCoordinate(double pi_X, double pi_Y) const;
    void                                ClearDirectModels() const;
    void                                ClearInverseModels() const;

    // Primary attributes

    HFCPtr<HGF2DTransfoModel> m_pPreTransfoModel;
    HFCPtr<HGF2DTransfoModel> m_pPostTransfoModel;

    double                   m_DirectStep;
#if (0)
    double                   m_InverseStep;
#endif


    // Acceleration attributes
    uint32_t m_NumberOfRows;
    uint32_t m_NumberOfColumns;
    HFCPtr<HGF2DCoordSys>               m_pDumCoordSys;
    mutable bool                       m_DirectModelsPresent;
    mutable HFCPtr<HGF2DTransfoModel>*  m_pArrayOfDirectModels;
#if (0)
    mutable bool                       m_InverseModelsPresent;
    mutable HFCPtr<HGF2DTransfoModel>*  m_pArrayOfInverseModels;
#else
    mutable HFCPtr<HVE2DQuadrilaterMesh> m_pArrayOfInverseModels;
#endif
    mutable HGF2DLiteExtent             m_CurrentDirectExtent;
#if (0)
    mutable HGF2DLiteExtent             m_CurrentInverseExtent;
#endif

    };
END_IMAGEPP_NAMESPACE

#include "HVE2DProjectiveGrid.hpp"
