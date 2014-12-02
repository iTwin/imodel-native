/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/LinearHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <Vu/VuApi.h>

enum ShapeVertexData
    {
    VERTEX_LINKAGE_Normal   = 0,
    VERTEX_LINKAGE_RGB      = 1,

    MAX_NVALUES             = 10,
    MAX_UVALUES             = 50,
    };

static double s_vertexDataValueScale = 32767.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    fenceStretchPoints
(
DPoint3dP       points,
int             npoints,
TransformInfoCR transform,
FenceParamsP    fp
)
    {
    if (0 == npoints)
        return ERROR;

    for (int i=0; i<npoints; ++i)
        {
        if (fp->PointInside (points[i]))
            transform.GetTransform ()->Multiply (points[i]);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::FenceStretch
(
EditElementHandleR  eeh,
TransformInfoCR     trans,
FenceParamsP        fp,
bool                canContainDisconnects
)
    {
    MSElementDescrP edP = eeh.GetElementDescrP (); // Make sure we have an element descriptor...

    if (!edP)
        return ERROR;

    PointVector pointVector;
    DPoint3dP   pointsDirect = NULL;
    int         nPointsDirect = 0;

    LineStringUtil::GetLineStringTransformPoints (&pointVector, &pointsDirect, &nPointsDirect, &edP->ElementR(), canContainDisconnects);

    DPoint3dP   points;
    int         npoints;

    if (pointsDirect)
        {
        points = pointsDirect;
        npoints = nPointsDirect;
        }
    else
        {
        points = &pointVector.front ();
        npoints = static_cast<int>(pointVector.size ());
        }

    if (0 == npoints)
        return ERROR;

    if (SUCCESS != fenceStretchPoints (points, npoints, trans, fp))
        return ERROR;

    return LineStringUtil::SetLineStringTransformPoints (&edP->ElementR(), points, npoints, canContainDisconnects);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
int             LineStringUtil::CyclicIndex (int index, int n)
    {
    if (n <= 1)
        return 0;

    while (index < 0)
        index += n;
    while (index >= n)
        index -= n;

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    EarlinLutz                      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ReplacePointAtIndex
(
DPoint3dP       pXYZArray,
int             numXYZ,
bool            bClosed,
DPoint3dCP      pXYZ,
int             index
)
    {
    if (numXYZ <= 0)
        return ERROR;

    if (bClosed)
        {
        index = CyclicIndex (index, numXYZ - 1);
        pXYZArray[index] = *pXYZ;
        pXYZArray[numXYZ - 1] = pXYZArray[0];
        }
    else
        {
        if (index < 0 || index >= numXYZ)
            return ERROR;

        pXYZArray[index] = *pXYZ;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    EarlinLutz                      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::InsertPointAtIndex
(
DPoint3dP       pXYZArray,
int*            pNumXYZ,
int             maxXYZ,
bool            bClosed,
DPoint3dCP      pXYZ,
int             index
)
    {
    int i;
    int numXYZ = *pNumXYZ;

    if (*pNumXYZ >= maxXYZ)
        return ERROR;

    if (bClosed)
        {
        numXYZ --;
        index = CyclicIndex (index, numXYZ);
        }
    else
        {
        if (index < 0 || index > numXYZ)
            return ERROR;
        }

    // Shift forward to make room ...
    for (i = numXYZ; i > index; i--)
        pXYZArray[i] = pXYZArray[i-1];

    pXYZArray[index] = *pXYZ;
    numXYZ ++;

    if (bClosed)
        pXYZArray[numXYZ] = pXYZArray[0];

    *pNumXYZ = numXYZ;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    EarlinLutz                      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::DeletePoints
(
DPoint3dP       pXYZArray,
int*            pNumXYZ,
bool            bClosed,
int             index,
int             numDelete
)
    {
    int numXYZ = *pNumXYZ;
    int numOut = 0;

    if (bClosed)
        {
        --numXYZ;

        if (numDelete >= numXYZ)
            {
            numOut = 0;
            }
        else
            {
            int iStartDelete = index;
            int iEndDelete   = index + numDelete - 1;

            if (iStartDelete > iEndDelete)
                std::swap (iStartDelete, iEndDelete);

            for (int i = 0; i<numXYZ; ++i)
                {
                if (i<iStartDelete || iEndDelete<i)
                    pXYZArray[numOut++] = pXYZArray[i];
                }

            pXYZArray[numOut++] = pXYZArray[0];
            }
        }
    else
        {
        if (index < 0)
            index = 0;

        // 0..index are already in place ...
        numOut = index;

        if (index >= numXYZ)
            numOut = numXYZ;

        for (int i = index + numDelete; i < numXYZ; i++)
            pXYZArray[numOut++] = pXYZArray[i];
        }

    *pNumXYZ = numOut;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ExtractToDPoint3dArray
(
DPoint3dP       outputDPoint3dArray,
UInt32*         nOutputDPoint2dArray,
DPoint2dCP      inputDPoint2dArray,
UInt32          nInputDPoint2dArray,
bool            checkForDisconnect
)
    {
    DPoint3dP   outputDPoint3d = outputDPoint3dArray;
    DPoint2dCP  inputDPoint2d  = inputDPoint2dArray;
    DPoint2dCP  inputDPoint2dX = inputDPoint2dArray + nInputDPoint2dArray;

    for (; inputDPoint2d < inputDPoint2dX; ++inputDPoint2d)
        {
        if (!checkForDisconnect || !inputDPoint2d->isDisconnect ())
            {
            outputDPoint3d->init (inputDPoint2d);
            ++outputDPoint3d;
            }
        }

    if (nOutputDPoint2dArray)
        *nOutputDPoint2dArray = static_cast<UInt32>(outputDPoint3d-outputDPoint3dArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::UpdateFromDPoint3dArray
(
DPoint2dP       outputDPoint2dArray,
UInt32          nOutputDPoint2dArray,
DPoint3dCP      inputDPoint3dArray,
bool            checkForDisconnect
)
    {
    DPoint3dCP  inputDPoint3d   = inputDPoint3dArray;
    DPoint2dP   outputDPoint2d  = outputDPoint2dArray;
    DPoint2dP   outputDPoint2dX = outputDPoint2dArray + nOutputDPoint2dArray;

    for (; outputDPoint2d < outputDPoint2dX; ++outputDPoint2d)
        {
        if (!checkForDisconnect || !outputDPoint2d->isDisconnect ())
            {
            outputDPoint2d->init (inputDPoint3d);
            ++inputDPoint3d;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ExtractToDPoint3dArray
(
DPoint3dP       outputDPoint3dArray,
UInt32*         nOutputDPoint3dArray,
DPoint3dCP      inputDPoint3dArray,
UInt32          nInputDPoint3dArray,
bool            checkForDisconnect
)
    {
    DPoint3dP   outputDPoint3d = outputDPoint3dArray;
    DPoint3dCP  inputDPoint3d  = inputDPoint3dArray;
    DPoint3dCP  inputDPoint3dX = inputDPoint3dArray + nInputDPoint3dArray;

    for (; inputDPoint3d < inputDPoint3dX; ++inputDPoint3d)
        {
        if (!checkForDisconnect || !inputDPoint3d->isDisconnect ())
            {
            *outputDPoint3d = *inputDPoint3d;
            ++outputDPoint3d;
            }
        }

    if (nOutputDPoint3dArray)
        *nOutputDPoint3dArray = static_cast<UInt32>(outputDPoint3d - outputDPoint3dArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::UpdateFromDPoint3dArray
(
DPoint3dP       outputDPoint3dArray,
int             nOutputDPoint3dArray,
DPoint3dCP      inputDPoint3dArray,
bool            checkForDisconnect
)
    {
    DPoint3dCP  inputDPoint3d = inputDPoint3dArray;
    DPoint3dP   outputDPoint3d  = outputDPoint3dArray;
    DPoint3dP   outputDPoint3dX = outputDPoint3dArray + nOutputDPoint3dArray;

    for (; outputDPoint3d < outputDPoint3dX; ++outputDPoint3d)
        {
        if (!checkForDisconnect || !outputDPoint3d->isDisconnect ())
            {
            *outputDPoint3d = *inputDPoint3d;
            ++inputDPoint3d;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringUtil::TransformQuat
(
double*         orientationP,
TransformCP     t,
int             nOrientation,
bool            is3d
)
    {
    RotMatrix   rotMatrix;

    for (int i=0; i<nOrientation; i++, orientationP += 4)
        {
        if (is3d)
            {
            rotMatrix.InitTransposedFromQuaternionWXYZ ( orientationP);
            rotMatrix.InitProduct(*t, rotMatrix);
            rotMatrix.SquareAndNormalizeColumns (rotMatrix, 0, 1);
            rotMatrix.GetQuaternion(orientationP, true);
            }
        else
            {
            rotMatrix.InitFromRowValuesXY ( orientationP);
            rotMatrix.InitProduct(*t, rotMatrix);
            rotMatrix.SquareAndNormalizeColumns (rotMatrix, 0, 1);
            rotMatrix.GetRowValuesXY(orientationP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringUtil::ExtractToVector
(
PointVector*    pointVector,
DPoint2dCP      inputDPoint2dArray,
UInt32          nInputDPoint2dArray,
bool            checkForDisconnect
)
    {
    DPoint2dCP  inputDPoint2d  = inputDPoint2dArray;
    DPoint2dCP  inputDPoint2dX = inputDPoint2dArray + nInputDPoint2dArray;

    for (; inputDPoint2d < inputDPoint2dX; ++inputDPoint2d)
        {
        if (!checkForDisconnect || !inputDPoint2d->isDisconnect ())
            {
            DPoint3d    p3;

            p3.init (inputDPoint2d);
            pointVector->push_back (p3);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringUtil::ExtractToVector
(
PointVector*    pointVector,
DPoint3dCP      inputDPoint3dArray,
int             nInputDPoint3dArray,
bool            checkForDisconnect
)
    {
    DPoint3dCP  inputDPoint3d  = inputDPoint3dArray;
    DPoint3dCP  inputDPoint3dX = inputDPoint3dArray + nInputDPoint3dArray;

    for (; inputDPoint3d < inputDPoint3dX; ++inputDPoint3d)
        {
        if (!checkForDisconnect || !inputDPoint3d->isDisconnect ())
            pointVector->push_back (*inputDPoint3d);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringUtil::GetLineStringTransformPoints
(
PointVector*    points,             // <= optional
DPoint3d**      pointsDirect,       // <= optional
int*            nPointsDirect,      // <= optional
DgnElementP      elP,
bool            canContainDisconnects
)
    {
    if (!elP->Is3d())
        {
        // 2-D:
        ExtractToVector (points, elP->ToLine_String_2d().vertice, elP->ToLine_String_2d().numverts, canContainDisconnects);
        }
    else
        {
        // 3-D:
        if (canContainDisconnects)
            {
            ExtractToVector (points, elP->ToLine_String_3d().vertice, elP->ToLine_String_3d().numverts, true);
            }
        else
            { // optimized for 3d w/ no disconnects: point to DPoint3d array in place
            *pointsDirect  = elP->ToLine_String_3dR().vertice;
            *nPointsDirect = elP->ToLine_String_3dR().numverts;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::SetLineStringTransformPoints
(
DgnElementP      elP,
DPoint3dCP      points,
int             npoints,
bool            canContainDisconnects
)
    {
    BeAssert (canContainDisconnects ? npoints <= (int) elP->ToLine_String_2d().numverts : npoints == elP->ToLine_String_2d().numverts);

    // 2-D:
    if (!elP->Is3d())
        return UpdateFromDPoint3dArray (elP->ToLine_String_2dR().vertice, elP->ToLine_String_2d().numverts, points, canContainDisconnects);

    // 3-D:
    if (canContainDisconnects)
        return UpdateFromDPoint3dArray (elP->ToLine_String_3dR().vertice, elP->ToLine_String_3d().numverts, points, true);

    // optimized for 3d w/ no disconnects: DPoint3d array was transformed in place
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     05/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineStringUtil::UpdateClosestPoint
(
DPoint3dR       pointQ,
double&         fractionQ,
double&         dxyQ,
int&            indexQ,
DPoint3dCR      point0,
DPoint3dCR      point1,
int             index,
DPoint3dCR      spacePoint,
DMatrix4dCR     worldToView,
bool            extend0,
bool            extend1
)
    {
    DSegment4d  segment;
    DPoint4d    spacePoint0;
    DPoint4d    viewPoint0, viewPoint1;
    double      fraction;
    bool        updated = false;

    bsiDSegment4d_initFromDPoint3d (&segment, &point0, &point1);
    bsiDSegment4d_transformDMatrix4d (&segment, &worldToView, &segment);

    spacePoint0.init (&spacePoint, 1.0);
    worldToView.multiply (&viewPoint0, &spacePoint0);

    // suppress z variation.  Doesn't affect fractions ...
    segment.point[0].z = segment.point[1].z = viewPoint0.z = 0.0;

    double      xyDistance;

    if (bsiDSegment4d_projectDPoint4d (&segment, &viewPoint1, &fraction, &viewPoint0))
        {
        if (fraction < 0.0 && !extend0)
            {
            fraction = 0.0;
            viewPoint1 = segment.point[0];
            }

        if (fraction > 1.0 && !extend1)
            {
            fraction = 1.0;
            viewPoint1 = segment.point[1];
            }
        }
    else
        {
        fraction = 0.0;
        viewPoint1 = segment.point[0];
        }

    xyDistance = bsiDPoint4d_realDistance (&viewPoint0, &viewPoint1);

    if (indexQ < 0 || xyDistance < dxyQ)
        {
        indexQ = index;
        fractionQ = fraction;
        pointQ.interpolate (&point0, fraction, &point1);
        dxyQ = xyDistance;
        updated = true;
        }

    return updated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::EncodeStringFraction
(
double*         pStringFraction,
int             edgeIndex,
double          edgeFraction,
int             numVertex
)
    {
    *pStringFraction = 0.0;

    // add additional deal for numVertex==2
    if(numVertex==2)   
        {
        *pStringFraction=edgeFraction;
        return SUCCESS;
        }

    if (numVertex < 1)
        return ERROR;

    int         numEdge = numVertex - 1;
    double      a;

    if (edgeIndex <= 0)
        {
        if (edgeFraction > 1.0)
            edgeFraction = 1.0;
        a = edgeFraction;
        }
    else if (edgeIndex < numEdge - 1)
        {
        if (edgeFraction > 1.0)
            edgeFraction = 1.0;
        if (edgeFraction < 0.0)
            edgeFraction = 0.0;
        a = edgeIndex + edgeFraction;
        }
    else
        {
        if (edgeFraction < 0.0)
            edgeFraction = 0.0;
        a = edgeIndex + edgeFraction;
        }

    *pStringFraction = a / (double)numEdge;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::DecodeStringFraction
(
int*            pPeriodSelect,
int*            pEdgeIndex,
double*         pEdgeFraction,
double          stringFraction,
int             numVertex,
bool            bClosed
)
    {
    *pPeriodSelect = 0;
    *pEdgeIndex = 0;
    *pEdgeFraction = 0.0;

    int periodSelect = 0;

    if (numVertex < 1)
        return ERROR;

    int numEdge = numVertex - 1;

    if (bClosed)
        {
        while (stringFraction < 0.0)
            {
            stringFraction += 1.0;
            periodSelect--;
            }
        while (stringFraction > 1.0)
            {
            stringFraction -= 1.0;
            periodSelect++;
            }
        }

    double e = (double)numEdge;
    double a = e * stringFraction;
    
    if (stringFraction <= 0.0)
        {
        *pEdgeIndex = 0;
        *pEdgeFraction = a;
        }
    else
        {
        *pEdgeIndex = (int)a;
        if (*pEdgeIndex >= numEdge)
            *pEdgeIndex = numEdge - 1;
        *pEdgeFraction = a - (double)(*pEdgeIndex);
        }

    *pPeriodSelect = periodSelect;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStringUtil::CyclicSegmentLength
(
DPoint3dCP      pXYZBuffer,
int             i0,
int             numSegment
)
    {
    while (i0 < 0)
        i0 += numSegment;

    while (i0 >= numSegment)
        i0 -= numSegment;

    int i1 = i0 + 1;

    if (i1 > numSegment)
        i1 = 0;
    
    return bsiDPoint3d_distance (&pXYZBuffer[i0], &pXYZBuffer[i1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::SignedDistanceAlong
(
double*         pLength, 
DPoint3dP       pXYZ,
int             numVertex,
bool            bClosed,
double          paramA,
double          paramB
)
    {
    int         numSegment = numVertex - 1;
    int         edgeIndexA, edgeIndexB;
    int         periodA, periodB;
    double      edgeFractionA, edgeFractionB;

    if (pLength)
        *pLength = 0.0;

    if (paramA > paramB)
        {
        StatusInt status = LineStringUtil::SignedDistanceAlong (pLength, pXYZ, numVertex, bClosed, paramB, paramA);

        if (pLength)
            *pLength *= -1.0;

        return status;
        }

    if (  SUCCESS != LineStringUtil::DecodeStringFraction (&periodA, &edgeIndexA, &edgeFractionA, paramA, numVertex, bClosed)
       || SUCCESS != LineStringUtil::DecodeStringFraction (&periodB, &edgeIndexB, &edgeFractionB, paramB, numVertex, bClosed))
        return ERROR;

    double length = 0.0;    

    // Both edge indices are within bounds.  Allow B to be wrapped.  The CyclicSegmentLength function will correct it.
    edgeIndexB += (periodB - periodA) * (numVertex - 1);

    if (edgeIndexB == edgeIndexA)
        {
        length = (edgeFractionB - edgeFractionA) * CyclicSegmentLength (pXYZ, edgeIndexA, numSegment);
        }
    else
        {
        double a;
        length = (1.0 - edgeFractionA) * (a = CyclicSegmentLength (pXYZ, edgeIndexA, numSegment));

        for (int edgeIndex = edgeIndexA + 1; edgeIndex < edgeIndexB; edgeIndex++)
            length += (a = CyclicSegmentLength (pXYZ, edgeIndex, numSegment));

        length += edgeFractionB * (a = CyclicSegmentLength (pXYZ, edgeIndexB, numSegment));
        }

    if (pLength)
        *pLength = length;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ClosestPoint
(
DPoint3dP       pXYZOut,
double*         pParamOut,
DPoint3dCP      pXYZArray,
int             numVertex,
bool            bExtend,
DPoint3dCP      pXYZIn
)
    {
    DPoint3d pointOn;
    if (numVertex < 0)
        return ERROR;
    double dMin, edgeFraction, fMin;
    int iMin;
    DPoint3d pointMin, edgePoint;
    DSegment3d segment;

    pointMin = pXYZArray[0];
    dMin = bsiDPoint3d_distance (&pointMin, pXYZIn);
    iMin = 0;
    fMin = 0.0;

    int numEdge = numVertex - 1;
    int iLast = numEdge - 1;
    for (int i = 0; i < numVertex - 1; i++)
        {
        segment.point[0] = pXYZArray[i];
        segment.point[1] = pXYZArray[i+1];

        bsiDSegment3d_projectPoint (&segment, &edgePoint, &edgeFraction, pXYZIn);
        // Pull left fraction back to zero EXCEPT on extendible initial segment ...
        if ((i > 0 || !bExtend) && edgeFraction < 0.0)
            {
            edgeFraction = 0.0;
            edgePoint    = segment.point[0];
            }
        // Pull right fraction back to one EXCEPT on extendible last segment ...
        if ((i < iLast || !bExtend) && edgeFraction > 1.0)
            {
            edgeFraction = 1.0;
            edgePoint    = segment.point[1];
            }

        double dEdge = bsiDPoint3d_distance (pXYZIn, &edgePoint);
        if (dEdge < dMin)
            {
            dMin = dEdge;
            iMin = i;
            fMin = edgeFraction;
            pointMin = edgePoint;
            }
        }

    pointOn = pointMin;

    double globalFraction;
    LineStringUtil::EncodeStringFraction (&globalFraction, iMin, fMin, numVertex);

    if (pXYZOut)
        *pXYZOut = pointOn;
    if (pParamOut)
        *pParamOut = globalFraction;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::InterpolateAndAppend
(
DPoint3dP       pXYZOut,
int*            pNumOut,
int             maxOut,
DPoint3dCP      pXYZArray,
int             numVertex,
bool            bClosed,
int             i0,
double          edgeFraction
)
    {
    int         i1;

    if (*pNumOut >= maxOut)
        return ERROR;

    if (!bClosed)
        {
        i1 = i0 + 1;

        if (i0 < 0 || i1 > numVertex)
            return ERROR;
        }
    else
        {
        int numSegment = numVertex - 1;

        while (i0 < 0)
            i0 += numSegment;

        while (i0 >= numSegment)
            i0 -= numSegment;

        i1 = i0 + 1;

        if (i1 >= numSegment)
            i1 = 0;
        }

    DPoint3d    xyz;

    if (edgeFraction == 0)
        xyz = pXYZArray[i0];
    else if (edgeFraction == 1)
        xyz = pXYZArray[i1];
    else 
        bsiDPoint3d_interpolate (&xyz, &pXYZArray[i0], edgeFraction, &pXYZArray[i1]);

    pXYZOut[*pNumOut] = xyz;
    *pNumOut += 1;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ExtractPartial
(
DPoint3dP       pXYZOut,
int*            pNumOut,
int             maxOut,
DPoint3dCP      pXYZArray,
int             numVertex,
bool            bClosed,
double          paramA,
double          paramB
)
    {
    int         edgeIndexA, edgeIndexB;
    int         periodA, periodB;
    double      edgeFractionA, edgeFractionB;

    *pNumOut = 0;

    if (paramA > paramB)
        {
        // Generate in reverse direction, swap output array in place.
        StatusInt status = LineStringUtil::ExtractPartial
                    (
                    pXYZOut, pNumOut, maxOut,
                    pXYZArray, numVertex, bClosed,
                    paramB, paramA
                    );

        if (SUCCESS == status)
            {
            int i = 0, j = *pNumOut - 1;
            for (; i < j; i++, j--)
                {
                DPoint3d xyz = pXYZOut[i];
                pXYZOut[i] = pXYZOut[j];
                pXYZOut[j] = xyz;
                }
            }

        return status;
        }

    if (  SUCCESS != LineStringUtil::DecodeStringFraction (&periodA, &edgeIndexA, &edgeFractionA, paramA, numVertex, bClosed)
       || SUCCESS != LineStringUtil::DecodeStringFraction (&periodB, &edgeIndexB, &edgeFractionB, paramB, numVertex, bClosed))
        return ERROR;

    // Both edge indices are within bounds.  Allow B to be wrapped.  The CyclicSegmentLength function will correct it.
    edgeIndexB += (periodB - periodA) * (numVertex - 1);

    // TODO: single point for matched parameters??
    // TODO: special case for paramB == paramA + 1??
    if (edgeIndexB == edgeIndexA)
        {
        if (SUCCESS != InterpolateAndAppend
                            (
                            pXYZOut, pNumOut, maxOut,
                            pXYZArray, numVertex, bClosed,
                            edgeIndexA, edgeFractionA
                            )
            || SUCCESS != InterpolateAndAppend
                            (
                            pXYZOut, pNumOut, maxOut,
                            pXYZArray, numVertex, bClosed,
                            edgeIndexA, edgeFractionB
                            ))
            return ERROR;
        }
    else
        {
        if (SUCCESS != InterpolateAndAppend (pXYZOut, pNumOut, maxOut, pXYZArray, numVertex, bClosed, edgeIndexA, edgeFractionA))
            return ERROR;

        for (int edgeIndex = edgeIndexA + 1; edgeIndex <= edgeIndexB; edgeIndex++)
            {
            if (SUCCESS != InterpolateAndAppend (pXYZOut, pNumOut, maxOut, pXYZArray, numVertex, bClosed, edgeIndex, 0.0))
                return ERROR;
            }

        if (edgeFractionB > 0.0)
            if (SUCCESS != InterpolateAndAppend (pXYZOut, pNumOut, maxOut, pXYZArray, numVertex, bClosed, edgeIndexB, edgeFractionB))
                return ERROR;
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::ClosestPointXY
(
DPoint3dP       pXYZOut,
double*         pParamOut,
double*         pXYDistanceOut,
DPoint3dCP      pXYZArray,
int             numVertex,
bool            bExtend,
DPoint3dCP      pXYZIn,
DMatrix4dCP     pWorldToView
)
    {
    if (numVertex < 0)
        return ERROR;

    int         iMin = 0;
    double      fMin = 0.0;
    DPoint3d    point0, point1, closePoint;

    point0 = pXYZArray[0];
    closePoint = point0;
    *pXYDistanceOut = DBL_MAX;

    for (int i = 1; i < numVertex ; ++i, point0 = point1)
        {
        point1 = pXYZArray[i];

        UpdateClosestPoint (closePoint, fMin, *pXYDistanceOut, iMin, point0, point1, i-1,
                            *pXYZIn, *pWorldToView,
                            bExtend && i == 1,              // extend at 0
                            bExtend && i == numVertex - 1); // extend at 1
        }

    double      f;

    LineStringUtil::EncodeStringFraction (&f, iMin, fMin, numVertex);

    if (pParamOut)
        *pParamOut = f;

    *pXYZOut = closePoint;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringUtil::PointAtSignedDistance
(
DPoint3dP       pXYZOut,
double*         pFractionOut,
DPoint3dCP      pXYZArray,
int             numVertex,
bool            bClosed,
double          fractionIn,
double          distance
)
    {
    int         edgeIndexA;
    double      edgeFractionA;
    double      fractionB;
    int         periodA;
    int         numSegment = numVertex - 1;

    if (SUCCESS != LineStringUtil::DecodeStringFraction(&periodA, &edgeIndexA, &edgeFractionA, fractionIn, numVertex, bClosed))
        return ERROR;

    int iEdge = edgeIndexA;
    fractionB = fractionIn;
    double d = 0.0;     // Accumulated distance
    double absDistance = fabs (distance);
    double di;
    double f;

    // Set up to walk forwards or backwards ..
    double entryFraction = 0.0;
    double direction = 1.0;
    int iLastEdge = numVertex - 2;
    int edgeStep = 1;
    if (distance < 0.0)
        {
        entryFraction = 1.0;
        direction = -1.0;
        iLastEdge = 0;
        edgeStep = -1;
        }

    if (bClosed)
        iLastEdge = edgeIndexA - edgeStep;  //  We will never reach this with
                                            //      steps by edgeStep !!
    di = CyclicSegmentLength (pXYZArray, iEdge, numSegment);

    // distance to exit from current edge ..
    double d0 = distance > 0.0 ? di * (1.0 - edgeFractionA) : di * edgeFractionA;

    if (d0 >= absDistance || iEdge == iLastEdge)
        {
        return bsiTrig_safeDivide (&f, absDistance, di, 0.0)
            && SUCCESS == LineStringUtil::EncodeStringFraction (pFractionOut,
                            iEdge, edgeFractionA + direction * f, numVertex)
            ? SUCCESS : ERROR;
        }

    // Walk along additional edges ...
    d = d0;
    while (d < fabs (distance))
        {
        iEdge += edgeStep;
        di = CyclicSegmentLength (pXYZArray, iEdge, numSegment);
        if (d + di >= absDistance || iEdge == iLastEdge)
            {
            double residual = absDistance - d;
            return bsiTrig_safeDivide (&f, residual, di, 0.0)
                && SUCCESS == LineStringUtil::EncodeStringFraction (pFractionOut,
                                    iEdge, entryFraction + direction * f, numVertex)
                ? SUCCESS : ERROR;
            }
        else
            d += di;
        }

    // hm.. can't really get here.
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_LINE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineHandler::_OnTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    StatusInt status;

    if ((status = T_Super::_OnTransform (elemHandle, trans)) != SUCCESS)
        return status;

    DgnElementP  el = elemHandle.GetElementP();

    BeAssert (LINE_ELM == el->GetLegacyType());

    if (el->Is3d())
        {
        trans.GetTransform()->Multiply (&el->ToLine_3dR().start, 2);
        }
    else
        {
        DPoint3d    points[2];

        LineStringUtil::ExtractToDPoint3dArray (points, NULL, &el->ToLine_2d().start, 2, false);
        trans.GetTransform()->Multiply (points, 2);
        LineStringUtil::UpdateFromDPoint3dArray (&el->ToLine_2dR().start, 2, points, false);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    DgnElementP  el = elemHandle.GetElementP();

    BeAssert (el->GetLegacyType() == LINE_ELM);

    if (el->Is3d())
        return fenceStretchPoints (&el->ToLine_3dR().start, 2, transform, fp);

    DPoint3d    points[2];

    LineStringUtil::ExtractToDPoint3dArray (points, NULL, &el->ToLine_2d().start, 2, false);
    fenceStretchPoints (points, 2, transform, fp);

    return LineStringUtil::UpdateFromDPoint3dArray (&el->ToLine_2dR().start, 2, points, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::GetStartEnd
(
ElementHandleCR source,
DPoint3dR       point0,
DPoint3dR       point1,
TransformCP     pElementToWorld
)
    {
    DgnElementCP  el = source.GetElementCP();

    if (Is3dElem (el))
        {
        point0 = el->ToLine_3d().start;
        point1 = el->ToLine_3d().end;
        }
    else
        {
        point0.x = el->ToLine_2d().start.x;
        point0.y = el->ToLine_2d().start.y;
        point0.z = 0.0;
        point1.x = el->ToLine_2d().end.x;
        point1.y = el->ToLine_2d().end.y;
        point1.z = 0.0;
        }

    if (pElementToWorld)
        {
        pElementToWorld->multiply (&point0);
        pElementToWorld->multiply (&point1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::SetStartEnd
(
EditElementHandleR source,
DPoint3dCR      point0,
DPoint3dCR      point1
)
    {
    DgnElementP  el = source.GetElementP();
    if (Is3dElem (el))
        {
        el->ToLine_3dR().start = point0;
        el->ToLine_3dR().end   = point1;
        }
    else
        {
        el->ToLine_3dR().start.x = point0.x;
        el->ToLine_3dR().start.y = point0.y;
        el->ToLine_3dR().end.x   = point1.x;
        el->ToLine_3dR().end.y   = point1.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_GetTransformOrigin (ElementHandleCR element, DPoint3dR origin)
    {
    DPoint3d    start, end;

    GetStartEnd (element, start, end);
    origin.interpolate (&start, 0.5, &end);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    DSegment3d  segment;
    DgnElementCP el = eh.GetElementCP ();

    if (Is3dElem (el))
        {
        segment.point[0] = el->ToLine_3d().start;
        segment.point[1] = el->ToLine_3d().end;
        }
    else
        {
        segment.point[0].init (&el->ToLine_2d().start);
        segment.point[1].init (&el->ToLine_2d().end);
        }

    curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateLine (segment));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line != path.HasSingleCurvePrimitive ())
        return ERROR;

    DSegment3d  segment = *path.front ()->GetLineCP ();
    DgnElementP  el = eeh.GetElementP ();

    if (LINE_ELM != el->GetLegacyType())
        return ERROR;

    if (Is3dElem (el))
        {
        el->ToLine_3dR().start = segment.point[0];
        el->ToLine_3dR().end   = segment.point[1];
        }
    else
        {
        el->ToLine_2dR().start.Init (segment.point[0]);
        el->ToLine_2dR().end.Init (segment.point[1]);
        }

    return eeh.GetDisplayHandler()->ValidateElementRange(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Line_3d) / 2);
    DataConvert::Points2dTo3d (&elm.ToLine_3dR().start, &eeh.GetElementCP ()->ToLine_2d().start, 2, elevation);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Line_2d) / 2);
    DataConvert::Points3dTo2d (&elm.ToLine_2dR().start, &eeh.GetElementCP ()->ToLine_3d().start, 2);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus LineHandler::_OnGeoCoordinateReprojection
(
EditElementHandleR                  source,
IGeoCoordinateReprojectionHelper&   reprojectionHelper,
bool                                inChain
)
    {
    ReprojectStatus status  = REPROJECT_Success;
    DgnElementP      el      = source.GetElementP();
    bool            is3d    = Is3dElem (el);

    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (settings->PostStrokeLinear())
        {
        DPoint3dP   points;
        int         numOutPoints;
        if (is3d)
            status = reprojectionHelper.ReprojectPointsMoreDetail (&points, &numOutPoints, &el->ToLine_3d().start, 2, reprojectionHelper.GetStrokeToleranceDestUors());
        else
            status = reprojectionHelper.ReprojectPointsMoreDetail2D (&points, &numOutPoints, &el->ToLine_2d().start, 2, reprojectionHelper.GetStrokeToleranceDestUors());

        if (REPROJECT_Success == status)
            status = GeoCoordinateReprojectionReplaceElement (source, points, numOutPoints, inChain, false);
        }
    else
        {
        if (Is3dElem (el))
            status = reprojectionHelper.ReprojectPoints (&el->ToLine_3dR().start, NULL, NULL, &el->ToLine_3d().start, 2);
        else
            status = reprojectionHelper.ReprojectPoints2D (&el->ToLine_2dR().start, NULL, NULL, &el->ToLine_2d().start, 2);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointStringHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_POINT_STRING_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PointStringHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    DgnElementCP el = eh.GetElementCP ();
    bvector<DPoint3d> points;

    points.resize (Is3dElem (el) ? el->ToLine_String_3d().numverts : el->ToLine_String_2d().numverts);

    for (size_t i = 0; i < points.size (); i++)
        {
        if (Is3dElem (el))
            points[i] = el->ToLine_String_3d().vertice [i];
        else
            points[i].init (el->ToLine_String_2d().vertice [i].x, el->ToLine_String_2d().vertice [i].y, 0.0);
        }

    if (el->IsHole() || points.size () < 2)
        {
        curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        curves->push_back (ICurvePrimitive::CreatePointString (&points[0], points.size ()));
        }
    else
        {
        curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        curves->push_back (ICurvePrimitive::CreateLineString (&points[0], points.size ()));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PointStringHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    bool disjoint;
    bvector<DPoint3d> const* points;

    switch (path.HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            disjoint = true;
            points = path.front ()->GetPointStringCP ();
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            disjoint = false;
            points = path.front ()->GetLineStringCP ();
            break;
            }

        default:
            return ERROR;
        }

    if (points->size () > MAX_VERTICES)
        return ERROR;

    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElement, Create methods uses SetElementDescr...
    if (SUCCESS != PointStringHandler::CreatePointStringElement (newEeh, &eeh, &points->front (), NULL, points->size (), disjoint, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
        return ERROR;

    eeh.ReplaceElement (newEeh.GetElementCP ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PointStringHandler::_OnTransform
(
EditElementHandleR eeh,
TransformInfoCR trans
)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    BeAssert (POINT_STRING_ELM == eeh.GetLegacyType());

    if (SUCCESS != LineStringBaseHandler::TransformLineString (eeh, trans, false))
        return ERROR;

    DgnElementP  elP = eeh.GetElementP ();
    int         numverts = elP->ToLine_String_2d().numverts;
    double*     orientationP = elP->Is3d() ? (double *) &elP->ToLine_String_3d().vertice[numverts] : (double *) &elP->ToLine_String_2d().vertice[numverts];

    // Handle the orientations
    LineStringUtil::TransformQuat (orientationP, trans.GetTransform(), numverts, elP->Is3d());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PointStringHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    BeAssert (POINT_STRING_ELM == elemHandle.GetLegacyType());

    return LineStringUtil::FenceStretch (elemHandle, transform, fp, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PointStringHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    DgnElementCP el = eh.GetElementCP ();

    if (!el->IsHole())
        {
        fp->ParseAcceptedElement (inside, outside, eh); // non-disjoint treated as linestring...

        return SUCCESS;
        }

    size_t                nPoints = (Is3dElem (el) ? el->ToLine_String_3d().numverts : el->ToLine_String_2d().numverts);
    bvector<DPoint3d> insidePoints;
    bvector<DPoint3d> outsidePoints;

    for (size_t i = 0; i < nPoints; i++)
        {
        DPoint3d    testPt;    

        if (Is3dElem (el))
            testPt = el->ToLine_String_3d().vertice[i];
        else
            testPt.Init (el->ToLine_String_2d().vertice[i].x, el->ToLine_String_2d().vertice[i].y, 0.0);

        if (fp->PointInside (testPt))
            insidePoints.push_back (testPt);
        else
            outsidePoints.push_back (testPt);
        }

    EditElementHandle   newEeh;

    if (inside && 0 != insidePoints.size () && SUCCESS == PointStringHandler::CreatePointStringElement (newEeh, &eh, &insidePoints.front (), NULL, insidePoints.size (), true, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
        inside->Insert (newEeh);

    if (outside && 0 != outsidePoints.size () && SUCCESS == PointStringHandler::CreatePointStringElement (newEeh, &eh, &outsidePoints.front (), NULL, outsidePoints.size (), true, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
        outside->Insert (newEeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PointStringHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointStringHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointStringHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int numpts = elm.ToPoint_string_2d().numpts;

    elm.SetSizeWordsNoAttributes((offsetof (Point_string_3d, point) + numpts * (sizeof (DPoint3d) + 4 * sizeof (double))) / 2);
    elm.ToPoint_string_3dR().numpts = numpts;

    DataConvert::Points2dTo3d (elm.ToPoint_string_3dR().point, eeh.GetElementCP ()->ToPoint_string_2d().point, numpts, elevation);

    double*         outPntr = &elm.ToPoint_string_3dR().point[numpts].x;
    double const*   inPntr  = &eeh.GetElementCP ()->ToPoint_string_2d().point[numpts].x;

    for (int iPoint=0; iPoint < numpts; iPoint++, outPntr += 4, inPntr += 4)
        {
        RotMatrix   rMatrix;

        rMatrix.InitFromRowValuesXY ( inPntr);
        rMatrix.GetQuaternion(outPntr, true);
        }

    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointStringHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numpts = elm.ToPoint_string_3d().numpts;

    elm.SetSizeWordsNoAttributes((offsetof (Point_string_2d, point) + numpts * (sizeof (DPoint2d) + 4 * sizeof (double))) / 2);
    elm.ToPoint_string_2dR().numpts = numpts;

    DataConvert::Points3dTo2d (elm.ToPoint_string_2dR().point, eeh.GetElementCP ()->ToPoint_string_3d().point, numpts);

    double*         outPntr = &elm.ToPoint_string_2dR().point[numpts].x;
    double const*   inPntr  = &eeh.GetElementCP ()->ToPoint_string_3d().point[numpts].x;

    for (int iPoint=0; iPoint < numpts; iPoint++, outPntr+=4, inPntr+=4)
        {
        RotMatrix   rMatrix;

        rMatrix.InitTransposedFromQuaternionWXYZ ( inPntr);
        rMatrix.GetRowValuesXY(outPntr);
        }

    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::TransformLineString
(
EditElementHandleR eeh,
TransformInfoCR trans,
bool            canContainDisconnects
)
    {
    MSElementDescrP edP = eeh.GetElementDescrP (); // Make sure we have an element descriptor...

    if (!edP)
        return ERROR;

    PointVector pointVector;
    DPoint3dP   pointsDirect = NULL;
    int         nPointsDirect = 0;

    LineStringUtil::GetLineStringTransformPoints (&pointVector, &pointsDirect, &nPointsDirect, &edP->ElementR(), canContainDisconnects);

    DPoint3dP   points;
    int         npoints;

    if (pointsDirect)
        {
        points = pointsDirect;
        npoints = nPointsDirect;
        }
    else
        {
        points = &pointVector.front ();
        npoints = static_cast<int>(pointVector.size ());
        }

    if (0 == npoints)
        return ERROR;

    trans.GetTransform()->multiply (points, npoints);

    return LineStringUtil::SetLineStringTransformPoints (&edP->ElementR(), points, npoints, canContainDisconnects);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineStringBaseHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::GetPoint
(
ElementHandleCR source,
DPoint3dR       point,
int             index,
TransformCP     pTransform
)
    {
    if (index < 0)
        return ERROR;

    int numVerts = GetPointCount (source);

    if (index >= numVerts)
        return ERROR;

    DgnElementCP  el = source.GetElementCP();

    if (Is3dElem (el))
        {
        point = el->ToLine_String_3d().vertice [index];
        }
    else
        {
        point.x = el->ToLine_String_2d().vertice [index].x;
        point.y = el->ToLine_String_2d().vertice [index].y;
        point.z = 0.0;
        }

    if (pTransform)
        pTransform->multiply (&point);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::GetPoint
(
ElementHandleCR    source,
DPoint3dR       point,
int             index,
double          edgeFraction
)
    {
    int numVerts = GetPointCount (source);

    if (numVerts <= 0)
        return ERROR;

    if (numVerts == 1)
        return GetPoint (source, point, 0);

    if (index <= 0)
        index = 0;
    if (index >= numVerts - 1)
        index = numVerts - 2;

    DgnElementCP  el = source.GetElementCP();

    if (Is3dElem (el))
        {
        point.interpolate (
                &el->ToLine_String_3d().vertice [index],
                edgeFraction,
                &el->ToLine_String_3d().vertice [index + 1]);
        }
    else
        {
        DPoint2d pointxy;
        pointxy.interpolate (
                &el->ToLine_String_2d().vertice [index],
                edgeFraction,
                &el->ToLine_String_2d().vertice [index + 1]);

        point.x = pointxy.x;
        point.y = pointxy.y;
        point.z = 0.0;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LineStringBaseHandler::GetPoints
(
ElementHandleCR    source,
DPoint3dP       pBuffer,
int             index,
int             count
)
    {
    if (count <= 0)
        return SUCCESS;

    int numVerts = GetPointCount (source);


    if (index < 0)
        return ERROR;
    if (index >= numVerts)
        return ERROR;
    if (index + count > numVerts)
        return count;

    DgnElementCP  el = source.GetElementCP();

    if (Is3dElem (el))
        {
        memcpy(pBuffer, el->ToLine_String_3d().vertice + index, count * sizeof (*pBuffer));
        }
    else
        {
        for (int i = 0; i < count; i++)
            {
            pBuffer[i].x = el->ToLine_String_2d().vertice [index + i].x;
            pBuffer[i].y = el->ToLine_String_2d().vertice [index + i].y;
            pBuffer[i].z = 0.0;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
int             LineStringBaseHandler::GetPointCount (ElementHandleCR source)
    {
    DgnElementCP el = source.GetElementCP();

    return (Is3dElem (el) ? el->ToLine_String_3d().numverts : el->ToLine_String_2d().numverts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::EncodeStringFraction
(
ElementHandleCR    source,
double          &stringFraction,
int             edgeIndex,
double          edgeFraction
)
    {
    stringFraction = 0.0;
    int numVertex = GetPointCount (source);
    return LineStringUtil::EncodeStringFraction (&stringFraction, edgeIndex, edgeFraction, numVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::DecodeStringFraction
(
ElementHandleCR    source,
int             &edgeIndex,
double          &edgeFraction,
double          stringFraction
)
    {
    int period = 0;
    edgeFraction = 0.0;
    edgeIndex = 0;
    bool bClosed = (SHAPE_ELM == source.GetLegacyType());
    int numVertex = GetPointCount (source);

    return LineStringUtil::DecodeStringFraction(&period, &edgeIndex, &edgeFraction, stringFraction, numVertex, bClosed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::SetPoint
(
EditElementHandleR source,
DPoint3dCR      point,
int             index
)
    {
    if (index < 0)
        return ERROR;

    int numVerts = GetPointCount (source);

    if (index >= numVerts)
        return false;

    DgnElementP  el = source.GetElementP();

    if (Is3dElem (el))
        {
        el->ToLine_String_3dR().vertice [index] = point;
        }
    else
        {
        el->ToLine_String_2dR().vertice [index].x = point.x;
        el->ToLine_String_2dR().vertice [index].y = point.y;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringBaseHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringBaseHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStringBaseHandler::_GetCurveVector (ElementHandleCR source, CurveVectorPtr& curves)
    {
    int         nPts = const_cast <LineStringBaseHandler*> (this)->GetPointCount (source);

    if (nPts < 1)
        return ERROR; // Bad element...

    ScopedArray<DPoint3d> points(nPts);

    const_cast <LineStringBaseHandler*> (this)->GetPoints (source, points.GetData(), 0, nPts);

    curves = CurveVector::Create (SHAPE_ELM == source.GetLegacyType() ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateLineString (points.GetData(), nPts));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStringBaseHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != path.HasSingleCurvePrimitive ())
        return ERROR;

    bvector<DPoint3d> const* points = path.front ()->GetLineStringCP ();

    if (points->size () > MAX_VERTICES)
        return ERROR;

    BentleyStatus       status = ERROR;
    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElement, Create methods uses SetElementDescr...
    if (path.IsClosedPath ())
        status = ShapeHandler::CreateShapeElement (newEeh, &eeh, &points->front (), points->size (), eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ());
    else if (LINE_STRING_ELM == eeh.GetLegacyType())
        status = LineStringHandler::CreateLineStringElement (newEeh, &eeh, &points->front (), points->size (), eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ());

    if (SUCCESS == status)
        status = (BentleyStatus) eeh.ReplaceElement (newEeh.GetElementCP ());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringBaseHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numverts = elm.ToLine_String_2d().numverts;

    elm.SetSizeWordsNoAttributes((offsetof (Line_String_3d, vertice) + numverts * sizeof (DPoint3d)) / 2);
    elm.ToLine_String_3dR().numverts = numverts;
    DataConvert::Points2dTo3d (elm.ToLine_String_3dR().vertice, eeh.GetElementCP ()->ToLine_String_2d().vertice, numverts, elevation);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringBaseHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numverts = elm.ToLine_String_3d().numverts;

    elm.SetSizeWordsNoAttributes((offsetof (Line_String_2d, vertice) + numverts * sizeof (DPoint2d)) / 2);
    elm.ToLine_String_2dR().numverts = numverts;
    DataConvert::Points3dTo2d (elm.ToLine_String_2dR().vertice, eeh.GetElementCP ()->ToLine_String_3d().vertice, numverts);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus LineStringBaseHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    ReprojectStatus status  = REPROJECT_Success;
    DgnElementP  el          = source.GetElementP();
    bool        is3d        = Is3dElem (el);
    int         numPoints   = is3d ? el->ToLine_String_3d().numverts : el->ToLine_String_2d().numverts;

    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (settings->PostStrokeLinear() && (numPoints > 1))
        {
        DPoint3dP   points;
        int         numOutPoints;
        if (is3d)
            status = reprojectionHelper.ReprojectPointsMoreDetail (&points, &numOutPoints, el->ToLine_String_3d().vertice, numPoints, reprojectionHelper.GetStrokeToleranceDestUors());
        else
            status = reprojectionHelper.ReprojectPointsMoreDetail2D (&points, &numOutPoints, el->ToLine_String_2d().vertice, numPoints, reprojectionHelper.GetStrokeToleranceDestUors());

        if ( (REPROJECT_Success == status) || (REPROJECT_CSMAPERR_OutOfUsefulRange == status) )
            status = GeoCoordinateReprojectionReplaceElement (source, points, numOutPoints, inChain, SHAPE_ELM == source.GetLegacyType());
        }
    else
        {
        if (is3d)
            status = reprojectionHelper.ReprojectPoints (el->ToLine_String_3dR().vertice, NULL, NULL, el->ToLine_String_3d().vertice, numPoints);
        else
            status = reprojectionHelper.ReprojectPoints2D (el->ToLine_String_2dR().vertice, NULL, NULL, el->ToLine_String_2d().vertice, numPoints);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringBaseHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_LinearSegments & geometry.GetOptions ()))
        return ERROR;

    int         nPoints = const_cast <LineStringBaseHandler*> (this)->GetPointCount (eh);

    if (nPoints < 2)
        return ERROR;

    bvector<DPoint3d> points;

    points.resize (nPoints);
    const_cast <LineStringBaseHandler*> (this)->GetPoints (eh, &points[0], 0, nPoints);

    for (int iPoint = 0; iPoint < nPoints-1; iPoint++)
        {
        if (points[iPoint].IsDisconnect () || points[iPoint+1].IsDisconnect ())
            continue;

        DSegment3d  segment;

        segment.Init (points[iPoint], points[iPoint+1]);

        EditElementHandle segmentEeh;

        // NOTE: Don't supply template to create...don't want fill/pattern linkages, etc.
        if (SUCCESS != LineHandler::CreateLineElement (segmentEeh, NULL, segment, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
            continue;

        ElementPropertiesSetter::ApplyTemplate (segmentEeh, eh);

        dropGeom.Insert (segmentEeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_LINE_STRING_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringHandler::_OnTransform
(
EditElementHandleR eeh,
TransformInfoCR trans
)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    BeAssert (LINE_STRING_ELM == eeh.GetLegacyType());

    return LineStringBaseHandler::TransformLineString (eeh, trans, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    BeAssert (LINE_STRING_ELM == elemHandle.GetLegacyType());

    return LineStringUtil::FenceStretch (elemHandle, transform, fp, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LineStringHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStringHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShapeHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_SHAPE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     transformVertexData (EditElementHandleR eeh, TransformInfoCR trans)
    {
    if (!mdlElement_attributePresent (eeh.GetElementCP (), LINKAGEID_RenderVertex, NULL))
        return;

    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_RenderVertex != li->primaryID)
            continue;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        Int16   dataType;
        Int16   nValues;

        reader.get (&dataType);
        reader.get (&nValues);

        if (VERTEX_LINKAGE_Normal != dataType)
            continue;

        RotMatrix   rMatrix;

        // multiply by the transpose of the inverse of the matrix part of the transform to preserve orthogonality
        rMatrix.InitFrom (*trans.GetTransform ());
        rMatrix.Invert ();

        for (int i=0; i < nValues; i++)
            {
            SPoint3d*   valueP = (SPoint3d*) reader.getPos ();
            DVec3d      normal;

            normal.Init (valueP->x * s_vertexDataValueScale, valueP->y * s_vertexDataValueScale, valueP->z * s_vertexDataValueScale);
            rMatrix.MultiplyTranspose (normal);
            normal.Normalize ();

            valueP->x = (Int16) DataConvert::RoundDoubleToLong (normal.x * s_vertexDataValueScale);
            valueP->y = (Int16) DataConvert::RoundDoubleToLong (normal.y * s_vertexDataValueScale);
            valueP->z = (Int16) DataConvert::RoundDoubleToLong (normal.z * s_vertexDataValueScale);

            SPoint3d    value;

            reader.get (&value.x, 3); // Advance to next normal...
            }
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ShapeHandler::_OnTransform (EditElementHandleR eeh, TransformInfoCR trans)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    BeAssert (SHAPE_ELM == eeh.GetLegacyType());

    if (SUCCESS != LineStringBaseHandler::TransformLineString (eeh, trans, false))
        return ERROR;

    transformVertexData (eeh, trans);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ShapeHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    BeAssert (SHAPE_ELM == elemHandle.GetLegacyType());

    return LineStringUtil::FenceStretch (elemHandle, transform, fp, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ShapeHandler::_OnFenceClip
(
ElementAgendaP      inside,
ElementAgendaP      outside,
ElementHandleCR     elemHandle,
FenceParamsP        fp,
FenceClipFlags      options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShapeHandler::DeleteVertexData (EditElementHandleR eeh, bool normals, bool params, bool colors)
    {
    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        if (!li->user)
            continue;

        switch (li->primaryID)
            {
            case LINKAGEID_RenderVertex:
                {
                if (!(normals || colors))
                    break;

                DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

                Int16   dataType;
                Int16   nValues;

                reader.get (&dataType);
                reader.get (&nValues);

                switch (dataType)
                    {
                    case VERTEX_LINKAGE_Normal:
                        {
                        if (normals)
                            eeh.RemoveElementLinkage (li);

                        break;
                        }

                    case VERTEX_LINKAGE_RGB:
                        {
                        if (colors)
                            eeh.RemoveElementLinkage (li);

                        break;
                        }
                    }
                break;
                }

            case LINKAGEID_UvVertex:
                {
                if (params)
                    eeh.RemoveElementLinkage (li);

                break;
                }
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShapeHandler::AppendVertexData (EditElementHandleR eeh, DVec3dCP normals, DPoint2dCP params, RgbColorDefCP colors)
    {
    int         nPoints = LineStringUtil::GetCount (*eeh.GetElementCP ());

    if (normals)
        {
        if (nPoints > MAX_NVALUES)
            return;

        DataExternalizer   writer;

        writer.put ((Int16) VERTEX_LINKAGE_Normal);
        writer.put ((Int16) nPoints);

        for (int i=0; i < nPoints; i++)
            {
            DVec3d      normal = normals[i == nPoints-1 ? 0 : i];

            if (normal.Normalize () <= 0.0)
                return;
                
            SPoint3d    value;

            value.x = (Int16) DataConvert::RoundDoubleToLong (normal.x * s_vertexDataValueScale);
            value.y = (Int16) DataConvert::RoundDoubleToLong (normal.y * s_vertexDataValueScale);
            value.z = (Int16) DataConvert::RoundDoubleToLong (normal.z * s_vertexDataValueScale);

            writer.put (&value.x, 3);
            }

        ShapeHandler::DeleteVertexData (eeh, true, false, false);
        ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_RenderVertex, writer);
        }

    if (params)
        {
        if (nPoints > MAX_UVALUES) // Old api tested against MAX_NVALUES...
            return;

        double  minU, minV, maxU, maxV, deltaU, deltaV;

        // compute range of parameter values
        minU = maxU = params[0].x;
        minV = maxV = params[0].y;

        for (int i = 1; i < nPoints; i++)
            {
            if (params[i].x < minU)
                minU = params[i].x;
            else if (params[i].x > maxU)
                maxU = params[i].x;

            if (params[i].y < minV)
                minV = params[i].y;
            else if (params[i].y > maxV)
                maxV = params[i].y;
            }

        deltaU = maxU - minU;
        deltaV = maxV - minV;

        if (deltaU <= 0.0 || deltaV <= 0.0)
            return;

        DataExternalizer   writer;

        writer.put (deltaU / s_vertexDataValueScale);
        writer.put (deltaV / s_vertexDataValueScale);
        writer.put (minU);
        writer.put (minV);
        writer.put ((Int16) nPoints);

        for (int i=0; i < nPoints; i++)
            {
            DPoint2d    param = params[i == nPoints-1 ? 0 : i];
            SPoint2d    value;

            param.x = (params[i].x - minU) / deltaU;
            param.y = (params[i].y - minV) / deltaV;

            value.x = (Int16) DataConvert::RoundDoubleToLong (param.x * s_vertexDataValueScale);
            value.y = (Int16) DataConvert::RoundDoubleToLong (param.y * s_vertexDataValueScale);

            writer.put (&value.x, 2);
            }

        ShapeHandler::DeleteVertexData (eeh, false, true, false);
        ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_UvVertex, writer);
        }

    if (colors)
        {
        if (nPoints > MAX_NVALUES)
            return;

        DataExternalizer   writer;

        writer.put ((Int16) VERTEX_LINKAGE_RGB);
        writer.put ((Int16) nPoints);

        double  scale = s_vertexDataValueScale / UCHAR_MAX;

        for (int i=0; i < nPoints; i++)
            {
            RgbColorDef color = colors[i == nPoints-1 ? 0 : i];
            SPoint3d    value;

            value.x = (Int16) DataConvert::RoundDoubleToLong (color.red   * scale);
            value.y = (Int16) DataConvert::RoundDoubleToLong (color.green * scale);
            value.z = (Int16) DataConvert::RoundDoubleToLong (color.blue  * scale);

            writer.put (&value.x, 3);
            }

        ShapeHandler::DeleteVertexData (eeh, false, false, true);
        ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_RenderVertex, writer);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShapeHandler::ExtractVertexData (ElementHandleCR eh, bvector<DVec3d>* normals, bvector<DPoint2d>* params, bvector<RgbColorDef>* colors)
    {
    int         nPoints = LineStringUtil::GetCount (*eh.GetElementCP ());

    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        if (!li->user)
            continue;

        switch (li->primaryID)
            {
            case LINKAGEID_RenderVertex:
                {
                // typedef struct renderVertexLinkageData
                // {
                // short    type;
                // short    nValues;
                // Spoint3d values[MAX_NVALUES];
                // } RenderVertexLinkageData;
                DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

                Int16   dataType;
                Int16   nValues;

                reader.get (&dataType);
                reader.get (&nValues);

                if (nValues != nPoints || nPoints > MAX_NVALUES)
                    break;

                switch (dataType)
                    {
                    case VERTEX_LINKAGE_Normal:
                        {
                        if (!normals)
                            break;

                        double  scale = (1.0 / s_vertexDataValueScale);

                        normals->reserve (nValues);

                        for (int i=0; i < nValues; i++)
                            {
                            SPoint3d    value;
                            DVec3d      normal;

                            reader.get (&value.x, 3);
                            normal.Init (value.x * scale, value.y * scale, value.z * scale);

                            normals->push_back (normal);
                            }

                        normals->back () = normals->front ();
                        break;
                        }

                    case VERTEX_LINKAGE_RGB:
                        {
                        if (!colors)
                            break;

                        double  scale = (UCHAR_MAX / s_vertexDataValueScale);

                        colors->reserve (nValues);

                        for (int i=0; i < nValues; i++)
                            {
                            SPoint3d    value;
                            RgbColorDef rgb;

                            reader.get (&value.x, 3);

                            rgb.red   = (byte) (value.x * scale);
                            rgb.green = (byte) (value.y * scale);
                            rgb.blue  = (byte) (value.z * scale);

                            colors->push_back (rgb);
                            }

                        colors->back () = colors->front ();
                        break;
                        }
                    }

                break;
                }

            case LINKAGEID_UvVertex:
                {
                // typedef struct renderUvVertexLinkageData
                // {
                // double   scaleU, scaleV;     /* multipliers for each U & V value */
                // double   offsetU, offsetV;   /* to be added to each U & V value */
                // short    nValues;
                // Spoint2d values[MAX_UVALUES];
                // } RenderUvVertexLinkageData;
                if (!params)
                    break;

                DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

                double  scaleU, scaleV, offsetU, offsetV;
                Int16   nValues;

                reader.get (&scaleU);
                reader.get (&scaleV);
                reader.get (&offsetU);
                reader.get (&offsetV);
                reader.get (&nValues);

                if (nValues != nPoints || nPoints > MAX_UVALUES)
                    break;

                params->reserve (nValues);

                for (int i=0; i < nValues; i++)
                    {
                    SPoint2d    value;
                    DPoint2d    param;

                    reader.get (&value.x, 2);
                    param.Init (offsetU + scaleU * value.x, offsetV + scaleV * value.y);

                    params->push_back (param);
                    }

                params->back () = params->front ();
                break;
                }
            }
        }
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/2009
+===============+===============+===============+===============+===============+======*/
struct  StrokeShapeWithVertexLinkage : IStrokeForCache
{
private:

PolyfaceHeaderPtr   m_meshData;

public:

virtual ~StrokeShapeWithVertexLinkage() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StrokeShapeWithVertexLinkage (ElementHandleCR eh)
    {
    int         nPoints = LineStringUtil::GetCount (*eh.GetElementCP ());

    if (nPoints < 4)
        return;

    bvector<DPoint3d>   points;

    points.resize (nPoints);
    LineStringUtil::Extract (&points.front (), NULL, *eh.GetElementCP (), *eh.GetDgnModelP ());

    bvector<DVec3d>      normals;
    bvector<DPoint2d>    params;
    bvector<RgbColorDef> colors;

    ShapeHandler::ExtractVertexData (eh, &normals, &params, &colors);

    m_meshData = PolyfaceHeader::New ();

    m_meshData->Point ().SetActive (true);
    m_meshData->PointIndex ().SetActive (true);

    if (0 != normals.size ())
        {
        m_meshData->Normal ().SetActive (true);
        m_meshData->NormalIndex ().SetActive (true);
        }

    if (0 != params.size ())
        {
        m_meshData->Param ().SetActive (true);
        m_meshData->ParamIndex ().SetActive (true);
        }

    m_meshData->AddPolygon (points, &normals, &params);

    if (0 == colors.size ())
        return;

    float       scale = 1.0f / UCHAR_MAX;

    for (size_t i = 0; i < colors.size (); i++)
        {
        FloatRgb    color;

        color.red   = colors[i].red   * scale;
        color.green = colors[i].green * scale;
        color.blue  = colors[i].blue  * scale;

        m_meshData->FloatColor ().Append (&color, 1);
        }

    m_meshData->FloatColor ().SetActive (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsValid () const
    {
    return !m_meshData.IsNull ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantLocateByStroker () override {return false;} // Don't call _StrokeForCache, locate interior by QvElem...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR drawHandle, ViewContextR context, double pixelSize = 0.0) override
    {
    context.GetIDrawGeom().DrawPolyface (*m_meshData);
    }

}; // StrokeShapeWithVertexLinkage

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShapeHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));

    if (0 != (info & DISPLAY_INFO_Surface))
        {
        if (mdlElement_attributePresent (thisElm.GetElementCP (), LINKAGEID_RenderVertex, NULL) ||
            mdlElement_attributePresent (thisElm.GetElementCP (), LINKAGEID_UvVertex, NULL))
            {
            CachedDrawHandle dh(&thisElm);
            StrokeShapeWithVertexLinkage  stroker (thisElm);

            if (stroker.IsValid ())
                {
                // NOTE: DrawCurveVector may use qvIndices 0 -> 2...
                context.DrawCached (dh, stroker, 10);
                info &= ~DISPLAY_INFO_Surface; 
                }
            }
        }

    context.DrawCurveVector (thisElm, *this, (GeomRepresentations) info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineHandler::CreateLineElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DSegment3dCR    segment,
bool            is3d,
DgnModelR    modelRef
)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, LINE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof(Line_3d));
        ElementUtil::SetRequiredFields (out, LINE_ELM, LevelId(LEVEL_DEFAULT_LEVEL_ID), false, (ElementUtil::ElemDim) is3d);
        }

    if (is3d)
        ElementUtil::PackLineWords3d (&out.ToLine_2dR().start, segment.point, 2);
    else
        ElementUtil::PackLineWords2d (&out.ToLine_2dR().start, segment.point, 2);

    int         elmSize = (is3d ? sizeof (Line_3d) : sizeof (Line_2d));

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStringHandler::CreateLineStringElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCP      points,
size_t          numVerts,
bool            is3d,
DgnModelR    modelRef
)
    {
    if (numVerts < 2 || numVerts > MAX_VERTICES)
        return ERROR; // NOTE: Used to truncate numVerts to MAX_VERTICES and create element anyway?!?

    size_t      elmSize;

    if (is3d)
        elmSize = sizeof (Line_String_3d) + (numVerts-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (numVerts-1) * sizeof (DPoint2d);

    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, LINE_STRING_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);

        if (LINE_STRING_ELM != in->GetLegacyType())
            out.ToLine_String_2dR().reserved = 0;
        }
    else
        {
        memset (&out, 0, elmSize);
        ElementUtil::SetRequiredFields (out, LINE_STRING_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    out.ToLine_String_2dR().numverts = static_cast<UInt32>(numVerts);

    if (is3d)
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, points, numVerts);
    else
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, points, numVerts);

    out.SetSizeWordsNoAttributes(static_cast<UInt32>(elmSize/2));
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ShapeHandler::CreateShapeElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCP      points,
size_t          numVerts,
bool            is3d,
DgnModelR    modelRef
)
    {
    if (numVerts < 2)
        return ERROR;

    if (!LegacyMath::RpntEqual (&points[0], &points[numVerts-1]))
        numVerts++;

    if (numVerts < 3 || numVerts > MAX_VERTICES)
        return ERROR; // NOTE: Used to truncate numVerts to MAX_VERTICES and create element anyway?!?

    size_t      elmSize;

    if (is3d)
        elmSize = sizeof (Line_String_3d) + (numVerts-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (numVerts-1) * sizeof (DPoint2d);

    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, SHAPE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);

        if (SHAPE_ELM != in->GetLegacyType())
            out.ToLine_String_2dR().reserved = 0;
        }
    else
        {
        memset (&out, 0, elmSize);
        ElementUtil::SetRequiredFields (out, SHAPE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    out.ToLine_String_2dR().numverts = static_cast<UInt32>(numVerts);

    if (is3d)
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, points, numVerts-1); // first always set equal to last...
    else
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, points, numVerts-1);

    if (is3d)
        out.ToLine_String_3dR().vertice[numVerts-1] = *points;
    else
        out.ToLine_String_2dR().vertice[numVerts-1].init (points);

    out.SetSizeWordsNoAttributes(static_cast<UInt32>(elmSize/2));
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PointStringHandler::CreatePointStringElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCP      points,
RotMatrixCP     matrices,
size_t          numVerts,
bool            disjoint,
bool            is3d,
DgnModelR    modelRef
)
    {
    if (0 == numVerts || numVerts > MAX_VERTICES)
        return ERROR; // NOTE: Used to truncate numVerts to MAX_VERTICES and create element anyway?!?

    size_t      elmSize;

    if (is3d)
        elmSize = sizeof (Line_String_3d) + (numVerts-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (numVerts-1) * sizeof (DPoint2d);

    double      rtrans[4];

    elmSize += (sizeof (rtrans) * numVerts);

    if (elmSize/2 > MAX_V8_ELEMENT_SIZE)
        return ERROR;

    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, POINT_STRING_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);

        if (POINT_STRING_ELM != in->GetLegacyType())
            out.ToLine_String_2dR().reserved = 0;
        }
    else
        {
        memset (&out, 0, elmSize);
        ElementUtil::SetRequiredFields (out, POINT_STRING_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    out.ToLine_String_2dR().numverts = static_cast<UInt32>(numVerts);

    if (is3d)
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, points, numVerts);
    else
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, points, numVerts);

    out.SetIsHole(disjoint);

    double*     transtart = (is3d ? (double *) &out.ToLine_String_3d().vertice[numVerts] : (double *) &out.ToLine_String_2d().vertice[numVerts]);

    if (!matrices)
        {
        RotMatrix   rMatrix;

        rMatrix.InitIdentity ();

        if (is3d)
            {
            rMatrix.GetQuaternion(rtrans, true);

            for (size_t i=0; i < numVerts; i++, transtart+=4)
                memcpy (transtart, rtrans, sizeof (rtrans));
            }
        else
            {
            rMatrix.GetRowValuesXY(rtrans);

            for (size_t i=0; i < numVerts; i++, transtart+=4)
                memcpy (transtart, rtrans, sizeof (rtrans));
            }
        }
    else
        {
        double const*   pRotation = (double const*) matrices;

        if (is3d)
            {
            for (size_t i=0; i < numVerts; i++, transtart += 4, pRotation += 9)
                ( (RotMatrixCP) pRotation)->GetQuaternion(transtart, true);
            }
        else
            {
            for (size_t i=0; i < numVerts; i++, transtart += 4, pRotation += 4)
                ( (RotMatrixCP) pRotation)->GetRowValuesXY(transtart);
            }
        }

    out.SetSizeWordsNoAttributes(static_cast<UInt32>(elmSize/2));
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }
