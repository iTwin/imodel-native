//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DGridModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DGridModel.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGFException.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

//-----------------------------------------------------------------------------
// HGF2DGridModel
//-----------------------------------------------------------------------------
HGF2DGridModel::HGF2DGridModel(const HGF2DTransfoModel&   pi_pModel,
                               const HGF2DLiteExtent&     pi_directExtent,
                               uint32_t                   pi_numOfTilesX,
                               uint32_t                   pi_numOfTilesY)
    :m_pAdaptedTransfoModel(pi_pModel.Clone()),
     m_nColumns(pi_numOfTilesX),
     m_nRows(pi_numOfTilesY),
     m_directExtent(pi_directExtent),
     m_inverseExtent(ConvertLiteExtent(pi_directExtent, pi_numOfTilesX, pi_numOfTilesY, pi_pModel, DIRECT)),
     m_arrayOfDirectModels(m_nRows* m_nColumns),
     m_inverseRegions(ScaleExtent(m_inverseExtent, 1.01))
    {
    CompleteInverseRegions();
    }

//-----------------------------------------------------------------------------
// HGF2DGridModel
//-----------------------------------------------------------------------------
HGF2DGridModel::HGF2DGridModel  (const HGF2DLiteExtent&                  pi_directExtent,
                                 uint32_t                                pi_numOfTilesX,
                                 uint32_t                                pi_numOfTilesY,
                                 const list<HFCPtr<HGF2DTransfoModel> >& pi_modelList)
    :m_pAdaptedTransfoModel(new HGF2DIdentity()),
     m_nColumns(pi_numOfTilesX),
     m_nRows(pi_numOfTilesY),
     m_directExtent(pi_directExtent),
     m_inverseExtent(0.0, 0.0, 1.0, 1.0),
     m_arrayOfDirectModels(m_nRows* m_nColumns),
     m_inverseRegions(ScaleExtent(m_inverseExtent, 1.01))
    {
    HPRECONDITION(pi_numOfTilesX > 0 && pi_numOfTilesY > 0);
    HPRECONDITION(pi_modelList.size() == pi_numOfTilesX * pi_numOfTilesY);

    // Copy direct models
    copy(pi_modelList.begin(), pi_modelList.end(), m_arrayOfDirectModels.begin());

    m_inverseExtent = ConvertLiteExtentWithGridModel(pi_directExtent, pi_numOfTilesX, pi_numOfTilesY);
    m_inverseRegions = QuadrilateralFacetQuadTree(ScaleExtent(m_inverseExtent, 1.01));

    CompleteInverseRegions();
    }

//-----------------------------------------------------------------------------
// HGF2DGridModel
//-----------------------------------------------------------------------------
HGF2DGridModel::HGF2DGridModel(HGF2DGridModel const& pi_rObj)
    :m_pAdaptedTransfoModel(pi_rObj.m_pAdaptedTransfoModel->Clone()),
     m_nColumns(pi_rObj.m_nColumns),
     m_nRows(pi_rObj.m_nRows),
     m_directExtent(pi_rObj.m_directExtent),
     m_inverseExtent(pi_rObj.m_inverseExtent),
     m_arrayOfDirectModels(pi_rObj.m_arrayOfDirectModels),
     m_inverseRegions(pi_rObj.m_inverseRegions)
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DGridModel
//-----------------------------------------------------------------------------
HGF2DGridModel::~HGF2DGridModel()
    {
    }

