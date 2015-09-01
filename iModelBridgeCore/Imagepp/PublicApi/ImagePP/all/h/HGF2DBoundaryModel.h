//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DBoundaryModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGFQuadTree.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DTriangle.h>
#include <Imagepp/all/h/HGF2DGridModel.h>
#include <Imagepp/all/h/HGF2DPieceWiseModel.h>

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    HGF2DBoundaryModel
    -----------------------------------------------------------------------------
*/
class HGF2DBoundaryModel :public HGF2DPieceWiseModel
    {
public:
    // Public Interface
    HGF2DBoundaryModel(HGF2DLiteExtent const&            pi_innerExtent,
                       HGF2DLiteExtent const&            pi_outerExtent,
                       HFCPtr<HGF2DGridModel> const      pi_innertModel,
                       HFCPtr<HGF2DTransfoModel> const   pi_outerModel,
                       uint32_t                         pi_nColumns,
                       uint32_t                         pi_nRows);

    HGF2DBoundaryModel(HGF2DBoundaryModel const& pi_rObj);
    ~HGF2DBoundaryModel();

    // From HGF2DPieceWiseModel
    HFCPtr<HGF2DTransfoModel>           GetModelFromCoordinate(double pi_X, double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>           GetModelFromInverseCoordinate(double pi_X, double pi_Y) const;

    TriangleFacet*                      GetFacetFromCoordinate(double pi_X, double pi_Y) const;
    TriangleFacet*                      GetFacetFromInverseCoordinate(double pi_X, double pi_Y) const;

    HGF2DLiteExtent                     GetOuterExtent() const;
    HFCPtr<HGF2DTransfoModel>           GetOuterModel() const;


    void                                Dump(ofstream& outStream) const;

private:
    // Private Interface
    void                                AddTriangleEntry (vector<double> const& pi_srcPts,
                                                          vector<double> const& pi_dstPts) const;

    HFCPtr<HGF2DTransfoModel>           ComputeDirectModelFromPoints (vector<double> const& pi_srcPts,
                                                                      vector<double> const& pi_dstPts) const;
    HGF2DLiteExtent                     ComputeInverseTransitExtent () const;

    void                                GenerateExtendedMesh() const;

private:
    // Private Members
    uint32_t                   m_nColumns;
    uint32_t                   m_nRows;
    HGF2DLiteExtent             m_innerExtent;
    HGF2DLiteExtent             m_outerExtent;
    HFCPtr<HGF2DGridModel>      m_pInnerModel;
    HFCPtr<HGF2DTransfoModel>   m_pOuterModel;

    typedef HGFQuadTree<TriangleFacet>  TriangleFacetQuadTree;

    mutable HAutoPtr<TriangleFacetQuadTree> m_pDirectRegions;
    mutable HAutoPtr<TriangleFacetQuadTree> m_pInverseRegions;


private:
    // Disabled interface
    HGF2DBoundaryModel& operator=(HGF2DBoundaryModel const&);
    };

END_IMAGEPP_NAMESPACE

#include <Imagepp/all/h/HGF2DBoundaryModel.hpp>
