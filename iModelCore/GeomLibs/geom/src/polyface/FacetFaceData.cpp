/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/FacetFaceData.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
FacetFaceData::FacetFaceData(){Init ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void FacetFaceData::Init ()
    {
    m_paramRange.Init ();
    m_paramDistanceRange.Init ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void FacetFaceData::ScaleDistances (double distanceScale)
    {
    m_paramDistanceRange.low.Scale (distanceScale);
    m_paramDistanceRange.high.Scale (distanceScale);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void FacetFaceData::ConvertParamToDistance (DPoint2dR distanceParam, DPoint2dCR param) const
    {
    DPoint2d        paramDelta;

    paramDelta.DifferenceOf (m_paramRange.high, m_paramRange.low);

    distanceParam.x = (0.0 == paramDelta.x) ? param.x  : (m_paramDistanceRange.low.x + (param.x  - m_paramRange.low.x) * (m_paramDistanceRange.high.x - m_paramDistanceRange.low.x) / paramDelta.x);
    distanceParam.y = (0.0 == paramDelta.y) ? param.y  : (m_paramDistanceRange.low.y + (param.y  - m_paramRange.low.y) * (m_paramDistanceRange.high.y - m_paramDistanceRange.low.y) / paramDelta.y);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void FacetFaceData::ConvertParamToNormalized (DPoint2dR normalizedParam, DPoint2dCR param) const
    {
    DPoint2d        paramDelta;

    paramDelta.DifferenceOf (m_paramRange.high, m_paramRange.low);

    normalizedParam.x = (0.0 == paramDelta.x) ? param.x  : ((param.x  - m_paramRange.low.x) / paramDelta.x);
    normalizedParam.y = (0.0 == paramDelta.y) ? param.y  : ((param.y  - m_paramRange.low.y) / paramDelta.y);
    }
#ifndef MinimalRefMethods
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void FacetFaceData::SetParamDistanceRangeFromNewFaceData (PolyfaceHeaderR polyface, size_t endIndex)
    {
    FacetFaceData       faceData;

    DPoint2d            dSTotal, dSSquaredTotal;
    int                 aveTotal = 0;

    size_t firstIndex = polyface.FaceIndex().size ();
    if (0 == endIndex)
        endIndex = polyface.PointIndex().size();

    dSTotal.Zero();
    dSSquaredTotal.Zero();

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (polyface, true);

    if (!visitor->MoveToFacetByReadIndex (firstIndex))
        {
        BeAssert (false);
        return;
        }
    do
        {
        size_t              numPointsThisPolygon = visitor->NumEdgesThisFace();
        DPoint3dCP          visitorPoints, trianglePoints[3] = {NULL, NULL, NULL};
        DPoint2dCP          visitorParams, triangleParams[3] = {NULL, NULL, NULL};


        if (NULL == (visitorPoints = visitor->GetPointCP()) ||
            NULL == (visitorParams = visitor->GetParamCP()))
            {
            BeAssert (false);
            return;
            }

        for (size_t k=0; k<numPointsThisPolygon; k++)
            {
            trianglePoints[2] = visitorPoints + k;
            triangleParams[2] = visitorParams + k;

            if (k > 1)
                {
                DPoint2d        dUV0, dUV1;
                DPoint3d        delta0, delta1;

                dUV0.DifferenceOf   (*triangleParams[0], *triangleParams[1]);
                dUV1.DifferenceOf   (*triangleParams[1], *triangleParams[2]);
                delta0.DifferenceOf (*trianglePoints[0], *trianglePoints[1]);
                delta1.DifferenceOf (*trianglePoints[1], *trianglePoints[2]);

                // W = point on triangle in xyz
                // Wu = dW/du
                // Wv = dW/dv
                // Local variation of u,v generates movement on plane with these two vectors.
                //  Solve x, y, z directions independently, viz
                //      [delta0.x] = [dUV0.x  dUV0.y][Wu.x]
                //      [delta1.x]   [dUV1.x  dUV1.y][Wv.x]
                double      uvCross = fabs (dUV0.x * dUV1.y - dUV1.x * dUV0.y);
                if (uvCross)
                    {
                    DPoint3d        dwDu, dwDv;
                    DPoint2d        dS;

                    dwDu.SumOf (delta0, dUV1.y, delta1, - dUV0.y);
                    dwDv.SumOf (delta1, dUV0.x, delta0, - dUV1.x);
                    dS.x = dwDu.Magnitude() / uvCross;
                    dS.y = dwDv.Magnitude() / uvCross;

                    dSTotal.x += dS.x;
                    dSTotal.y += dS.y;
                    dSSquaredTotal.x += dS.x * dS.x;
                    dSSquaredTotal.y += dS.y * dS.y;
                    aveTotal++;
                    }
                }
            triangleParams[0] = triangleParams[1];
            triangleParams[1] = triangleParams[2];
            trianglePoints[0] = trianglePoints[1];
            trianglePoints[1] = trianglePoints[2];
            }
        } while (visitor->AdvanceToNextFace() && visitor->GetReadIndex() < endIndex);

    
    if (0.0 != aveTotal)
        {
        DPoint2d            dS, standardDeviation;

        dS.Scale (dSTotal, 1.0 / aveTotal);

        standardDeviation.x = sqrt (fabs ((dSSquaredTotal.x / aveTotal) - dS.x * dS.x));
        standardDeviation.y = sqrt (fabs ((dSSquaredTotal.y / aveTotal) - dS.y * dS.y));

        // TR# 268980 - Add standard deviation to match QV....
        m_paramDistanceRange.low.Init (0.0, 0.0);
        m_paramDistanceRange.high.x = (dS.x + standardDeviation.x) * (m_paramRange.high.x - m_paramRange.low.x);
        m_paramDistanceRange.high.y = (dS.y + standardDeviation.y) * (m_paramRange.high.y - m_paramRange.low.y);
        }
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
