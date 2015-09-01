//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DGridModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>
#include <Imagepp/all/h/HGF2DLiteQuadrilateral.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGFQuadTree.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DPieceWiseModel.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    HGF2DGridModel
    -----------------------------------------------------------------------------
*/
class HGF2DGridModel : public HGF2DPieceWiseModel
    {
public:
    // Primary methods
    IMAGEPP_EXPORT HGF2DGridModel  (const HGF2DTransfoModel&   pi_pModel,
                            const HGF2DLiteExtent&     pi_directExtent,
                            uint32_t                   pi_numOfTilesX,
                            uint32_t                   pi_numOfTilesY);

    HGF2DGridModel  (const HGF2DLiteExtent&     pi_directExtent,
                     uint32_t                   pi_numOfTilesX,
                     uint32_t                   pi_numOfTilesY,
                     const list<HFCPtr<HGF2DTransfoModel> >& pi_modelList);

    ~HGF2DGridModel();

    // From HGF2DPieceWiseModel
    HFCPtr<HGF2DTransfoModel>   GetModelFromCoordinate(double pi_X, double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>   GetModelFromInverseCoordinate(double pi_X, double pi_Y) const;

    // HGF2DGridModel api
    HFCPtr<HGF2DTransfoModel>   GetModelFromRowColumn(uint32_t pi_Row, uint32_t pi_Column) const;
    HFCPtr<HGF2DTransfoModel>   GetModelFromCoordinateTol(double pi_X, double pi_Y) const;
    QuadrilateralFacet*         GetFacetFromCoordinate(double pi_X, double pi_Y) const;
    QuadrilateralFacet*         GetFacetFromInverseCoordinate(double pi_X, double pi_Y) const;

    HGF2DLiteExtent             GetExtent() const;
    HGF2DLiteExtent             GetInverseExtent() const;
    HFCPtr<HGF2DTransfoModel>   GetAdaptedModel() const;

    uint32_t                    GetNumberOfColumns() const;
    uint32_t                    GetNumberOfRows() const;

    IMAGEPP_EXPORT void                 ConvertDirect(double pi_XIn, double pi_YIn, double*   po_pXOut, double*   po_pYOut) const;

    void                        Dump(ofstream& outStream) const;





private:

    // private methods
    HGF2DGridModel(HGF2DGridModel const&);

    HFCPtr<HGF2DTransfoModel>   ComputeProjectionModel (vector<double> const& srcPoints,
                                                        vector<double> const& dstPoints) const;
    HGF2DLiteQuadrilateral      ComputeInverseShapeFromDirectExtent(HGF2DLiteExtent&                  pi_rExtent,
                                                                    HFCPtr<HGF2DTransfoModel> const& pModel) const;

    double                     GetDirectGridWidth() const;
    double                     GetDirectGridHeight() const;

    HFCPtr<HGF2DTransfoModel>   CreateModelFromRowColumn(uint32_t pi_Row, uint32_t pi_Column) const;

    HFCPtr<HGF2DTransfoModel>   CreateModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const;
    HGF2DLiteExtent             CreateModelExtentFromRowRol(uint32_t pi_Row, uint32_t pi_Col) const;
    HGF2DLiteQuadrilateral      CreateQuadrilateralFromRowRol(uint32_t pi_Row, uint32_t pi_Col) const;


    HFCPtr<HGF2DTransfoModel>   CreateModelFromInverseCoordinate(double pi_x, double pi_y) const;

    void                        CompleteInverseRegions () const;

    void                        ComputeRowColumnFromDirectCoordinate (uint32_t* po_row,
                                                                      uint32_t* po_column,
                                                                      double pi_x,
                                                                      double pi_y) const;

    uint32_t                    ComputeModelIndexFromRowColumn (uint32_t pi_row, uint32_t pi_column) const;


    HGF2DLiteExtent             ConvertLiteExtentWithGridModel(const HGF2DLiteExtent&   pi_rExtent,
                                                               uint32_t                 nCols,
                                                               uint32_t                 nRows) const;


private:
    typedef HGFQuadTree<QuadrilateralFacet> QuadrilateralFacetQuadTree;

    // Primary attributes
    uint32_t                  m_nColumns;
    uint32_t                  m_nRows;

    HGF2DLiteExtent           m_directExtent;             // Primary region of adaptation
    HGF2DLiteExtent           m_inverseExtent;            // Primary region of adaptation)

    // Mutable for optimisation
    mutable vector<HFCPtr<HGF2DTransfoModel> > m_arrayOfDirectModels;
    mutable QuadrilateralFacetQuadTree         m_inverseRegions;

    HFCPtr<HGF2DTransfoModel>                  m_pAdaptedTransfoModel;

private:
    // Disabled methods
    HGF2DGridModel& operator=(HGF2DGridModel const&);


    };
END_IMAGEPP_NAMESPACE

#include <Imagepp/all/h/HGF2DGridModel.hpp>