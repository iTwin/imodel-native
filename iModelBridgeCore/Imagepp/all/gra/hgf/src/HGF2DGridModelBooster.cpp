//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DGridModelBooster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DGridModelBooster.h>
#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>


//-----------------------------------------------------------------------------
// HGF2DGridModelBooster
//-----------------------------------------------------------------------------
HGF2DGridModelBooster::HGF2DGridModelBooster  (HFCPtr<HGF2DGridModel> const&     pi_pGridModel,
                                               HFCPtr<HGF2DTransfoModel> const& pi_pPreTransfo,
                                               HFCPtr<HGF2DTransfoModel> const& pi_pPostTransfo)
    : m_directExtent(0.0, 0.0, 1.0, 1.0),
      m_inverseExtent(0.0, 0.0, 1.0, 1.0),
      m_directRegions(QuadrilateralFacetQuadTree(m_directExtent)),
      m_inverseRegions(QuadrilateralFacetQuadTree(m_inverseExtent)),
      m_pGridModel(pi_pGridModel),
      m_pPreTransfo(pi_pPreTransfo),
      m_pPostTransfo(pi_pPostTransfo)
    {
    // Compute direct extent
    m_directExtent  = ConvertLiteExtent(pi_pGridModel->GetExtent(), *pi_pPreTransfo, HGF2DPieceWiseModel::INVERSE);
    m_directExtent  = ScaleExtent(m_directExtent, 1.001);
    m_directRegions = QuadrilateralFacetQuadTree(m_directExtent);

    // Compute inverse extent
    HFCPtr<HGF2DComplexTransfoModel> pComplex(new HGF2DComplexTransfoModel());
    pComplex->AddModel(*pi_pGridModel->GetAdaptedModel());
    pComplex->AddModel(*pi_pPostTransfo);

    m_inverseExtent  = ConvertLiteExtent(pi_pGridModel->GetExtent(), pi_pGridModel->GetNumberOfColumns(), pi_pGridModel->GetNumberOfRows(), *pComplex, HGF2DPieceWiseModel::DIRECT);
    m_inverseExtent  = ScaleExtent(m_inverseExtent, 1.001);
    m_inverseRegions = QuadrilateralFacetQuadTree(m_inverseExtent);
    }

//-----------------------------------------------------------------------------
// HGF2DGridModelBooster
//-----------------------------------------------------------------------------
HGF2DGridModelBooster::HGF2DGridModelBooster  (HGF2DGridModelBooster const& pi_rObj)
    : m_directExtent(pi_rObj.m_directExtent),
      m_inverseExtent(pi_rObj.m_inverseExtent),
      m_directRegions(pi_rObj.m_directRegions),
      m_inverseRegions(pi_rObj.m_inverseRegions),
      m_pGridModel(pi_rObj.m_pGridModel),
      m_pPreTransfo(pi_rObj.m_pPreTransfo),
      m_pPostTransfo(pi_rObj.m_pPostTransfo)
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DGridModelBooster
//-----------------------------------------------------------------------------
HGF2DGridModelBooster::~HGF2DGridModelBooster  ()
    {
    }

//-----------------------------------------------------------------------------
// GetModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModelBooster::GetModelFromCoordinate(double pi_X, double pi_Y) const
    {
    QuadrilateralFacet const* pEntry = m_directRegions.QuerySingle(pi_X, pi_Y);

    if (NULL != pEntry)
        {
        return pEntry->GetTransfoModel();
        }

    // The model is not cached in the direct regions,
    // ask the grid for a model
    m_pPreTransfo->ConvertDirect(&pi_X, &pi_Y);
    HAutoPtr<QuadrilateralFacet> pFacet (m_pGridModel->GetFacetFromCoordinate(pi_X, pi_Y));

    if (pFacet != NULL)
        {
        HGF2DLiteQuadrilateral newShape (ConvertLiteQuadrilateral(pFacet->GetQuadrilateral(), m_pPreTransfo, true));
        QuadrilateralFacet newFacet (m_pPreTransfo->ComposeInverseWithDirectOf(*pFacet->GetTransfoModel())->ComposeInverseWithDirectOf(*m_pPostTransfo), newShape);

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
HFCPtr<HGF2DTransfoModel> HGF2DGridModelBooster::GetModelFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    QuadrilateralFacet const* pEntry = m_inverseRegions.QuerySingle(pi_X, pi_Y);

    if (NULL != pEntry)
        {
        return pEntry->GetTransfoModel();
        }

    // The model is not cached in the inverse regions,
    // ask the grid for a model
    m_pPostTransfo->ConvertInverse(&pi_X, &pi_Y);
    HAutoPtr<QuadrilateralFacet> pFacet (m_pGridModel->GetFacetFromInverseCoordinate(pi_X, pi_Y));

    if (pFacet != NULL)
        {
        HGF2DLiteQuadrilateral newShape (ConvertLiteQuadrilateral(pFacet->GetQuadrilateral(), m_pPostTransfo, false));

        QuadrilateralFacet newFacet (m_pPreTransfo->ComposeInverseWithDirectOf(*pFacet->GetTransfoModel())->ComposeInverseWithDirectOf(*m_pPostTransfo), newShape);

        if (!m_inverseRegions.AddItem(newFacet))
            {
            HASSERT(false);
            }

        return newFacet.GetTransfoModel();
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// ConvertLiteQuadrilateral
//-----------------------------------------------------------------------------
HGF2DLiteQuadrilateral HGF2DGridModelBooster::ConvertLiteQuadrilateral (HGF2DLiteQuadrilateral const&     pi_rShape,
                                                                        HFCPtr<HGF2DTransfoModel> const& pi_pModel,
                                                                        bool                              pi_reverse) const
    {
    double x0,x1,x2,x3;
    double y0,y1,y2,y3;

    pi_rShape.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

    if (pi_reverse)
        {
        pi_pModel->ConvertInverse(&x0, &y0);
        pi_pModel->ConvertInverse(&x1, &y1);
        pi_pModel->ConvertInverse(&x2, &y2);
        pi_pModel->ConvertInverse(&x3, &y3);
        }
    else
        {
        pi_pModel->ConvertDirect(&x0, &y0);
        pi_pModel->ConvertDirect(&x1, &y1);
        pi_pModel->ConvertDirect(&x2, &y2);
        pi_pModel->ConvertDirect(&x3, &y3);
        }

    return HGF2DLiteQuadrilateral(x0, y0, x1, y1, x2, y2, x3, y3);
    }