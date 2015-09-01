//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DBoundaryModelBooster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DBoundaryModelBooster.h>
#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>

//-----------------------------------------------------------------------------
// HGF2DBoundaryModelBooster
//-----------------------------------------------------------------------------
HGF2DBoundaryModelBooster::HGF2DBoundaryModelBooster  (HFCPtr<HGF2DBoundaryModel> const& pi_pBoundaryModel,
                                                       HFCPtr<HGF2DTransfoModel> const&   pi_pPreTransfo,
                                                       HFCPtr<HGF2DTransfoModel> const&   pi_pPostTransfo)
    : m_directExtent(0.0, 0.0, 1.0, 1.0),
      m_inverseExtent(0.0, 0.0, 1.0, 1.0),
      m_directRegions(TriangleFacetQuadTree(m_directExtent)),
      m_inverseRegions(TriangleFacetQuadTree(m_inverseExtent)),
      m_pBoundaryModel(pi_pBoundaryModel),
      m_pPreTransfo(pi_pPreTransfo),
      m_pPostTransfo(pi_pPostTransfo)
    {
    // Compute direct extent
    m_directExtent  = ConvertLiteExtent(pi_pBoundaryModel->GetOuterExtent(), *pi_pPreTransfo, HGF2DPieceWiseModel::INVERSE);
    m_directExtent  = ScaleExtent(m_directExtent, 1.001);
    m_directRegions = TriangleFacetQuadTree(m_directExtent);

    // Compute inverse extent
    HFCPtr<HGF2DComplexTransfoModel> pComplex(new HGF2DComplexTransfoModel());
    pComplex->AddModel(*pi_pBoundaryModel->GetOuterModel());
    pComplex->AddModel(*pi_pPostTransfo);

    HASSERT(pi_pBoundaryModel->GetOuterModel()->PreservesLinearity());
    HASSERT(pi_pPostTransfo->PreservesLinearity());

    m_inverseExtent  = ConvertLiteExtent(pi_pBoundaryModel->GetOuterExtent(), *pComplex, HGF2DPieceWiseModel::DIRECT);
    m_inverseExtent  = ScaleExtent(m_inverseExtent, 1.001);
    m_inverseRegions = TriangleFacetQuadTree(m_inverseExtent);
    }

//-----------------------------------------------------------------------------
// HGF2DBoundaryModelBooster
//-----------------------------------------------------------------------------
HGF2DBoundaryModelBooster::HGF2DBoundaryModelBooster  (HGF2DBoundaryModelBooster const& pi_rObj)
    : m_directExtent(pi_rObj.m_directExtent),
      m_inverseExtent(pi_rObj.m_inverseExtent),
      m_directRegions(pi_rObj.m_directRegions),
      m_inverseRegions(pi_rObj.m_inverseRegions),
      m_pBoundaryModel(pi_rObj.m_pBoundaryModel),
      m_pPreTransfo(pi_rObj.m_pPreTransfo),
      m_pPostTransfo(pi_rObj.m_pPostTransfo)
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DBoundaryModelBooster
//-----------------------------------------------------------------------------
HGF2DBoundaryModelBooster::~HGF2DBoundaryModelBooster  ()
    {
    }

//-----------------------------------------------------------------------------
// GetModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModelBooster::GetModelFromCoordinate(double pi_X, double pi_Y) const
    {
    TriangleFacet const* pEntry = m_directRegions.QuerySingle(pi_X, pi_Y);

    if (NULL != pEntry)
        {
        return pEntry->GetTransfoModel();
        }

    // The model is not cached in the direct regions,
    // ask the boundary for a model
    m_pPreTransfo->ConvertDirect(&pi_X, &pi_Y);
    HAutoPtr<TriangleFacet> pFacet (m_pBoundaryModel->GetFacetFromCoordinate(pi_X, pi_Y));

    if (pFacet != NULL)
        {
        HGF2DTriangle newShape (ConvertTriangle(pFacet->GetTriangle(), m_pPreTransfo, true));
        TriangleFacet newFacet (m_pPreTransfo->ComposeInverseWithDirectOf(*pFacet->GetTransfoModel())->ComposeInverseWithDirectOf(*m_pPostTransfo), newShape);

        if (!m_directRegions.AddItem(newFacet))
            {
            HASSERT(false);
            }

        return newFacet.GetTransfoModel();
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// GetModelFromInverseCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModelBooster::GetModelFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    TriangleFacet const* pEntry = m_inverseRegions.QuerySingle(pi_X, pi_Y);

    if (NULL != pEntry)
        {
        return pEntry->GetTransfoModel();
        }

    // The model is not cached in the inverse regions,
    // ask the boundary for a model
    m_pPostTransfo->ConvertInverse(&pi_X, &pi_Y);
    HAutoPtr<TriangleFacet> pFacet (m_pBoundaryModel->GetFacetFromInverseCoordinate(pi_X, pi_Y));

    if (pFacet != NULL)
        {
        HGF2DTriangle newShape (ConvertTriangle(pFacet->GetTriangle(), m_pPostTransfo, false));

        TriangleFacet newFacet (m_pPreTransfo->ComposeInverseWithDirectOf(*pFacet->GetTransfoModel())->ComposeInverseWithDirectOf(*m_pPostTransfo), newShape);

        if (!m_inverseRegions.AddItem(newFacet))
            {
            HASSERT(false);
            }

        return newFacet.GetTransfoModel();
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// ConvertTriangle
//-----------------------------------------------------------------------------
HGF2DTriangle HGF2DBoundaryModelBooster::ConvertTriangle (HGF2DTriangle const&     pi_rShape,
                                                          HFCPtr<HGF2DTransfoModel> const& pi_pModel,
                                                          bool                              pi_reverse) const
    {
    double x0,x1,x2;
    double y0,y1,y2;

    pi_rShape.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2);

    if (pi_reverse)
        {
        pi_pModel->ConvertInverse(&x0, &y0);
        pi_pModel->ConvertInverse(&x1, &y1);
        pi_pModel->ConvertInverse(&x2, &y2);
        }
    else
        {
        pi_pModel->ConvertDirect(&x0, &y0);
        pi_pModel->ConvertDirect(&x1, &y1);
        pi_pModel->ConvertDirect(&x2, &y2);
        }

    return HGF2DTriangle(x0, y0, x1, y1, x2, y2);
    }