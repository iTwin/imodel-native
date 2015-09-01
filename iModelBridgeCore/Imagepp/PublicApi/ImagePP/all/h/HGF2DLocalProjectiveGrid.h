//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLocalProjectiveGrid.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLocalProjectiveGrid
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DAffine.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DBoundaryModel;
class HGF2DGridModel;
class HGF2DPieceWiseModel;

#if defined NDEBUG
// Must be defined to false in release
#define RECORD_GRID_STAT (false)
#else
// Define to true for recording statistics
#define RECORD_GRID_STAT (false)
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
template <bool value> class GridStat
    {
public:
    GridStat() {}
    ~GridStat() {}
    inline void IncGetComposedModelFromCoordinateCount()           {}
    inline void IncGetComposedModelFromCoordinateFoundInGrid()     {}
    inline void IncGetComposedModelFromCoordinateFoundInBoundary() {}
    inline void IncGetComposedModelFromCoordinateByApproximation() {}
    inline void IncGetComposedModelFromCoordinateByUnAdapted()     {}

    inline void IncGetComposedModelFromInverseCoordinateCount()           {}
    inline void IncGetComposedModelFromInverseCoordinateFoundInGrid()     {}
    inline void IncGetComposedModelFromInverseCoordinateFoundInBoundary() {}
    inline void IncGetComposedModelFromInverseCoordinateByApproximation() {}
    inline void IncGetComposedModelFromInverseCoordinateByUnAdapted()     {}


private:
    // No implementation given
    GridStat(GridStat const&);
    GridStat& operator=(GridStat const&);
    };

