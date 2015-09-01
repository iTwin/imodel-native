//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DBoundaryModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DBoundaryModel.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGFException.h>


//-----------------------------------------------------------------------------
// HGF2DBoundaryModel
//-----------------------------------------------------------------------------
HGF2DBoundaryModel::HGF2DBoundaryModel(HGF2DLiteExtent const&           pi_innerExtent,
                                       HGF2DLiteExtent const&           pi_outerExtent,
                                       HFCPtr<HGF2DGridModel> const     pi_innertModel,
                                       HFCPtr<HGF2DTransfoModel> const  pi_outerModel,
                                       uint32_t                         pi_nColumns,
                                       uint32_t                         pi_nRows)
    :m_nColumns(pi_nColumns),
     m_nRows(pi_nRows),
     m_innerExtent(pi_innerExtent),
     m_outerExtent(pi_outerExtent),
     m_pInnerModel(pi_innertModel),
     m_pOuterModel(pi_outerModel)
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DBoundaryModel
//-----------------------------------------------------------------------------
HGF2DBoundaryModel::HGF2DBoundaryModel(HGF2DBoundaryModel const& pi_rObj)
    :m_nColumns(pi_rObj.m_nColumns),
     m_nRows(pi_rObj.m_nRows),
     m_innerExtent(pi_rObj.m_innerExtent),
     m_outerExtent(pi_rObj.m_outerExtent),
     m_pInnerModel(pi_rObj.m_pInnerModel),
     m_pOuterModel(pi_rObj.m_pOuterModel)
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DBoundaryModel
//-----------------------------------------------------------------------------
HGF2DBoundaryModel::~HGF2DBoundaryModel()
    {
    }

//-----------------------------------------------------------------------------
// AddTriangleEntry
//-----------------------------------------------------------------------------
void HGF2DBoundaryModel::AddTriangleEntry (vector<double> const& pi_srcPts,
                                           vector<double> const& pi_dstPts) const
    {
    // Direct
    HGF2DTriangle DirectTriangle(pi_srcPts[0],  pi_srcPts[1], pi_srcPts[2], pi_srcPts[3], pi_srcPts[4], pi_srcPts[5]);

    HFCPtr<HGF2DTransfoModel> pModel (ComputeDirectModelFromPoints(pi_srcPts, pi_dstPts));

    TriangleFacet DirectEntry(pModel, DirectTriangle);
    if (!m_pDirectRegions->AddItem(DirectEntry))
        {
        HASSERT(false);
        }

    // Inverse
    HGF2DTriangle  InverseTriangle(pi_dstPts[0],  pi_dstPts[1], pi_dstPts[2], pi_dstPts[3], pi_dstPts[4], pi_dstPts[5]);

    TriangleFacet InverseEntry(pModel, InverseTriangle);
    if (!m_pInverseRegions->AddItem(InverseEntry))
        {
        HASSERT(false);
        }
    }

//-----------------------------------------------------------------------------
// ComputeInverseTransitExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DBoundaryModel::ComputeInverseTransitExtent () const
    {
    return ConvertLiteExtent(m_outerExtent, m_nColumns, m_nRows, *m_pOuterModel, DIRECT);
    }

//-----------------------------------------------------------------------------
// GetModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModel::GetModelFromCoordinate(double pi_X, double pi_Y) const
    {
    try
        {
        if (m_pDirectRegions == NULL)
            {
            GenerateExtendedMesh();
            }

        TriangleFacet const* pEntry = m_pDirectRegions->QuerySingle(pi_X, pi_Y);

        if (NULL != pEntry)
            {
            return pEntry->GetTransfoModel();
            }
        }
	catch(HGFmzGCoordException&)
	{
	}

    return NULL;
    }

//-----------------------------------------------------------------------------
// GetModelFromInverseCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModel::GetModelFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    try
        {
        if (m_pInverseRegions == NULL)
            {
            GenerateExtendedMesh();
            }

        TriangleFacet const* pTriangleEntry = m_pInverseRegions->QuerySingle(pi_X, pi_Y);

        if (NULL != pTriangleEntry)
            {
            return pTriangleEntry->GetTransfoModel();
            }
        }
	catch(HGFmzGCoordException&)
	{
	}

    return NULL;
    }

//-----------------------------------------------------------------------------
// GetFacetFromCoordinate
//-----------------------------------------------------------------------------
TriangleFacet* HGF2DBoundaryModel::GetFacetFromCoordinate(double pi_X, double pi_Y) const
    {
    HAutoPtr<TriangleFacet> pFacet(NULL);

    try
        {
        if (m_pInverseRegions == NULL)
            {
            GenerateExtendedMesh();
            }

        TriangleFacet const* pTriangleEntry = m_pDirectRegions->QuerySingle(pi_X, pi_Y);

        if (NULL != pTriangleEntry)
            {
            pFacet = new TriangleFacet(*pTriangleEntry);
            return pFacet.release();
            }
        }
	catch(HGFmzGCoordException&)
	{
	}

    return pFacet.release();
    }

