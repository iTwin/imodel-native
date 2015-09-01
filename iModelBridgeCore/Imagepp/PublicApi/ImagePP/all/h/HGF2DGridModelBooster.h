//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DGridModelBooster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DPieceWiseModel.h>
#include <Imagepp/all/h/HGF2DGridModel.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGFQuadTree.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    HGF2DGridModelBooster
    -----------------------------------------------------------------------------
*/
class HGF2DGridModelBooster : public HGF2DPieceWiseModel
    {
public:
    // Primary methods
    HGF2DGridModelBooster  (HFCPtr<HGF2DGridModel> const&     pi_pGridModel,
                            HFCPtr<HGF2DTransfoModel> const& pi_preTransfo,
                            HFCPtr<HGF2DTransfoModel> const& pi_postTransfo);

    HGF2DGridModelBooster(HGF2DGridModelBooster const&);

    virtual                     ~HGF2DGridModelBooster();

    // From HGF2DPieceWiseModel
    HFCPtr<HGF2DTransfoModel>   GetModelFromCoordinate       (double pi_X, double pi_Y) const;
    HFCPtr<HGF2DTransfoModel>   GetModelFromInverseCoordinate(double pi_X, double pi_Y) const;

private:
    // private methods
    HGF2DLiteQuadrilateral      ConvertLiteQuadrilateral (HGF2DLiteQuadrilateral const&     pi_rShape,
                                                          HFCPtr<HGF2DTransfoModel> const& pi_pModel,
                                                          bool                              pi_reverse) const;


private:
    // private types
    typedef HGFQuadTree<QuadrilateralFacet>
    QuadrilateralFacetQuadTree;

    // Primary attributes
    HGF2DLiteExtent                     m_directExtent;
    HGF2DLiteExtent                     m_inverseExtent;

    // Mutable for optimisation
    mutable QuadrilateralFacetQuadTree  m_directRegions;
    mutable QuadrilateralFacetQuadTree  m_inverseRegions;

    HFCPtr<HGF2DGridModel>              m_pGridModel;
    HFCPtr<HGF2DTransfoModel>           m_pPreTransfo;
    HFCPtr<HGF2DTransfoModel>           m_pPostTransfo;

private:
    // Disabled methods
    HGF2DGridModelBooster& operator=(HGF2DGridModelBooster const&);
    };
END_IMAGEPP_NAMESPACE