/*---------------------------------------------------------------------------------**//**
* Template specialization for true
@bsimethod                                                    StephanePoulin  11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
template <> class GridStat<true>
    {
public:
    GridStat() : m_GetComposedModelFromCoordinateCount(0),  m_GetComposedModelFromCoordinateFoundInGrid(0), m_GetComposedModelFromCoordinateFoundInBoundary(0), m_GetComposedModelFromCoordinateByApproximation(0), m_GetComposedModelFromCoordinateByUnAdapted(0),
        m_GetComposedModelFromInverseCoordinateCount(0), m_GetComposedModelFromInverseCoordinateFoundInGrid(0), m_GetComposedModelFromInverseCoordinateFoundInBoundary(0), m_GetComposedModelFromInverseCoordinateByApproximation(0), m_GetComposedModelFromInverseCoordinateByUnAdapted(0)
        {
        }

    ~GridStat()
        {
        if (m_GetComposedModelFromCoordinateCount > 0 || m_GetComposedModelFromInverseCoordinateCount > 0)
            {
            ofstream myStream;
            myStream.open("c:\\tmp\\stat.txt", ios_base::out | ios_base::app );
            if (m_GetComposedModelFromCoordinateCount > 0)
                {
                myStream.precision(5);
                myStream << "GetComposedModelFromCoordinateCount:           " << m_GetComposedModelFromCoordinateCount << endl;
                myStream << "GetComposedModelFromCoordinateFoundInGrid:     " << m_GetComposedModelFromCoordinateFoundInGrid     << "  (" << m_GetComposedModelFromCoordinateFoundInGrid    /(double)m_GetComposedModelFromCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromCoordinateFoundInBoundary: " << m_GetComposedModelFromCoordinateFoundInBoundary << "  (" << m_GetComposedModelFromCoordinateFoundInBoundary/(double)m_GetComposedModelFromCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromCoordinateByApproximation: " << m_GetComposedModelFromCoordinateByApproximation << "  (" << m_GetComposedModelFromCoordinateByApproximation/(double)m_GetComposedModelFromCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromCoordinateByUnAdapted:     " << m_GetComposedModelFromCoordinateByUnAdapted     << "  (" << m_GetComposedModelFromCoordinateByUnAdapted    /(double)m_GetComposedModelFromCoordinateCount * 100 << "%)" << endl;
                }

            if (m_GetComposedModelFromInverseCoordinateCount > 0)
                {
                myStream << "GetComposedModelFromInverseCoordinateCount:           " << m_GetComposedModelFromInverseCoordinateCount << endl;
                myStream << "GetComposedModelFromInverseCoordinateFoundInGrid:     " << m_GetComposedModelFromInverseCoordinateFoundInGrid     << "  (" << m_GetComposedModelFromInverseCoordinateFoundInGrid    /(double)m_GetComposedModelFromInverseCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromInverseCoordinateFoundInBoundary: " << m_GetComposedModelFromInverseCoordinateFoundInBoundary << "  (" << m_GetComposedModelFromInverseCoordinateFoundInBoundary/(double)m_GetComposedModelFromInverseCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromInverseCoordinateByApproximation: " << m_GetComposedModelFromInverseCoordinateByApproximation << "  (" << m_GetComposedModelFromInverseCoordinateByApproximation/(double)m_GetComposedModelFromInverseCoordinateCount * 100 << "%)" << endl;
                myStream << "GetComposedModelFromInverseCoordinateByUnAdapted:     " << m_GetComposedModelFromInverseCoordinateByUnAdapted     << "  (" << m_GetComposedModelFromInverseCoordinateByUnAdapted    /(double)m_GetComposedModelFromInverseCoordinateCount * 100 << "%)" << endl;
                }

            myStream << "----------------------------------------------------------" << endl;
            myStream.close();
            }
        }

    inline void IncGetComposedModelFromCoordinateCount()           {
        ++m_GetComposedModelFromCoordinateCount;
        }
    inline void IncGetComposedModelFromCoordinateFoundInGrid()     {
        ++m_GetComposedModelFromCoordinateFoundInGrid;
        }
    inline void IncGetComposedModelFromCoordinateFoundInBoundary() {
        ++m_GetComposedModelFromCoordinateFoundInBoundary;
        }
    inline void IncGetComposedModelFromCoordinateByApproximation() {
        ++m_GetComposedModelFromCoordinateByApproximation;
        }
    inline void IncGetComposedModelFromCoordinateByUnAdapted()     {
        ++m_GetComposedModelFromCoordinateByUnAdapted;
        }

    inline void IncGetComposedModelFromInverseCoordinateCount()           {
        ++m_GetComposedModelFromInverseCoordinateCount;
        }
    inline void IncGetComposedModelFromInverseCoordinateFoundInGrid()     {
        ++m_GetComposedModelFromInverseCoordinateFoundInGrid;
        }
    inline void IncGetComposedModelFromInverseCoordinateFoundInBoundary() {
        ++m_GetComposedModelFromInverseCoordinateFoundInBoundary;
        }
    inline void IncGetComposedModelFromInverseCoordinateByApproximation() {
        ++m_GetComposedModelFromInverseCoordinateByApproximation;
        }
    inline void IncGetComposedModelFromInverseCoordinateByUnAdapted()     {
        ++m_GetComposedModelFromInverseCoordinateByUnAdapted;
        }

private:
    // No implementation given
    GridStat(GridStat const&);
    GridStat& operator=(GridStat const&);

private:
    uint64_t m_GetComposedModelFromCoordinateCount;
    uint64_t m_GetComposedModelFromCoordinateFoundInGrid;
    uint64_t m_GetComposedModelFromCoordinateFoundInBoundary;
    uint64_t m_GetComposedModelFromCoordinateByApproximation;
    uint64_t m_GetComposedModelFromCoordinateByUnAdapted;

    uint64_t m_GetComposedModelFromInverseCoordinateCount;
    uint64_t m_GetComposedModelFromInverseCoordinateFoundInGrid;
    uint64_t m_GetComposedModelFromInverseCoordinateFoundInBoundary;
    uint64_t m_GetComposedModelFromInverseCoordinateByApproximation;
    uint64_t m_GetComposedModelFromInverseCoordinateByUnAdapted;
    };

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
class HGF2DLocalProjectiveGrid : public HGF2DTransfoModelAdapter
    {
    HDECLARE_CLASS_ID(HGF2DLocalProjectiveGridId, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT                   HGF2DLocalProjectiveGrid();

    IMAGEPP_EXPORT                   HGF2DLocalProjectiveGrid(const HGF2DTransfoModel&   pi_rNonLinearTransfoModel,
                                                             const HGF2DLiteExtent&     pi_rAdaptedDirectExtent,
                                                             uint32_t                   pi_NumOfTilesX,
                                                             uint32_t                   pi_NumOfTilesY);

    IMAGEPP_EXPORT                   HGF2DLocalProjectiveGrid(const HGF2DAffine&         pi_rpGlobalAffine,
                                                             const HGF2DLiteExtent&     pi_rAdaptedDirectExtent,
                                                             uint32_t                   pi_NumOfTilesX,
                                                             uint32_t                   pi_NumOfTilesY,
                                                             const list<HFCPtr<HGF2DTransfoModel> >&  pi_rModelsList);

    IMAGEPP_EXPORT                   HGF2DLocalProjectiveGrid(const HGF2DLocalProjectiveGrid& pi_rObj);

    IMAGEPP_EXPORT virtual           ~HGF2DLocalProjectiveGrid();

    HGF2DLocalProjectiveGrid&       operator=(const HGF2DLocalProjectiveGrid& pi_rObj);


    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}

    // Conversion interface
    virtual StatusInt               ConvertDirect(double*   pio_pXInOut,
                                                  double*   pio_pYInOut) const override;

    virtual StatusInt               ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t    pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

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
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               ConvertInverse(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

    virtual StatusInt               ConvertInverse(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                    IsIdentity      () const;
    virtual bool                    IsStretchable   (double pi_AngleTolerance = 0) const;
    virtual void                    GetStretchParams(double*           po_pScaleFactorX,
                                                     double*           po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*      Clone () const override;

    virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


    // Model definition
    virtual bool                    CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>         GetMatrix() const;

    virtual HFCPtr<HGF2DTransfoModel>
                                    CreateSimplifiedModel() const;

    // Geometric properties
    virtual bool                    PreservesLinearity() const;
    virtual bool                    PreservesParallelism() const;
    virtual bool                    PreservesShape() const;
    virtual bool                    PreservesDirection() const;

    // Operations
    virtual void                    Reverse ();

    const HGF2DLiteExtent&          GetExtent() const;

    uint32_t                        GetNumberOfRow() const;
    uint32_t                        GetNumberOfColumn() const;
    const HFCPtr<HGF2DTransfoModel> GetGlobalAffine() const;
    HFCPtr<HGF2DTransfoModel>       GetModel(uint32_t pi_Row,
                                             uint32_t pi_Column) const;

    IMAGEPP_EXPORT void              GetPSSParameters (HGF2DLiteExtent&                  po_rExtent,
                                                      uint32_t&                           po_rNColumns,
                                                      uint32_t&                           po_rNRows,
                                                      list<HFCPtr<HGF2DTransfoModel> >& po_rModelList) const;

    IMAGEPP_EXPORT void              Dump(ofstream& outStream) const;

protected:

    virtual void                    Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;

    HGF2DLocalProjectiveGrid(bool                     pi_Mode,
                             const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                             const HGF2DTransfoModel& pi_rPreTransfo,
                             const HGF2DTransfoModel& pi_rPostTransfo,
                             HFCPtr<HGF2DGridModel>   pi_gridModel,
                             HFCPtr<HGF2DBoundaryModel> pi_boundaryModel,
                             HGF2DLiteExtent          pi_AdaptedDirectExtent,
                             uint32_t                 pi_NumOfTilesX,
                             uint32_t                 pi_NumOfTilesY);


private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        HGF2DTransfoModelAdapter::ValidateInvariants();
        }
#endif

    typedef enum
        {
        AFFINE,
        PROJECTIVE
        } MODELTYPE;

    // Private methods
    void                        Copy (const HGF2DLocalProjectiveGrid& pi_rObj);

    HFCPtr<HGF2DTransfoModel>   GetModelFromCoordinate(double pi_X,
                                                       double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>   GetModelFromInverseCoordinate(double pi_X,
                                                              double pi_Y) const;

    HFCPtr<HGF2DTransfoModel>   GetComposedModelFromCoordinate(double pi_X,
                                                               double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>   GetComposedModelFromInverseCoordinate(double pi_X,
                                                                      double pi_Y) const;

    HFCPtr<HGF2DTransfoModel>   GetComposedGlobalAffine() const;
    HFCPtr<HGF2DTransfoModel>   GetComposedAdaptedModel() const;

    HGF2DLiteExtent             ComputeDirectModelExtentFromRowRol(int32_t pi_Row,  int32_t pi_Col) const;
    HGF2DLiteExtent             ComputeDirectModelExtentFromCoordinate(double pi_X, double pi_Y) const;

    void                        ComputeModelIndexFromDirectCoordinate(int32_t* po_pRow,
                                                                      int32_t* po_pCol,
                                                                      double pi_X,
                                                                      double pi_Y) const;

    double                      GetDirectGridWidth() const;
    double                      GetDirectGridHeight() const;

    HFCPtr<HGF2DTransfoModel>   ComputeGlobalAffineModel() const;

    HFCPtr<HGF2DTransfoModel>   ComputeDirectModelFromPoints (double* pi_pPoints,
                                                              uint32_t  pi_nbPoints,
                                                              MODELTYPE pi_ModelType) const;

    HGF2DLiteExtent             ComputeTransitExtent() const;

    bool                        GetUseApproximation() const;
    void                        SetUseApproximation(bool pi_Value) const;


    // Primary attributes
    bool                        m_Direct;
    mutable bool                m_useGlobalAffineApproximation;
    HFCPtr<HGF2DTransfoModel>   m_pPreTransfoModel;
    HFCPtr<HGF2DTransfoModel>   m_pPostTransfoModel;

    // All mutable for optimisation
    mutable HFCPtr<HGF2DTransfoModel> m_pGlobalAffine;
    mutable HFCPtr<HGF2DTransfoModel> m_pComposedAffine;
    mutable HFCPtr<HGF2DTransfoModel> m_pComposedAdaptedModel;

    HGF2DLiteExtent            m_directExtent;
    uint32_t                   m_nColumns;
    uint32_t                   m_nRows;

    HFCPtr<HGF2DGridModel>       m_pGridModel;
    HFCPtr<HGF2DPieceWiseModel>  m_pGridModelBooster;
    HFCPtr<HGF2DBoundaryModel>   m_pBoundaryModel;
    HFCPtr<HGF2DPieceWiseModel>  m_pBoundaryModelBooster;

    mutable GridStat<RECORD_GRID_STAT> m_stat;

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DLocalProjectiveGrid.hpp"

