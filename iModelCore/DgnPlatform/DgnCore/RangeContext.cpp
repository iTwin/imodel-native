/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RangeContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <Bentley/ScopedArray.h>
#include    <DgnPlatformInternal/DgnCore/ElemRangeCalc.h>

struct SafeDPoint3dArray : ScopedArray<DPoint3d,500> {SafeDPoint3dArray(size_t n) : ScopedArray<DPoint3d,500>(n){}};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09.06
+---------------+---------------+---------------+---------------+---------------+------*/
RangeClip::RangeClip (ClipPlaneSetCP pClip, TransformCP pTransform)
    {
    m_isCamera = false;
    if (NULL == pTransform)
        m_transform.initIdentity();
    else
        m_transform = *pTransform;

    if (NULL != pClip)
        for (ConvexClipPlaneSetCR planes: *pClip)
            m_planeSets.push_back (RangeClipPlanes  (planes.size(), &planes.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09.06
+---------------+---------------+---------------+---------------+---------------+------*/
RangeClip::RangeClip  (DPoint3dCR camera, double focalLength)
    {
    m_isCamera = true;
    m_camera = camera;
    m_focalLength = focalLength;
    }       
           
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeClip::ApplyTransform (DPoint3dP transformedPoints, DPoint3dCP points, int numPoints) const
    {
    if (m_isCamera)
        {
        for (int i=0; i<numPoints; i++)
            {
            DPoint3d        tmpPoint;
            static double   s_cameraScaleLimit = 1.0 / 300.0;
                                                                                                                                             
            tmpPoint.differenceOf (&points[i], &m_camera);

            double      cameraScale =  MAX (s_cameraScaleLimit, -m_focalLength / tmpPoint.z);

            transformedPoints[i].x = m_camera.x + tmpPoint.x * cameraScale;
            transformedPoints[i].y = m_camera.y + tmpPoint.y * cameraScale;
            transformedPoints[i].z = points[i].z;
            }
        }
    else
        {
        m_transform.multiply (transformedPoints, points, numPoints);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool   ClipStack::ContainsCamera () const
    {
    for (bvector<RangeClip>::const_iterator curr = m_clips.begin(); curr != m_clips.end(); curr++)
        if (curr->IsCamera())
            return true;
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipStack::ClipRange (ElemRangeCalc* rangeCalculator, DPoint3dCP corners, size_t clipIndex, bool fastClip) const
    {
    if (clipIndex == m_clips.size() || m_clips.empty())
        rangeCalculator->Union (8, corners, NULL);
    else
        m_clips[m_clips.size() - clipIndex - 1].ClipRange (rangeCalculator, this, corners, clipIndex+1, fastClip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipStack::ClipEllipse (ElemRangeCalc* rangeCalculator, DEllipse3dCR ellipse, size_t clipIndex) const
    {
    if (clipIndex == m_clips.size() || m_clips.empty())
        rangeCalculator->Union (&ellipse, NULL);
    else
        m_clips[m_clips.size() - clipIndex - 1].ClipEllipse (rangeCalculator, this, ellipse, clipIndex+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipStack::ClipPoints (ElemRangeCalc* rangeCalculator, int numPoints, DPoint3dCP pPoints, size_t clipIndex) const
    {
    if (numPoints <= 0)
        return;

    if (clipIndex == m_clips.size() || m_clips.empty())
        {
        rangeCalculator->Union (numPoints, pPoints, NULL);
        return;
        }

    // If there are plane sets then the involved are is the union of all sets, so we have
    // to pass the points through each set. sequentially.
    // Each plane set will pass the clipped points on to the next clip (as designated by startindex).

    size_t      nClips = m_clips.size();
    DPoint3dCP  pFirst, pEnd = pPoints + numPoints;

    // Handle leading disconnects.
    for (pFirst = pPoints; (DISCONNECT == pFirst->x) && numPoints > 0; pFirst++, numPoints--)
        ;

    int             nBatch;
    DPoint3dCP pCurr = pFirst + 1;
    for (; pCurr < pEnd; pCurr++)
        {
        if (DISCONNECT == pCurr->x)
            {
            if ((nBatch = static_cast<int>(pCurr - pFirst)) > 0)
                {
                m_clips[nClips-clipIndex-1].ClipPoints (rangeCalculator, this, nBatch, pFirst, clipIndex+1);
                pFirst = pCurr + 1;
                }
            }
        }
    if ((nBatch = static_cast<int>(pCurr - pFirst)) > 0)
        m_clips[nClips-clipIndex-1].ClipPoints (rangeCalculator, this, nBatch, pFirst, clipIndex+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeClip::ClipPoints (ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, int numPoints, DPoint3dCP points, size_t clipIndex) const
    {
    SafeDPoint3dArray tPts(numPoints);
    DPoint3dP transformedPoints = tPts.GetData();

    ApplyTransform (transformedPoints, points, numPoints);
    if (m_planeSets.empty())
        {
        clipStack->ClipPoints (rangeCalculator, numPoints, transformedPoints, clipIndex);
        }
    else
        {
        for (size_t i=0, count=m_planeSets.size(); i<count; i++)
            m_planeSets[i].ClipPoints (rangeCalculator, clipStack, numPoints, transformedPoints, clipIndex);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeClip::ClipEllipse (ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, DEllipse3dCR ellipse, size_t clipIndex) const
    {
    DRange3d            range;
    DPoint3d            corners[8];

    if (m_isCamera)
        {
        ellipse.getRange (&range);
        range.get8Corners (corners);
        ClipRange (rangeCalculator, clipStack, corners, clipIndex, false);
        return;
        }

    DEllipse3d          transformedEllipse;
    transformedEllipse.productOf (&m_transform, &ellipse);
    if (m_planeSets.empty())
        {
        clipStack->ClipEllipse (rangeCalculator, transformedEllipse, clipIndex);
        }
    else
        {
        transformedEllipse.getRange (&range);
        range.get8Corners (corners);

        for (size_t i=0, count=m_planeSets.size(); i<count; i++)
            if (!m_planeSets[i].ClipRange (rangeCalculator, clipStack, corners,  clipIndex, false))
                break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeClip::ClipRange (ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, DPoint3dCP corners, size_t clipIndex, bool fastClip) const
    {
    DPoint3d    transformedCorners[8];

    ApplyTransform (transformedCorners, corners, 8);
    if (m_planeSets.empty())
        {
        clipStack->ClipRange (rangeCalculator, transformedCorners, clipIndex, fastClip);
        }
    else
        {
        for (size_t i=0, count=m_planeSets.size(); i<count; i++)
            if (!m_planeSets[i].ClipRange (rangeCalculator, clipStack, transformedCorners,  clipIndex, fastClip))
                break;
        }
    }

/*=================================================================================**//**
* @bsimethod                                                    Keith.Bentley   09/03
+===============+===============+===============+===============+===============+======*/
StatusInt       ElemRangeCalc::GetRange (DRange3dR range)
    {
    if (m_range.isNull())
        {
        memset (&range, 0, sizeof (range));
        return ERROR;
        }

    range = m_range;
    return SUCCESS;
    }

            ElemRangeCalc::ElemRangeCalc ()             { m_range.init (); }
void        ElemRangeCalc::Invalidate ()                { m_range.init (); }
void        ElemRangeCalc::SetRange (DRange3dCR range)  { m_range = range; }

static const double RMINDESIGNRANGE = (-4503599627370496.0);
static const double RMAXDESIGNRANGE = (4503599627370495.0);

/*---------------------------------------------------------------------------------**//**
* union this range with an array of Point3d's
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElemRangeCalc::Union (int numPoints, DPoint3dCP points, ClipStackCP currClip)
    {
    if (NULL == currClip)
        {
        for ( ; numPoints > 0; points++, numPoints--)
            {
            if (points->x > RMINDESIGNRANGE && points->y > RMINDESIGNRANGE && points->z > RMINDESIGNRANGE &&
                points->x < RMAXDESIGNRANGE && points->y < RMAXDESIGNRANGE && points->z < RMAXDESIGNRANGE)         // This will handle disconnects.
                {
                m_range.extend (points);
                }
            }
        }
    else
        {
        currClip->ClipPoints (this, numPoints, points);
        }
    }

/*---------------------------------------------------------------------------------**//**
* union this range with an array of Point2d's
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemRangeCalc::Union (int numPoints, DPoint2dCP points, ClipStackCP currClip)
    {
    if (NULL == currClip)
        {
        for ( ; numPoints > 0; points++, numPoints--)
            {
            if (points->x > RMINDESIGNRANGE && points->y > RMINDESIGNRANGE &&
                points->x < RMAXDESIGNRANGE && points->y < RMAXDESIGNRANGE)         // This will handle disconnects.
                {
                DPoint3d    tPt;     // this must be 3d, because of 2d elements in 3d shared cell definitions

                tPt.x = points->x;
                tPt.y = points->y;
                tPt.z = 0.0;

                m_range.extend (&tPt);
                }
            }
        }
    else
        {
        SafeDPoint3dArray tPts(numPoints);
        DPoint3dP pTmpPoints = tPts.GetData();

        bsiDPoint3d_copyDPoint2dArray (pTmpPoints, points, numPoints);
        currClip->ClipPoints (this, numPoints, pTmpPoints);
        }
    }

/*---------------------------------------------------------------------------------**//**
* union this range with a Range3d
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElemRangeCalc::Union (DRange3dCP in, ClipStackCP currClip)
    {
    if (NULL == currClip)
        {
        m_range.extend (in);
        }
    else
        {
        DPoint3d            corners[8];

        in->get8Corners (corners);
        currClip->ClipRange (this, corners, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* union this range with an ellipse
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElemRangeCalc::Union (DEllipse3dCP ellipse, ClipStackCP currClip)
    {
    if (NULL == currClip)
        {
        DRange3d    ellipseRange;

        ellipse->getRange (&ellipseRange);
        m_range.extend (&ellipseRange);
        }
    else
        {
        currClip->ClipEllipse (this, *ellipse);
        }
    }

/*---------------------------------------------------------------------------------**//**
* convert the value of this range into a ScanRange.
* @return ERROR if the range is not valid.
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElemRangeCalc::ToScanRange (AxisAlignedBox3dR range, bool is3d)
    {
    if (!IsValid())
        {
        range.Init();
        return ERROR;
        }

    range = AxisAlignedBox3d(m_range);

    static const double s_smallVal = .0005;

    // low and high are no longer allowed to be equal...
    if (range.low.x == range.high.x)
        {
        range.low.x -= s_smallVal;
        range.high.x += s_smallVal;
        }

    if (range.low.y == range.high.y)
        {
        range.low.y -= s_smallVal;
        range.high.y += s_smallVal;
        }

    if (is3d)
        {
        if (range.low.z == range.high.z)
            {
            range.low.z -= s_smallVal;
            range.high.z += s_smallVal;
            }
        }
    else
        {
        range.low.z = range.high.z = 0.0;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void addPlaneFromPoints (ClipPlane& plane, DPoint3dCR origin, DPoint3dCR cross0, DPoint3dCR cross1, DPoint3d rayEnd)
    {
    DVec3d      normal;

    normal.crossProductToPoints (&cross1, &cross0, &origin);
    if (0.0 == normal.normalize ())                   // If the cross product is NULL then use ray from opposite end (to handle planar polyhedra).
        normal.normalizedDifference (&rayEnd, &origin);

    plane = ClipPlane (normal, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
*
*  This differs from clip_rangePlanesFromPolyhedra in that it handles degenerate polyhedra.
*  For a polyhedron that denerates to a line the two line points are returned.  For a 
*  polyhedron that degenerates to a portion of a plane, 6 planes are returned that enclose
*  this portion.   (TR# 344110)
+---------------+---------------+---------------+---------------+---------------+------*/
bool computeRangePlanesFromCorners (ClipPlane rangePlanes[6], DPoint3d degeneratePoints[2], DPoint3d corners[8])
    {
    bool     zeroLength[3];

    for (size_t i=0; i<3; i++)
        zeroLength[i] = corners[0].isEqual (&corners[(size_t) 1 << i]);

    // If the polyhedron degenerates to either a line or a point...
    for (int i=0; i<3; i++)
        {
        if (zeroLength[i] && zeroLength[(i+1)%3])
            {
            degeneratePoints[0] = corners[0];
            degeneratePoints[1] = corners[i << ((i+2) % 3)];
            return false;
            }
        }
    
    addPlaneFromPoints (rangePlanes[0], corners[1], corners[3], corners[5], corners[0]);                 // Right
    addPlaneFromPoints (rangePlanes[1], corners[0], corners[4], corners[2], corners[1]);                 // Left
    addPlaneFromPoints (rangePlanes[2], corners[2], corners[6], corners[3], corners[0]);                 // Top
    addPlaneFromPoints (rangePlanes[3], corners[0], corners[1], corners[4], corners[2]);                 // Bottom
    addPlaneFromPoints (rangePlanes[4], corners[0], corners[2], corners[1], corners[4]);                 // Back
    addPlaneFromPoints (rangePlanes[5], corners[4], corners[5], corners[6], corners[0]);                 // Front

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool RangeClipPlanes::ClipRange
(
ElemRangeCalc*          rangeCalculator,
ClipStackCP             clipStack,
DPoint3d                corners[8],
size_t                  clipIndex,
bool                    fastClip
) const
    {
    switch (m_planes.ClassifyPointContainment (corners, 8))
        {
        case ClipPlaneContainment_StronglyInside:
            clipStack->ClipRange (rangeCalculator, corners, clipIndex, fastClip);
            return false;   // If this range is totally inside, return false to denote that there is no reason to test any other clip plane sets. 

        case ClipPlaneContainment_StronglyOutside:
            return true;    // Nothing to see here - But return true to continue clipping against any remaining range planes.
        }

    // If we are just testing for range containment, it is not worthwhile to do the relatively expensive true range testing.
    // If we have a potential overlap just send the whole range through.
    if (fastClip)
        {
        clipStack->ClipRange (rangeCalculator, corners, clipIndex, fastClip);
        return false;
        }

    ClipPlane       rangePlanes[6];
    DRange3d        intersectRange;
    DPoint3d        degeneratePoints[2];

    // In SS2 we always fed the range box edges through to do the clipping - This was incorrect as
    // a totally contained interior would be ignored. - Switched in SS3 to intersect the clip plane
    // sets instead. - In Vancouver refined to use RangePlanesFromPolyhedron (the implementation for SS3
    // was not correct for camera frustum - and to handle the degenerate case by processing edges.
    if (!computeRangePlanesFromCorners (rangePlanes, degeneratePoints, corners))
        {
        ClipPoints (rangeCalculator, clipStack, 2, degeneratePoints, clipIndex);
        }
    else if (ClipUtil::IntersectClipPlaneSets (&intersectRange, &m_planes[0], m_planes.size(), rangePlanes, 6))
        {
        DPoint3d        intersectCorners[8];

        intersectRange.get8Corners (intersectCorners);

        clipStack->ClipRange (rangeCalculator, intersectCorners, clipIndex, fastClip);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     testPointsAgainstPlane
(
bool*           anyInside,
DPoint3dCP      point,
DPoint3dCP      end,
ClipPlaneCP     plane
)                   // Return true if all points are inside the plane.
    {
    bool     allInside = true;

    *anyInside = false;
    for (; point < end; point++)
        {
        if (plane->EvaluatePoint (*point) > -1.0E-3)
            *anyInside = true;
        else
            allInside = false;
        }

    return allInside;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeClipPlanes::ClipPoints (ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, int numPoints, DPoint3dCP pPoints, size_t clipIndex, size_t planeIndex) const
    {
    bool    anyInside = false;

    size_t  planeCount = m_planes.size();
    for (; planeIndex < planeCount && testPointsAgainstPlane (&anyInside,  pPoints, pPoints + numPoints, &m_planes[planeIndex]); planeIndex++)
        ;

    if (planeIndex == planeCount)
        {
        clipStack->ClipPoints (rangeCalculator, numPoints, pPoints, clipIndex);
        }
    else if (anyInside)
        {
        ClipPlaneCP         pPlane = &m_planes[planeIndex];

        // Combination of inside and outside, clip required.
        int             nOutputPoints;
        bool            thisInside, lastInside;
        double          thisDistance, lastDistance;
        DPoint3dCP      pThisPoint, pLastPoint, pEnd = pPoints + numPoints;

        pLastPoint    = pPoints;
        lastDistance  = pPlane->EvaluatePoint (*pLastPoint);
        lastInside    = lastDistance > 0.0;

        pThisPoint    = pPoints + 1;

        SafeDPoint3dArray tPts(numPoints);
        DPoint3dP outputPoints = tPts.GetData();
        DPoint3dP outputPoint  = outputPoints;

        if (lastInside)
            *outputPoint++ = *pLastPoint;

        for (; pThisPoint < pEnd; pThisPoint++)
            {
            thisDistance  = pPlane->EvaluatePoint (*pThisPoint);
            thisInside    = thisDistance > 0.0;

            if (lastInside != thisInside)
                {
                double      t = lastDistance / (lastDistance - thisDistance);

                outputPoint->sumOf (NULL, pLastPoint, 1.0 - t, pThisPoint, t);
                outputPoint++;

                if ((nOutputPoints = static_cast<int>((outputPoint - outputPoints))) > 1)
                    {
                    ClipPoints (rangeCalculator, clipStack, nOutputPoints, outputPoints, clipIndex, planeIndex+1);
                    outputPoint = outputPoints;
                    }
                }

            if (thisInside)
                *outputPoint++ = *pThisPoint;

            pLastPoint    = pThisPoint;
            lastDistance = thisDistance;
            lastInside   = thisInside;
            }

        if ((nOutputPoints = static_cast<int>((outputPoint - outputPoints))) > 0)
            ClipPoints (rangeCalculator, clipStack, nOutputPoints, outputPoints, clipIndex, planeIndex+1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::Init (ViewContextP context)
    {
    SetViewContext (context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_PushTransClip (TransformCP trans, ClipPlaneSetCP clip)
    {
    T_Super::_PushTransClip (trans, clip);

    m_rangeClipStack.Push (clip, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::UpdateRange (int numPoints, DPoint3dCP points)
    {
    m_elRange.Union (numPoints, points,  GetCurrRangeClip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::UpdateRange (int numPoints, DPoint2dCP points)
    {
    m_elRange.Union (numPoints, points, GetCurrRangeClip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::UpdateRange (DEllipse3dCP ellipse)
    {
    m_elRange.Union (ellipse, GetCurrRangeClip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RangeOutput::_ProcessCurvePrimitive (ICurvePrimitiveCR primitive, bool closed, bool filled)
    {
    switch (primitive.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = primitive.GetLineCP ();

            UpdateRange (2, segment->point);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = primitive.GetLineStringCP ();

            UpdateRange ((int) points->size (), &points->front ());
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP ellipse = primitive.GetArcCP ();

            UpdateRange (ellipse);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCP bcurve = primitive.GetProxyBsplineCurveCP ();

            // NOTE: Using un-weighted pole range is better for fit than MSBsplineCurve::GetPoleRange...
            if (bcurve->HasWeights ())
                {
                bvector<DPoint3d> poles;

                bcurve->GetUnWeightedPoles (poles);
                UpdateRange ((int) poles.size (), &poles.front ());
                break;
                }

            UpdateRange (bcurve->GetIntNumPoles (), bcurve->GetPoleCP ());
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = primitive.GetPointStringCP ();

            UpdateRange ((int) points->size (), &points->front ());
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            {
            // Do nothing, SimplifyViewDrawGeom will recurse...
            break;
            }

        default:
            {
            BeAssert (false && "Unexpected entry in CurveVector.");
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RangeOutput::_ProcessSolidPrimitive (ISolidPrimitiveCR primitive)
    {
    DRange3d    range;

    if (!primitive.GetRange (range))
        return ERROR;

    m_elRange.Union (&range, GetCurrRangeClip ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range)
    {
    UpdateRange (numPoints, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RangeOutput::_DrawBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP, double)
    {
    DRange3d    range;

    // Entity box better than edges, edge range may not be large enough for curved surfaces and it's a lot more work!
    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._GetEntityRange (range, entity))
        return ERROR;

    _PushTransClip (&entity.GetEntityTransform (), NULL);
    m_elRange.Union (&range, GetCurrRangeClip ());
    _PopTransClip ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range)
    {
    UpdateRange (numPoints, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range)
    {
    UpdateRange (numPoints, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool fill, double zDepth, DPoint2dCP range)
    {
    UpdateRange (&ellipse);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY,
                              int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range)
    {
    UpdateRange (4, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawTextString (TextStringCR text, double* zDepth)
    {
    if (text.GetText().empty())
        return;

    double      height      = text.GetStyle().GetHeight();
    DPoint3d    pts[5];

    text.ComputeBoundingShape(pts, 0.0, (fabs(height) / 2.0));
    text.ComputeTransform().Multiply(pts, _countof(pts));

    UpdateRange (5, pts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RangeOutput::_DrawPolyface (PolyfaceQueryCR meshData, bool filled)
    {
    size_t numPoint = meshData.GetPointCount ();

    if (numPoint > 0)
        UpdateRange (static_cast<int>(numPoint), meshData.GetPointCP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    RangeOutput::_PopTransClip () 
    {
    T_Super::_PopTransClip ();
    m_rangeClipStack.Pop ();
    }

/*=================================================================================**//**
* Context to caclulate the range of all elements within a view.
* @bsiclass                                                     RayBentley    09/06
+===============+===============+===============+===============+===============+======*/
struct FitContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
private:
    RangeOutput         m_output;
    FitViewParams&      m_params;

protected:
    virtual DgnModelP    _GetViewTarget () override {return NULL == m_viewport ? m_params.m_modelIfNoViewport : m_viewport->GetViewController ().GetTargetModel(); }
    virtual QvElem*      _DrawCached (IStrokeForCache& stroker) override { stroker._StrokeForCache (*this); return nullptr;}
    virtual void         _SetupOutputs () override {SetIViewDraw (m_output);}

public:
    FitContext (FitViewParams& params) : NullContext (NULL, true), m_params(params)
        {
        m_ignoreViewRange = !params.m_useScanRange;
        m_purpose         = DrawPurpose::FitView;
        m_is3dView        = true;

        m_output.Init (this);
        _SetupOutputs ();
        }

    ElemRangeCalc*          GetElemRange ()                          { return m_output.GetElemRange (); }
    void                    SetViewport (DgnViewportP viewport)         { m_viewport = viewport; }
    void                    SetDrawPurpose (DrawPurpose drawPurpose) { m_purpose = drawPurpose; }   
    void                    SetIs3dView (bool is3dView)              { m_is3dView  = is3dView; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawSymbol (IDisplaySymbol* symbol, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) override
    {
    DRange3d    range;

    if (symbol->_GetRange (range) != BSISUCCESS)
        return;
    
    DPoint3d    corners [8];

    range.get8Corners (corners);

    if (NULL != clip)
        PushClipPlanes (*clip);

    if (NULL != trans)
        PushTransform (*trans);

    m_output.DrawPointString3d (8, corners, NULL);

    if (NULL != trans)
        PopTransformClip ();

    if (NULL != clip)
        PopTransformClip ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void InitFitContext ()
    {
    InvalidateScanRange ();
    m_frustumTransClipDepth = 0;

    m_frustumToNpc  = *m_viewport->GetWorldToNpcMap();
    m_frustumToView = *m_viewport->GetWorldToViewMap();
    m_output.SetViewFlags (*m_viewport->GetViewFlags());

    if (m_params.m_rMatrix || m_viewport)
        {
        Transform  transform;
        transform.InitFrom ((NULL == m_params.m_rMatrix) ? m_viewport->GetRotMatrix() : *m_params.m_rMatrix);

        PushTransform (transform);
        }

    m_transformClipStack.Clear ();      // It is important to clear after the _PushTransform above (TFS# 16267)

    DgnModelP rootModel = _GetViewTarget();

    SetDgnDb (rootModel->GetDgnDb ());
    m_is3dView = rootModel->Is3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsRangeContainedInCurrentRange (DRange3dCR range, bool is3d)
    {
    // If the range of the element is within our current fit range then don't bother visiting it.
    DRange3d    currentRange;

    if (SUCCESS != m_output.GetElemRange()->GetRange (currentRange))
        return false;

    DPoint3d            dRangeCorners[8];
    ElemRangeCalc       elemRangeCalc;

    DRange3d dRange = range;
    if (!is3d)
        dRange.low.z = dRange.high.z = 0;

    dRange.get8Corners (dRangeCorners);
    m_output.GetCurrRangeClip()->ClipRange (&elemRangeCalc, dRangeCorners, 0, true);

    if (SUCCESS != elemRangeCalc.GetRange (dRange))
        return true;

     if (m_params.m_fitMaxDepth || m_params.m_fitMinDepth)
        {
        return (!m_params.m_fitMinDepth || dRange.low.z > currentRange.low.z) &&
               (!m_params.m_fitMaxDepth || dRange.high.z < currentRange.high.z);
        }

    return dRange.IsContained(currentRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ScanTestResult _CheckNodeRange (ScanCriteriaCR criteria, DRange3dCR scanRange, bool is3d, bool isElement) override
    {
    if (ScanTestResult::Fail == T_Super::_CheckNodeRange (criteria, scanRange, is3d, isElement))
        return  ScanTestResult::Fail;

    return IsRangeContainedInCurrentRange (scanRange, is3d) ? ScanTestResult::Fail : ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ScanRangeFromPolyhedron()
    {
    if (m_ignoreViewRange && !m_useNpcSubRange)
        {
        // Rather than no range test - use a big range so range tree still gets used (and we can reject nodes).
        DRange3d bigRange;
        bigRange.low.x = bigRange.low.y = bigRange.low.z = -1.0e20;
        bigRange.high.x = bigRange.high.y = bigRange.high.z = 1.0e20;

        m_scanCriteria->SetRangeTest (&bigRange);
        return  true;
        }
    return  T_Super::_ScanRangeFromPolyhedron ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            _SetScanReturn () override
    {
    T_Super::_SetScanReturn ();
    m_scanCriteria->SetReturnType (MSSCANCRIT_ITERATE_ELEMENT_UNORDERED, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _VisitElement (GeometricElementCR element) override
    {
    if (IsRangeContainedInCurrentRange(element._GetRange3d(), element.Is3d()))
        return SUCCESS;

    // NOTE: Can just draw bounding box instead of drawing element geometry...
    DPoint3d  corners[8];
    DRange3d  range = element._GetElementBox3d();
    Transform placementTrans = element._GetPlacementTrans();

    range.Get8Corners(corners);
    placementTrans.Multiply(corners, 8);
    GetIDrawGeom().DrawPointString3d(8, corners, nullptr);

    return SUCCESS;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::ComputeViewRange (DRange3dR range, FitViewParams& params) 
    {
    Json::Value oldState;
    m_viewController->SaveToSettings(oldState);

    // first give the viewController a chance to decide the range and/or load elements.
    // the default implementation of _OnComputeFitRange will simply return the range of the range tree.
    if (m_viewController->_OnComputeFitRange(range, *this, params))
        return  SUCCESS;

    // now do a normal query to find the elements that are within this range.
    // the purpose of this query is to make the returned range include only the elements that are
    // actually displayed in the view. That might be smaller than 'range'.
    FitContext  context (params);
    context.AllocateScanCriteria ();

    if (SUCCESS != SetupFromViewController()) // can't proceed if viewport isn't valid (e.g. not active)
        return ERROR;

    context.SetViewport(this); // Don't want to attach...but transients have view display mask!
    context.InitFitContext();
    context.VisitAllViewElements(params.m_includeTransients, NULL);

    m_viewController->RestoreFromSettings(oldState);
    _SynchWithViewController(false);

#if defined (NEED_NEW_APPROACH)

    // This approach doesn't work correctly with QueryViews, because sometimes the database can be so large that nearly all of the elements 
    // are filtered on size criteria. But if at least one isn't filtered, we take them to be the entire universe. We need a better approach.
    // For now, every fit will always go to the extent of all elements in the project, regardless of whether they're displayed in this view or not. KAB 3/2015

    // get the fitted range from the FitContext and return that in 'range'. 
    // however, if the fit found no elements bigger than 1 pixel (because 'range' is very, very large), then 
    // don't return an invalid range -- return the full range. we know it's a good range, even if it's not as tight as it could be.
    DRange3d fullRange;
    if (SUCCESS == context.GetElemRange()->GetRange(fullRange))
        range = fullRange;
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnViewport::ComputeFittedElementRange (DRange3dR rangeUnion, IElementSetR elements, RotMatrixCP rMatrix)
    {
    FitViewParams params;
    params.m_rMatrix = rMatrix;//Old function had this feature. So retaining it

    FitContext  context (params);
    context.SetViewport(this);
    context.AllocateScanCriteria ();
    context.InitFitContext ();

    ElementHandle curr;  
    for (bool more = elements.GetFirst(curr); more; more = elements.GetNext(curr))
        {
        if (!curr.IsValid())
            continue;

        GeometricElementCP geom = curr.GetGeometricElement();

        if (nullptr == geom)
            continue;

        ViewContext::ContextMark mark (&context);

        context.VisitElement (*geom);
        }
    
    return context.GetElemRange()->GetRange (rangeUnion);
    }

/*=================================================================================**//**
* Context to caclulate the range of all elements within a view.
* @bsiclass                                                     RayBentley    09/06
+===============+===============+===============+===============+===============+======*/
struct DepthFitContext : public FitContext
{
    DEFINE_T_SUPER(FitContext)    
    DepthFitContext (FitViewParams& params) : FitContext (params) {}

    virtual void _PushFrustumClip () override {ViewContext::_PushFrustumClip(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _VisitElement (GeometricElementCR element)
    {
    // Check range - this is much less expensive than clipping and accumulating ranges for geometry outside the view.
    if (_FilterRangeIntersection (element))
        return SUCCESS;

    return T_Super::_VisitElement (element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void InitDepthFitContext ()
    {
    InitFitContext ();

#if defined (NEEDS_WORK_UNITS)
    Frustum frustum = GetFrustum();
    int         nPlanes;
    ClipPlane   frustumPlanes[6];
    ViewFlagsCP viewFlags = GetViewFlags();

    // DepthFitContext needs the frustum planes in RangeOutput also to properly
    // clip elements that span outside the view.
    if (0 != (nPlanes = ClipUtil::RangePlanesFromPolyhedra (frustumPlanes, frustum.GetPts(), NULL != viewFlags && !viewFlags->noFrontClip, NULL != viewFlags && !viewFlags->noBackClip, 1.0E-6)))
        {
        m_transformClipStack.PushClipPlanes (frustumPlanes, nPlanes);

        ClipPlaneSet planeSet (frustumPlanes, nPlanes);
        DirectPushTransClipOutput (*m_IDrawGeom, NULL, &planeSet);
        }
#endif

    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::DetermineVisibleDepthNpc (double& lowNpc, double& highNpc, DRange3dCP subRectNpc)
    {
    FitViewParams params;
    params.m_useScanRange = true;
    params.m_fitMinDepth = params.m_fitMaxDepth = true;

    DepthFitContext context (params);

    context.SetViewport (this);
    if (subRectNpc)
        context.SetSubRectNpc(*subRectNpc);

    context.AllocateScanCriteria ();
    context.InitDepthFitContext ();
    context.VisitAllViewElements (true, NULL);

    lowNpc = 0.0;
    highNpc = 1.0;
    DRange3d range;

    if (SUCCESS != context.GetElemRange()->GetRange (range))
        return ERROR;
    
    m_rotMatrix.MultiplyTranspose(&range.low, &range.low, 2);
    WorldToNpc(&range.low, &range.low, 2);

    lowNpc = range.low.z;
    highNpc = range.high.z;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IViewTransients::ComputeRange (DRange3dR range, DgnViewportP vp)
    {
    // NEEDSWORK_WIP_RANGE - Do we need to keep this method if fit starts using project extents???
    //                       Can maybe just have a virtual _GetRange method on IViewTransient and
    //                       require implemention to keep "geometry" range if needed...
    if (SUCCESS != vp->SetupFromViewController()) // can't proceed if viewport isn't valid (e.g. not active)
        return  ERROR;

    FitViewParams params;
    params.m_fitRasterRefs = true;

    FitContext  context (params);
    context.SetViewport (vp); // Don't want to attach...but transients have view display mask!
    context.AllocateScanCriteria ();
    context.InitFitContext ();

    _DrawTransients (context, false);

    return context.GetElemRange()->GetRange (range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/99
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::ComputeTransientRange (DRange3dP range, RotMatrixP rMatrix, bool checkLevelClass) const
    {
    // NEEDSWORK_WIP_RANGE - Called by QvViewport::_AdjustZPlanesToModel, goes aways when we have project "extents".
    return ERROR;
    }
