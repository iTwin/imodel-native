//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DDCTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DDCTransfoModel
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


// The class declaration must be the last include file.
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

struct HDPoint3d
    {
    double x;
    double y;
    double z;
    };

//-----------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------
static void s_subtractPoint(HDPoint3d& po_rResult, HDPoint3d const& pi_rPoint1, HDPoint3d const& pi_rPoint2)
    {
    po_rResult.x = pi_rPoint1.x - pi_rPoint2.x;
    po_rResult.y = pi_rPoint1.y - pi_rPoint2.y;
    po_rResult.z = pi_rPoint1.z - pi_rPoint2.z;
    }

static double s_computeMagnitude(HDPoint3d const& pi_rVec)
    {
    return (sqrt (pi_rVec.x*pi_rVec.x + pi_rVec.y*pi_rVec.y + pi_rVec.z*pi_rVec.z));
    }

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DDCTransfoModel::HGF2DDCTransfoModel()
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HGF2DDCTransfoModel::~HGF2DDCTransfoModel()
    {
    }

//-----------------------------------------------------------------------------
// Projective transformation
//-----------------------------------------------------------------------------
uint32_t HGF2DDCTransfoModel::GetAlignTransfoMatrixFromScaleAndTiePts(double        po_pMatrix[4][4],
                                                                     unsigned short pi_NbVal_GeoTiePoint,
                                                                     const double* pi_pVal_GeoTiePoint)
    {
    // Code was moved from MS, this is equivalent to >> mdlMath_DUorEqual (value, 0.0). Which use 0.25 epsilon.
#define ALIGN_EQUAL_ZERO(v) HDOUBLE_EQUAL(v, 0.0, (0.25-DBL_EPSILON))
#define TOLERANCE_SineAngleZero 0.01

    // Align supports only 2 pairs of points.
    if(12 != pi_NbVal_GeoTiePoint)
        return TRF_ERROR_BAD_NUMBER_POINTS;

    HDPoint3d const* pGeoTiePoints = (HDPoint3d const*)pi_pVal_GeoTiePoint;

    HDPoint3d inDelta, outDelta;
    s_subtractPoint (inDelta, pGeoTiePoints[2], pGeoTiePoints[0]);
    s_subtractPoint (outDelta, pGeoTiePoints[3], pGeoTiePoints[1]);
    inDelta.z = outDelta.z = 0.0;

    double inMagnitude = s_computeMagnitude (inDelta);
    double outMagnitude = s_computeMagnitude (outDelta);

    if (ALIGN_EQUAL_ZERO(outMagnitude) || ALIGN_EQUAL_ZERO(inMagnitude))
        return H_ERROR;

    double xScale, yScale;

    if (fabs (inDelta.x  / inMagnitude)  < TOLERANCE_SineAngleZero ||
        fabs (outDelta.x / outMagnitude) < TOLERANCE_SineAngleZero)
        {
        if(ALIGN_EQUAL_ZERO(inDelta.y))
            return H_ERROR;

        // Destination is essentially vertical
        xScale = yScale = fabs (outDelta.y / inDelta.y);
        }
    else if (fabs (inDelta.y  / inMagnitude)  < TOLERANCE_SineAngleZero ||
             fabs (outDelta.y / outMagnitude) < TOLERANCE_SineAngleZero)
        {
        if(ALIGN_EQUAL_ZERO(inDelta.x))
            return H_ERROR;

        // Destination is essentially horizontal
        xScale = yScale = fabs (outDelta.x / inDelta.x);
        }
    else if (!ALIGN_EQUAL_ZERO (inDelta.x) && !ALIGN_EQUAL_ZERO (inDelta.y) &&
             !ALIGN_EQUAL_ZERO (outDelta.x) && !ALIGN_EQUAL_ZERO (outDelta.y))
        {
        // Pick which to scale about
        xScale = fabs (outDelta.x / inDelta.x);
        yScale = fabs (outDelta.y / inDelta.y);
        if (xScale > yScale)
            yScale = xScale;
        else
            xScale = yScale;
        }
    else
        {
        return H_ERROR;
        }

    memset(po_pMatrix, 0, sizeof(double)*4*4);
    po_pMatrix[0][0] = xScale;
    po_pMatrix[0][3] = pGeoTiePoints[1].x - xScale * pGeoTiePoints[0].x;
    po_pMatrix[1][1] = yScale;
    po_pMatrix[1][3] = pGeoTiePoints[1].y - yScale * pGeoTiePoints[0].y;
    po_pMatrix[2][2] = 1.0;
    po_pMatrix[2][3] = 0.0;
    po_pMatrix[3][3] = 1.0;

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// Projective transformation
//-----------------------------------------------------------------------------
uint32_t HGF2DDCTransfoModel::GetHelmertTransfoMatrixFromScaleAndTiePts(double          po_pMatrix[4][4],
                                                                       unsigned short  pi_NbVal_GeoTiePoint,
                                                                       const double*   pi_pVal_GeoTiePoint)
    {
    return GetTransfoMatrixFromTiePts(po_pMatrix, pi_NbVal_GeoTiePoint, pi_pVal_GeoTiePoint, TRANSFO_HELM_2D);
    }

//-----------------------------------------------------------------------------
// Projective transformation
//-----------------------------------------------------------------------------
uint32_t HGF2DDCTransfoModel::GetSimilitudeTransfoMatrixFromScaleAndTiePts(double           po_pMatrix[4][4],
                                                                          unsigned short   pi_NbVal_GeoTiePoint,
                                                                          const double*    pi_pVal_GeoTiePoint)
    {
    return GetTransfoMatrixFromTiePts(po_pMatrix, pi_NbVal_GeoTiePoint, pi_pVal_GeoTiePoint, TRANSFO_SIMI_2D);
    }

//-----------------------------------------------------------------------------
// Projective transformation
//-----------------------------------------------------------------------------
uint32_t HGF2DDCTransfoModel::GetAffineTransfoMatrixFromScaleAndTiePts(double           po_pMatrix[4][4],
                                                                      unsigned short   pi_NbVal_GeoTiePoint,
                                                                      const double*    pi_pVal_GeoTiePoint)
    {
    return GetTransfoMatrixFromTiePts(po_pMatrix, pi_NbVal_GeoTiePoint, pi_pVal_GeoTiePoint, TRANSFO_AFFI_2D);
    }

//-----------------------------------------------------------------------------
// Projective transformation
//-----------------------------------------------------------------------------
uint32_t HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(double           po_pMatrix[4][4],
                                                                          unsigned short   pi_NbVal_GeoTiePoint,
                                                                          const double*    pi_pVal_GeoTiePoint)
    {
    return GetTransfoMatrixFromTiePts(po_pMatrix, pi_NbVal_GeoTiePoint, pi_pVal_GeoTiePoint, TRANSFO_PROJ_2D);
    }
