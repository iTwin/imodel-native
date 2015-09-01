//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DBoundaryModelBooster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DPieceWiseModel.h>
#include <Imagepp/all/h/HGF2DBoundaryModel.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGFQuadTree.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    HGF2DBoundaryModelBooster
    -----------------------------------------------------------------------------
*/
class HGF2DBoundaryModelBooster : public HGF2DPieceWiseModel
    {
public:
    // Primary methods
    HGF2DBoundaryModelBooster  (HFCPtr<HGF2DBoundaryModel> const& pi_pBoundaryModel,
                                HFCPtr<HGF2DTransfoModel> const&   pi_preTransfo,
                                HFCPtr<HGF2DTransfoModel> const&   pi_postTransfo);

    HGF2DBoundaryModelBooster(HGF2DBoundaryModelBooster const&);

    virtual                     ~HGF2DBoundaryModelBooster();

    // HGF2DPieceWiseModel api
    HFCPtr<HGF2DTransfoModel>   GetModelFromCoordinate       (double pi_X, double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>   GetModelFromInverseCoordinate(double pi_X, double pi_Y) const;

private:
    // private methods
    HGF2DTriangle               ConvertTriangle (HGF2DTriangle const&              pi_rShape,
                                                 HFCPtr<HGF2DTransfoModel> const& pi_pModel,
                                                 bool                              pi_reverse) const;



private:
    // private types
    typedef HGFQuadTree<TriangleFacet> TriangleFacetQuadTree;

    // Primary attributes
    HGF2DLiteExtent                     m_directExtent;
    HGF2DLiteExtent                     m_inverseExtent;

    // Mutable for optimisation
    mutable TriangleFacetQuadTree       m_directRegions;
    mutable TriangleFacetQuadTree       m_inverseRegions;

    HFCPtr<HGF2DBoundaryModel>          m_pBoundaryModel;
    HFCPtr<HGF2DTransfoModel>           m_pPreTransfo;
    HFCPtr<HGF2DTransfoModel>           m_pPostTransfo;

private:
    // Disabled methods
    HGF2DBoundaryModelBooster& operator=(HGF2DBoundaryModelBooster const&);
    };

END_IMAGEPP_NAMESPACE