//-----------------------------------------------------------------------------
// GetFacetFromInverseCoordinate
//-----------------------------------------------------------------------------
TriangleFacet* HGF2DBoundaryModel::GetFacetFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    HAutoPtr<TriangleFacet> pFacet(NULL);

    try
        {
        if (m_pInverseRegions == NULL)
            {
            GenerateExtendedMesh();
            }

        TriangleFacet const* pTriangleEntry = m_pInverseRegions->QuerySingle(pi_X, pi_Y);

        if (NULL != pTriangleEntry)
            {
            pFacet = new TriangleFacet(*pTriangleEntry);
            return pFacet.release();
            }
        }
	catch(HGFmzGCoordException&)
	{
	}

    return pFacet.release();
    }

//-----------------------------------------------------------------------------
// GenerateExtendedMesh
//-----------------------------------------------------------------------------
void HGF2DBoundaryModel::GenerateExtendedMesh() const
    {
    // Compute the mesh using the grid, not the non-linear model. Result will result the same for points on the grid
    // This will allows to recompute the mesh even if the non linear model is absent.
    HGF2DLiteExtent Direct (ScaleExtent(m_outerExtent, 1.01));
    HGF2DLiteExtent Inverse (ScaleExtent(ComputeInverseTransitExtent(), 1.01));

    m_pDirectRegions  = new TriangleFacetQuadTree(Direct);
    m_pInverseRegions = new TriangleFacetQuadTree(Inverse);

    double OriginX = m_innerExtent.GetOrigin().GetX();
    double OriginY = m_innerExtent.GetOrigin().GetY();
    double CornerX = m_innerExtent.GetCorner().GetX();
    double CornerY = m_innerExtent.GetCorner().GetY();
    double InnerStepX  = m_innerExtent.GetWidth() / m_nColumns;
    double InnerStepY = m_innerExtent.GetHeight() / m_nRows;

    HGF2DLiteExtent ExtendedExtent (m_outerExtent);
    double ExtendedOriginX = ExtendedExtent.GetOrigin().GetX();
    double ExtendedOriginY = ExtendedExtent.GetOrigin().GetY();
    double ExtendedCornerX = ExtendedExtent.GetCorner().GetX();
    double ExtendedCornerY = ExtendedExtent.GetCorner().GetY();
    double OuterStepX   = ExtendedExtent.GetWidth() / m_nColumns;
    double OuterStepY   = ExtendedExtent.GetHeight() / m_nRows;

    uint32_t x;

    vector<double> InnerPoints;
    vector<double> OuterPoints;

    InnerPoints.reserve((m_nColumns * 4) + (m_nRows * 4) + 2);
    OuterPoints.reserve((m_nColumns * 4) + (m_nRows * 4) + 2);


    // Bottom (Left-Right)
    for (x=0; x<m_nColumns; ++x)
        {
        double InnerX = OriginX + x * InnerStepX;
        double InnerY = OriginY;
        InnerPoints.push_back(InnerX);
        InnerPoints.push_back(InnerY);

        double OuterX = ExtendedOriginX + x * OuterStepX;
        double OuterY = ExtendedOriginY;
        OuterPoints.push_back(OuterX);
        OuterPoints.push_back(OuterY);
        }

    // Right (Bottom-Top)
    for (x=0; x<m_nRows; ++x)
        {
        double InnerX = CornerX;
        double InnerY = OriginY + x * InnerStepY;
        InnerPoints.push_back(InnerX);
        InnerPoints.push_back(InnerY);

        double OuterX = ExtendedCornerX;
        double OuterY = ExtendedOriginY + x * OuterStepY;
        OuterPoints.push_back(OuterX);
        OuterPoints.push_back(OuterY);
        }

    // Top (Right-Left)
    for (x=0; x<m_nColumns; ++x)
        {
        double InnerX = CornerX - x * InnerStepX;
        double InnerY = CornerY;
        InnerPoints.push_back(InnerX);
        InnerPoints.push_back(InnerY);

        double OuterX = ExtendedCornerX - x * OuterStepX;
        double OuterY = ExtendedCornerY;
        OuterPoints.push_back(OuterX);
        OuterPoints.push_back(OuterY);
        }

    // Left (Top-Bottom)
    for (x=0; x<=m_nRows; ++x)
        {
        double InnerX = OriginX;
        double InnerY = CornerY - x * InnerStepY;
        InnerPoints.push_back(InnerX);
        InnerPoints.push_back(InnerY);

        double OuterX = ExtendedOriginX;
        double OuterY = ExtendedCornerY - x * OuterStepY;
        OuterPoints.push_back(OuterX);
        OuterPoints.push_back(OuterY);
        }


    vector<double> srcPts(6);  // Three 2d pts
    vector<double> dstPts(6);  // Three 2d pts
    HFCPtr<HGF2DTransfoModel> pOuterModel (m_pOuterModel);
    HFCPtr<HGF2DTransfoModel> pInnerModel;

    for (x=0; x<InnerPoints.size()-2; x+=2)
        {
        // First triangle
        // build points to compute model
        srcPts[0] = InnerPoints[x];
        srcPts[1] = InnerPoints[x+1];

        pInnerModel = m_pInnerModel->GetModelFromCoordinate(srcPts[0], srcPts[1]);
        if (pInnerModel == NULL)
            pInnerModel = m_pInnerModel->GetModelFromCoordinateTol(srcPts[0], srcPts[1]);
        HASSERT(pInnerModel != NULL);
        if (pInnerModel!=NULL && pOuterModel!=NULL)
            {
            pInnerModel->ConvertDirect(srcPts[0], srcPts[1], &dstPts[0], &dstPts[1]);

            srcPts[2] = OuterPoints[x];
            srcPts[3] = OuterPoints[x+1];
            pOuterModel->ConvertDirect(srcPts[2], srcPts[3], &dstPts[2], &dstPts[3]);

            srcPts[4] = OuterPoints[x+2];
            srcPts[5] = OuterPoints[x+3];
            pOuterModel->ConvertDirect(srcPts[4], srcPts[5], &dstPts[4], &dstPts[5]);

            AddTriangleEntry(srcPts, dstPts);
            }



        // Second triangle
        // build points to compute model
        srcPts[0] = InnerPoints[x];
        srcPts[1] = InnerPoints[x+1];

        pInnerModel = m_pInnerModel->GetModelFromCoordinate(srcPts[0], srcPts[1]);
        if (pInnerModel == NULL)
            pInnerModel = m_pInnerModel->GetModelFromCoordinateTol(srcPts[0], srcPts[1]);
        HASSERT(pInnerModel != NULL);
        if (pInnerModel!=NULL && pOuterModel!=NULL)
            {
            pInnerModel->ConvertDirect(srcPts[0], srcPts[1], &dstPts[0], &dstPts[1]);

            srcPts[2] = OuterPoints[x+2];
            srcPts[3] = OuterPoints[x+3];
            pOuterModel->ConvertDirect(srcPts[2], srcPts[3], &dstPts[2], &dstPts[3]);

            srcPts[4] = InnerPoints[x+2];
            srcPts[5] = InnerPoints[x+3];

            pInnerModel = m_pInnerModel->GetModelFromCoordinate(srcPts[4], srcPts[5]);
            if (pInnerModel == NULL)
                pInnerModel = m_pInnerModel->GetModelFromCoordinateTol(srcPts[4], srcPts[5]);
            HASSERT(pInnerModel != NULL);
            if (pInnerModel != NULL)
                {
                pInnerModel->ConvertDirect(srcPts[4], srcPts[5], &dstPts[4], &dstPts[5]);

                AddTriangleEntry(srcPts, dstPts);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// ComputeDirectModelFromPoints
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModel::ComputeDirectModelFromPoints (vector<double> const& pi_srcPts,
                                                                            vector<double> const& pi_dstPts) const
    {
    double MyMat[4][4];
    double pt[18];

    for (int x=0; x<3; x++)
        {
        pt[x*6]   = pi_srcPts[x*2];
        pt[x*6+1] = pi_srcPts[x*2+1];
        pt[x*6+2] = 0.0;
        pt[x*6+3] = pi_dstPts[x*2];
        pt[x*6+4] = pi_dstPts[x*2+1];
        pt[x*6+5] = 0.0;
        }

    GetAffineTransfoMatrixFromScaleAndTiePts(MyMat, 18, pt);

    HFCMatrix<3, 3> affMatrix;
    affMatrix[0][0] = MyMat[0][0];
    affMatrix[0][1] = MyMat[0][1];
    affMatrix[0][2] = MyMat[0][3];
    affMatrix[1][0] = MyMat[1][0];
    affMatrix[1][1] = MyMat[1][1];
    affMatrix[1][2] = MyMat[1][3];
    affMatrix[2][0] = MyMat[3][0];
    affMatrix[2][1] = MyMat[3][1];
    affMatrix[2][2] = MyMat[3][3];

    // Create model
    HFCPtr<HGF2DTransfoModel> pAffModel = new HGF2DProjective(affMatrix);

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pAffModel->CreateSimplifiedModel();

    if (pSimplifiedModel != NULL)
        pAffModel = pSimplifiedModel;

    return pAffModel;
    }

//-----------------------------------------------------------------------------
// Dump
//-----------------------------------------------------------------------------
void HGF2DBoundaryModel::Dump(ofstream& outStream) const
    {
    if (m_pInverseRegions == NULL)
        {
        GenerateExtendedMesh();
        }

    m_pDirectRegions->Dump(outStream);
    m_pInverseRegions->Dump(outStream);
    }
