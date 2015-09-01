//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DPieceWiseModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGF2DPieceWiseModel.h>

//-----------------------------------------------------------------------------
// HGF2DPieceWiseModel
//-----------------------------------------------------------------------------
HGF2DPieceWiseModel::HGF2DPieceWiseModel()
    {
    }

//-----------------------------------------------------------------------------
// ~HGF2DPieceWiseModel
//-----------------------------------------------------------------------------
HGF2DPieceWiseModel::~HGF2DPieceWiseModel()
    {
    }

//-----------------------------------------------------------------------------
// ScaleExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DPieceWiseModel::ScaleExtent (HGF2DLiteExtent const& pi_rExtent,
                                                  double                 pi_Scale)
    {
    double CenterX ((pi_rExtent.GetXMin() + pi_rExtent.GetXMax()) / 2.0);
    double CenterY ((pi_rExtent.GetYMin() + pi_rExtent.GetYMax()) / 2.0);

    double PointsX[4];
    double PointsY[4];

    PointsX[0] = pi_rExtent.GetXMin();
    PointsY[0] = pi_rExtent.GetYMin();
    PointsX[1] = pi_rExtent.GetXMin();
    PointsY[1] = pi_rExtent.GetYMax();
    PointsX[2] = pi_rExtent.GetXMax();
    PointsY[2] = pi_rExtent.GetYMax();
    PointsX[3] = pi_rExtent.GetXMax();
    PointsY[3] = pi_rExtent.GetYMin();


    PointsX[0] = ((PointsX[0] - CenterX) * pi_Scale) + CenterX;
    PointsY[0] = ((PointsY[0] - CenterY) * pi_Scale) + CenterY;
    PointsX[1] = ((PointsX[1] - CenterX) * pi_Scale) + CenterX;
    PointsY[1] = ((PointsY[1] - CenterY) * pi_Scale) + CenterY;
    PointsX[2] = ((PointsX[2] - CenterX) * pi_Scale) + CenterX;
    PointsY[2] = ((PointsY[2] - CenterY) * pi_Scale) + CenterY;
    PointsX[3] = ((PointsX[3] - CenterX) * pi_Scale) + CenterX;
    PointsY[3] = ((PointsY[3] - CenterY) * pi_Scale) + CenterY;

    return HGF2DLiteExtent(MIN(MIN(PointsX[0], PointsX[1]), MIN(PointsX[2], PointsX[3])),
                           MIN(MIN(PointsY[0], PointsY[1]), MIN(PointsY[2], PointsY[3])),
                           MAX(MAX(PointsX[0], PointsX[1]), MAX(PointsX[2], PointsX[3])),
                           MAX(MAX(PointsY[0], PointsY[1]), MAX(PointsY[2], PointsY[3])));
    }

//-----------------------------------------------------------------------------
// ConvertLiteExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DPieceWiseModel::ConvertLiteExtent(const HGF2DLiteExtent&   pi_rExtent,
                                                       const HGF2DTransfoModel& pi_rpModel,
                                                       const ChannelMode        pi_channel)
    {
    double corners_x[4];
    double corners_y[4];

    corners_x[0] = pi_rExtent.GetXMin();
    corners_y[0] = pi_rExtent.GetYMin();
    corners_x[1] = pi_rExtent.GetXMax();
    corners_y[1] = pi_rExtent.GetYMin();
    corners_x[2] = pi_rExtent.GetXMax();
    corners_y[2] = pi_rExtent.GetYMax();
    corners_x[3] = pi_rExtent.GetXMin();
    corners_y[3] = pi_rExtent.GetYMax();


    if (pi_channel == DIRECT)
        {
        pi_rpModel.ConvertDirect(&corners_x[0], &corners_y[0]);
        pi_rpModel.ConvertDirect(&corners_x[1], &corners_y[1]);
        pi_rpModel.ConvertDirect(&corners_x[2], &corners_y[2]);
        pi_rpModel.ConvertDirect(&corners_x[3], &corners_y[3]);
        }
    else
        {
        pi_rpModel.ConvertInverse(&corners_x[0], &corners_y[0]);
        pi_rpModel.ConvertInverse(&corners_x[1], &corners_y[1]);
        pi_rpModel.ConvertInverse(&corners_x[2], &corners_y[2]);
        pi_rpModel.ConvertInverse(&corners_x[3], &corners_y[3]);
        }

    double OriginX = MIN(MIN(corners_x[0], corners_x[1]), MIN(corners_x[2], corners_x[3]));
    double OriginY = MIN(MIN(corners_y[0], corners_y[1]), MIN(corners_y[2], corners_y[3]));
    double CornerX = MAX(MAX(corners_x[0], corners_x[1]), MAX(corners_x[2], corners_x[3]));
    double CornerY = MAX(MAX(corners_y[0], corners_y[1]), MAX(corners_y[2], corners_y[3]));

    return HGF2DLiteExtent(OriginX, OriginY, CornerX, CornerY);
    }

//-----------------------------------------------------------------------------
// ConvertLiteExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DPieceWiseModel::ConvertLiteExtent(const HGF2DLiteExtent&   pi_rExtent,
                                                       uint32_t                 nCols,
                                                       uint32_t                 nRows,
                                                       const HGF2DTransfoModel& pi_rpModel,
                                                       const ChannelMode        pi_channel)
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


    vector<double> ConvertedPoints;
    ConvertedPoints.reserve((nCols * 2 + nRows * 2) * 2);
    if (pi_channel == DIRECT)
        {
        for (x=0; x<Points.size(); x+=2)
            {
            try
                {
                pi_rpModel.ConvertDirect(&Points[x], &Points[x+1]);
                ConvertedPoints.push_back(Points[x]);
                ConvertedPoints.push_back(Points[x+1]);
                }
            catch (...)
                {
                //Cannot convert...skip that point
                }
            }
        }
    else
        {
        for (x=0; x<Points.size(); x+=2)
            {
            try
                {
                pi_rpModel.ConvertInverse(&Points[x], &Points[x+1]);
                ConvertedPoints.push_back(Points[x]);
                ConvertedPoints.push_back(Points[x+1]);
                }
            catch (...)
                {
                //Cannot convert...skip that point
                }
            }
        }


    double OriginX = ConvertedPoints[0];
    double OriginY = ConvertedPoints[1];
    double CornerX = ConvertedPoints[0];
    double CornerY = ConvertedPoints[1];

    for (x=2; x<ConvertedPoints.size(); x+=2)
        {
        OriginX = MIN(OriginX, ConvertedPoints[x]);
        OriginY = MIN(OriginY, ConvertedPoints[x+1]);
        CornerX = MAX(CornerX, ConvertedPoints[x]);
        CornerY = MAX(CornerY, ConvertedPoints[x+1]);
        }

    return HGF2DLiteExtent(OriginX, OriginY, CornerX, CornerY);
    }