//-----------------------------------------------------------------------------
// GetModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::GetModelFromCoordinate(double pi_X, double pi_Y) const
    {
    // Check if coordinate is part of current extent
    if (m_directExtent.IsPointIn(HGF2DPosition(pi_X, pi_Y)))
        {
        // Calculate the row and column specific to coordinate
        uint32_t row, column;
        ComputeRowColumnFromDirectCoordinate(&row, &column, pi_X, pi_Y);

        return(GetModelFromRowColumn(row, column));
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// GetModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::GetModelFromCoordinateTol(double pi_X, double pi_Y) const
    {
    // Check if coordinate is part of current extent
    if (m_directExtent.IsPointOutterIn(HGF2DPosition(pi_X, pi_Y)))
        {
        // Calculate the row and column specific to coordinate
        uint32_t row, column;
        ComputeRowColumnFromDirectCoordinate(&row, &column, pi_X, pi_Y);

        return(GetModelFromRowColumn(row, column));
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// GetModelFromInverseCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::GetModelFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    // First, search in Quadrilateres
    QuadrilateralFacet const* pEntry = m_inverseRegions.QuerySingle(pi_X, pi_Y);

    if (NULL != pEntry)
        {
        return pEntry->GetTransfoModel();
        }

    return CreateModelFromInverseCoordinate(pi_X, pi_Y);
    }

//-----------------------------------------------------------------------------
// GetFacetFromCoordinate
//-----------------------------------------------------------------------------
QuadrilateralFacet* HGF2DGridModel::GetFacetFromCoordinate(double pi_X, double pi_Y) const
    {
    HAutoPtr<QuadrilateralFacet> pFacet(NULL);

    // Check if coordinate is part of current extent
    if (m_directExtent.IsPointIn(HGF2DPosition(pi_X, pi_Y)))
        {
        // Calculate the row and column specific to coordinate
        uint32_t row, column;
        ComputeRowColumnFromDirectCoordinate(&row, &column, pi_X, pi_Y);

        HGF2DLiteQuadrilateral myShape(CreateQuadrilateralFromRowRol(row, column));

        HFCPtr<HGF2DTransfoModel> pModel(GetModelFromRowColumn(row, column));
        pFacet = new QuadrilateralFacet(pModel, myShape);

        return pFacet.release();
        }

    return pFacet.release();
    }

//-----------------------------------------------------------------------------
// GetFacetFromInverseCoordinate
//-----------------------------------------------------------------------------
QuadrilateralFacet* HGF2DGridModel::GetFacetFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    QuadrilateralFacet const (* pEntry) (m_inverseRegions.QuerySingle(pi_X, pi_Y));
    HAutoPtr<QuadrilateralFacet> pFacet(NULL);

    if (NULL != pEntry)
        {
        pFacet = new QuadrilateralFacet(*pEntry);
        return pFacet.release();
        }

    if (CreateModelFromInverseCoordinate(pi_X, pi_Y) != NULL)
        {
        // Creation succeeded
        pEntry = m_inverseRegions.QuerySingle(pi_X, pi_Y);

        if (NULL != pEntry)
            {
            pFacet = new QuadrilateralFacet(*pEntry);
            return pFacet.release();
            }
        }

    return pFacet.release();
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DGridModel::GetExtent() const
    {
    return m_directExtent;
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DGridModel::GetInverseExtent() const
    {
    return m_inverseExtent;
    }

//-----------------------------------------------------------------------------
// GetAdaptedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::GetAdaptedModel() const
    {
    return m_pAdaptedTransfoModel;
    }

//-----------------------------------------------------------------------------
// GetModelFromRowColumnModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::GetModelFromRowColumn(uint32_t pi_Row, uint32_t pi_Column) const
    {
    HPRECONDITION(pi_Row < m_nRows && pi_Column < m_nColumns);
    HPRECONDITION(pi_Row >= 0 && pi_Column >= 0);

    uint32_t Index = pi_Row * m_nColumns + pi_Column;

    if (m_arrayOfDirectModels[Index] == NULL)
        m_arrayOfDirectModels[Index] = CreateModelFromRowColumn(pi_Row, pi_Column);

    return m_arrayOfDirectModels[Index];
    }

//-----------------------------------------------------------------------------
// CreateModelFromRowColumn
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::CreateModelFromRowColumn(uint32_t pi_Row, uint32_t pi_Column) const
    {
    HPRECONDITION(pi_Row < m_nRows && pi_Column < m_nColumns);
    HPRECONDITION(pi_Row >= 0 && pi_Column >= 0);

    uint32_t Index (ComputeModelIndexFromRowColumn(pi_Row, pi_Column));
    HASSERT (m_arrayOfDirectModels[Index] == NULL);

    HGF2DLiteExtent DirectExtent(CreateModelExtentFromRowRol(pi_Row, pi_Column));

    return CreateModelFromExtent(DirectExtent);
    }

//-----------------------------------------------------------------------------
// CreateModelFromExtent
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::CreateModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const
    {
    // try, catch to prevent from gcoord library throw
    try
        {
        vector <double> srcPoints(8);
        vector <double> dstPoints(8);

        // Compute src pts
        srcPoints[0] = pi_rExtent.GetXMin();
        srcPoints[1] = pi_rExtent.GetYMin();
        srcPoints[2] = pi_rExtent.GetXMax();
        srcPoints[3] = pi_rExtent.GetYMin();
        srcPoints[4] = pi_rExtent.GetXMax();
        srcPoints[5] = pi_rExtent.GetYMax();
        srcPoints[6] = pi_rExtent.GetXMin();
        srcPoints[7] = pi_rExtent.GetYMax();

        // Compute destination points
        m_pAdaptedTransfoModel->ConvertDirect(srcPoints[0], srcPoints[1], &dstPoints[0], &dstPoints[1]);
        m_pAdaptedTransfoModel->ConvertDirect(srcPoints[2], srcPoints[3], &dstPoints[2], &dstPoints[3]);
        m_pAdaptedTransfoModel->ConvertDirect(srcPoints[4], srcPoints[5], &dstPoints[4], &dstPoints[5]);
        m_pAdaptedTransfoModel->ConvertDirect(srcPoints[6], srcPoints[7], &dstPoints[6], &dstPoints[7]);

        HFCPtr<HGF2DTransfoModel> perspectiveModel (ComputeProjectionModel(srcPoints, dstPoints));

        return perspectiveModel;
        }
	catch(HGFmzGCoordException&)
	{
	}


    return NULL;
    }

//-----------------------------------------------------------------------------
// CreateModelExtentFromRowRol
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DGridModel::CreateModelExtentFromRowRol(uint32_t pi_Row, uint32_t pi_Col) const
    {
    double Width (GetDirectGridWidth());
    double Height (GetDirectGridHeight());

    HGF2DLiteExtent MyModelExtent(m_directExtent.GetXMin() + (pi_Col * Width),
                                  m_directExtent.GetYMin() + (pi_Row * Height),
                                  m_directExtent.GetXMin() + ((pi_Col + 1) * Width),
                                  m_directExtent.GetYMin() + ((pi_Row + 1) * Height));

    return MyModelExtent;
    }

//-----------------------------------------------------------------------------
// CreateQuadrilateralFromRowRol
//-----------------------------------------------------------------------------
HGF2DLiteQuadrilateral HGF2DGridModel::CreateQuadrilateralFromRowRol(uint32_t pi_Row, uint32_t pi_Col) const
    {
    HGF2DLiteExtent MyModelExtent(CreateModelExtentFromRowRol(pi_Row, pi_Col));

    return HGF2DLiteQuadrilateral ( MyModelExtent.GetXMin(), MyModelExtent.GetYMin(),
                                    MyModelExtent.GetXMax(), MyModelExtent.GetYMin(),
                                    MyModelExtent.GetXMax(), MyModelExtent.GetYMax(),
                                    MyModelExtent.GetXMin(), MyModelExtent.GetYMax());
    }

//-----------------------------------------------------------------------------
// CreateModelFromInverseCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::CreateModelFromInverseCoordinate(double pi_x, double pi_y) const
    {
    // try, catch to prevent from gcoord library throw
    try
        {
        double x, y;

        // convert to direct coordinate
        m_pAdaptedTransfoModel->ConvertInverse(pi_x, pi_y, &x, &y);

        if (m_directExtent.IsPointIn(HGF2DPosition(x, y)))
            {
            uint32_t row, column;
            ComputeRowColumnFromDirectCoordinate(&row, &column, x, y);

            HGF2DLiteExtent DirectExtent(CreateModelExtentFromRowRol(row, column));

            uint32_t index (ComputeModelIndexFromRowColumn(row, column));

            if (m_arrayOfDirectModels[index] == NULL)
                m_arrayOfDirectModels[index] = CreateModelFromExtent(DirectExtent);

            HGF2DLiteQuadrilateral InverseShape (ComputeInverseShapeFromDirectExtent(DirectExtent, m_arrayOfDirectModels[index]));

            // Add model to the quad tree
            QuadrilateralFacet entry(m_arrayOfDirectModels[index], InverseShape);

            if (!m_inverseRegions.AddItem(entry))
                {
                HASSERT(false);
                }

            return m_arrayOfDirectModels[index];
            }
        }
	catch(HGFmzGCoordException&)
	{
	}

    return NULL;
    }

//-----------------------------------------------------------------------------
// ComputeProjectionModel
// The points provided must not be equals or colinear.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DGridModel::ComputeProjectionModel (vector<double> const& srcPoints,
                                                                  vector<double> const& dstPoints) const

    {
    // Build the transformation from the grid to the unit cube.
    // The source points is a rectangle, the transformation is a stretch
    double scaleX (1.0/(srcPoints[2] - srcPoints[0]));
    double scaleY (1.0/(srcPoints[5] - srcPoints[3]));

    double Stretch[3][3];
    Stretch[0][0] = scaleX;
    Stretch[0][1] = 0.0;
    Stretch[0][2] = scaleX * -srcPoints[0];
    Stretch[1][0] = 0.0;
    Stretch[1][1] = scaleY;
    Stretch[1][2] = scaleY * -srcPoints[1];
    Stretch[2][0] = 0.0;
    Stretch[2][1] = 0.0;
    Stretch[2][2] = 1.0;

    // Build the projective transformation from the unit cube to the destination quadrilatere.
    double dx3 = dstPoints[0] - dstPoints[2] + dstPoints[4] - dstPoints[6];
    double dy3 = dstPoints[1] - dstPoints[3] + dstPoints[5] - dstPoints[7];

    double Proj[3][3];
    Proj[2][2] = 1.0;

    double dx1 = dstPoints[2] - dstPoints[4];
    double dx2 = dstPoints[6] - dstPoints[4];
    double dy1 = dstPoints[3] - dstPoints[5];
    double dy2 = dstPoints[7] - dstPoints[5];

    double denominator = (dx1 * dy2 - dy1 * dx2);
    if (denominator==0.0)
        {
        throw HGFDomainException();
        }

    // The transformation is projective
    Proj[2][0] = (dx3 * dy2 - dy3 * dx2) / denominator;
    Proj[2][1] = (dx1 * dy3 - dy1 * dx3) / denominator;
    Proj[0][0] = dstPoints[2] - dstPoints[0] + Proj[2][0] * dstPoints[2];
    Proj[0][1] = dstPoints[6] - dstPoints[0] + Proj[2][1] * dstPoints[6];
    Proj[0][2] = dstPoints[0];
    Proj[1][0] = dstPoints[3] - dstPoints[1] + Proj[2][0] * dstPoints[3];
    Proj[1][1] = dstPoints[7] - dstPoints[1] + Proj[2][1] * dstPoints[7];
    Proj[1][2] = dstPoints[1];

    // Combine both transformation
    // Grid -> Unit cube -> quadrilatere
    double Result[3][3];

    Result[0][0] = Proj[0][0] * Stretch[0][0];
    Result[0][1] = Proj[0][1] * Stretch[1][1];
    Result[0][2] = Proj[0][0] * Stretch[0][2] + Proj[0][1] * Stretch[1][2] + Proj[0][2];
    Result[1][0] = Proj[1][0] * Stretch[0][0];
    Result[1][1] = Proj[1][1] * Stretch[1][1];
    Result[1][2] = Proj[1][0] * Stretch[0][2] + Proj[1][1] * Stretch[1][2] + Proj[1][2];
    Result[2][0] = Proj[2][0] * Stretch[0][0];
    Result[2][1] = Proj[2][1] * Stretch[1][1];
    Result[2][2] = Proj[2][0] * Stretch[0][2] + Proj[2][1] * Stretch[1][2] + 1.0;

    HFCPtr<HGF2DTransfoModel> pModel (new HGF2DProjective(Result));

    return pModel;
    }

//-----------------------------------------------------------------------------
// ComputeInverseShapeFromDirectExtent
//-----------------------------------------------------------------------------
HGF2DLiteQuadrilateral HGF2DGridModel::ComputeInverseShapeFromDirectExtent(HGF2DLiteExtent&                  pi_rExtent,
                                                                           HFCPtr<HGF2DTransfoModel> const& pModel) const
    {
    if (pModel == NULL)
        {
        throw HGFDomainException();
        }

    // Create a vector for 5 points
    vector<double> Points(8);

    Points[0] = pi_rExtent.GetXMin();
    Points[1] = pi_rExtent.GetYMin();
    Points[2] = pi_rExtent.GetXMin();
    Points[3] = pi_rExtent.GetYMax();
    Points[4] = pi_rExtent.GetXMax();
    Points[5] = pi_rExtent.GetYMax();
    Points[6] = pi_rExtent.GetXMax();
    Points[7] = pi_rExtent.GetYMin();

    pModel->ConvertDirect(&Points[0], &Points[1]);
    pModel->ConvertDirect(&Points[2], &Points[3]);
    pModel->ConvertDirect(&Points[4], &Points[5]);
    pModel->ConvertDirect(&Points[6], &Points[7]);

    // Create a shape from the transformed Extent
    return HGF2DLiteQuadrilateral(Points[0],
                                  Points[1],
                                  Points[2],
                                  Points[3],
                                  Points[4],
                                  Points[5],
                                  Points[6],
                                  Points[7]);
    }

//-----------------------------------------------------------------------------
// ConvertLiteExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DGridModel::ConvertLiteExtentWithGridModel(const HGF2DLiteExtent&   pi_rExtent,
                                                               uint32_t                 nCols,
                                                               uint32_t                 nRows) const
    {
    double StepX = (pi_rExtent.GetXMax() - pi_rExtent.GetXMin()) / nCols;
    double StepY = (pi_rExtent.GetYMax() - pi_rExtent.GetYMin()) / nRows;
    uint32_t x;

    vector<double> Points;
    Points.reserve((nCols * 2 + nRows * 2) * 2);

    // Bottom (Left-Right)
    for (x=0; x<nCols; ++x)
        {
        double PtX = pi_rExtent.GetXMin() + x * StepX;
        double PtY = pi_rExtent.GetYMin();
        Points.push_back(PtX);
        Points.push_back(PtY);
        }

    // Right (Bottom-Top)
    for (x=0; x<nRows; ++x)
        {
        double PtX = pi_rExtent.GetXMax();
        double PtY = pi_rExtent.GetYMin() + x * StepY;
        Points.push_back(PtX);
        Points.push_back(PtY);
        }

    // Top (Right-Left)
    for (x=0; x<nCols; ++x)
        {
        double PtX = pi_rExtent.GetXMax() - x * StepX;
        double PtY = pi_rExtent.GetYMax();
        Points.push_back(PtX);
        Points.push_back(PtY);
        }

    // Left (Top-Bottom)
    for (x=0; x<nRows; ++x)
        {
        double PtX = pi_rExtent.GetXMin();
        double PtY = pi_rExtent.GetYMax() - x * StepY;
        Points.push_back(PtX);
        Points.push_back(PtY);
        }

    for (x=0; x<Points.size(); x+=2)
        {
        HFCPtr<HGF2DTransfoModel> pModel (GetModelFromCoordinate(Points[x], Points[x+1]));
        if (pModel == NULL)
            pModel = GetModelFromCoordinateTol(Points[x], Points[x+1]);

        HASSERT(pModel != NULL);
        pModel->ConvertDirect(&Points[x], &Points[x+1]);
        }


    double OriginX = Points[0];
    double OriginY = Points[1];
    double CornerX = Points[0];
    double CornerY = Points[1];

    for (x=2; x<Points.size(); x+=2)
        {
        OriginX = MIN(OriginX, Points[x]);
        OriginY = MIN(OriginY, Points[x+1]);
        CornerX = MAX(CornerX, Points[x]);
        CornerY = MAX(CornerY, Points[x+1]);
        }

    return HGF2DLiteExtent(OriginX, OriginY, CornerX, CornerY);
    }


//-----------------------------------------------------------------------------
// CompleteInverseRegions
//-----------------------------------------------------------------------------
void HGF2DGridModel::CompleteInverseRegions () const
    {
    for (uint32_t Row = 0; Row < m_nRows; ++Row)
        {
        for (uint32_t Col = 0; Col < m_nColumns; ++Col)
            {
            // Compute the shape for this model
            HGF2DLiteExtent DirectExtent (CreateModelExtentFromRowRol(Row, Col));
            HGF2DLiteQuadrilateral InverseShape (ComputeInverseShapeFromDirectExtent(DirectExtent, GetModelFromRowColumn(Row, Col)));

            // Add model to the quad tree
            QuadrilateralFacet entry(GetModelFromRowColumn(Row, Col), InverseShape);
            if (!m_inverseRegions.AddItem(entry))
                {
                HASSERT(false);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  1/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void HGF2DGridModel::ConvertDirect(double pi_XIn, double pi_YIn, double*   po_pXOut, double*   po_pYOut) const
    {
    HFCPtr<HGF2DTransfoModel> pModel (GetModelFromCoordinate(pi_XIn, pi_YIn));

    if (pModel == NULL)
        {
        throw HGFDomainException();
        }

    pModel->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Dump
//-----------------------------------------------------------------------------
void HGF2DGridModel::Dump(ofstream& outStream) const
    {
    CompleteInverseRegions();

    for (uint32_t Row = 0; Row < m_nRows; ++Row)
        {
        for (uint32_t Col = 0; Col < m_nColumns; ++Col)
            {
            // Direct
            HGF2DLiteExtent DirectExtent(CreateModelExtentFromRowRol(Row, Col));

            outStream << "place smartline" << endl;
            outStream.precision(20);
            outStream << "xy=" << DirectExtent.GetXMin() << "," << DirectExtent.GetYMin() << endl;
            outStream << "xy=" << DirectExtent.GetXMax() << "," << DirectExtent.GetYMin() << endl;
            outStream << "xy=" << DirectExtent.GetXMax() << "," << DirectExtent.GetYMax() << endl;
            outStream << "xy=" << DirectExtent.GetXMin() << "," << DirectExtent.GetYMax() << endl;
            outStream << "xy=" << DirectExtent.GetXMin() << "," << DirectExtent.GetYMin() << endl;
            }
        }
    m_inverseRegions.Dump(outStream);
    }
