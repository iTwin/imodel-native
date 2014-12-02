/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MultilineHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define MLBREAK_FROM_JOINT              0x8000
#define MLBREAK_TO_JOINT                0x4000

/*=================================================================================**//**
*
* @bsiclass                                                     Keith.Bentley   10/04
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct          MlineTopology : IElemTopology
{
private:
    int     m_lineNumber;  // multiline line number
    int     m_capNumber;   // multiline cap number
    int     m_lSBase;      // multiline base for m_closeVertex/m_segmentNumber

public:
    MlineTopology();
    explicit MlineTopology (MlineTopology const&);
    virtual MlineTopology* _Clone () const override {return new MlineTopology (*this);}

    int GetLineNumber () const      {return m_lineNumber;}
    int GetCapNumber () const       {return m_capNumber;}
    int GetLsBase () const          {return m_lSBase;}

    void SetLineNumber (int num)    {m_lineNumber = num;}
    void SetCapNumber (int num)     {m_capNumber  = num;}
    void SetLsBase (int num)        {m_lSBase     = num;}
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
MlineTopology::MlineTopology ()
    {
    m_lineNumber = 0;
    m_capNumber  = 0;
    m_lSBase     = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
MlineTopology::MlineTopology (MlineTopology const& from)
    {
    m_lineNumber = from.m_lineNumber;
    m_capNumber  = from.m_capNumber;
    m_lSBase     = from.m_lSBase;
    }

/*---------------------------------------------------------------------------------**//**
* get the parameters from a hitpath to a multiline
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       HitPath::GetMultilineParameters
(
DSegment3dP     segPoints,          // <=
int*            vertex,             // <=
int*            segmentNumber,      // <=
int*            lineNumber,         // <=
int*            capNumber,          // <=
int*            patIndex            // <=
)   const
    {
    GeomDetail const&   detail = GetGeomDetail ();

    // NOTE: Always return patIndex...won't have mline topo for a pattern hit...
    if (patIndex)
        *patIndex = detail.GetPatternIndex ();

    MlineTopology const *topoP = dynamic_cast <MlineTopology const *> (GetElemTopology ());

    if (NULL == topoP)
        return ERROR;

    if (segPoints)
        {
        // Return segment in active coords...
        if (!detail.GetSegment (*segPoints))
            segPoints->Init (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        else
            detail.GetLocalToWorld ().MultiplyAndRenormalize (segPoints->point, segPoints->point, 2);
        }

    if (vertex)
        {
        switch (topoP->GetCapNumber ())
            {
            case 1: /* org cap */
                *vertex = topoP->GetLsBase ();
                break;
            case 2: /* mid cap */
                *vertex = topoP->GetLsBase ();
                break;
            case 3: /* end cap */
                *vertex = topoP->GetLsBase () + 1;
                break;
            default:
                *vertex = topoP->GetLsBase () + (int) detail.GetCloseVertex ();
                break;
            }
        }

    if (segmentNumber)
        {
        switch (topoP->GetCapNumber ())
            {
            case 1: /* org cap */
            case 2: /* mid cap */
            case 3: /* end cap */
                *segmentNumber = topoP->GetLsBase ();
                break;
            default:
                *segmentNumber = topoP->GetLsBase () + (int) detail.GetSegmentNumber ();
                break;
            }
        }

    if (lineNumber)
        *lineNumber = topoP->GetLineNumber ();

    if (capNumber)
        *capNumber = topoP->GetCapNumber ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineHandler::IsIndexValid (ElementHandleCP source, UInt32 index) const
    {
    if (index > MULTILINE_MAX)
        return false;

    if (NULL != source && index > GetProfileCount (*source))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::GetAnyJointDef
(
ElementHandleCR eh,
JointDef*       jointDef,       /* <= Definition of joint              */
DSegment3dCP lineSeg,        /* => Current line segment             */
int             pointNo,        /* => Current point number at joint    */
int             pointAtOrg      /* => True if pointNo at origin of seg */
)
    {
    DPoint3d    segPoints[3];

    if (pointNo == 0)
        {
        MultilineHandler::GetCapJointDef (*jointDef, eh, &lineSeg->point[0], 0);
        }
    else if (pointNo == eh.GetElementCP ()->ToMlineElm().nPoints - 1)
        {
        MultilineHandler::GetCapJointDef (*jointDef, eh, &lineSeg->point[0], 1);
        }
    else
        {
        if (pointAtOrg)
            {
            MultilinePointPtr mlinePoint = GetMlinePointStatic (eh, pointNo-1);
            segPoints[0] = mlinePoint->GetPoint ();

            segPoints[1] = lineSeg->point[0];
            segPoints[2] = lineSeg->point[1];
            }
        else
            {
            segPoints[0] = lineSeg->point[0];
            segPoints[1] = lineSeg->point[1];

            MultilinePointPtr mlinePoint = GetMlinePointStatic (eh, pointNo+1);
            segPoints[2] = mlinePoint->GetPoint ();
            }

        MultilineHandler::GetJointDef (*jointDef, eh, segPoints, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::CalculatePerpVec
(
DPoint3dP       perpvec,
ElementHandleCR    eh,
DPoint3dCP      linePoints
)
    {
    DVec3d      parvec = DVec3d::FromStartEnd (linePoints[0], linePoints[1]), zvec;

    zvec.init (&eh.GetElementCP ()->ToMlineElm().zVector);
    bsiDVec3d_crossProduct ((DVec3d *) perpvec, &zvec, &parvec);
    ((DVec3d *) perpvec)->Normalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::CreateMlineAssoc
(
ElementHandleCR    eh,
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,           /* => allowed association type mask */
bool            createFarPathElems,
DgnModelP    parentModel
)
    {
    // NEEDSWORK: Should use IMultilineQuery...
    if (0 == (MLINE_ASSOC & ~modifierMask))
        return ERROR;

    int         vertex, segment, lineNo, capNo;

    if (SUCCESS != snapPath.GetMultilineParameters (NULL, &vertex, &segment, &lineNo, &capNo, NULL))
        return ERROR;

    // No association allowed for end-caps
    if (0 != capNo)
        return ERROR;

    bool        keyPoint = true, joint = false;
    DPoint3d    jointPoint, dPoints[2], hitPoint;

    assoc_getSnapPoint (hitPoint, &snapPath, parentModel, true);
    ExtractPointArray (eh, dPoints, segment, 2);

    DgnElementCP elemP = eh.GetElementCP();

    if (elemP->ToMlineElm().flags.closed && elemP->ToMlineElm().nPoints > 3 && vertex == elemP->ToMlineElm().nPoints - 1)
        vertex = 0;

    /*---------------------------------------------------------------
    Compute the actual joint point to check for joint assoc
    ---------------------------------------------------------------*/
    double      offset = elemP->ToMlineElm().profile[lineNo].dist;
    JointDef    jointDef;

    if (vertex == segment)
        {
        DSegment3d segmentPoints;
        GetAnyJointDef (eh, &jointDef,  &segmentPoints, segment, true);
        bsiDPoint3d_addScaledDVec3d (&jointPoint, &segmentPoints.point[0], (DVec3dP) &jointDef.dir, offset * jointDef.scale);
        }
    else
        {
        DSegment3d segmentPoints;
        GetAnyJointDef (eh, &jointDef,  &segmentPoints, segment+1, false);
        bsiDPoint3d_addScaledDVec3d (&jointPoint, &segmentPoints.point[1], (DVec3dP) &jointDef.dir, offset * jointDef.scale);
        }

    int         pointNo = 0;
    double      snapDist, segDist;

    /*---------------------------------------------------------------
    Check for snap to joint point - make joint assoc
    The actual joint point stroked will have been rounded (Point3d)
    so it is necessary to convert to int for comparison.
    ---------------------------------------------------------------*/
    if (LegacyMath::RpntEqual (&hitPoint, &jointPoint))
        {
        pointNo = vertex;
        joint   = true;
        offset  = 0.0;
        }
    else
        {
        DVec3d  hitDir, segDir;

        /*-------------------------------------------------------
        Not a joint association. Store the offset of the point
        projected onto the work line.
        -------------------------------------------------------*/
        pointNo = segment;
        joint   = false;

        segDir.NormalizedDifference (dPoints[1], *dPoints);
        bsiDVec3d_subtractDPoint3dDPoint3d (&hitDir, &hitPoint, dPoints);

        snapDist = bsiDVec3d_dotProduct (&segDir, &hitDir);

        if (keyPoint)
            {
            segDist = bsiDPoint3d_distance (dPoints, dPoints+1);

            if (segDist != 0.0)
                offset = snapDist / segDist;
            else
                offset = 0.0;
            }
        else
            {
            offset = snapDist;
            }
        }

    AssociativePoint::InitMline (assoc, (UInt16)pointNo, elemP->ToMlineElm().nPoints, (UInt16)lineNo, (UInt16)offset, joint);

    if (SUCCESS != AssociativePoint::SetRoot (assoc, &snapPath, parentModel, createFarPathElems))
        return ERROR;

    return AssociativePoint::IsValid (assoc, snapPath.GetEffectiveRoot (), parentModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MultilineHandler::EvaluateMlineAssoc (ElementHandleCR eh, AssocPoint& assocPoint, DPoint3dR outPoint)
    {
    DSegment3d seg1;
    DPoint3d    dPoint;
    DVec3d      segDir;
    JointDef    joint;
    double      dist, fraction;
    MlineAssoc* assoc = (MlineAssoc*) &assocPoint;

    if (MULTILINE_ELM != eh.GetLegacyType())
        return ERROR;

    DgnElementCP elm = eh.GetElementCP ();

    if (assoc->pointNo >= 0 && assoc->nVertex && assoc->nVertex != MultilineHandler::GetPointCountStatic (eh))
        return ASSOC_TOPO_CHANGE;

    if (assoc->pointNo < 0 || assoc->pointNo >= elm->ToMlineElm().nPoints)
        return BAD_ASSOCIATION;

    /*-------------------------------------------------------------------
    Get a line segment and a start point for the association
    -------------------------------------------------------------------*/
    if (assoc->pointNo == elm->ToMlineElm().nPoints - 1)
        {
        ExtractPointArray (eh, (DPoint3dP) &seg1, assoc->pointNo - 1, 2);
        dPoint = seg1.point[1];
        }
    else
        {
        ExtractPointArray (eh, (DPoint3dP) &seg1, assoc->pointNo, 2);
        dPoint = seg1.point[0];
        }

    /*-------------------------------------------------------------------
    If there is an offset and its not a joint association project the
    start point along the work line.
    -------------------------------------------------------------------*/
    if (!assoc->b.joint)
        {
        if (assoc->b.project)
            {
            fraction = assoc->offsetVal;
            dPoint.x = seg1.point[0].x + fraction * (seg1.point[1].x - seg1.point[0].x);
            dPoint.y = seg1.point[0].y + fraction * (seg1.point[1].y - seg1.point[0].y);
            dPoint.z = seg1.point[0].z + fraction * (seg1.point[1].z - seg1.point[0].z);
            }
        else
            {
            segDir.NormalizedDifference (*( &seg1.point[1]), *( &seg1.point[0]));
            dPoint.SumOf(dPoint, segDir,  assoc->offsetVal);
            }
        }

    /*-------------------------------------------------------------------
    If the association is not to the work line, the point must be
    projected out to the association line.
    -------------------------------------------------------------------*/
    if (assoc->b.lineNo < elm->ToMlineElm().nLines && (dist = elm->ToMlineElm().profile[assoc->b.lineNo].dist))
        {
        /*---------------------------------------------------------------
        If its a joint association, project the point out the joint
        bvector. Otherwise project it out the perpendicular.
        ---------------------------------------------------------------*/
        if (assoc->b.joint)
            {
            GetAnyJointDef (eh, &joint, &seg1, assoc->pointNo, true);
            dPoint.SumOf(dPoint, *( (DVec3dP) &joint.dir),  dist * joint.scale);
            }
        else
            {
            CalculatePerpVec (&segDir, eh, seg1.point);
            dPoint.SumOf(dPoint, *( (DVec3dP) &segDir),  dist);
            }
        }

    outPoint = dPoint;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::ExtractPointArray (ElementHandleCR eh, DPoint3dP outPoints, UInt32 start, size_t numPoints)
    {
    BeAssert (start+numPoints <= GetPointCountStatic (eh));
    if (start+numPoints > GetPointCountStatic (eh))
        return ERROR;

    for (UInt32 iPoint=start, count=0; count<numPoints; iPoint++, count++)
        {
        MultilinePointPtr mlinePoint  = GetMlinePointStatic (eh, iPoint);
        if (mlinePoint.IsValid())
            outPoints[count] = mlinePoint->GetPoint ();
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @param outPoints OUT buffer for points
* @param outCount OUT number of returned points.
* @param el IN source element
* @param index IN first point index
* @param maxOut IN max allowed points to return.
* @param nPoints IN number of points to access
* @param pTransform IN optional transform.
* @return SUCCESS if el is an mline and one or more points were returned.
* @bsimethod                                                    EarlinLutz  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::ExtractTransformedPointsExt
(
DPoint3dP       outPoints,
UInt32*         outCount,
ElementHandleCR eh,
UInt32          index,
UInt32          maxOut,
TransformCP     pTransform
) const
    {
    int         sourceCount;

    *outCount = 0;

    if (MULTILINE_ELM != eh.GetLegacyType())
        return ERROR;

    sourceCount = eh.GetElementCP ()->ToMlineElm().nPoints;
    *outCount = sourceCount - index;

    if (*outCount > maxOut)
        *outCount = maxOut;

    if (*outCount <= 0)
        return ERROR;

    ExtractPointArray (eh, outPoints, index, *outCount);

    if (pTransform)
        pTransform->multiply (outPoints, *outCount);

    return *outCount > 0 ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @param pXYZOut OUT closest point
* @param pParmaOut OUT parameter at closest point.
* @param el IN subject multiline element
* @param bExtend IN true to extend non-closed multiline
* @param pXYZIn IN space point
* @param pTransform IN optional transform
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::ClosestCenterlinePoint
(
DPoint3dP       pXYZOut,
double*         pParamOut,
ElementHandleCR    eh,
bool            bExtend,
DPoint3dCP      pXYZIn,
TransformCP     pTransform
) const
    {
    UInt32      numVertex;
    DPoint3d    xyz[MAX_VERTICES];

    if (SUCCESS != ExtractTransformedPointsExt (xyz, &numVertex, eh, 0, MAX_VERTICES, pTransform))
        return ERROR;

    if (eh.GetElementCP ()->ToMlineElm().flags.closed)
        bExtend = FALSE;

    return (BentleyStatus)LineStringUtil::ClosestPoint (pXYZOut, pParamOut, xyz, numVertex, bExtend, pXYZIn);
    }

/*---------------------------------------------------------------------------------**//**
* @param pXYZOut OUT closest point. This is in xyz world coordinates -- the coordinates
*             of the element as transformed by pElementTransform.
* @param pParmaOut OUT parameter at closest point.
* @param pXYDistanceOut OUT
* @param el IN subject multiline element
* @param bExtend IN true to extend non-closed multiline
* @param pXYZIn IN space point, in world coordinates
* @param pElementTransform IN optional transform.
* @param pViewTransorm IN transform from world to view in which xy is measured.
* @bsimethod                                     EarlinLutz         05/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::ClosestCenterlinePointXY
(
DPoint3dP       pXYZOut,
double*         pParamOut,
double*         pXYDistanceOut,
ElementHandleCR    eh,
bool            bExtend,
DPoint3dCP      pXYZIn,
TransformCP     pElementTransform,
DMatrix4dCP     pViewTransform
) const
    {
    UInt32      numVertex;
    DPoint3d    xyz[MAX_VERTICES];

    if (SUCCESS != ExtractTransformedPointsExt (xyz, &numVertex, eh, 0, MAX_VERTICES, pElementTransform))
        return ERROR;

    if (eh.GetElementCP ()->ToMlineElm().flags.closed)
        bExtend = FALSE;

    return (BentleyStatus)LineStringUtil::ClosestPointXY (pXYZOut, pParamOut, pXYDistanceOut, xyz, numVertex, bExtend, pXYZIn, pViewTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @param pXYZOut OUT points on linestring subset.
* @param pNumOut OUT number of output points
* @param maxOut IN size of output buffers
* @param el IN source element
* @param parameterA IN start parameter.
* @param parameterB IN end parameter.
* @param pTransform IN optional transform.
* @return SUCCESS if el is an mline and output buffer is sufficient.
* @bsimethod                                                    EarlinLutz  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::ExtractTransformedPartial
(
DPoint3dP       pXYZOut,
int*            pNumOut,
UInt32          maxOut,
ElementHandleCR    eh,
double          parameterA,
double          parameterB,
TransformCP     pTransform
) const
    {
    UInt32      numVertex;
    DPoint3d    xyz[MAX_VERTICES];

    if (SUCCESS != ExtractTransformedPointsExt (xyz, &numVertex, eh, 0, MAX_VERTICES, pTransform))
        return ERROR;

    bool     bClosed = eh.GetElementCP ()->ToMlineElm().flags.closed;

    return (BentleyStatus)LineStringUtil::ExtractPartial (pXYZOut, pNumOut, maxOut, xyz, numVertex, bClosed, parameterA, parameterB);
    }

/*---------------------------------------------------------------------------------**//**
* @param pLength OUT computed linestring length. Open mlines are "extended".  Closed are wrapped.
* @param el IN source element
* @param parameterA IN start parameter.
* @param parameterB IN end parameter.
* @param pTransform IN optional transform.
* @return SUCCESS if el is an mline
* @bsimethod                                                    EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::SignedDistanceAlong
(
double*         pLength,
ElementHandleCR    eh,
double          parameterA,
double          parameterB,
TransformCP     pTransform
) const
    {
    UInt32      numVertex;
    DPoint3d    xyz[MAX_VERTICES];

    if (SUCCESS != ExtractTransformedPointsExt (xyz, &numVertex, eh, 0, MAX_VERTICES, pTransform))
        return ERROR;

    bool     bClosed = eh.GetElementCP ()->ToMlineElm().flags.closed;

    return (BentleyStatus)LineStringUtil::SignedDistanceAlong (pLength, xyz, numVertex, bClosed, parameterA, parameterB);
    }

/*---------------------------------------------------------------------------------**//**
* @param pXYZOut OUT computed point.
* @param pFractionOut OUT parameter at computed point.
* @param el IN source element
* @param fractionA IN fractional parameter for start point.
* @param distance IN signed distance from start point A to computed point B.
* @param pTransform IN optional transform.
* @return SUCCESS if el is an mline and point computed.
* @bsimethod                                                    EarlinLutz  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::PointAtSignedDistance
(
DPoint3dP       pXYZOut,
double*         pFractionOut,
ElementHandleCR    eh,
double          fractionA,
double          distanceAB,
TransformCP     pTransform
) const
    {
    UInt32      numVertex;
    DPoint3d    xyz[MAX_VERTICES];

    if (SUCCESS != ExtractTransformedPointsExt (xyz, &numVertex, eh, 0, MAX_VERTICES, pTransform))
        return ERROR;

    bool     bClosed = eh.GetElementCP ()->ToMlineElm().flags.closed;

    return (BentleyStatus)LineStringUtil::PointAtSignedDistance (pXYZOut, pFractionOut, xyz, numVertex, bClosed, fractionA, distanceAB);
    }

/*---------------------------------------------------------------------------------**//**
* @param point OUT point on mline primary linestring
* @param tangent OUT forward tangent.
* @param el IN source element
* @param fraction IN fraction parameter
* @return SUCCESS if el is an mline
* @bsimethod                                                    EarlinLutz  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineHandler::EvaluatePoint
(
DPoint3dP       point,
DVec3dP         tangent,
ElementHandleCR    eh,
double          fraction
) const
    {
    if (point)
        bsiDPoint3d_zero (point);

    if (tangent)
        bsiDVec3d_zero (tangent);

    UInt32 numPoints = MultilineHandler::GetPointCount (eh);

    if (numPoints == 1)
        {
        ExtractTransformedPointsExt (point, &numPoints, eh, 0, 1, NULL);
        }
    else
        {
        int         period, edgeIndex;
        double      edgeFraction;
        DPoint3d    segpoint[2];

        bool     bClosed = eh.GetElementCP ()->ToMlineElm().flags.closed;
        LineStringUtil::DecodeStringFraction (&period, &edgeIndex, &edgeFraction, fraction, numPoints, bClosed);
        ExtractTransformedPointsExt (segpoint, &numPoints, eh, edgeIndex, 2, NULL);

        if (point)
            bsiDPoint3d_interpolate (point, &segpoint[0], edgeFraction, &segpoint[1]);

        if (tangent)
            bsiDVec3d_subtractDPoint3dDPoint3d (tangent, &segpoint[1], &segpoint[0]);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Apply transform pNormal0, returning pNormal1, allowing skew or projection in the transform.
* @bsimethod                                                    EarlinLutz 08/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::TransformMlineNormal (DVec3dP pNormal1, DVec3dP pNormal0, TransformCP pTransform) const
    {
    RotMatrix   A;

    bsiRotMatrix_initFromTransform (&A, pTransform);

    *pNormal1 = *pNormal0;

    if (bsiRotMatrix_isOrthogonal (&A))
        {
        // simple case, just multiply the bvector.
        bsiRotMatrix_multiplyDPoint3d (&A, pNormal1);
        }
    else
        {
        // apply the matrix to the inplane vectors.
        // the normal to the image plane is our result.
        DPoint3d xVec, yVec, zVec;
        bsiDPoint3d_getNormalizedTriad (pNormal0, &xVec, &yVec, &zVec);
        bsiRotMatrix_multiplyDPoint3d (&A, &xVec);
        bsiRotMatrix_multiplyDPoint3d (&A, &yVec);
        bsiDPoint3d_crossProduct (pNormal1, &xVec, &yVec);
        }

    if (pNormal1->Normalize () < 1.0E-15)
        *pNormal1 = *pNormal0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::TransformMline (EditElementHandleR eeh, TransformCP ct, bool mirrorOffsets, bool scaleOffsets)
    {
    MlinePoint  *mlinePoints, *mPointP;
    MlineBreak  *mBreaks;
    DVec3d      zVector, segDir, perpVec, tmpVec;
    DPoint3d    dPoints[MLINE_MAXPOINTS];
    UInt32      nPts;

    // NOTE: Make sure we have an edP so methods like GetFirstBreak that use GetElementCP return &edP->el..
    if (NULL == eeh.GetElementDescrP ())
        return ERROR;

    mlinePoints = mPointP = (MlinePoint*) MultilineHandler::GetFirstPoint (eeh);
    mBreaks = (MlineBreak*) MultilineHandler::GetFirstBreak (eeh);
    ExtractTransformedPointsExt (dPoints, &nPts, eeh, 0, eeh.GetElementP ()->ToMlineElm().nPoints, NULL);

    MlineElm*   mline = &eeh.GetElementP ()->ToMlineElmR();

    if (scaleOffsets)
        {
        CalculatePerpVec (&perpVec, eeh, dPoints);

        for (int i=0; i<mline->nLines; i++)
            {
            double  sign;

            bsiDVec3d_scale (&tmpVec, &perpVec, (double)mline->profile[i].dist);
            ct->MultiplyMatrixOnly (tmpVec);

            /*-----------------------------------------------------------
            Make sure we preserve the sign of the offset.
            -----------------------------------------------------------*/
            sign = mline->profile[i].dist >= 0 ? 1.0 : -1.0;
            mline->profile[i].dist = sign * bsiDVec3d_magnitude (&tmpVec);
            }

        /* Also set the style scale if applicable.  I believe that the complex calculations
           are to handle non-linear scaling, but the end result has to be a fixed scale since
           mlines are planar */
        if (0 != mline->styleParentId)
            {
            if (0 == mline->styleScale)
                mline->styleScale = 1.0;

            tmpVec = perpVec; // Magnitude 1
            ct->MultiplyMatrixOnly (tmpVec);
            mline->styleScale *= bsiDVec3d_magnitude (&tmpVec);
            }
        }

    for (int i=0; i<mline->nPoints; i++)
        {
        if (mBreaks && i < mline->nPoints-1 && mPointP->nBreaks)
            {
            segDir.NormalizedDifference (dPoints[(i+1)], dPoints[i]);

            for (int j=mPointP->breakNo; j<mPointP->breakNo+mPointP->nBreaks; j++)
                {
                bsiDVec3d_scale (&tmpVec, &segDir, (double)mBreaks[j].offset);
                ct->MultiplyMatrixOnly (tmpVec);
                mBreaks[j].offset = (bsiDVec3d_magnitude (&tmpVec));

                if (scaleOffsets)
                    {
                    bsiDVec3d_scale (&tmpVec, &segDir, (double)mBreaks[j].length);
                    ct->MultiplyMatrixOnly (tmpVec);
                    mBreaks[j].length = (bsiDVec3d_magnitude (&tmpVec));
                    }
                }
            }

        ct->Multiply(dPoints[i]);
        mPointP++;
        }

    for (int i=0; i<mline->nPoints; i++)
        mlinePoints[i].point = *(dPoints+i);

    zVector.init (&mline->zVector);

    if (mline->Is3d())
        {
        DVec3d  originalZVector = zVector;

        TransformMlineNormal (&zVector, &originalZVector, ct);
        }

    if ((ct->Determinant () < 0.0) && mirrorOffsets)
        bsiDPoint3d_negateInPlace (&zVector);

    mline->zVector = zVector;

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_MULTILINE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
MlinePoint const*   MultilineHandler::GetFirstPoint (ElementHandleCR eh) const
    {
    DgnElementCR el = *eh.GetElementCP ();

    return ((MlinePoint const*) (el.ToMlineElm().profile + el.ToMlineElm().nLines));
    }

/*---------------------------------------------------------------------------------**//**
* This is a lot lighter than creating a point object and trying to query it.
* @bsimethod                                                    Chuck.Kirschman  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32   MultilineHandler::GetNumBreaksOnSegment (ElementHandleCR eh, UInt32 pointNum)
    {
    DgnElementCR el = *eh.GetElementCP ();

    MlinePoint const * firstPoint = (MlinePoint*) (el.ToMlineElm().profile + el.ToMlineElm().nLines);
    return firstPoint[pointNum].nBreaks;
    }

/*---------------------------------------------------------------------------------**//**
* This is a lot lighter than creating a point object and trying to query it.
* @bsimethod                                                    Chuck.Kirschman  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32   MultilineHandler::GetSegmentBreakStart (ElementHandleCR eh, UInt32 pointNum)
    {
    DgnElementCR el = *eh.GetElementCP ();

    MlinePoint const * firstPoint = (MlinePoint*) (el.ToMlineElm().profile + el.ToMlineElm().nLines);
    return firstPoint[pointNum].breakNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void   MultilineHandler::GetZVector (DVec3dR zVector, ElementHandleCR eh) const
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM == el.GetLegacyType())
        {
        zVector.init (&el.ToMlineElm().zVector);
        }
    else
        {
        BeAssert (0);
        zVector.init (0.0, 0.0, 0.0);
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::_SetZVector (EditElementHandleR eeh, DVec3dCR zVector)
    {
    DgnElementP  el = eeh.GetElementP ();

    if (MULTILINE_ELM != el->GetLegacyType())
        return ERROR;

    el->ToMlineElmR().zVector = zVector;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::_SetClosed (EditElementHandleR eeh, bool isClosed)
    {
    DgnElementP  el = eeh.GetElementP ();

    if (MULTILINE_ELM != el->GetLegacyType())
        return ERROR;

    if (isClosed == el->ToMlineElm().flags.closed)
        return SUCCESS;

    if (isClosed)
        {
        if (el->ToMlineElm().nPoints < 4)
            return ERROR;

        SetClosePoint (eeh);
        }

    el->ToMlineElmR().flags.closed = isClosed;

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            MultilineHandler::GetPerpVector (DVec3dR perpVec, ElementHandleCR eh, DVec3dR segment)
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return ERROR;

    CalculatePerpVec (&perpVec, eh,  &segment);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::GetLimitProfiles
(
int&            minProfile,
int&            maxProfile,
ElementHandleCR    eh
)
    {
    int         min, max;
    double      minDist, maxDist;
    DgnElementCR el = *eh.GetElementCP ();

    minDist = maxDist = el.ToMlineElm().profile[0].dist;
    min = max = 0;

    for (int i=1; i < el.ToMlineElm().nLines; i++)
        {
        if (el.ToMlineElm().profile[i].dist > maxDist)
            {
            maxDist = el.ToMlineElm().profile[i].dist;
            max = i;
            }

        if (el.ToMlineElm().profile[i].dist < minDist)
            {
            minDist = el.ToMlineElm().profile[i].dist;
            min = i;
            }
        }

    minProfile = min;
    maxProfile = max;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::GetJointDef
(
JointDef&       jointDef,       // <= Definition of joint
ElementHandleCR    eh,
DPoint3dCP      points,         // => Base of point array
int             pointNo         // => Current point number
)
    {
    double      dot;
    DVec3d      perpVec1, perpVec2;

    CalculatePerpVec (&perpVec1, eh,  (points + pointNo));
    CalculatePerpVec (&perpVec2, eh,  (points + (pointNo + 1)));

    bsiDPoint3d_interpolate (&jointDef.dir, &perpVec1, 0.5, &perpVec2);
    ((DVec3dP) &jointDef.dir)->Normalize ();

    /*-------------------------------------------------------------------
    Keep the joint within reason. Limiting the scale to 50 works well.
    -------------------------------------------------------------------*/
    dot = perpVec2.DotProduct (*( (DVec3dP) &jointDef.dir));
    jointDef.scale = fabs (dot) < 0.02 ? 50.0 : 1.0 / dot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
bool     MultilineHandler::GetCloseDef
(
DPoint3dP       capDir,         /* <= Definition of joint       */
ElementHandleCR    eh
)
    {
    DPoint3d    dPoints[4];
    DVec3d      startDir, endDir;

    ExtractPointArray (eh, dPoints, 0, 2);
    ExtractPointArray (eh, dPoints+2, eh.GetElementCP ()->ToMlineElm().nPoints-2, 2);

    if (!LegacyMath::RpntEqual (dPoints, dPoints+3))
        return false;

    startDir.NormalizedDifference (*dPoints, dPoints[1]);
    endDir.NormalizedDifference (dPoints[3], dPoints[2]);
    bsiDPoint3d_interpolate (capDir, &startDir, 0.5, &endDir);
    ((DVec3d *)capDir)->Normalize ();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::GetCapJointDef
(
JointDef&       jointDef,       // <= Definition of joint
ElementHandleCR    eh,
DPoint3dCP      points,         // => Start and end of line segment
int             pointNo         // => Cap joint 0 or 1
)
    {
    DVec3d      lineDir, perpDir, capDir;
    double      theta, cosTheta, sinTheta, dot;

    MlineElm const* mline = &eh.GetElementCP ()->ToMlineElm();

    CalculatePerpVec (&perpDir, eh, points);

    if (mline->nPoints < 4 || !(mline->flags.closed && GetCloseDef (&capDir, eh)))
        {
        theta = pointNo ? mline->endAngle : mline->orgAngle;

        if (theta == msGeomConst_piOver2)
            {
            CalculatePerpVec (&jointDef.dir, eh, points);
            jointDef.scale = 1.0;
            return;
            }
        else
            {
            cosTheta = cos (-theta);
            sinTheta = sin (-theta);

            lineDir.NormalizedDifference (*points, points[1]);

            capDir.x = lineDir.x * cosTheta + perpDir.x * sinTheta;
            capDir.y = lineDir.y * cosTheta + perpDir.y * sinTheta;
            capDir.z = lineDir.z * cosTheta + perpDir.z * sinTheta;
            }
        }

    jointDef.dir = capDir;

    /*-------------------------------------------------------------------
    Keep the joint within reason. Limiting the scale to 50 works well.
    -------------------------------------------------------------------*/
    dot = perpDir.DotProduct (capDir);
    jointDef.scale = fabs (dot) < 0.02 ? 50.0 : 1.0 / dot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32         MultilineHandler::GetPointCountStatic (ElementHandleCR eh)
    {
    // Would like to not need a static method; haven't evaluated if it's possible.
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return 0;

    return el.ToMlineElm().nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_IsClosed (ElementHandleCR eh) const
    {
    return eh.GetElementCP ()->ToMlineElm().flags.closed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32         MultilineHandler::_GetPointCount (ElementHandleCR eh) const
    {
    return GetPointCountStatic(eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineProfile const*     MultilineHandler::_GetMlineProfileDefCP (ElementHandleCR eh, UInt32 index) const
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return NULL;

    if (!IsIndexValid (&eh, index))
        return NULL;

    return &el.ToMlineElm().profile[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineSymbology const*   MultilineHandler::_GetMlineCapSymbologyCP (ElementHandleCR eh, MultilineCapType capType) const
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return NULL;

    switch (capType)
        {
        case MULTILINE_ORG_CAP:
            return &el.ToMlineElm().orgCap;

        case MULTILINE_END_CAP:
            return &el.ToMlineElm().endCap;

        case MULTILINE_MID_CAP:
            return &el.ToMlineElm().midCap;

        default:
            return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr   MultilineHandler::GetMlineCapByIndex (ElementHandleCR eh, MultilineCapType capType) const
    {
    LineStyleParams params;
    MultilineHandler::GetLinestyleParams (eh, params, true, capType, true);

    return new MultilineSymbology (_GetMlineCapSymbologyCP (eh, capType), &params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::SetMlineCapByIndex (EditElementHandleR element, MultilineCapType capType, MultilineSymbologyCR capSymbology)
    {
    // Forces creation of element in handle
    DgnElementP el = element.GetElementP ();

    if (NULL == el || MULTILINE_ELM != el->GetLegacyType())
        return ERROR;

    switch (capType)
        {
        case MULTILINE_ORG_CAP:
            el->ToMlineElmR().orgCap = capSymbology.GetSymbologyCR();
            break;

        case MULTILINE_END_CAP:
            el->ToMlineElmR().endCap = capSymbology.GetSymbologyCR();
            break;

        case MULTILINE_MID_CAP:
            el->ToMlineElmR().midCap = capSymbology.GetSymbologyCR();
            break;

        default:
            return ERROR;
        }

    return SetLinestyleParams (element, capSymbology.GetLinestyleParamsCR(), true, capType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr   MultilineHandler::_GetOriginCap (ElementHandleCR source) const
    {
    return GetMlineCapByIndex (source, MULTILINE_ORG_CAP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr   MultilineHandler::_GetEndCap (ElementHandleCR source) const
    {
    return GetMlineCapByIndex (source, MULTILINE_END_CAP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr   MultilineHandler::_GetMidCap (ElementHandleCR source) const
    {
    return GetMlineCapByIndex (source, MULTILINE_MID_CAP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           MultilineHandler::_SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology)
    {
    return SetMlineCapByIndex (element, MULTILINE_ORG_CAP, capSymbology);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           MultilineHandler::_SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology)
    {
    return SetMlineCapByIndex (element, MULTILINE_END_CAP, capSymbology);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           MultilineHandler::_SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology)
    {
    return SetMlineCapByIndex (element, MULTILINE_MID_CAP, capSymbology);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfilePtr   MultilineHandler::_GetProfile (ElementHandleCR eh, int index) const
    {
    LineStyleParams params;
    MultilineHandler::GetLinestyleParams (eh, params, false, index, true);

    return new MultilineProfile (_GetMlineProfileDefCP (eh, index), &params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::_SetProfile (EditElementHandleR element, UInt32 index, MultilineProfileCR profile)
    {
    DgnElementP el = element.GetElementP ();

    if (NULL == el || MULTILINE_ELM != el->GetLegacyType())
        return ERROR;

    el->ToMlineElmR().profile[index] = profile.GetProfileCR();
    return SetLinestyleParams (element, profile.GetLinestyleParamsCR(), false, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  MultilineHandler::TestScanClassMask (ElementHandleCR eh, int classMask)
    {
    MlineElm const* mline = (MlineElm const*) eh.GetElementCP();

    int         i, nLines, dgnClass;
    MlineProfile const* pMlineProfile;

    nLines  = mline->nLines;
    dgnClass   = 1 << mline->GetElementClassValue();

    /*-------------------------------------------------------------------
    Check the class of the three caps
    -------------------------------------------------------------------*/
    if (mline->orgCap.useClass && (mline->orgCap.capLine || mline->orgCap.capOutArc || mline->orgCap.capInArc))
        if (0 != (classMask & (mline->orgCap.conClass ? (int) DgnElementClass::Construction : (int) DgnElementClass::Primary)))
            return ScanTestResult::Pass;

    if (mline->endCap.useClass && (mline->endCap.capLine || mline->endCap.capOutArc || mline->endCap.capInArc))
        if (0 != (classMask & (mline->endCap.conClass ? (int) DgnElementClass::Construction : (int) DgnElementClass::Primary)))
            return ScanTestResult::Pass;

    if (mline->midCap.useClass && (mline->midCap.capLine || mline->midCap.capOutArc || mline->midCap.capInArc))
        if (0 != (classMask & (mline->midCap.conClass ? (int) DgnElementClass::Construction : (int) DgnElementClass::Primary)))
            return ScanTestResult::Pass;

    /*-------------------------------------------------------------------
    Step through the profile array and check the class of each line
    -------------------------------------------------------------------*/
    for (pMlineProfile = &mline->profile[0], i=0; i<nLines; i++, pMlineProfile++)
        {
        if (pMlineProfile->symb.useClass)
            if (0 != (classMask & (pMlineProfile->symb.conClass ? (int) DgnElementClass::Construction : (int) DgnElementClass::Primary)))
                return ScanTestResult::Pass;
        }

    if (classMask & dgnClass)
        return ScanTestResult::Pass;

    return ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  MultilineHandler::TestScanLevelMask (ElementHandleCR eh, BitMaskCP levelMask)
    {
    MlineElm const* mline = (MlineElm const*) eh.GetElementCP();

    int  iLine, hdrLev, accHdr, level, nLines;
    hdrLev  = mline->GetLevelValue() - 1;

    /* this should never happen */
    if (hdrLev < 0)
        hdrLev = 0;

    accHdr  = NULL == levelMask || levelMask->Test (hdrLev);

    nLines  = mline->nLines;

    /*-------------------------------------------------------------------
    Check the level of the three caps
    -------------------------------------------------------------------*/
    if (mline->orgCap.capLine || mline->orgCap.capOutArc || mline->orgCap.capInArc)
        {
        if (0 == (level = mline->orgCap.level))  /* level == 0 means use header level */
            {
            if (accHdr)
                return ScanTestResult::Pass;
            }
        else
            {
            if (NULL == levelMask || levelMask->Test (level - 1))
                return ScanTestResult::Pass;
            }
        }

    if (mline->endCap.capLine || mline->endCap.capOutArc || mline->endCap.capInArc)
        {
        if (0 == (level = mline->endCap.level))  /* level == 0 means use header level */
            {
            if (accHdr)
                return ScanTestResult::Pass;
            }
        else
            {
            if (NULL == levelMask || levelMask->Test (level - 1))
                return ScanTestResult::Pass;
            }
        }

    if (mline->midCap.capLine || mline->midCap.capOutArc || mline->midCap.capInArc)
        {
        if (0 == (level = mline->midCap.level))  /* level == 0 means use header level */
            {
            if (accHdr)
                return ScanTestResult::Pass;
            }
        else
            {
            if (NULL == levelMask || levelMask->Test (level - 1))
                return ScanTestResult::Pass;
            }
        }

    /*-------------------------------------------------------------------
    Step through the profile array and check the level of each line
    -------------------------------------------------------------------*/
    for (iLine=0; iLine<nLines; iLine++)
        {
        const MlineProfile    *pMlineProfile = &mline->profile[iLine];

        if (0 == (level = pMlineProfile->symb.level))  /* level == 0 means use header level */
            {
            if (accHdr)
                return ScanTestResult::Pass;
            }
        else
            {
            if (NULL == levelMask || levelMask->Test (level - 1))
                return ScanTestResult::Pass;
            }
        }
    return ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  MultilineHandler::_DoScannerTests (ElementHandleCR eh, BitMaskCP levelMask, UInt32 const* classMask, ViewContextP)
    {
    if (levelMask && (ScanTestResult::Pass != TestScanLevelMask (eh, levelMask)))
        return ScanTestResult::Fail;

    if (classMask && (ScanTestResult::Pass != TestScanClassMask (eh, *classMask)))
        return ScanTestResult::Fail;

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    if (normal)
        GetZVector (*normal, thisElm);

    if (point)
        {
        UInt32     numVertex;
        ExtractTransformedPointsExt (point, &numVertex, thisElm, 0, 1, NULL);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR trans)
    {
    if (0 == (trans.GetOptions () & TRANSFORM_OPTIONS_MlineMirrorOffsets))
        {
        double      determ;
        RotMatrix   rMatrix;

        rMatrix.InitFrom(*( trans.GetTransform ()));
        determ = rMatrix.Determinant ();

        if (determ < 0.0)
            return false;
        }

    if (0 == (trans.GetOptions () & TRANSFORM_OPTIONS_MlineScaleOffsets))
        {
        DPoint3d    scaleVector;
        RotMatrix   rMatrix;

        rMatrix.InitFrom(*( trans.GetTransform ()));
        LegacyMath::RMatrix::GetColumnScaleVector (NULL, &scaleVector, &rMatrix);

        if (fabs (scaleVector.x - 1.0) > mgds_fc_epsilon ||
            fabs (scaleVector.y - 1.0) > mgds_fc_epsilon ||
            fabs (scaleVector.z - 1.0) > mgds_fc_epsilon)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MultilineHandler::_OnTransform
(
EditElementHandleR elemHandle,
TransformInfoCR trans
)
    {
    StatusInt   status;

    if ((status = T_Super::_OnTransform (elemHandle, trans)) != SUCCESS)
        return status;

    return TransformMline (elemHandle, trans.GetTransform(),
                                 TO_BOOL(trans.GetOptions () & TRANSFORM_OPTIONS_MlineMirrorOffsets),
                                 TO_BOOL(trans.GetOptions () & TRANSFORM_OPTIONS_MlineScaleOffsets));
    }

/*=================================================================================**//**
For Fence Stretch
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MLineBreakInfo
    {
    int                 segNo;
    int                 breakNo;
    double              length;
    double              offset;
    int                 lineMask;
    int                 breakFlags;
    DPoint3d            startPt;
    DPoint3d            endPt;

    MLineBreakInfo ()
        {
        memset (&startPt, 0, sizeof(startPt));
        memset (&endPt, 0, sizeof(endPt));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JVB             08/90
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus      GetBreak (UInt32 segNum, UInt32 breakNum, ElementHandleCR eh, MultilineHandler* handler)
        {
        if (segNum >= (int) handler->GetPointCount (eh) || handler->GetNumBreaksOnSegment (eh, segNum) == 0 ||
              breakNum >= handler->GetNumBreaksOnSegment (eh, segNum))
            return ERROR;

        MultilineBreakPtr mlBreak = handler->GetBreak (eh, segNum, breakNum);

        segNo = segNum;
        breakNo = breakNum;

        offset = mlBreak->GetOffset();
        length = mlBreak->GetLength();
        lineMask = mlBreak->GetProfileMask();
        breakFlags = mlBreak->GetLengthType();

        return SUCCESS;
        }
    };

/*=================================================================================**//**
For Fence Stretch
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MLineStretchInfo
    {
    UInt32              numVerts;
    DPoint3d            pts[MAX_VERTICES];
    DVec3d              segVecs[MAX_VERTICES];

    UInt32              numBreaks;
    UInt32              numAllocatedBreaks;
    MLineBreakInfo*     breaksP;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Chuck.Kirschman 10/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    MLineStretchInfo ()
        {
        numVerts = numBreaks = numAllocatedBreaks = 0;
        memset (pts, 0, sizeof(pts));
        memset (segVecs, 0, sizeof(segVecs));
        breaksP = NULL;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Rob.Brown       02/95
    +---------------+---------------+---------------+---------------+---------------+------*/
    void     AllocateBreaks (UInt32 number)
        {
        numBreaks = number;

        if (number > numAllocatedBreaks)
            {
            if (numAllocatedBreaks)
                delete[] breaksP;

            if (number)
                breaksP = new MLineBreakInfo[number];

            numAllocatedBreaks = number;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Rob.Brown       02/95
    +---------------+---------------+---------------+---------------+---------------+------*/
    void     GetBreakInfo (ElementHandleCR eh, MultilineHandler* handler)
        {
        UInt32          numSegs = numVerts-1;

        AllocateBreaks (handler->GetBreakCount(eh));

        MLineBreakInfo* currBreakP = breaksP;
        for (UInt32 segNo=0; segNo<numSegs; segNo++)
            {
            UInt32 numBreaks = handler->GetNumBreaksOnSegment (eh, segNo);

            if (segNo >= (int) handler->GetPointCount (eh) || numBreaks == 0)
                continue;

            for (UInt32 breakNo=0; breakNo<numBreaks; breakNo++)
                {
                currBreakP->GetBreak (segNo, breakNo, eh, handler);
                currBreakP++;
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Rob.Brown       02/95
    +---------------+---------------+---------------+---------------+---------------+------*/
    void     GetSegmentVectors ()
        {
        DPoint3dP curPoint = pts;
        DVec3dP vecs = segVecs;
        for (DPoint3dP endP = pts+numVerts-1; curPoint < endP; curPoint++, vecs++)
            vecs->NormalizedDifference (*(curPoint+1), *(curPoint));
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      deleteBreak
(
MlineElm*       mline,          // <=> Multiline element to be changed
int             segNo,          //  => Multiline segment containing break
int             breakNo         //  => Break number in segment
)
    {
    MlinePoint *mPoints = ((MlinePoint*)(mline->profile + mline->nLines));
    MlineBreak *mBreaks = ((MlineBreak*)(mPoints + mline->nPoints));
    char       *src, *dst, *end;
    int        i, inSeg;

    if (segNo > mline->nPoints-1 || mPoints[segNo].nBreaks == 0 ||
        breakNo > mPoints[segNo].nBreaks-1)
        return  ERROR;

    breakNo += mPoints[segNo].breakNo;

    src = (char*) (mBreaks+(breakNo+1));
    dst = (char*) (mBreaks+breakNo);
    end = (char*) mline + ((DgnElementP) mline)->Size ();
    memmove (dst, src, end - src);

    for (i=0; i<mline->nPoints; i++) // Decrement later break addresses
        {
        if (mPoints[i].nBreaks && (breakNo >= mPoints[i].breakNo &&
            breakNo < mPoints[i].breakNo + mPoints[i].nBreaks))
            inSeg = true;
        else
            inSeg = false;

        if (mPoints[i].nBreaks && mPoints[i].breakNo > breakNo)
            mPoints[i].breakNo--;

        if (inSeg)
            mPoints[i].nBreaks--;

        if (mPoints[i].nBreaks == 0)
            mPoints[i].breakNo = 0;
        }

    mline->DecrementSizeWords(sizeof(MlineBreak) / 2);
    mline->nBreaks--;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus      MultilineHandler::_DeleteBreak (EditElementHandleR element, UInt32 segNo, UInt32 breakNo)
    {
    if (!IsMultilineElement (element))
        return ERROR;

    if (segNo >= GetPointCount(element))
        return ERROR;

    if (GetNumBreaksOnSegment (element, segNo) == 0 || breakNo >= GetNumBreaksOnSegment (element, segNo) )
        return ERROR;

    // Have to get in and mess with the element.
    DgnV8ElementBlank   fullElm;
    element.GetElementCP ()->CopyTo (fullElm);
    MlineElm* mline = &fullElm.ToMlineElmR();

    MlinePoint *mPoints = ((MlinePoint*)(mline->profile + mline->nLines));
    MlineBreak *mBreaks = ((MlineBreak*)(mPoints + mline->nPoints));

    breakNo += mPoints[segNo].breakNo;

    char* src = (char*) (mBreaks+(breakNo+1));
    char* dst = (char*) (mBreaks+breakNo);
    char* end = (char*) mline + ((DgnElementP) mline)->Size ();
    memmove (dst, src, end - src);

    bool inSeg = false;
    for (UInt32 iPoint=0; iPoint<mline->nPoints; iPoint++) // Decrement later break addresses
        {
        if (mPoints[iPoint].nBreaks && (breakNo >= (UInt32)mPoints[iPoint].breakNo &&
            breakNo < (UInt32)(mPoints[iPoint].breakNo + mPoints[iPoint].nBreaks) ))
            inSeg = true;
        else
            inSeg = false;

        if (mPoints[iPoint].nBreaks && mPoints[iPoint].breakNo > breakNo)
            mPoints[iPoint].breakNo--;

        if (inSeg)
            mPoints[iPoint].nBreaks--;

        if (mPoints[iPoint].nBreaks == 0)
            mPoints[iPoint].breakNo = 0;
        }

    mline->DecrementSizeWords(sizeof (MlineBreak) / 2);
    mline->nBreaks--;

    element.ReplaceElement (&fullElm);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void   MultilineHandler::GetSegmentOffsets (double* orgOff, double* endOff, ElementHandleCR eh, UInt32 pointNo)
    {
    DSegment3d       workLine;
    MultilineHandler::ExtractPointArray (eh, &workLine.point[0], pointNo, 2);
    double length = (workLine.point[0]).Distance (*( &workLine.point[1]));
    DVec3d lineDir;
    lineDir.NormalizedDifference (*( &workLine.point[1]), *( &workLine.point[0]));

    /*-------------------------------------------------------------------
    Get the joint definition for each end of the segment
    -------------------------------------------------------------------*/
    JointDef        orgJoint, endJoint;
    MultilineHandler::GetAnyJointDef (eh, &orgJoint, &workLine, pointNo, true);
    MultilineHandler::GetAnyJointDef (eh, &endJoint, &workLine, pointNo+1, false);

    /*-------------------------------------------------------------------
    Calculate the offset along the work line, from each line segment origin
    to the work line origin. Do the same for end points.
    -------------------------------------------------------------------*/
    UInt32 nLines = GetProfileCount (eh);
    for (UInt32 iProfile=0; iProfile<nLines; iProfile++)
        {
        MultilineProfilePtr curProfile = GetProfile (eh, iProfile);
        double dist = curProfile->GetDistance();

        DVec3d          jointVec;
        jointVec.Scale (*( (DVec3d *) &orgJoint.dir),  dist * orgJoint.scale);
        orgOff[iProfile] = lineDir.DotProduct (jointVec);

        jointVec.Scale (*( (DVec3d *) &endJoint.dir),  dist * endJoint.scale);
        endOff[iProfile] = length + lineDir.DotProduct (jointVec);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     MultilineHandler::ClipBadBreaks (EditElementHandleR element, UInt32 pointNo)
    {
    if (pointNo >= GetPointCount(element))
        return;

    if (GetNumBreaksOnSegment(element, pointNo) == 0)
        return;

    double      orgOff[MULTILINE_MAX], endOff[MULTILINE_MAX];
    GetSegmentOffsets (orgOff, endOff, element, pointNo);

    UInt32 nLines = GetProfileCount (element);

    // Using signed int because it gets decremented on a delete.
    for (Int32 iBreak=0; iBreak<(Int32)GetNumBreaksOnSegment(element, pointNo); iBreak++)
        {
        MultilineBreakPtr curBreak = GetBreak (element, pointNo, iBreak);

        for (UInt32 iProfile=0; iProfile<nLines; iProfile++)
            {
            if (curBreak->ProfileIsMasked (iProfile))
                {
                if (!(curBreak->GetLengthType() & MLBREAK_FROM_JOINT))
                    {
                    double diff = orgOff[iProfile] - (double)curBreak->GetOffset();
                    if (diff > 0.0)
                        {
                        curBreak->AdjustOffset(diff);
                        curBreak->AdjustLength(-diff);
                        }

                    if (curBreak->GetLength() <= 0L && !(curBreak->GetLengthType() & MLBREAK_TO_JOINT))
                        {
                        _DeleteBreak (element, pointNo, iBreak);
                        iBreak--;
                        break;
                        }
                    }

                if (!(curBreak->GetLengthType() & MLBREAK_TO_JOINT))
                    {
                    double diff = curBreak->GetOffset() + curBreak->GetLength() - endOff[iProfile];
                    if (diff > 0.0)
                        curBreak->AdjustLength(-diff);

                    if (curBreak->GetLength() <= 0L && !(curBreak->GetLengthType() & MLBREAK_FROM_JOINT))
                        {
                        _DeleteBreak (element, pointNo, iBreak);
                        iBreak--;
                        break;
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      removeConflictingBreaks
(
MlineElm        *mline,      /* <=> Multiline element to work on       */
int             pointNo,     /*  => Vertex number before break         */
double          offset,      /*  => Offset from vertex to break start  */
double          length,      /*  => Length of break                    */
unsigned        lineMask,    /*  => Bit mask of lines to break (0-15)  */
unsigned        breakFlags   /*  => Break option or type               */
)
    {
    MlineBreak  *lastBreak;
    MlinePoint  *mPoint;
    int         breakNo;
    int         i, nBreaks;
    double      oldEnd, oldOrg, newEnd, newOrg;

    mPoint = ((MlinePoint*)(mline->profile + mline->nLines));

    if ((nBreaks = mPoint[pointNo].nBreaks) == 0)
        return SUCCESS;

    breakNo = mPoint[pointNo].breakNo;
    lastBreak = ((MlineBreak*)(mPoint + mline->nPoints)) + (breakNo + nBreaks -1);

    for (i=nBreaks-1; i>=0; i--, lastBreak--)
        {
        if (lastBreak->lineMask & lineMask)
            {
            newOrg = DataConvert::RoundDoubleToLong (breakFlags & MLBREAK_FROM_JOINT ? 0L:offset);
            newEnd = DataConvert::RoundDoubleToLong (breakFlags & MLBREAK_TO_JOINT ? MLBREAK_MAX:offset+length);
            oldOrg = lastBreak->flags & MLBREAK_FROM_JOINT ? 0L:lastBreak->offset;
            oldEnd = lastBreak->flags & MLBREAK_TO_JOINT ? MLBREAK_MAX:lastBreak->offset + lastBreak->length;

            if ((newOrg >= oldOrg && newOrg <= oldEnd) ||
                (newEnd >= oldOrg && newEnd <= oldEnd) ||
                (oldOrg >= newOrg && oldOrg <= newEnd) ||
                (oldEnd >= newOrg && oldEnd <= newEnd))
                deleteBreak (mline, pointNo, i);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineBreakPtr MultilineBreak::Create (double offset, MlineBreakLengthType lengthType, double length, UInt32 profileMask)
    {
    MultilineBreak* mlbreak = new MultilineBreak();

    mlbreak->m_offset        = offset;
    mlbreak->m_length        = length;
    mlbreak->m_lengthFlags   = lengthType;
    mlbreak->m_angle         = 0.0;
    mlbreak->m_lineMask      = profileMask;

    return mlbreak;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineBreakPtr MultilineBreak::Create (MlineBreak const & elmBreak)
    {
    MultilineBreak* mlbreak = new MultilineBreak();
    mlbreak->FromElementData (elmBreak);
    return mlbreak;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MlineBreakLengthType        MultilineBreak::GetLengthType () const
    {
    return m_lengthFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineBreak::SetLengthType (MlineBreakLengthType type)
    {
    m_lengthFlags = type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double            MultilineBreak::GetLength () const
    {
    return m_length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void              MultilineBreak::SetLength (double length)
    {
    m_length = length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void             MultilineBreak::AdjustLength (double delta)
    {
    m_length+=delta;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double            MultilineBreak::GetOffset () const
    {
    return m_offset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void              MultilineBreak::SetOffset (double offset)
    {
    BeAssert (offset > 0.0);
    if (offset > 0.0)
        m_offset = offset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void             MultilineBreak::AdjustOffset (double delta)
    {
    if (m_offset+delta > 0)
        m_offset+=delta;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32             MultilineBreak::GetProfileMask () const
    {
    return m_lineMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void               MultilineBreak::SetProfileMask (UInt32 lineMask)
    {
    m_lineMask = lineMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool              MultilineBreak::ProfileIsMasked (UInt32 profileNum)
    {
    return 0 != (m_lineMask & (1 << profileNum));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double            MultilineBreak::GetAngle () const
    {
    return m_angle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void              MultilineBreak::SetAngle (double angle)
    {
    m_angle = angle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineBreak::ToElementData (MlineBreak& mlbreak) const
    {
    mlbreak.flags    = GetLengthType();
    mlbreak.lineMask = GetProfileMask();
    mlbreak.offset   = GetOffset();
    mlbreak.length   = GetLength();
    mlbreak.angle    = GetAngle();       // For future expansion - DWG
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineBreak::FromElementData (MlineBreak const & mlbreak)
    {
    m_lineMask      = mlbreak.lineMask;
    m_offset        = mlbreak.offset;
    m_length        = mlbreak.length;
    m_angle         = mlbreak.angle;       // For future expansion - DWG
    m_lengthFlags   = (MlineBreakLengthType)mlbreak.flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
MlineBreak const*   MultilineHandler::GetFirstBreakCP (ElementHandleCR eh) const
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return NULL;

    return ((MlineBreak*) (MultilineHandler::GetFirstPoint (eh) + el.ToMlineElm().nPoints));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
MlineBreak*   MultilineHandler::GetFirstBreak (EditElementHandleR eh)
    {
    DgnElementR el = *eh.GetElementP ();

    return ((MlineBreak*) (MultilineHandler::GetFirstPoint (eh) + el.ToMlineElm().nPoints));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              MultilineHandler::_GetBreakCount (ElementHandleCR source) const
    {
    DgnElementCR el = *source.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return 0;

    return el.ToMlineElm().nBreaks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Chuck.Kirschman     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineBreakPtr       MultilineHandler::_GetBreak (ElementHandleCR source, UInt32 segmentNumber, UInt32 segBreakNumber) const
    {
    if (MULTILINE_ELM != source.GetLegacyType() || segmentNumber > GetPointCount (source))
        return NULL;

    MultilinePointPtr curPoint = GetPoint (source, segmentNumber);

    if (curPoint->GetNumBreaks() == 0 || segBreakNumber >= curPoint->GetNumBreaks())
        return NULL;

    UInt32 index = curPoint->GetBreakNumber() + segBreakNumber;
    return MultilineBreak::Create (*(GetFirstBreakCP (source)+index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::_InsertBreak (EditElementHandleR element, MultilineBreakCR mlbreak, UInt32 pointNo)
    {
    if (!IsMultilineElement (element))
        return ERROR;

    if (pointNo >= GetPointCount(element))
        return ERROR;

    size_t mlineSize = element.GetElementCP()->Size ();

    if (mlineSize + sizeof(MlineBreak) > MAX_V8_ELEMENT_SIZE)
        return ERROR;

    // Create a real element and dig into the bits
    DgnV8ElementBlank   fullElm;
    element.GetElementCP ()->CopyTo (fullElm);
    MlineElm* mline = &fullElm.ToMlineElmR();

    removeConflictingBreaks (mline, pointNo, mlbreak.GetOffset(), mlbreak.GetLength(), mlbreak.GetProfileMask(), mlbreak.GetLengthType());

    MlinePoint* mPoint   = ((MlinePoint*)(mline->profile + mline->nLines));
    UInt16      breakNo  = mPoint[pointNo].nBreaks ? mPoint[pointNo].breakNo : mline->nBreaks;
    MlineBreak* newBreak = ((MlineBreak*)(mPoint + mline->nPoints)) + breakNo;

    for (UInt32 iBreak=0; iBreak<mPoint[pointNo].nBreaks; iBreak++)
        {
        if (mlbreak.GetOffset() > newBreak->offset)
            {
            newBreak++;
            breakNo++;
            }
        else
            {
            break;
            }
        }

    for (UInt32 iPoint=0; iPoint<mline->nPoints; iPoint++)   // Increment later break addresses
        if (iPoint != pointNo && mPoint[iPoint].nBreaks && mPoint[iPoint].breakNo >= breakNo)
            mPoint[iPoint].breakNo++;

    ptrdiff_t moveSize = ((char*)mline + mlineSize) - (char*)newBreak;
    memmove (newBreak+1, newBreak, moveSize);

    mlbreak.ToElementData (*newBreak);

    mline->IncrementSizeWords((sizeof(MlineBreak) / 2));
    mline->nBreaks++;

    if (0 == mPoint[pointNo].nBreaks)
        mPoint[pointNo].breakNo = breakNo;

    mPoint[pointNo].nBreaks++;

    element.ReplaceElement (&fullElm);
    ClipBadBreaks (element, pointNo);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MultilineHandler::_OnFenceStretch
(
EditElementHandleR  eeh,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    MLineStretchInfo    mlineData;

    mlineData.numVerts = GetPointCount (eeh);
    ExtractPointArray (eeh, mlineData.pts, 0, mlineData.numVerts);
    mlineData.GetBreakInfo (eeh, this);
    mlineData.GetSegmentVectors ();

    UInt32          i;
    double          lineOffset[MULTILINE_MAX];
    MLineBreakInfo  *currBreakP;

    for (i=0; i < GetProfileCount (eeh); i++)
        lineOffset[i] = _GetMlineProfileDefCP (eeh, i)->dist;

    for (i=0, currBreakP=mlineData.breaksP; i<mlineData.numBreaks; i++, currBreakP++)
        {
        int         lineMask = currBreakP->lineMask;
        bool        useStartPoint, useEndPoint;
        DPoint3d    *breakStartPtP, *breakEndPtP;

        breakStartPtP = &currBreakP->startPt;
        breakEndPtP = &currBreakP->endPt;

        if (useStartPoint = ((currBreakP->breakFlags & MLBREAK_FROM_JOINT) == 0))
            breakStartPtP->SumOf(*( &mlineData.pts[currBreakP->segNo]), *( &mlineData.segVecs[currBreakP->segNo]),  currBreakP->offset);
        else
            *breakStartPtP = mlineData.pts[currBreakP->segNo];

        for (int j=0; j < (int) GetProfileCount (eeh); j++)
            {
            if (lineMask == 1<<j)
                {
                DVec3d    perpVec, zVector;

                GetZVector (zVector, eeh);
                perpVec.CrossProduct (zVector, *( &mlineData.segVecs[currBreakP->segNo]));
                perpVec.Normalize ();
                breakStartPtP->SumOf(*breakStartPtP, perpVec,  lineOffset[j]);
                }
            }

        if (useEndPoint = ((currBreakP->breakFlags & MLBREAK_TO_JOINT) == 0))
            breakEndPtP->SumOf(*breakStartPtP, *( &mlineData.segVecs[currBreakP->segNo]),  currBreakP->length);

        if (useEndPoint && fp->PointInside (*breakEndPtP))
            ( transform.GetTransform ())->Multiply(*breakEndPtP);

        if (useStartPoint && fp->PointInside (*breakStartPtP))
            ( transform.GetTransform ())->Multiply(*breakStartPtP);
        }

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    // more efficient in reverse order...
    Int32 iDel = mlineData.numBreaks-1;
    for (currBreakP=mlineData.breaksP+iDel; iDel >= 0; iDel--, currBreakP--)
        deleteBreak (&elm.ToMlineElmR(), currBreakP->segNo, currBreakP->breakNo);

    eeh.ReplaceElement (&elm);

    for (i=0; i<mlineData.numVerts; i++)
        {
        if (fp->PointInside (mlineData.pts[i]))
            {
            ( transform.GetTransform ())->Multiply(*(&mlineData.pts[i]));

            ReplacePoint (eeh, mlineData.pts[i], i, MlineModifyPoint::None);
            }
        }

    double      length = 0.0; // Should this really be outside loop?!? -BB 07/04

    mlineData.GetSegmentVectors ();

    for (i=0, currBreakP=mlineData.breaksP; i<mlineData.numBreaks; i++, currBreakP++)
        {
        double      offset;
        DVec3d      vec;

        if (currBreakP->breakFlags & MLBREAK_FROM_JOINT)
            {
            offset = 0.0;
            }
        else
            {
            vec.DifferenceOf (currBreakP->startPt, *( &mlineData.pts[currBreakP->segNo]));
            offset = vec.DotProduct (*( &mlineData.segVecs[currBreakP->segNo]));
            }

        if (!(currBreakP->breakFlags & MLBREAK_TO_JOINT))
            {
            vec.DifferenceOf (currBreakP->endPt, *( &mlineData.pts[currBreakP->segNo]));
            length = vec.DotProduct (*( &mlineData.segVecs[currBreakP->segNo])) - offset;
            }
        MultilineBreakPtr newBreak = MultilineBreak::Create (offset, (MlineBreakLengthType)currBreakP->breakFlags, length, currBreakP->lineMask);
        InsertBreak (eeh, *newBreak, currBreakP->segNo);
        }

    if (mlineData.breaksP)
        free (mlineData.breaksP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MultilineHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    ElementAgenda agenda;

    if (SUCCESS != Drop (eh, agenda, DropGeometry (DropGeometry::OPTION_Mlines)))
        return ERROR;

    for (EditElementHandleP curr = agenda.GetFirstP (), end = curr + agenda.GetCount (); curr < end ; curr++)
        curr->GetHandler().FenceClip (inside, outside, *curr, fp, options);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MultilineHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Mlines & geometry.GetOptions ()))
        return ERROR;

    return DropToElementDrawGeom::DoDrop (eh, dropGeom, geometry, DropGraphics ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_AddPattern
(
EditElementHandleR  eeh,
PatternParamsR      params,
DwgHatchDefLineP    hatchDefLinesP,
int                 index
)
    {
    // NOTE: Mlines support multiple patterns...if index is 0 append, don't replace!
    return IAreaFillPropertiesEdit::_AddPattern (eeh, params, hatchDefLinesP, 0 == index ? -1 : index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    MlinePoint* mP = (MlinePoint*) GetFirstPoint (eeh);

    for (int iPoint = eeh.GetElementCP ()->ToMlineElm().nPoints; iPoint > 0; mP++, iPoint--)
        mP->point.z = elevation;

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    DVec3d      zVec;

    // Keep the Z bvector...
    GetZVector (zVec, eeh);

    double      dot = bsiDVec3d_dotProduct (&zVec, &flattenDir);

    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    // set z bvector to point in z direction; handle mirrored
    zVec.init (0.0, 0.0, dot < 0.0 ? -1.0 : 1.0);
    _SetZVector (eeh, zVec);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*=================================================================================**//**
* Mline Display Code
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineHandler::TestLevelAndClass
(
ElementHandleCR         thisElm,
ViewContextR            context,
MlineSymbology const*   symb
) const
    {
    ScanCriteriaCP  scP = context.GetScanCriteria ();

    if (!scP)
        return true;

    DgnElementCR elm = *thisElm.GetElementCP ();
    LevelId     elemLevel = ((symb && symb->level) ? LevelId(symb->level) : elm.GetLevel());
    UInt32      elemClass = ((symb && symb->useClass) ? (symb->conClass ? 2 : 0) : elm.GetElementClassValue());

    ElemHeaderOverrides const* ovr = context.GetHeaderOvr();

    if (ovr)
        elemLevel = ovr->AdjustLevel (elemLevel);

    if (elemLevel.IsValid())
        {
        BitMaskCP   bitMask = scP->GetLevelBitMask();

        if (NULL != bitMask && ! bitMask->Test (elemLevel.GetValue()-1))
            return false;
        }

    UInt32      classMask = scP->GetClassMask();

    if (!(classMask & (1 << elemClass)))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineHandler::SetCurrentDisplayParams
(
ElementHandleCR         thisElm,
ViewContextR            context,
MlineSymbology const*   symb,
int                     lsIndex,
bool                    isCap
) const
    {
    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams ();
    DgnElementCR         elm = *thisElm.GetElementCP ();

    elParams->SetLevelSubLevelId (LevelSubLevelId((symb && symb->level)? LevelId(symb->level): elm.GetLevel()));
    elParams->SetElementClass ((DgnElementClass) ((symb && symb->useClass) ? (symb->conClass ? 2 : 0) : elm.GetElementClassValue()));
    elParams->SetLineColor (((symb && symb->useColor) ? symb->color : elm.GetSymbology().color));
    elParams->SetWeight (((symb && symb->useWeight) ? symb->weight : elm.GetSymbology().weight));
    elParams->SetLineStyle (((symb && symb->useStyle) ? symb->style : elm.GetSymbology().style));

    // Apply custom linestyle params...
    if (elm.GetSizeWords() > elm.GetAttributeOffset() && !IS_LINECODE (elParams->GetLineStyle ()))
        {
        bool            useBase = (!symb || !symb->useStyle);
        StatusInt       extractStatus = ERROR;
        LineStyleParams styleParams;

        if (!useBase)
            extractStatus = LineStyleLinkageUtil::ExtractStyleLinkageFromMultiLine (styleParams, &elm, isCap, lsIndex);
        else
            extractStatus = LineStyleLinkageUtil::ExtractHeaderStyleLinkageFromMultiLine (styleParams, &elm);

        if (SUCCESS == extractStatus)
            {
            if (!useBase && elm.Is3d())
                {
                // For 3d orient style to mline if style doesn't specify an orientation...
                if (!(styleParams.modifiers & (STYLEMOD_RMATRIX | STYLEMOD_NORMAL)))
                    {
                    styleParams.normal = elm.ToMlineElm().zVector;
                    styleParams.rMatrix.initFrom1Vector ((DVec3dP) &styleParams.normal, 2, false);
                    styleParams.modifiers |= (STYLEMOD_RMATRIX | STYLEMOD_NORMAL);
                    }
                }

            elParams->SetLineStyle (elParams->GetLineStyle (), &styleParams);
            }
        }

    context.CookDisplayParams ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
int     MultilineHandler::OffsetLineBuffer
(
DPoint3dP       pPoints,
JointDef*       pJoint,
double          dist,
int             nPoints
)
    {
    DPoint3d    *pSrc, *pDst, *pEnd;

    pSrc = pDst = pPoints;
    pEnd = pSrc + nPoints;

    pPoints->SumOf(*pPoints, *( (DVec3d *)&pJoint->dir),  dist * pJoint->scale);

    pJoint++;
    pSrc = pDst = pPoints+1;
    pEnd = pPoints + nPoints;

    while (pSrc < pEnd)
        {
        pDst->SumOf(*pSrc, *( (DVec3d *)&pJoint->dir),  dist * pJoint->scale);

        if (!pDst->isEqual (pDst-1)) // was (!isSameUORpoint (pDst-1, pDst))
            pDst++;

        pSrc++;
        pJoint++;
        }

    return static_cast<int>(pDst - pPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::OutputLineString
(
ElementHandleCR         thisElm,
ViewContextR            context,
DPoint3dP               inPoints,   /* => Line string vertex points            */
int                     nPoints,    /* => Number of points in line string     */
MlineSymbology const*   symb,       /* => Line string symbology               */
int                     lsIndex,
bool                    isCap
)
    {
    if (nPoints < 2)
        return SUCCESS;

    // Filter display by level/class...
    if (!TestLevelAndClass (thisElm, context, symb))
        return SUCCESS;

    DgnElementCR elm = *thisElm.GetElementCP ();

    // Setup symbology...
    SetCurrentDisplayParams (thisElm, context, symb, lsIndex, isCap);

    // Draw profile line...
    if (DisplayHandler::Is3dElem (&elm))
        {
        context.DrawStyledLineString3d (nPoints, inPoints, NULL, false);

        return SUCCESS;
        }

    std::valarray<DPoint2d> localPoints2dBuf (nPoints);

    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        localPoints2dBuf[iPoint].x = inPoints[iPoint].x;
        localPoints2dBuf[iPoint].y = inPoints[iPoint].y;
        }

    context.DrawStyledLineString2d (nPoints, &localPoints2dBuf[0], context.GetDisplayPriority(), NULL, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::OutputLine
(
ElementHandleCR         thisElm,
ViewContextR            context,
DSegment3dCP            line,        /* => Two end points of line              */
MlineSymbology const*   symb,        /* => Line symbology                      */
int                     lsIndex
)
    {
    DPoint3d    points[2];

    points[0] = line->point[0];
    points[1] = line->point[1];

    return OutputLineString (thisElm, context, points, 2, symb, lsIndex, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::DrawLineString
(
ElementHandleCR         thisElm,
ViewContextR            context,
DPoint3dP               inPoints,
JointDef*               joints,
int                     nPoints,
MlineSymbology const*   symb,        /* => Line string symbology               */
int                     lineNo,
struct MlineTopology&   topology
)
    {
    DgnElementCR elm = *thisElm.GetElementCP ();
    int         nBuf = 0, lsBase = 0;
    double      lineOffset = elm.ToMlineElm().profile[lineNo].dist;
    DPoint3d    lineBuf[MLINE_MAXPOINTS], breakStart, breakEnd;
    JointDef    jointBuf[MLINE_MAXPOINTS];

    MlineBreak const* breaks = MultilineHandler::GetFirstBreakCP (thisElm);
    MlinePoint const* points = MultilineHandler::GetFirstPoint (thisElm);

    for (int i=0; i<nPoints; i++)
        {
        lineBuf[nBuf]  = inPoints[i];
        jointBuf[nBuf] = joints[i];
        nBuf++;

        if (points[i].nBreaks)
            {
            DVec3d      segDir, perpVec;

            segDir.NormalizedDifference (inPoints[(i+1)], inPoints[i]);
            CalculatePerpVec (&perpVec, thisElm, (inPoints+i));

            JointDef    tmpJoint;

            tmpJoint.dir = perpVec;
            tmpJoint.scale = 1.0;

            for (int j=0; j<points[i].nBreaks; j++)
                {
                if (!(breaks[points[i].breakNo+j].lineMask & (1 << lineNo)))
                    continue;

                /*-----------------------------------------------------------
                If the break flag MLBREAK_FROM_JOINT is set, the break starts
                at the previous joint bvector. Otherwise it is measured
                along the work line and then projected out the perpendicular.
                -----------------------------------------------------------*/
                if (breaks[points[i].breakNo+j].flags & MLBREAK_FROM_JOINT)
                    {
                    breakStart = inPoints[i];
                    goto endOfBreak;
                    }
                else
                    {
                    breakStart.SumOf(inPoints[i], segDir,  (double) breaks[points[i].breakNo+j].offset);

                    lineBuf[nBuf]  = breakStart;
                    jointBuf[nBuf] = tmpJoint;
                    nBuf++;
                    }

                if (nBuf > 1)
                    {
                    nBuf = OffsetLineBuffer (lineBuf, jointBuf, lineOffset, nBuf);

                    topology.SetLsBase (lsBase);

                    if (SUCCESS != OutputLineString (thisElm, context, lineBuf, nBuf, symb, lineNo, false))
                        return ERROR;
                    }

endOfBreak:
                /*---------------------------------------------------------
                If the break flag MLBREAK_TO_JOINT is set,the break continues
                to the following joint. Otherwise it is measured along the
                work line and projected out the perpendicular.
                ---------------------------------------------------------*/
                lsBase = i;

                if (breaks[points[i].breakNo+j].flags & MLBREAK_TO_JOINT)
                    {
                    nBuf = 0;
                    }
                else
                    {
                    breakEnd.SumOf(breakStart, segDir,  (double) breaks[points[i].breakNo+j].length);
                    lineBuf[0]  = breakEnd;
                    jointBuf[0] = tmpJoint;
                    nBuf = 1;
                    }
                }
            }
        }

    if (nBuf)
        {
        nBuf = OffsetLineBuffer (lineBuf, jointBuf, lineOffset, nBuf);

        topology.SetLsBase (lsBase);

        if (SUCCESS != OutputLineString (thisElm, context, lineBuf, nBuf, symb, lineNo, false))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::DrawJointLine
(
ElementHandleCR         thisElm,
ViewContextR            context,
DSegment3dP             pJointLine,
MlineSymbology const*   pSymbology,
int                     lsIndex
)
    {
    DgnElementCR elm = *thisElm.GetElementCP ();

    if (!pSymbology->capColorFromSeg)
        return OutputLine (thisElm, context, pJointLine, pSymbology, lsIndex);

    int             minProfile=0, maxProfile=0;
    DSegment3d splitLine;
    MlineSymbology  symb = *pSymbology;

    MultilineHandler::GetLimitProfiles (minProfile, maxProfile, thisElm);

    symb.level      = elm.ToMlineElm().profile[minProfile].symb.level;
    symb.color      = elm.ToMlineElm().profile[minProfile].symb.color;
    symb.useColor   = elm.ToMlineElm().profile[minProfile].symb.useColor;

    if (0 == memcmp (&elm.ToMlineElm().profile[minProfile].symb, &elm.ToMlineElm().profile[maxProfile].symb, sizeof (elm.ToMlineElm().profile[minProfile].symb)))
        return OutputLine (thisElm, context, pJointLine, &symb, lsIndex);

    splitLine.point[0] = pJointLine->point[0];

    bsiDPoint3d_interpolate (&splitLine.point[1], &pJointLine->point[0], 0.5, &pJointLine->point[1]);

    if (SUCCESS != OutputLine (thisElm, context, &splitLine, &symb, lsIndex))
        return ERROR;

    symb.level      = elm.ToMlineElm().profile[maxProfile].symb.level;
    symb.color      = elm.ToMlineElm().profile[maxProfile].symb.color;
    symb.useColor   = elm.ToMlineElm().profile[maxProfile].symb.useColor;

    splitLine.point[0] = splitLine.point[1];
    splitLine.point[1] = pJointLine->point[1];

    return OutputLine (thisElm, context, &splitLine, &symb, lsIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::OutputArc
(
ElementHandleCR         thisElm,
ViewContextR            context,
DPoint3dP               points,      /* => Three points defining an arc        */
MlineSymbology const*   symb,        /* => Arc symbology                       */
MlineSymbology const*   symb2,       /* => 2nd Arc symbology or NULL           */
int                     lsIndex
) const
    {
    DEllipse3d  ellipse;

    if (!ellipse.initFromPointsOnArc (&points[0], &points[1], &points[2]))
        return SUCCESS;

    DgnElementCR elm = *thisElm.GetElementCP ();
    bool        is3d = DisplayHandler::Is3dElem (&elm);
    double      quaternion[4], r0, r1, start, sweep;
    DPoint3d    center;

    // Draw profile line...
    if (is3d)
        ellipse.getDGNFields3d (&center, quaternion, NULL, NULL, &r0, &r1, &start, &sweep);
    else
        ellipse.getDGNFields2d ((DPoint2dP) &center, &quaternion[0], NULL, &r0, &r1, &start, &sweep);

    // Support arcs w/color of the associated line...if 2nd symb supplied cut the arc in half...
    if (symb2)
        sweep /= 2.0;

    // Filter display by level/class...
    if (TestLevelAndClass (thisElm, context, symb))
        {
        // Setup symbology...
        SetCurrentDisplayParams (thisElm, context, symb, lsIndex, true);

        if (is3d)
            context.DrawStyledArc3d (ellipse, false, NULL);
        else
            context.DrawStyledArc2d (ellipse, false, context.GetDisplayPriority(), NULL);
        }

    // Filter display by level/class...
    if (!symb2 || !TestLevelAndClass (thisElm, context, symb2))
        return SUCCESS;

    // Set to second half of arc cap for last call
    start += sweep;

    // Setup symbology...
    SetCurrentDisplayParams (thisElm, context, symb2, lsIndex, true);

    if (is3d)
        context.DrawStyledArc3d (ellipse, false, NULL);
    else
        context.DrawStyledArc2d (ellipse, false, context.GetDisplayPriority(), NULL);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineHandler::DrawArcEndCap
(
ElementHandleCR thisElm,
ViewContextR    context,
DPoint3dP       min,
double          minDist,
DPoint3dP       max,
double          maxDist,
DPoint3dP       perpVec,
int             end
) const
    {
    DPoint3d    points[3];

    points[0] = *min;
    points[2] = *max;

    bsiDPoint3d_interpolate (points+1, points, 0.5, points+2);

    double      length = points->Distance (points[2]);

    // don't bother trying to draw an arc cap of less than 2 UOR's <-- Questionable?!?
    if (length <- 2.0)
        return SUCCESS;

    DgnElementCR elm = *thisElm.GetElementCP ();

    length = end ? -length : length;
    points[1].SumOf(points[1], *( (DVec3d *)perpVec),  length / 2.0);

    MlineSymbology const*   capSym = (end ? &elm.ToMlineElm().endCap : &elm.ToMlineElm().orgCap);
    int                     lsIndex = (end ? 1 : 0);

    // Just draw in normal color
    if (!capSym->capColorFromSeg)
        return OutputArc (thisElm, context, points, capSym, NULL, lsIndex);

    // Set up to get colors from end lines
    int             minProfile=0, maxProfile=0;
    MlineSymbology  symb = *capSym;
    MlineSymbology  symb2 = *capSym;

    // Find matching profile line. Could be done more efficiently, but this is a small number of items.
    for (int i=0; i < elm.ToMlineElm().nLines; i++)
        {
        if (maxDist == elm.ToMlineElm().profile[i].dist)
            maxProfile = i;

        if (minDist == elm.ToMlineElm().profile[i].dist)
            minProfile = i;
        }

    symb.level      = elm.ToMlineElm().profile[minProfile].symb.level;
    symb.color      = elm.ToMlineElm().profile[minProfile].symb.color;
    symb.useColor   = elm.ToMlineElm().profile[minProfile].symb.useColor;

    symb2.level     = elm.ToMlineElm().profile[maxProfile].symb.level;
    symb2.color     = elm.ToMlineElm().profile[maxProfile].symb.color;
    symb2.useColor  = elm.ToMlineElm().profile[maxProfile].symb.useColor;

    return OutputArc (thisElm, context, points, &symb, &symb2, lsIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineHandler::DrawEndCap
(
ElementHandleCR thisElm,
ViewContextR    context,
DPoint3dP       origin,
JointDef*       joint,
int             end
)
    {
    DgnElementCR elm = *thisElm.GetElementCP ();

    if (elm.ToMlineElm().nLines < 2)
        return SUCCESS;

    MlineSymbology const*   capSym = (end ? &elm.ToMlineElm().endCap : &elm.ToMlineElm().orgCap);
    int                     lsIndex = (end ? 1 : 0);

    if (!(capSym->capOutArc || capSym->capInArc || capSym->capLine))
        return SUCCESS;

    int         min, max;
    double      dists[MULTILINE_MAX];

    MultilineHandler::GetLimitProfiles (min, max, thisElm);

    dists[0]  = elm.ToMlineElm().profile[min].dist;
    dists[15] = elm.ToMlineElm().profile[max].dist;

    DSegment3d line;

    (line.point[0]).SumOf(*origin, *( (DVec3dP) &joint->dir),  joint->scale * dists[0]);
    (line.point[1]).SumOf(*origin, *( (DVec3dP) &joint->dir),  joint->scale * dists[15]);

    if (capSym->capLine)
        {
        if (SUCCESS != DrawJointLine (thisElm, context, &line, capSym, lsIndex))
            return ERROR;
        }

    if (!(capSym->capOutArc || capSym->capInArc))
        return SUCCESS;

    DPoint3d    perpVec;

    CalculatePerpVec (&perpVec, thisElm, line.point);

    if (capSym->capOutArc)
        DrawArcEndCap (thisElm, context, &line.point[0], dists[0], &line.point[1], dists[15], &perpVec, end);

    if (!capSym->capInArc)
        return SUCCESS;

    int         i;

    for (i=0; i < elm.ToMlineElm().nLines; i++)
        dists[i] = elm.ToMlineElm().profile[i].dist;

    std::sort (dists, dists+elm.ToMlineElm().nLines);

    DPoint3d    points[3];

    for (i=1; i < (elm.ToMlineElm().nLines-1)/2.0; i++)
        {
        points->SumOf(*origin, *( (DVec3dP) &joint->dir),  joint->scale * dists[i]);
        points[2].SumOf(*origin, *( (DVec3dP) &joint->dir),  joint->scale * dists[elm.ToMlineElm().nLines-i-1]);

        DrawArcEndCap (thisElm, context, points, dists[i], points+2, dists[elm.ToMlineElm().nLines-i-1], &perpVec, end);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::StrokePolygons
(
ElementHandleCR thisElm,
ViewContextR    context,
int             minLine,        /* =>  Minimum profile line (-1 for limit) */
int             maxLine         /* =>  Maximum profile line (-1 for limit) */
)
    {
    DgnElementCR elm = *thisElm.GetElementCP ();
    int         i, nPoints, nLines;
    DPoint3d    workLine[MLINE_MAXPOINTS];
    JointDef    joints[MLINE_MAXPOINTS];

    nLines  = elm.ToMlineElm().nLines;
    nPoints = elm.ToMlineElm().nPoints;

    MultilineHandler::ExtractPointArray (thisElm, workLine, 0, nPoints);

    if (elm.ToMlineElm().flags.closed)
        workLine[0] = workLine[nPoints-1];

    MultilineHandler::GetCapJointDef (*joints, thisElm, workLine, 0);

    for (i=1; i<nPoints-1; i++)
        MultilineHandler::GetJointDef (*(joints+i), thisElm, workLine, i-1);

    MultilineHandler::GetCapJointDef (*(joints+(nPoints-1)), thisElm, workLine+(nPoints-2), 1);

    int         min, max;
    double      minDist, maxDist;

    MultilineHandler::GetLimitProfiles (min, max, thisElm);

    minDist = elm.ToMlineElm().profile[min].dist;
    maxDist = elm.ToMlineElm().profile[max].dist;

    if (minLine >= 0 && minLine < nLines && minLine != maxLine)
        minDist = elm.ToMlineElm().profile[minLine].dist;

    if (maxLine >= 0 && maxLine < nLines && maxLine != minLine)
        maxDist = elm.ToMlineElm().profile[maxLine].dist;

    DPoint3d    lineBuf1[MLINE_MAXPOINTS], lineBuf2[MLINE_MAXPOINTS];

    for (i=0; i<nPoints; i++)
        {
        bsiDPoint3d_addScaledDVec3d (lineBuf1+i,  workLine+i, (DVec3dP) &joints[i].dir, (double)minDist * joints[i].scale);
        bsiDPoint3d_addScaledDVec3d (lineBuf2+i,  workLine+i, (DVec3dP) &joints[i].dir, (double)maxDist * joints[i].scale);
        }

    IPickGeom*  pickGeom = context.GetIPickGeom();

    // NOTE: Want edge hit from fill poly to be given lower priority than mline profile geom!
    if (NULL != pickGeom)
        pickGeom->SetHitPriorityOverride (HitPriority::Interior);

    // Make sure fill displays behind mline profile lines and pattern in 3d...
    bool        pushOffset = (DisplayHandler::Is3dElem (&elm) && context.GetIViewDraw().IsOutputQuickVision ());

    if (pushOffset)
        {
        double      offsetDist = context.GetPixelSizeAtPoint (&workLine[0]);
        DVec3d      offsetDir = *((DVec3dP) &elm.ToMlineElm().zVector), viewZDir, testDir;
        DPoint3d    offsetPt, viewPt[2];
        Transform   transform;

        offsetPt.zero ();
        offsetDir.normalize ();
        testDir = offsetDir;

        viewPt[0].zero ();
        viewPt[1].init (0.0, 0.0, 1.0);

        context.NpcToFrustum (viewPt, viewPt, 2);
        viewZDir.differenceOf (&viewPt[0], &viewPt[1]);

        if (SUCCESS == context.GetCurrLocalToFrustumTrans (transform))
            transform.multiplyMatrixOnly (&testDir, &testDir);

        if (testDir.dotProduct (&viewZDir) < 0.0)
            offsetDist *= -1.0;

        offsetPt.sumOf (&offsetPt, &offsetDir, offsetDist);
        transform.initFrom (&offsetPt);

        context.PushTransform (transform);
        }

    for (i=0; i<nPoints-1; i++)
        {
        DPoint3d    shapeBuf[5];

        shapeBuf[0] = lineBuf1[i];
        shapeBuf[1] = lineBuf1[i+1];
        shapeBuf[2] = lineBuf2[i+1];
        shapeBuf[3] = lineBuf2[i];
        shapeBuf[4] = lineBuf1[i];

        // NOTE: Fill color already be setup in current display params...
        if (DisplayHandler::Is3dElem (&elm))
            {
            context.GetIDrawGeom().DrawShape3d (5, shapeBuf, true, NULL);
            }
        else
            {
            double  fillPriority = context.GetDisplayPriority();

#if defined (NEEDSWORK_FILL_PRIORITY)
            // Make sure fill displays behind mline profile lines and pattern in 2d...
            if (context.GetIDrawGeom().IsOutputQuickVision ())
                fillPriority -= 1.0; // Doesn't seem to have any affect?!?
#endif

            std::valarray<DPoint2d> localPoints2dBuf (5);

            for (int iPoint = 0; iPoint < 5; iPoint++)
                {
                localPoints2dBuf[iPoint].x = shapeBuf[iPoint].x;
                localPoints2dBuf[iPoint].y = shapeBuf[iPoint].y;
                }

            context.GetIDrawGeom().DrawShape2d (5, &localPoints2dBuf[0], true, fillPriority, NULL);
            }
        }

    if (pushOffset)
        context.PopTransformClip ();

    if (NULL != pickGeom)
        pickGeom->SetHitPriorityOverride (HitPriority::Highest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     MultilineHandler::StrokeMline (ElementHandleCR thisElm, ViewContextR context)
    {
    DgnElementCR elm = *thisElm.GetElementCP ();

    // Skip frozen elements
    if (elm.ToMlineElm().freezeGroup)
        return SUCCESS;

    // Ignore mlines with bad version numbers because we could not have properly converted them from V7->V8
    if (elm.ToMlineElm().version != MLINE_VERSION)
        return ERROR;

    MlineTopology  topology;

    context.SetElemTopology (&topology);

    int         i, nPoints, nLines;
    DPoint3d    workLine[MLINE_MAXPOINTS];
    JointDef    joints[MLINE_MAXPOINTS];

    nPoints = elm.ToMlineElm().nPoints;
    nLines  = elm.ToMlineElm().nLines;

    MultilineHandler::ExtractPointArray (thisElm, workLine, 0, nPoints);

    if (elm.ToMlineElm().flags.closed)
        workLine[0] = workLine[nPoints-1];

    MultilineHandler::GetCapJointDef (*joints, thisElm, workLine, 0);

    for (i=1; i<nPoints-1; i++)
        MultilineHandler::GetJointDef (*(joints+i), thisElm, workLine, i-1);

    MultilineHandler::GetCapJointDef (*(joints+(nPoints-1)), thisElm, workLine+(nPoints-2), 1);

    topology.SetCapNumber (0);

    BentleyStatus   status = SUCCESS;

    for (i=0; i<nLines; i++)
        {
        topology.SetLineNumber (i);
        topology.SetLsBase (0);

        if (SUCCESS != (status = DrawLineString (thisElm, context, workLine, joints, nPoints, &elm.ToMlineElm().profile[i].symb, i, topology)))
            goto cleanupMline;
        }

    topology.SetLineNumber (0);
    topology.SetLsBase (0);

    if (elm.ToMlineElm().midCap.capLine)
        {
        int         min, max;
        double      dist1, dist2;

        MultilineHandler::GetLimitProfiles (min, max, thisElm);

        dist1 = elm.ToMlineElm().profile[min].dist;
        dist2 = elm.ToMlineElm().profile[max].dist;

        for (int j = (elm.ToMlineElm().flags.closed ? 0 : 1); j<nPoints-1; j++)
            {
            DSegment3d jointLine;

            bsiDPoint3d_addScaledDVec3d (&jointLine.point[0], workLine+j, (DVec3dP) &joints[j].dir, (double) dist1 * joints[j].scale);
            bsiDPoint3d_addScaledDVec3d (&jointLine.point[1], workLine+j, (DVec3dP) &joints[j].dir, (double) dist2 * joints[j].scale);

            topology.SetLsBase(j);
            topology.SetCapNumber (2);

            if (SUCCESS != (status = DrawJointLine (thisElm, context, &jointLine, &elm.ToMlineElm().midCap, 2)))
                goto cleanupMline;
            }
        }

    if (!elm.ToMlineElm().flags.closed)
        {
        topology.SetLsBase(0);
        topology.SetCapNumber(1);

        if (SUCCESS != (status = DrawEndCap (thisElm, context, workLine, joints, 0)))
            goto cleanupMline;

        topology.SetLsBase (nPoints-2);
        topology.SetCapNumber (3);

        if (SUCCESS != (status = DrawEndCap (thisElm, context, workLine+(nPoints-1), joints+(nPoints-1), 1)))
            goto cleanupMline;
        }

cleanupMline:

    context.SetElemTopology (NULL);

    return status;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          MLineFillStroker : IStrokeForCache
{
int             m_minLine;
int             m_maxLine;
bool            m_extrude;

explicit MLineFillStroker (int minLine, int maxLine, bool extrude = false) {m_minLine = minLine; m_maxLine = maxLine; m_extrude = extrude;}

virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize = 0.0) override
    {
    BeAssert(NULL != dh.GetElementHandleCP());
    ElementHandleCR thisElm = *dh.GetElementHandleCP();
    CurveVectorPtr  curve = MultilineHandler::BoundaryToCurveVector (thisElm, m_minLine, m_maxLine);

    if (!curve.IsValid ())
        return;

    if (m_extrude)
        {
        ElemDisplayParamsP  params = context.GetCurrentDisplayParams ();
        bool                isCapped;
        DVec3dCP            thicknessVector = params->GetThickness (isCapped);
        DgnExtrusionDetail  detail (curve, thicknessVector ? *thicknessVector : DVec3d::From (0.0, 0.0, 1.0), isCapped);
        ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

        context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);

        return;
        }

    context.GetIDrawGeom ().DrawCurveVector (*curve, true);
    }

}; // MLineFillStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::DrawPatterns (ElementHandleCR thisElm, ViewContextR context)
    {
    // Pattern display follows level/class of mline dhdr...
    if (!TestLevelAndClass (thisElm, context, NULL))
        return;

    // Re-cook mline symb which may have changed after visiting profile lines/end caps, etc.
    context.CookElemDisplayParams (thisElm);

    PatternParamsP   params;
    DwgHatchDefLineP hatchLines;

    for (int index = 0; !context.CheckStop (); index++)
        {
        ViewContext::PatternParamSource source (index);

        if (NULL == (params = source.GetParams (thisElm, NULL, &hatchLines, NULL, &context)))
            break;

        // mline origin always used as pattern origin...
        MultilinePointPtr   mlinePoint = MultilineHandler::GetPoint (thisElm, 0);

        params->origin = mlinePoint->GetPoint ();

        if (PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Offset))
            bsiDPoint3d_addDPoint3dDPoint3d (&params->origin, &params->origin, (DVec3dP) &params->offset);

        MLineFillStroker    stroker (params->minLine, params->maxLine);

        // Don't save qvElem since we may have N pattern linkages with different mline profile boundaries...
        context.DrawAreaPattern (thisElm, ViewContext::ClipStencil (stroker, 0, false), ViewContext::PatternParamSource (params, hatchLines, index));
        };
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          MLineThicknessStroker : IStrokeForCache
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    MLineFillStroker  stroker (-1, -1, true);

    stroker._StrokeForCache (dh, context, pixelSize);
    }

}; // MLineThicknessStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::DrawMline (ElementHandleCR thisElm, ViewContextR context, UInt32 info)
    {
    if (0 != (info & DISPLAY_INFO_Thickness))
        {
        MLineThicknessStroker stroker;

        context.DrawWithThickness (thisElm, stroker, 0);
        }

    if (0 != (info & DISPLAY_INFO_Fill) || 0 != (info & DISPLAY_INFO_Surface))
        {
        // NOTE: For drop output single filled shape instead of a shape per-segment...
        if (DrawPurpose::CaptureGeometry == context.GetDrawPurpose ())
            context.DrawCached (thisElm, MLineFillStroker (0, 0), 0);
        else
            StrokePolygons (thisElm, context, -1, -1);
        }

    // Always drawOutline...thickness only displays boundary...
    // Don't want to test DISPLAY_INFO_Edge because of TR #186097 - when the mline is filled and the edge color
    // matches the fill color and weight is 0, DISPLAY_INFO_Edge is not set.
    if (info != DISPLAY_INFO_None)
        StrokeMline (thisElm, context);

    // NOTE: Changes current matsymb...should do this last...
    if (context.WantAreaPatterns ())
        DrawPatterns (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
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
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UInt32      info = context.GetDisplayInfo (_IsRenderable (thisElm));
    Transform   newScTrans, newTopTrans;

    if (context.GetIgnoreScaleForMultilines () && SUCCESS == ElementUtil::GetIgnoreScaleDisplayTransforms (&newTopTrans, &newScTrans, context))
        {
        EditElementHandle  workElm (thisElm, true);

        // Redefine top of stack to just reference transforms...
        ViewContext::ContextMark    mark (context, workElm);

        context.PushTransform (newTopTrans);

        // Apply non-WYSIWYG transform from shared cells to element
        TransformInfo   newScTransInfo (newScTrans);

        workElm.GetHandler().ApplyTransform (workElm, newScTransInfo);

        DrawMline (workElm, context, info);

        return;
        }

    DrawMline (thisElm, context, info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::_IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    bool accHdr = T_Super::_IsVisible (elHandle, context, testRange, testLevel, testClass);

    if (!testLevel)
        return  accHdr;

    MlineElm const* mline  = &elHandle.GetElementCP()->ToMlineElm();
    int             nLines = mline->nLines;

    ScanCriteriaCP scanCrit = context.GetScanCriteria();
    if (NULL == scanCrit)
        return  true;

    ElemHeaderOverrides const* ovr = context.GetHeaderOvr();
    BitMaskCP levelMask = scanCrit->GetLevelBitMask ();

    LevelId level;
    /*-------------------------------------------------------------------
    Check the level of the three caps
    -------------------------------------------------------------------*/
    if (mline->orgCap.capLine || mline->orgCap.capOutArc || mline->orgCap.capInArc)
        {
        if (!(level = LevelId(mline->orgCap.level)).IsValid())  /* level == 0 means use header level */
            {
            if (accHdr)
                return true;
            }
        else
            {
            if (ovr)
                level = ovr->AdjustLevel (level);

            if (NULL == levelMask || levelMask->Test (level.GetValue() - 1))
                return true;
            }
        }

    if (mline->endCap.capLine || mline->endCap.capOutArc || mline->endCap.capInArc)
        {
        if (!(level = LevelId(mline->endCap.level)).IsValid())  /* level == 0 means use header level */
            {
            if (accHdr)
                return true;
            }
        else
            {
            if (ovr)
                level = ovr->AdjustLevel (level);

            if (NULL == levelMask || levelMask->Test (level.GetValue() - 1))
                return true;
            }
        }

    if (mline->midCap.capLine || mline->midCap.capOutArc || mline->midCap.capInArc)
        {
        if (!(level = LevelId(mline->midCap.level)).IsValid())  /* level == 0 means use header level */
            {
            if (accHdr)
                return true;
            }
        else
            {
            if (ovr)
                level = ovr->AdjustLevel (level);

            if (NULL == levelMask || levelMask->Test (level.GetValue() - 1))
                return true;
            }
        }

    /*-------------------------------------------------------------------
    Step through the profile array and check the level of each line
    -------------------------------------------------------------------*/
    for (int iLine=0; iLine<nLines; iLine++)
        {
        const MlineProfile    *pMlineProfile = &mline->profile[iLine];

        if (!(level = LevelId(pMlineProfile->symb.level)).IsValid())  /* level == 0 means use header level */
            {
            if (accHdr)
                return true;
            }
        else
            {
            if (ovr)
                level = ovr->AdjustLevel (level);

            if (NULL == levelMask || levelMask->Test (level.GetValue() - 1))
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void       MultilineHandler::ApplyMaskedSymbology
(
MlineSymbology*             destSymb,
MultilineSymbologyCR        sourceSymb,
MultilineStylePropMaskCR    pStyleShields,
bool                        isCap,
int                         pcIndex       // Profile or cap index
)
    {
    // Common symbology
    if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_Class))
        {
        destSymb->conClass = static_cast<UInt32>(sourceSymb.GetClass());
        destSymb->useClass = sourceSymb.UsesClass();
        }

    if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_Color))
        {
        destSymb->color     = sourceSymb.GetColor();
        destSymb->useColor  = sourceSymb.UsesColor();
        }

    if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_Weight))
        {
        destSymb->weight    = sourceSymb.GetWeight();
        destSymb->useWeight = sourceSymb.UsesWeight();
        }

    if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_LineStyle))
        {
        destSymb->style         = sourceSymb.GetLinestyle();
        destSymb->useStyle      = sourceSymb.UsesLinestyle();
        destSymb->customStyle   = sourceSymb.GetCustomLinestyleBit();
        }

    if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_Level))
        {
        destSymb->level = sourceSymb.GetLevel().GetValue();
        }

    // Cap symbology
    if (isCap)
        {
        if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_CapColorFromSeg))
            destSymb->capColorFromSeg = sourceSymb.UseCapColorFromSegment();

        if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_CapLine))
            destSymb->capLine = sourceSymb.UseCapLine();

        if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_CapOutArc))
            destSymb->capOutArc = sourceSymb.UseCapOuterArc();

        if (!pStyleShields.GetCapOrProfileBit (isCap, pcIndex, MLINESTYLE_PROP_CapInArc))
            destSymb->capInArc = sourceSymb.UseCapInnerArc();
        }
    else
        {
        destSymb->capColorFromSeg   = 0;
        destSymb->capLine           = 0;
        destSymb->capOutArc         = 0;
        destSymb->capInArc          = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
double      MultilineHandler::GetOffsetDistance (ElementHandleR element, MlineOffsetMode offsetMode) const
    {
    if (MlineOffsetMode::ByWork == offsetMode || MlineOffsetMode::Unknown == offsetMode) // Shortcut to avoid limit line calculation
        return 0.0;

    int         min, max;
    MultilineHandler::GetLimitProfiles (min, max, element);

    double minDist = GetProfileDistance (element, min);
    double maxDist = GetProfileDistance (element, max);

    double      placementOffset = 0.0;
    switch (offsetMode)
        {
        case MlineOffsetMode::ByCenter:   /* Place by center    */
            placementOffset = (minDist + maxDist) / 2.0;
            break;

        case MlineOffsetMode::ByMax:   /* Place by maximum   */
            placementOffset = maxDist;
            break;

        case MlineOffsetMode::ByMin:   /* Place by minimum   */
            placementOffset = minDist;
            break;

        case MlineOffsetMode::Custom:   /* Rob's wacky "shift the work line" gizmo   */
            placementOffset = GetPlacementOffset (element);
            break;

        case MlineOffsetMode::ByWork:
        case MlineOffsetMode::Unknown:
        default:
            placementOffset = 0.0;
            break;
        }

    return placementOffset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void       MultilineHandler::GetMaskedProfile (MlineProfile* mlineProfile, MultilineStyleCR mlineStyle, int profileNum, MultilineStylePropMaskCR styleShields, double styleScale)
    {
    MlineProfile    maskedProfile = *mlineProfile;

    MultilineProfileCP  profile = mlineStyle.GetProfileCP (profileNum);

    ApplyMaskedSymbology (&maskedProfile.symb, *profile, styleShields, false, profileNum);

    if (!styleShields.GetProfileBit (profileNum, MLINESTYLE_PROP_Offset))
        maskedProfile.dist = profile->GetDistance() * styleScale;

    *mlineProfile = maskedProfile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::AppendLineStyleData (DgnElementR pMline, bool& wasAdded, int lineNo, int isCap, LineStyleParamsCR lsParams)
    {
    LineStyleParams  params = lsParams;

    wasAdded = false;
    // Don't append a linkage with no info
    if (0 == params.modifiers)
        return;

    params.lineMask   |= (1 << lineNo);
    params.mlineFlags  = (isCap ? MLSFLAG_CAP : MLSFLAG_LINE);
    params.modifiers  |=  (STYLEMOD_LINEMASK|STYLEMOD_MLINEFLAGS);

    LineStyleLinkageUtil::SetStyleParams (&pMline, &params);
    wasAdded = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::AppendCapLineStyle (DgnElementR pMline, bool& wasAdded, int lineNo, MultilineSymbologyCR pCap)
    {
    wasAdded = false;
    if (pCap.UseCapLine() || pCap.UseCapOuterArc() || pCap.UseCapInnerArc())
        {
        if (pCap.UsesLinestyle())
            AppendLineStyleData (pMline, wasAdded, lineNo, true, pCap.GetLinestyleParamsCR());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineHandler::_ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double newStyleScale)
    {
    if (MULTILINE_ELM != element.GetLegacyType())
        {
        BeAssert (0 && "Not a multiline element");
        return ERROR;
        }

    MultilineStylePropMaskPtr elmShields = MultilineStylePropMask::Create ();
    elmShields->FromElement (element);

    // Have to scale to model...
    double      styleScale = mlineStyle.GetUorScaleToModel (element.GetDgnModelP());

    // Set up style
    SetStyleScale (element, newStyleScale == 0.0 ? 1.0 : newStyleScale); // Will be 0 coming from older elements.
    SetStyleID (element, mlineStyle.GetID ());

    styleScale *= GetStyleScale(element); // Compensate if mline style is scaled on this element

    UInt32 numProfiles = mlineStyle.GetProfileLineCount();

    // Resize space for profiles if necessary
    bool     numProfilesChanged = false;
    if (numProfiles != GetProfileCount (element))
        {
        DgnV8ElementBlank   fullElm;
        element.GetElementCP ()->CopyTo (fullElm);

        int chgLines = numProfiles - fullElm.ToMlineElm().nLines;
        char        *src, *dst, *end;

        end  = (char*)&fullElm + fullElm.Size ();
        src  = (char*)(MlinePoint*) (fullElm.ToMlineElm().profile + fullElm.ToMlineElm().nLines);
        dst  = src + (chgLines * sizeof(MlineProfile));
        ptrdiff_t size = end - src;
        memmove (dst, src, size);

        int words = (chgLines * (int)sizeof(MlineProfile)) / 2;
        fullElm.IncrementSizeWords(words);

        fullElm.ToMlineElmR().nLines = static_cast<byte>(numProfiles);

        numProfilesChanged = true;
        element.ReplaceElement (&fullElm);
        }

    MlineElm*  mline = &element.GetElementP()->ToMlineElmR();

    // Get the profiles
    for (int iProfile=0; iProfile<mline->nLines; iProfile++)
        {
        if (numProfilesChanged)
            {
            memset (&(mline->profile[iProfile]), 0, sizeof(mline->profile[iProfile]));
            // Overrides can't apply when number of profiles changes.
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_Class, 0);
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_Color, 0);
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_Weight, 0);
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_LineStyle, 0);
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_Level, 0);
            elmShields->SetProfileBit (iProfile, MLINESTYLE_PROP_Offset, 0);
            }
        GetMaskedProfile (&(mline->profile[iProfile]), mlineStyle, iProfile, *elmShields, styleScale);
        }

    // Honor current offsets
    if (!IsOffsetModeValid (element))
        {
        SetOffsetMode (element, MlineOffsetMode::ByWork);
        SetPlacementOffset (element, 0.0);
        }
    else
        {
        // Shift all the lines by the correct offset for this placement type.
        SetPlacementOffset (element, GetOffsetDistance (element, GetOffsetMode (element)));

        double placementOffset = GetPlacementOffset (element);
        if (0.0 != placementOffset)
            {
            for (int iProfile=0; iProfile<mline->nLines; iProfile++)
                mline->profile[iProfile].dist -= placementOffset;
            }
        }

    ApplyMaskedSymbology (&mline->orgCap, mlineStyle.GetOrgCapCR(), *elmShields, true, MULTILINE_ORG_CAP);
    ApplyMaskedSymbology (&mline->midCap, mlineStyle.GetMidCapCR(), *elmShields, true, MULTILINE_MID_CAP);
    ApplyMaskedSymbology (&mline->endCap, mlineStyle.GetEndCapCR(), *elmShields, true, MULTILINE_END_CAP);

    if (!elmShields->GetGeneralBit (MLINESTYLE_PROP_OrgAngle))
        SetOriginAngle (element, mlineStyle.GetOriginAngle());

    if (!elmShields->GetGeneralBit (MLINESTYLE_PROP_EndAngle))
        SetEndAngle (element, mlineStyle.GetEndAngle());

    if (!elmShields->GetGeneralBit (MLINESTYLE_PROP_Fill))
        {
        if (mlineStyle.GetFilled())
            {
            UInt32 fillColor = mlineStyle.GetFillColor();
            AddSolidFill (element, &fillColor, NULL);
            }
        else
            {
            RemoveAreaFill (element);
            }
        }

    { // This just limits the scope of fullElm to make sure it can't be used after
        // Need a full element to mess with line style linkages.
        DgnV8ElementBlank   fullElm;
        element.GetElementCP ()->CopyTo (fullElm);
        mline = (MlineElm *) &fullElm.ToMlineElmR();

        // Flush all unshielded line styles except header style
        UInt32 lineMask = 0;
        for (int iProfile=0; iProfile<mline->nLines; iProfile++)
            {
            if (!elmShields->GetProfileBit (iProfile, MLINESTYLE_PROP_LineStyle))
                lineMask |= (1<<iProfile);
            }
        LineStyleLinkageUtil::ClearElementStyle (&fullElm, false, lineMask, MLSFLAG_LINE);

        lineMask = 0;
        for (int iCap=0; iCap<3; iCap++)
            {
            if (!elmShields->GetCapBit ((MultilineCapType)iCap, MLINESTYLE_PROP_LineStyle))
                lineMask |= (1<<iCap);
            }
        LineStyleLinkageUtil::ClearElementStyle (&fullElm, false, lineMask, MLSFLAG_CAP);

        bool    wasAdded = false;

        /* Add the active line style parameters */
        for (int iProfile=0; iProfile<mline->nLines; iProfile++)
            {
            if (elmShields->GetProfileBit (iProfile, MLINESTYLE_PROP_LineStyle))
                {
                // customStyle is obsolete since style holds all style ids.  Just check if there are any modifiers.
                LineStyleParamsCR lsInfo = mlineStyle.GetProfileCP(iProfile)->GetLinestyleParamsCR ();
                if (mlineStyle.GetProfileCP(iProfile)->UsesLinestyle() && 0 != lsInfo.modifiers)
                    {
                    AppendLineStyleData (fullElm, wasAdded, iProfile, false, lsInfo);
                    // For backward compatability, make sure the custom line style bit is set.
                    if (wasAdded)
                        mline->profile[iProfile].symb.customStyle = 1;
                    }
                }
            }

        if (!elmShields->GetCapBit (MULTILINE_ORG_CAP, MLINESTYLE_PROP_LineStyle))
            {
            AppendCapLineStyle (fullElm, wasAdded, MULTILINE_ORG_CAP, mlineStyle.GetOrgCapCR());
            // For backward compatability, make sure the custom line style bit is set.
            if (wasAdded)
                mline->orgCap.customStyle = 1;
            }
        if (!elmShields->GetCapBit (MULTILINE_END_CAP, MLINESTYLE_PROP_LineStyle))
            {
            AppendCapLineStyle (fullElm, wasAdded, MULTILINE_END_CAP, mlineStyle.GetEndCapCR());
            // For backward compatability, make sure the custom line style bit is set.
            if (wasAdded)
                mline->endCap.customStyle = 1;
            }
        if (!elmShields->GetCapBit (MULTILINE_MID_CAP, MLINESTYLE_PROP_LineStyle))
            {
            AppendCapLineStyle (fullElm, wasAdded, MULTILINE_MID_CAP, mlineStyle.GetMidCapCR());
            // For backward compatability, make sure the custom line style bit is set.
            if (wasAdded)
                mline->midCap.customStyle = 1;
            }
        element.ReplaceElement (&fullElm);
        mline = (MlineElm *) &(element.GetElementP()->ToMlineElmR());
    }

    // If the style is modified from the base style, we need to update the local style shields
    WString styleName = mlineStyle.GetName();

    if (styleName.length() > 0)
        {
        MultilineStylePropMaskPtr activeStyleShields = MultilineStylePropMask::Create ();
        MultilineStylePtr namedStyle = MultilineStyle::GetByName (styleName.c_str(), element.GetDgnModelP()->GetDgnProject());

        if (namedStyle.IsValid())  // If the style hasn't been saved to the file yet, it won't be valid
            {
            activeStyleShields->CompareStyles (mlineStyle, *namedStyle);
            /* If the number of profiles is overridden, we can't really re-apply the style so we have to punt and
               make it style-less */
            if (activeStyleShields->GetGeneralBit (MLINESTYLE_PROP_NumLines))
                {
                SetStyleID (element, ElementId());
                }
            else if (activeStyleShields->AnyBitSet())
                {
                activeStyleShields->LogicalOperation (*activeStyleShields, *elmShields, BitMaskOperation::Or);
                MultilineStylePropMask::RemoveFromElement (element);
                activeStyleShields->AddToElement (element);
                }
            }
        }

    return element.GetDisplayHandler ()->ValidateElementRange (element);
    }

/*---------------------------------------------------------------------------------**//**
* This function is designed so that the mline style can be initialized before it comes in,
* and then the relevant parts will be overwritten here.  However, this only applies to caps
* and general stuff as the profiles have to be cleared in case there is a different number of them.
*
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr MultilineHandler::_GetStyle (ElementHandleCR eh, MultilineStyleCP seedStyle, UInt32 options) const
    {
    MultilineStylePtr   newStyle = MultilineStyle::Create(L"", eh.GetDgnModelP()->GetDgnProject());
    if (NULL != seedStyle)
        newStyle->CopyValues (*seedStyle);

    if (options & MLINE_MATCH_ENDCAPS)
        {
        newStyle->SetOriginAngle (GetOriginAngle (eh));
        newStyle->SetEndAngle (GetEndAngle (eh));

        MultilineSymbologyPtr orgCap = newStyle->GetOrgCap();
        orgCap->SetSymbology (eh.GetElementCP()->ToMlineElm().orgCap);
        newStyle->SetOrgCap (*orgCap);

        MultilineSymbologyPtr endCap = newStyle->GetEndCap();
        endCap->SetSymbology (eh.GetElementCP()->ToMlineElm().endCap);
        newStyle->SetEndCap (*endCap);
        }

    newStyle->GetMidCap()->SetSymbology (eh.GetElementCP()->ToMlineElm().midCap);

    UInt32          fillColor=0;
    if (GetSolidFill (eh, &fillColor, NULL))
        {
        fillColor = ColorUtil::GetV8ElementColor (fillColor);
        newStyle->SetFilled (true);
        newStyle->SetFillColor (fillColor);
        }

    // Remove any existing profiles from the style.  There's no way to use a different number of profiles.
    newStyle->ClearProfiles ();

    // copy profiles
    double styleScale = (GetStyleScale(eh)==0.0 ? 0.0 : 1.0/GetStyleScale(eh));
    for (UInt32 iProfile=0; iProfile < GetProfileCount(eh); iProfile++)
        {
        // See if there is a custom style on this mline; if so grab the parameters
        LineStyleParams params;
        MultilineHandler::GetLinestyleParams (eh, params, false, iProfile, true);

        MlineProfile const* pMlineProfile = _GetMlineProfileDefCP (eh, iProfile);

        MultilineProfilePtr    profile = MultilineProfile::Create(pMlineProfile, &params);
        if (0.0 != styleScale)
            profile->ScaleDistance (styleScale);
        newStyle->InsertProfile (*profile, iProfile);
        }

    LineStyleParams params;
    GetLinestyleParams (eh, params, true, MULTILINE_MID_CAP, true);
    if (0 != params.modifiers)
        {
        MultilineSymbologyPtr midCap = newStyle->GetMidCap();
        midCap->SetLinestyleParams (params);
        newStyle->SetMidCap (*midCap);
        }

    if (options & MLINE_MATCH_ENDCAPS)
        {
        GetLinestyleParams (eh, params, true, MULTILINE_ORG_CAP, true);
        if (0 != params.modifiers)
            {
            MultilineSymbologyPtr orgCap = newStyle->GetOrgCap();
            orgCap->SetLinestyleParams (params);
            newStyle->SetOrgCap (*orgCap);
            }

       GetLinestyleParams (eh, params, true, MULTILINE_END_CAP, true);
        if (0 != params.modifiers)
            {
            MultilineSymbologyPtr endCap = newStyle->GetEndCap();
            endCap->SetLinestyleParams (params);
            newStyle->SetEndCap (*endCap);
            }
        }

#ifdef DGN_IMPORTER_REORG_WIP
    if (0 != GetStyleID(eh))
        {
        WString styleString =  MultilineStyle::GetNameFromId (GetStyleID(eh), *eh.GetDgnModelP()->GetDgnProject());
        newStyle->SetName (styleString.c_str());
        }
#endif
    return newStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::HasMultilineStyle (ElementHandleCR element)
    {
    return (GetStyleID(element).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId        MultilineHandler::GetStyleID (ElementHandleCR element) const
    {
    return ElementId(element.GetElementCP()->ToMlineElm().styleParentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineHandler::SetStyleID (EditElementHandleR element, ElementId styleId)
    {
    element.GetElementP()->ToMlineElmR().styleParentId = styleId.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        MultilineHandler::GetStyleName (ElementHandleCR element) const
    {
#ifdef DGN_IMPORTER_REORG_WIP
    return MultilineStyle::GetNameFromId (element.GetElementCP()->ToMlineElm().styleParentId, *element.GetDgnModelP()->GetDgnProject());
#endif
    return  L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double        MultilineHandler::_GetStyleScale (ElementHandleCR element) const
    {
    return element.GetElementCP()->ToMlineElm().styleScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineHandler::SetStyleScale (EditElementHandleR element, double scale)
    {
    element.GetElementP()->ToMlineElmR().styleScale = scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double        MultilineHandler::_GetPlacementOffset (ElementHandleCR element) const
    {
    return element.GetElementCP()->ToMlineElm().placementOffset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MultilineHandler::_SetPlacementOffset (EditElementHandleR element, double placementOffset)
    {
    if (MULTILINE_ELM != element.GetLegacyType())
        return BSIERROR;

    element.GetElementP()->ToMlineElmR().placementOffset = placementOffset;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineOffsetMode        MultilineHandler::_GetOffsetMode (ElementHandleCR element) const
    {
    if (IsOffsetModeValid (element))
        return (MlineOffsetMode)element.GetElementCP()->ToMlineElm().offsetMode;
    return MlineOffsetMode::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus        MultilineHandler::_SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode)
    {
    if (MULTILINE_ELM != element.GetLegacyType())
        return BSIERROR;

    if (MlineOffsetMode::Unknown == offsetMode)
        {
        SetOffsetModeValid (element, false);
        }
    else
        {
        element.GetElementP()->ToMlineElmR().offsetMode = static_cast<byte>(offsetMode);
        SetOffsetModeValid (element, true);
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MultilineHandler::IsOffsetModeValid (ElementHandleCR element) const
    {
    return element.GetElementCP()->ToMlineElm().flags.offsetModeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineHandler::SetOffsetModeValid (EditElementHandleR element, bool offsetModeValid)
    {
    element.GetElementP()->ToMlineElmR().flags.offsetModeValid = offsetModeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double        MultilineHandler::_GetOriginAngle (ElementHandleCR element) const
    {
    return element.GetElementCP()->ToMlineElm().orgAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus        MultilineHandler::_SetOriginAngle (EditElementHandleR element, double angle)
    {
    if (MULTILINE_ELM != element.GetLegacyType())
        return BSIERROR;

    element.GetElementP()->ToMlineElmR().orgAngle = angle;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double        MultilineHandler::_GetEndAngle (ElementHandleCR element) const
    {
    return element.GetElementCP()->ToMlineElm().endAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus        MultilineHandler::_SetEndAngle (EditElementHandleR element, double angle)
    {
    if (MULTILINE_ELM != element.GetLegacyType())
        return BSIERROR;

    element.GetElementP()->ToMlineElmR().endAngle = angle;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
Int32  MultilineHandler::GetLinestyle (ElementHandleCR element, bool isCap, UInt32 lsIndex) const
    {
    if (isCap)
        {
        switch (lsIndex)
            {
            case MULTILINE_ORG_CAP:
                return element.GetElementCP()->ToMlineElm().orgCap.style;
                break;

            case MULTILINE_END_CAP:
                return element.GetElementCP()->ToMlineElm().endCap.style;
                break;

            case MULTILINE_MID_CAP:
                return element.GetElementCP()->ToMlineElm().midCap.style;
                break;
            }
        }
    else if (IsIndexValid(&element, lsIndex))
        {
        return element.GetElementCP()->ToMlineElm().profile[lsIndex].symb.style;
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
void  MultilineHandler::SetLinestyle (EditElementHandleR element, UInt32 styleNo, bool isCap, UInt32 lsIndex)
    {
    if (isCap)
        {
        switch (lsIndex)
            {
            case MULTILINE_ORG_CAP:
                element.GetElementP()->ToMlineElmR().orgCap.style = styleNo;
                break;

            case MULTILINE_END_CAP:
                element.GetElementP()->ToMlineElmR().endCap.style = styleNo;
                break;

            case MULTILINE_MID_CAP:
                element.GetElementP()->ToMlineElmR().midCap.style = styleNo;
                break;
            }
        }
    else if (IsIndexValid(&element, lsIndex))
        {
        element.GetElementP()->ToMlineElmR().profile[lsIndex].symb.style = styleNo;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MultilineHandler::GetLinestyleParams (ElementHandleCR element, LineStyleParamsR params, bool isCap, UInt32 lsIndex, bool allowSizeChange) const
    {
    params.Init();  // Defaults

    if (!isCap && !IsIndexValid(&element, lsIndex))
        return  ERROR;

    /* Look for any customization linkages */
    LineStyleParams     mlParams;
    LineStyleLinkageUtil::ExtractStyleLinkageFromMultiLine (mlParams, element.GetElementCP(), isCap, lsIndex);

    if (mlParams.modifiers == 0)
        return SUCCESS; // Nothing found

    // Copy in any customizations
    params = mlParams;

    // If these aren't overridden, use from mline elm
    if (allowSizeChange && element.GetElementCP()->Is3d() && !(params.modifiers & (STYLEMOD_RMATRIX | STYLEMOD_NORMAL)))
        {
        DVec3d zVector;
        GetZVector (zVector, element);
        params.normal.init (&zVector);
        LegacyMath::RMatrix::FromNormalVector (&params.rMatrix, (DVec3d *)&params.normal);
        params.modifiers |= (STYLEMOD_RMATRIX | STYLEMOD_NORMAL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MultilineHandler::SetLinestyleParams (EditElementHandleR element, LineStyleParamsCR params, bool isCap, UInt32 lsIndex)
    {
    if (!isCap && !IsIndexValid(&element, lsIndex))
        return  ERROR;

    LineStyleParams  tmpParams = params;
    tmpParams.lineMask   |= (1 << lsIndex);
    tmpParams.mlineFlags  = (isCap ? MLSFLAG_CAP : MLSFLAG_LINE);
    tmpParams.modifiers  |=  (STYLEMOD_LINEMASK|STYLEMOD_MLINEFLAGS);

    DgnV8ElementBlank elm;
    element.GetElementCP ()->CopyTo (elm);
    LineStyleLinkageUtil::SetStyleParams (&elm, &tmpParams);
    element.ReplaceElement (&elm);

    // CustomStyle is not used, but keep it up to date for legacy versions.
    bool         bCustomStyle = (0 == params.modifiers ? false : true);
    if (isCap)
        {
        switch (lsIndex)
            {
            case MULTILINE_ORG_CAP:
                element.GetElementP()->ToMlineElmR().orgCap.customStyle = bCustomStyle;
                break;

            case MULTILINE_END_CAP:
                element.GetElementP()->ToMlineElmR().endCap.customStyle = bCustomStyle;
                break;

            case MULTILINE_MID_CAP:
                element.GetElementP()->ToMlineElmR().midCap.customStyle = bCustomStyle;
                break;
            }
        }
    else
        {
        element.GetElementP()->ToMlineElmR().profile[lsIndex].symb.customStyle = bCustomStyle;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double        MultilineHandler::GetProfileDistance (ElementHandleCR element, UInt32 profileNum) const
    {
    return element.GetElementCP()->ToMlineElm().profile[profileNum].dist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineHandler::SetProfileDistance (EditElementHandleR element, UInt32 profileNum, double profileDistance)
    {
    element.GetElementP()->ToMlineElmR().profile[profileNum].dist = profileDistance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineHandler::SetOverrides
(
EditElementHandleR             element,
MultilineStylePropMaskCP    shieldFlags,        // <=> Modified if we can't set them as they are (fewer lines, etc); NULL to clear
bool                        applyStyle
)
    {

    if (MULTILINE_ELM != element.GetElementCP()->GetLegacyType())
        {
        BeAssert (0 && "Not a multiline element");
        return ERROR;
        }

    if (!HasMultilineStyle (element) || NULL == shieldFlags || !shieldFlags->AnyBitSet() )
        {
        MultilineStylePropMask::RemoveFromElement (element);
        }
    else
        {
        // Need to make sure that shield flags apply; don't shield lines that don't exist
        //stripUnusedShieldFlags (shieldFlags, &(*ppMline)->el);
        shieldFlags->AddToElement (element);
        }

    if (!applyStyle)
        return SUCCESS;

    MultilineStylePtr   mlineStyle = MultilineStyle::GetByID (GetStyleID(element), element.GetDgnModelP()->GetDgnProject());
    if (mlineStyle.IsValid())
        {
        return ApplyStyle (element, *mlineStyle, GetStyleScale(element));
        }

    return ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                Chuck.Kirschman         10/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineHandler::UpdateShieldsByCompareWithStyle (EditElementHandleR element)
    {
    // If there is no style, exit
    if (!HasMultilineStyle (element))
        return false;

    // Get the style representing the element
    MultilineStylePtr   elementMlineStyle = GetStyle (element, NULL, 0);

    // Get the saved style
    WString styleName = elementMlineStyle->GetName ();
    MultilineStylePtr savedMlineStyle = MultilineStyle::GetByName (styleName.c_str(), element.GetDgnModelP()->GetDgnProject());

    if (!savedMlineStyle.IsValid())
        return false;  // Can't find saved style?!

    // Find the differences between the styles
    MultilineStylePropMaskPtr newShields = MultilineStylePropMask::Create ();
    newShields->CompareStyles (*elementMlineStyle, *savedMlineStyle);

    // Get the existing shields
    MultilineStylePropMaskPtr elmShields = MultilineStylePropMask::Create ();
    elmShields->FromElement (element);

    // OR the existing shields with the new differences
    newShields->LogicalOperation (*newShields, *elmShields, BitMaskOperation::Or);

    // Put new shields back on the element
    SetOverrides (element, newShields.get(), false);  // Will remove the mask if no bits set
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::ProcessLineStyleIDs
(
PropertyContextR        proc,
ElementHandleCR            eh,
EditElementHandleP         eehP,
Int32*                  outStyleP,
const MlineSymbology&   inSymb,
bool                    isCap,
int                     profileIndex
)
    {
    DgnElementCP elmCP = eh.GetElementCP ();
    DgnElementP  elmP  = (NULL != eehP) ? eehP->GetElementP () : NULL;
    bool        changed = false;

    LineStyleParams     currentParams;

    currentParams.Init();
    LineStyleLinkageUtil::ExtractStyleLinkageFromMultiLine (currentParams, elmCP, isCap, profileIndex);

    EachLineStyleArg    lstyleArg (inSymb.style, &currentParams, SETPROPCBEIFLAG(inSymb.useStyle), proc);

    if (proc.DoLineStyleCallback (outStyleP, lstyleArg))
        {
        if (lstyleArg.GetParamsChanged ())
            {
            eehP->GetElementDescrCP (); // Make sure there's an elmdscr allocated in this elem handle

            if (0 == lstyleArg.GetParams ()->modifiers) // No modifiers is a delete.
                {
                if (0 != currentParams.modifiers)
                    LineStyleLinkageUtil::ClearElementStyle (elmP, false, 1<<profileIndex, isCap ? MLSFLAG_CAP : MLSFLAG_LINE);
                return false;
                }

            LineStyleParams  newParams;

            memcpy (&newParams, lstyleArg.GetParams (), sizeof (newParams));
            newParams.mlineFlags = (isCap ? MLSFLAG_CAP : MLSFLAG_LINE);
            newParams.lineMask = 1<<profileIndex;
            newParams.modifiers |= (STYLEMOD_LINEMASK|STYLEMOD_MLINEFLAGS);

            DgnElementP  pTempElm = (DgnElementP) _alloca (elmP->Size () + sizeof (LineStyleParams));

            elmP->CopyTo (*pTempElm);
            LineStyleLinkageUtil::SetStyleParams (pTempElm, &newParams);

            eehP->ReplaceElement (pTempElm);
            changed = true;
            }
        }
    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineHandler::ProcessSymbologyProperties
(
PropertyContextR        proc,
ElementHandleCR            eh,
EditElementHandleP         eehP,
MlineSymbology*         pOutSymb,
const MlineSymbology&   inSymb,
bool                    isCap,
int                     profileIndex
)
    {
    DgnElementClass    newClass = (DgnElementClass) inSymb.conClass;
    bool                changed = false;

    if (proc.DoElementClassCallback (&newClass, EachElementClassArg (newClass, SETPROPCBEIFLAG(inSymb.useClass), proc)) && pOutSymb)
        {
        switch (newClass)
            {
            case DgnElementClass::Primary: pOutSymb->conClass = 0;  break;
            case DgnElementClass::PatternComponent: pOutSymb->conClass = 1;  break;
            }
        }

    changed |= proc.DoColorCallback  (pOutSymb ? &pOutSymb->color  : NULL, EachColorArg (inSymb.color,  SETPROPCBEIFLAG(inSymb.useColor), proc));
    changed |= proc.DoWeightCallback (pOutSymb ? &pOutSymb->weight : NULL, EachWeightArg (inSymb.weight, SETPROPCBEIFLAG(inSymb.useWeight), proc));

    changed |= ProcessLineStyleIDs (proc, eh, eehP, pOutSymb ? &pOutSymb->style : NULL, inSymb, isCap, profileIndex);
    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool  MultilineHandler::ProcessPropertiesHelper (PropertyContextR proc, ElementHandleCR eh, EditElementHandleP eehP)
    {
    DgnElementCP elmCP = eh.GetElementCP ();
    DgnElementP  elmP  = (NULL != eehP) ? eehP->GetElementP () : NULL;
    bool                changed = false;

    // Levels
    if (0 != elmCP->ToMlineElm().orgCap.level)
        changed |= proc.DoLevelCallback (elmP ? (LevelId*)&elmP->ToMlineElm().orgCap.level : NULL, EachLevelArg (LevelId(elmCP->ToMlineElm().orgCap.level), PROPSCALLBACK_FLAGS_NoFlagsSet, proc));

    if (0 != elmCP->ToMlineElm().endCap.level)
        changed |= proc.DoLevelCallback (elmP ? (LevelId*)&elmP->ToMlineElm().endCap.level : NULL, EachLevelArg (LevelId(elmCP->ToMlineElm().endCap.level), PROPSCALLBACK_FLAGS_NoFlagsSet, proc));

    if (0 != elmCP->ToMlineElm().midCap.level)
        changed |= proc.DoLevelCallback (elmP ? (LevelId*)&elmP->ToMlineElm().midCap.level : NULL, EachLevelArg (LevelId(elmCP->ToMlineElm().midCap.level), PROPSCALLBACK_FLAGS_NoFlagsSet, proc));

    for (int i=0; i < elmCP->ToMlineElm().nLines; i++)
        {
        if (0 != elmCP->ToMlineElm().profile[i].symb.level)
            changed |= proc.DoLevelCallback (elmP ? (LevelId*)&elmP->ToMlineElm().profile[i].symb.level : NULL, EachLevelArg (LevelId(elmCP->ToMlineElm().profile[i].symb.level), PROPSCALLBACK_FLAGS_NoFlagsSet, proc));
        }

    // Optimization when only processing level during add...
    if (ELEMENT_PROPERTY_Level == proc.GetElementPropertiesMask ())
        return changed;

    // Symbology - Class, Color, Weight, LineStyle, announce level for BY_LEVEL ids...
    proc.SetCurrentLevelID (0 != elmCP->ToMlineElm().orgCap.level ? LevelId(elmCP->ToMlineElm().orgCap.level) : elmCP->GetLevel());
    changed |= ProcessSymbologyProperties (proc, eh, eehP, elmP ? &elmP->ToMlineElmR().orgCap : NULL, elmCP->ToMlineElm().orgCap, true, MULTILINE_ORG_CAP);

    proc.SetCurrentLevelID (0 != elmCP->ToMlineElm().endCap.level ? LevelId(elmCP->ToMlineElm().endCap.level) : elmCP->GetLevel());
    changed |= ProcessSymbologyProperties (proc, eh, eehP, elmP ? &elmP->ToMlineElmR().endCap : NULL, elmCP->ToMlineElm().endCap, true, MULTILINE_END_CAP);

    proc.SetCurrentLevelID (0 != elmCP->ToMlineElm().midCap.level ? LevelId(elmCP->ToMlineElm().midCap.level) : elmCP->GetLevel());
    changed |= ProcessSymbologyProperties (proc, eh, eehP, elmP ? &elmP->ToMlineElmR().midCap : NULL, elmCP->ToMlineElm().midCap, true, MULTILINE_MID_CAP);

    for (int iLine=0; iLine < elmCP->ToMlineElm().nLines; iLine++)
        {
        proc.SetCurrentLevelID (0 != elmCP->ToMlineElm().profile[iLine].symb.level ? LevelId(elmCP->ToMlineElm().profile[iLine].symb.level) : elmCP->GetLevel());
        changed |= ProcessSymbologyProperties (proc, eh, eehP, elmP ? &elmP->ToMlineElmR().profile[iLine].symb : NULL, elmCP->ToMlineElm().profile[iLine].symb, false, iLine);
        }

    // Restore base level id as current...
    proc.SetCurrentLevelID (elmCP->GetLevel());

    // Style
    EachMLineStyleArg mlStyleArg (elmCP->ToMlineElm().styleParentId, PROPSCALLBACK_FLAGS_NoFlagsSet, proc);

    changed |= proc.DoMLineStyleCallback (elmP ? (ElementId*) &elmP->ToMlineElm().styleParentId : NULL, mlStyleArg);

    if (NULL == elmP || 0 == elmP->ToMlineElm().styleParentId)
        return changed;

    StyleParamsRemapping    paramsRemapping = mlStyleArg.GetRemappingAction ();

    if (paramsRemapping <= StyleParamsRemapping::NoChange)
        return changed;

    /*--------------------------------------------------------------------------
      The element potentially been assigned to a new style.  This change may
      have an impact on the parameters of elmP that are derived from the style.

         *** This step should be done after all ids are remapped ***
    --------------------------------------------------------------------------*/
    DgnModelP    modelRef = proc.GetDestinationDgnModel ();

    // Save off a copy so we can memcmp to test if there was any change
    size_t sizElm = eehP->GetElementCP()->Size (); 
    DgnElement* saveMlineElm = (DgnElement*)alloca(sizElm); 
    memcpy (saveMlineElm, eehP->GetElementCP(), sizElm);

    switch (paramsRemapping)
        {
        case StyleParamsRemapping::Override:
            {
            // If any of the element's properties don't match the style, set up overrides for them.
            if (UpdateShieldsByCompareWithStyle (*eehP))
                {
                elmP = eehP->GetElementP ();
                }
            break;
            }

        case StyleParamsRemapping::ApplyStyle:
            {
            ElementId styleId = MultilineHandler::GetStyleID (*eehP);
            if (styleId.IsValid())
                {
                MultilineStylePtr  mlineStyle = MultilineStyle::GetByID (styleId, modelRef->GetDgnProject());

                if (!mlineStyle.IsValid())
                    {BeAssert (0);  return changed;}

                double  styleScale = GetStyleScale (*eehP);

                // Apply the style
                ApplyStyle (*eehP, *mlineStyle, styleScale);
                }
            break;
            }
        }

    if (saveMlineElm->GetSizeWords() != eehP->GetElementCP()->GetSizeWords() ||
        0 != memcmp (saveMlineElm, eehP->GetElementCP(), eehP->GetElementCP()->Size ()))
        {
        changed = true;
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::ProcessProperties (PropertyContextR context, ElementHandleCR eh, EditElementHandleP eehP)
    {
    // Optimization when only processing level during add...
    if (ELEMENT_PROPERTY_Level == context.GetElementPropertiesMask ())
        return;

    if (NULL == eehP)
        {
        ProcessPropertiesHelper (context, eh, nullptr);

        return;
        }

    if (!ProcessPropertiesHelper (context, eh, eehP))
        return;

    context.SetElementChanged ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    HitPathCP   path = context.GetQueryPath ();

    if (path && QueryPropertyPurpose::Match == context.GetIQueryPropertiesP ()->_GetQueryPropertiesPurpose ()) // Report properties for hit detail...
        {
        int     lineNo = 0, capNo = 0;

        if (SUCCESS == path->GetMultilineParameters (NULL, NULL, NULL, &lineNo, &capNo, NULL))
            {
            DgnElementCP             elmCP = eh.GetElementCP ();
            int                     profileIndex = lineNo;
            LevelId                 partLevel = elmCP->GetLevel();
            MlineSymbology const*   symbP = NULL;

            switch (capNo)
                {
                case 0: // Not a cap
                    {
                    if (0 != elmCP->ToMlineElm().profile[lineNo].symb.level)
                        partLevel = LevelId(elmCP->ToMlineElm().profile[lineNo].symb.level);

                    symbP = &elmCP->ToMlineElm().profile[lineNo].symb;
                    break;
                    }

                case 1: // Origin cap
                    {
                    if (0 != elmCP->ToMlineElm().orgCap.level)
                        partLevel = LevelId(elmCP->ToMlineElm().orgCap.level);

                    symbP = &elmCP->ToMlineElm().orgCap;
                    profileIndex = MULTILINE_ORG_CAP;
                    break;
                    }

                case 2: // Mid cap
                    {
                    if (0 != elmCP->ToMlineElm().midCap.level)
                        partLevel = LevelId(elmCP->ToMlineElm().midCap.level);

                    symbP = &elmCP->ToMlineElm().midCap;
                    profileIndex = MULTILINE_MID_CAP;
                    break;
                    }

                case 3: // End cap
                    {
                    if (0 != elmCP->ToMlineElm().endCap.level)
                        partLevel = LevelId(elmCP->ToMlineElm().endCap.level);

                    symbP = &elmCP->ToMlineElm().endCap;
                    profileIndex = MULTILINE_END_CAP;
                    break;
                    }
                }

            // Announce level for BY_LEVEL ids...
            context.SetCurrentLevelID (partLevel);

            // NOTE: In this case announce component symbology as the base id!
            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                context.DoColorCallback (NULL, EachColorArg (symbP && symbP->useColor ? symbP->color : elmCP->GetSymbology().color, PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                context.DoWeightCallback (NULL, EachWeightArg (symbP && symbP->useWeight ? symbP->weight : elmCP->GetSymbology().weight, PROPSCALLBACK_FLAGS_IsBaseID, context));

           if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                {
                LineStyleParams lsParams;

                if (!symbP || !symbP->useStyle || SUCCESS != LineStyleLinkageUtil::ExtractStyleLinkageFromMultiLine (lsParams, elmCP, 0 != capNo, profileIndex))
                    LineStyleLinkageUtil::ExtractParams (&lsParams, elmCP);

                context.DoLineStyleCallback (NULL, EachLineStyleArg (symbP && symbP->useStyle ? symbP->style : elmCP->GetSymbology().style, &lsParams, PROPSCALLBACK_FLAGS_IsBaseID, context));
                }

            // Report properties common to all components...
            if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
                context.DoLevelCallback (NULL, EachLevelArg (partLevel, PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_DisplayPriority & context.GetElementPropertiesMask ()) && !elmCP->Is3d())
                context.DoDisplayPriorityCallback (NULL, EachDisplayPriorityArg (elmCP->GetDisplayPriority(), PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_ElementClass & context.GetElementPropertiesMask ()))
                context.DoElementClassCallback (NULL, EachElementClassArg ((DgnElementClass) (symbP && symbP->useClass ? symbP->conClass : elmCP->GetElementClassValue()), PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
                {
                Display_attribute   dispAttr;

                context.DoTransparencyCallback (NULL, EachTransparencyArg (mdlElement_displayAttributePresent (elmCP, TRANSPARENCY_ATTRIBUTE, &dispAttr) ? dispAttr.attr_data.transparency.transparency : 0, PROPSCALLBACK_FLAGS_IsBaseID, context));
                }

            if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
                {
                IMaterialPropertiesExtension*   mExtension;
                MaterialCP                      material;

                if (NULL != (mExtension = IMaterialPropertiesExtension::Cast (eh.GetHandler ())) && NULL != (material = mExtension->FindMaterialAttachment (eh)))
                    context.DoMaterialCallback (NULL, EachMaterialArg (material->GetId(), PROPSCALLBACK_FLAGS_IsBaseID, context));
                }

#ifdef WIP_VANCOUVER_MERGE // template
            if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
                context.DoElementTemplateCallback (NULL, EachElementTemplateArg (TemplateRefAttributes::GetReferencedTemplateIDFromHandle (eh), PROPSCALLBACK_FLAGS_IsBaseID, context));
#endif

            return;
            }
        }

    T_Super::_QueryProperties (eh, context);

    ProcessProperties (context, eh, NULL);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    ProcessProperties (context, eeh, &eeh);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::InsertPoint (DgnElementP elm, DPoint3dCP inPoint)
    {
    MlineElm*   mline = (MlineElm *) elm;
    int         pointNo = mline->nPoints;
    size_t      mlineSize = elm->Size ();

    if (mlineSize + sizeof (MlinePoint) > MAX_V8_ELEMENT_SIZE)
        return ERROR;

    MlinePoint* newMlinePoint = ((MlinePoint*) (mline->profile + mline->nLines)) + pointNo;

    // Open space for the new point
    char* src = (char*)newMlinePoint;
    char* dst = src + sizeof(MlinePoint);
    char* end = (char*)mline + mlineSize;
    memmove (dst, src, end - src);

    // Clear opened space and add new point
    memset (newMlinePoint, 0, sizeof (MlinePoint));

    mline->IncrementSizeWords(sizeof (MlinePoint) / 2);

    newMlinePoint->flags.assoc = false;
    newMlinePoint->point = *inPoint;

    mline->nPoints++;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::CreateMultilineElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DVec3dCR            normal,
DPoint3dCP          points,
int                 numVerts,
bool                is3d,
DgnModelR        modelRef
)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;
    int         elmSize;

    if (in && MULTILINE_ELM == in->GetLegacyType())
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, MULTILINE_ELM, in->GetLevel(), false, (ElementUtil::ElemDim) is3d);

        if (in->ToMlineElm().nLines > 1)
            memcpy (out.ToMlineElmR().profile+1, in->ToMlineElm().profile+1, (in->ToMlineElm().nLines-1) * sizeof (MlineProfile));

        out.ToMlineElmR().nPoints = out.ToMlineElmR().nBreaks = 0;

        elmSize = sizeof (MlineElm) + ((out.ToMlineElm().nLines-1) * sizeof (MlineProfile));
        }
    else
        {
        memset (&out, 0, sizeof (MlineElm));

        if (in)
            memcpy (&out, in, sizeof (DgnElement));

        ElementUtil::SetRequiredFields (out, MULTILINE_ELM, in ? in->GetLevel(): LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        out.ToMlineElmR().version = MLINE_VERSION;

        elmSize = sizeof (MlineElm) - sizeof (MlineProfile);
        }

    out.ToMlineElmR().zVector = normal;

    out.SetSizeWordsNoAttributes(elmSize/2);

    for (int i=0; i < numVerts; i++)
        InsertPoint (&out, points+i);

    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::CreateMultilineElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
MultilineStyleCR    mlineStyle,
double              styleScale,
DVec3dCR            normal,
DPoint3dCP          points,
int                 numVerts,
bool                is3d,
DgnModelR        modelRef
)
    {
    BentleyStatus status;
    if (SUCCESS != (status = CreateMultilineElement (eeh, templateEh, normal, points, numVerts, is3d, modelRef)))
        return status;

    MultilineHandler*   mlHandler = dynamic_cast <MultilineHandler*> (&eeh.GetHandler ());
    return mlHandler->ApplyStyle (eeh, mlineStyle, styleScale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32             MultilineHandler::_GetProfileCount (ElementHandleCR source) const
    {
    DgnElementCR el = *source.GetElementCP ();

    if (MULTILINE_ELM != el.GetLegacyType())
        return 0;

    return el.ToMlineElm().nLines;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MultilinePointPtr       MultilineHandler::GetMlinePointStatic (ElementHandleCR source, UInt32 pointNum)
    {
    if (!IsMultilineElement (source))
        return NULL;

    if (pointNum > GetPointCountStatic (source))
        return NULL;

    MlinePoint* mlPointStart = (MlinePoint*)(source.GetElementCP()->ToMlineElm().profile + source.GetElementCP()->ToMlineElm().nLines);
    return MultilinePoint::CreateFromPoint (mlPointStart[pointNum]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     6/90
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineHandler::SetClosePoint (EditElementHandleR eeh)
    {
    // This has to be low-level because it's called from ReplacePoint.
    UInt32 lastPointNum = GetPointCount(eeh)-1;
    MlinePoint* mlPointStart = (MlinePoint*)(eeh.GetElementCP()->ToMlineElm().profile + eeh.GetElementCP()->ToMlineElm().nLines);

    mlPointStart[lastPointNum].point = mlPointStart[0].point;
    mlPointStart[lastPointNum].flags = mlPointStart[0].flags;
    }

/*---------------------------------------------------------------------------------**//**
* Follows the old behavior of mdlMline_modifyPoint that you can't change an associative
*   point, and you have to specifically clear it if it is associative.
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineHandler::_ReplacePoint (EditElementHandleR source, DPoint3dCR newPoint, UInt32 pointNum, MlineModifyPoint options)
    {
    if (!IsMultilineElement (source))
        return ERROR;

    if (pointNum >= GetPointCount (source))
        return ERROR;

    // Modify point 0 if last point of closed ToMlineElm()...
    if (IsClosed (source) && pointNum == GetPointCount(source)-1)
        pointNum = 0;

    MlinePoint* mlPointStart = (MlinePoint*)(source.GetElementP()->ToMlineElm().profile + source.GetElementP()->ToMlineElm().nLines);
    MlinePoint newMlPoint = mlPointStart[pointNum];
    newMlPoint.point = newPoint;

    // Store these to shift breaks
    DPoint3d segment[2];

    if (pointNum < GetPointCount(source)-1)
        ExtractPointArray (source, segment, pointNum, 2);

    // If it's associative, you must explicitly clear it or it's an error.
    if (newMlPoint.flags.assoc)
        {
        if (MlineModifyPoint::None != (options & MlineModifyPoint::RemoveAssociations))
            {
            newMlPoint.flags.assoc = false;
            mlPointStart[pointNum] = newMlPoint;

            AssociativePoint::RemovePoint (source, pointNum, GetPointCount(source));
            }
        else
            {
            return ERROR;
            }
        }
    else
        {
        mlPointStart[pointNum] = newMlPoint;
        }

    /*-------------------------------------------------------------------
    If bit one of the option flag is set and this is not the last point,
    adjust the offset of each break in the segment so that the absolute
    location of the break remains the same.
    -------------------------------------------------------------------*/
    if ( (MlineModifyPoint::None != (options & MlineModifyPoint::ShiftBreaks)) && pointNum+1 <  GetPointCount(source) && newMlPoint.nBreaks > 0)
        {
        double oldLength = segment[0].distance (segment+1);
        double newLength = newPoint.distance (segment+1);
        double offset    = newLength - oldLength;

        // TODO Need break API
        MlineBreak* mBreaks = GetFirstBreak (source) + mlPointStart[pointNum].breakNo;
        for (UInt32 iBreak=0; iBreak<mlPointStart[pointNum].nBreaks; iBreak++, mBreaks++)
            if (!(mBreaks->flags & MLBREAK_FROM_JOINT))
                mBreaks->offset += offset;
        }

    if (0 == pointNum && IsClosed (source))
        SetClosePoint (source);

    ClipBadBreaks (source, pointNum);
    ClipBadBreaks (source, pointNum+1);
    ClipBadBreaks (source, pointNum-1);
    ClipBadBreaks (source, pointNum-2);

    if (0 == pointNum && IsClosed (source))
        ClipBadBreaks (source, GetPointCount(source)-2);

    return source.GetDisplayHandler ()->ValidateElementRange(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineHandler::ReplaceMultilinePoint (EditElementHandleR source, MultilinePointCR newPoint, UInt32 pointNum)
    {
    if (!IsMultilineElement (source))
        return ERROR;

    if (pointNum > GetPointCount (source))
        return ERROR;

    // Modify point 0 if last point of closed ToMlineElm()...
    if (IsClosed (source) && pointNum == GetPointCount(source)-1)
        pointNum = 0;

    MlinePoint newMlPoint;
    newPoint.ToElementData (newMlPoint);

    MlinePoint* mlPointStart = (MlinePoint*)(source.GetElementP()->ToMlineElm().profile + source.GetElementP()->ToMlineElm().nLines);
    mlPointStart[pointNum] = newMlPoint;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineHandler::_InsertPoint (EditElementHandleR source, DPoint3dCR newDPoint, AssocPoint const * assocPointP, UInt32 pointNum)
    {
    if (!IsMultilineElement (source))
        return ERROR;

    size_t mlineSize = source.GetElementCP()->Size ();

    if (mlineSize + sizeof(MlinePoint) > MAX_V8_ELEMENT_SIZE)
        return ERROR;

    if (pointNum == (UInt32)-1)  // -1 means "at end"
        pointNum = GetPointCount (source);

    MultilinePointPtr newPoint = MultilinePoint::CreateFromPoint (newDPoint);

    UInt32 preAddNumPoints = GetPointCount(source);

    {  // Bracket off fullElm so it can't be used later
        DgnV8ElementBlank   fullElm;
        source.GetElementCP ()->CopyTo (fullElm);

        MlinePoint*  pointStart = (MlinePoint*)(fullElm.ToMlineElm().profile + fullElm.ToMlineElm().nLines);
        MlinePoint*  newMlinePoint = pointStart + pointNum;

        // Open space for the new point
        char* src = (char*)newMlinePoint;
        char* dst = src + sizeof(MlinePoint);
        char* end = (char*)&fullElm + mlineSize;
        memmove (dst, src, end - src);

        // Clear opened space and add new point
        memset (newMlinePoint, 0, sizeof(MlinePoint));

        fullElm.ToMlineElmR().IncrementSizeWords(sizeof(MlinePoint) / 2);

        fullElm.ToMlineElmR().nPoints++;
        source.ReplaceElement (&fullElm);
    }

    newPoint->SetAssociative (false);

    // shift assoc point dependency indices to account for insert that isn't at end
    AssociativePoint::VertexAddedOrRemoved (source, pointNum, preAddNumPoints, true);

    if (NULL != assocPointP)
        {
        if (SUCCESS == AssociativePoint::InsertPoint (source, *assocPointP, pointNum,  GetPointCount(source)))
            {
            DPoint3d outPoint;
            if (SUCCESS == AssociativePoint::GetPoint (&outPoint, *assocPointP, source.GetDgnModelP()))
                {
                newPoint->SetPoint (outPoint);
                newPoint->SetAssociative (true);
                }
            else
                {
                AssociativePoint::RemovePoint (source, pointNum, GetPointCount(source));
                }
            }
        }

    // Put in the data
    ReplaceMultilinePoint (source, *newPoint, pointNum);

    return source.GetDisplayHandler ()->ValidateElementRange(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void     deletePointBreaks
(
MlineElm*       mline,
int             pointNo
)
    {
    MlinePoint  *mPoints = ((MlinePoint*)(mline->profile + mline->nLines));
    int         i,j,k, remove, breakNo, nBreaks;

    breakNo = mPoints[pointNo].breakNo;
    nBreaks = mPoints[pointNo].nBreaks;

    for (i=breakNo+(nBreaks-1), k=nBreaks-1; i>=breakNo; i--, k--)
        {
        remove = true;

        for (j=0; j<mline->nPoints; j++)
            {
            if (j != pointNo && i >= mPoints[j].breakNo && i < mPoints[j].breakNo + mPoints[j].nBreaks)
                {
                remove = false;
                break;
                }
            }

        if (remove)
            deleteBreak (mline, pointNo, k);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::_DeletePoint (EditElementHandleR source, UInt32 pointNo)
    {
    if (!IsMultilineElement (source))
        return ERROR;

    if (GetPointCount(source) == 0 || pointNo > GetPointCount (source)-1)
        return ERROR;

    DgnV8ElementBlank   fullElm;

    source.GetElementCP ()->CopyTo (fullElm);

    MlinePoint* mPoint = (MlinePoint*) (fullElm.ToMlineElm().profile + fullElm.ToMlineElm().nLines) + pointNo;

    if (mPoint->nBreaks)
        deletePointBreaks (&fullElm.ToMlineElmR(), pointNo);

    if (pointNo == fullElm.ToMlineElm().nPoints - 1)
        deletePointBreaks (&fullElm.ToMlineElmR(), pointNo - 1);

    if (mPoint->flags.assoc)
        {
        source.ReplaceElement (&fullElm);

        if (SUCCESS == AssociativePoint::RemovePoint (source, pointNo, GetPointCount (source)))
            mPoint[pointNo].flags.assoc = false;

        source.GetElementCP ()->CopyTo (fullElm);
        }

    // shift assoc point dependency indices to account for delete that isn't at end
    source.ReplaceElement (&fullElm);
    AssociativePoint::VertexAddedOrRemoved (source, pointNo, fullElm.ToMlineElm().nPoints, false);
    source.GetElementCP ()->CopyTo (fullElm);

    char        *src, *dst, *end;

    end = (char*) &fullElm + fullElm.Size ();
    src = (char*) (mPoint+1);
    dst = (char*) (mPoint);
    memmove (dst, src, end - src);

    fullElm.ToMlineElmR().nPoints--;
    fullElm.ToMlineElmR().DecrementSizeWords(sizeof (MlinePoint) / 2);

    if (fullElm.ToMlineElm().flags.closed && fullElm.ToMlineElm().nPoints < 4)
        fullElm.ToMlineElmR().flags.closed = false;

    source.ReplaceElement (&fullElm);

    return source.GetDisplayHandler ()->ValidateElementRange(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MultilinePointPtr       MultilineHandler::_GetPoint (ElementHandleCR source, UInt32 pointNum) const
    {
    return GetMlinePointStatic (source, pointNum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
JointDef        MultilineHandler::_ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const
    {
    JointDef    jointDef;

    MultilineHandler::GetJointDef (jointDef, source, pts, pointNo);

    return jointDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
JointDef        MultilineHandler::_ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const
    {
    JointDef    jointDef;

    MultilineHandler::GetCapJointDef (jointDef, source, pts, pointNo);

    return jointDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MultilineHandler::_ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const
    {
    if (!IsMultilineElement (source))
        return ERROR;

    numPoints = MultilineHandler::GetPointCount (source);

    if (NULL != pXYZBuffer)
        {
        size_t  numGet = numPoints;

        if (numGet > maxOut)
            numGet = maxOut;

        if (numGet > 0)
            MultilineHandler::ExtractPointArray (source, pXYZBuffer, 0, numGet);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr MultilineHandler::WorklineToCurveVector (ElementHandleCR eh)
    {
    IMultilineQuery* mlineQuery;

    if (NULL == (mlineQuery = dynamic_cast <IMultilineQuery*> (&eh.GetHandler ())))
        return NULL;

    size_t            numPoints = mlineQuery->GetPointCount (eh);
    bvector<DPoint3d> points;

    points.resize (numPoints);
    mlineQuery->ExtractPoints (eh, &points[0], numPoints, numPoints);

    CurveVectorPtr  curves = CurveVector::Create (mlineQuery->IsClosed (eh) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);

    curves->push_back (ICurvePrimitive::CreateLineString (&points[0], points.size ()));

    return curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr MultilineHandler::BoundaryToCurveVector (ElementHandleCR thisElm, int minLine, int maxLine)
    {
    if (!MultilineHandler::IsMultilineElement (thisElm))
        return NULL;

    DgnElementCR     elm     = *thisElm.GetElementCP ();
    int             nLines  = elm.ToMlineElm().nLines;
    int             nPoints = elm.ToMlineElm().nPoints;
    DPoint3d        workLine[MLINE_MAXPOINTS];

    MultilineHandler::ExtractPointArray (thisElm, workLine, 0, nPoints);

    if (elm.ToMlineElm().flags.closed)
        workLine[0] = workLine[nPoints-1];

    int         i;
    JointDef    joints[MLINE_MAXPOINTS];

    MultilineHandler::GetCapJointDef (*joints, thisElm, workLine, 0);

    for (i=1; i<nPoints-1; i++)
        MultilineHandler::GetJointDef (*(joints+i), thisElm, workLine, i-1);

    MultilineHandler::GetCapJointDef (*(joints+(nPoints-1)), thisElm, workLine+(nPoints-2), 1);

    int         min, max;
    double      minDist, maxDist;

    MultilineHandler::GetLimitProfiles (min, max, thisElm);

    minDist = elm.ToMlineElm().profile[min].dist;
    maxDist = elm.ToMlineElm().profile[max].dist;

    if (minLine >= 0 && minLine < nLines && minLine != maxLine)
        minDist = elm.ToMlineElm().profile[minLine].dist;

    if (maxLine >= 0 && maxLine < nLines && maxLine != minLine)
        maxDist = elm.ToMlineElm().profile[maxLine].dist;

    DPoint3d    lineBuf1[MAX_VERTICES];

    // Process the minimum distance line
    for (i=0; i<nPoints; i++)
        bsiDPoint3d_addScaledDVec3d (lineBuf1+i, workLine+i, (DVec3dP) &joints[i].dir, (double) minDist * joints[i].scale);

    DPoint3d    lineBuf2[MLINE_MAXPOINTS];

    for (i=0; i<nPoints; i++)
        bsiDPoint3d_addScaledDVec3d (lineBuf2+nPoints-i-1, workLine+i, (DVec3dP) &joints[i].dir, (double) maxDist * joints[i].scale);

    if (elm.ToMlineElm().flags.closed)
        {
        // Create parity region instead of a shape with overlapping edge (called by mdlSolid_elementToBody)...
        CurveVectorPtr  curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
        CurveVectorPtr  childLoopMax = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        CurveVectorPtr  childLoopMin = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);

        childLoopMax->push_back (ICurvePrimitive::CreateLineString (lineBuf2, nPoints));
        childLoopMin->push_back (ICurvePrimitive::CreateLineString (lineBuf1, nPoints));

        curves->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childLoopMax.get ()));
        curves->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childLoopMin.get ()));

        return curves;
        }

    memcpy (&lineBuf1[nPoints], lineBuf2, nPoints*sizeof(DPoint3d));
    lineBuf1[nPoints*2] = lineBuf1[0];

    CurveVectorPtr  curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

    curves->push_back (ICurvePrimitive::CreateLineString (lineBuf1, nPoints*2+1));

    return curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool     MultilineHandler::IsMultilineElement (ElementHandleCR pCandidate)
    {
    return (MULTILINE_ELM == pCandidate.GetLegacyType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         MultilineHandler::_OnGeoCoordinateReprojection
(
EditElementHandleR                  source,
IGeoCoordinateReprojectionHelper&   reprojectionHelper,
bool                                inChain
)
    {
    DgnElementP  el              = source.GetElementP();
    int         numPoints       = GetPointCount (source);
    DPoint3dP   pointBuffer     = (DPoint3dP)_alloca (numPoints * sizeof(DPoint3d));
    MlinePoint* sourcePoints    = const_cast <MlinePoint*> (GetFirstPoint (source));

    // copy all points into our temporary buffer.
    DPoint3dP       outPointP;
    MlinePoint*     inMlinePointP;
    int             iPoint;
    for (iPoint=0, outPointP = pointBuffer, inMlinePointP = sourcePoints; iPoint < numPoints; iPoint++, inMlinePointP++, outPointP++)
        *outPointP = inMlinePointP->point;

    // don't do anything fancy, since there's breaks and all that kind of stuff.
    ReprojectStatus                     status   = reprojectionHelper.ReprojectPoints (pointBuffer, NULL, NULL, pointBuffer, numPoints);
    BeAssert (REPROJECT_Success == status);

    // get the approximate scale factor by dividing the transformed length of the first segment by its original length. That's the
    // factor that we're going to use to scale all of the profile spacings.
    if (numPoints > 1)
        {
        double  scale = pointBuffer[1].Distance (pointBuffer[0]) / (sourcePoints[1].point).Distance (*( &sourcePoints[0].point));
        int             iProfile;
        MlineProfile*   profileP;
        for (iProfile=0, profileP = &el->ToMlineElmR().profile[0]; iProfile < el->ToMlineElm().nLines; iProfile++, profileP++)
            profileP->dist = scale * profileP->dist;

        // fix up the breaks
        int             iBreak;
        MlineBreak*     breakP;
        for (iBreak=0, breakP = const_cast <MlineBreak*> (GetFirstBreak (source)); iBreak < el->ToMlineElm().nBreaks; iBreak++, breakP++)
            {
            breakP->length = scale * breakP->length;
            breakP->offset = scale * breakP->offset;
            }
        }

    // put the points back in.
    DPoint3dP       inPointP;
    MlinePoint*     outMlinePointP;
    for (iPoint=0, inPointP = pointBuffer, outMlinePointP = sourcePoints; iPoint < numPoints; iPoint++, inPointP++, outMlinePointP++)
        outMlinePointP->point = *inPointP;

    return  REPROJECT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* Provide the effective values of multiline symbology properties which may be inherited
* from the base element properties.
* @bsimethod                                    Paul.Connelly                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineHandler::GetEffectiveMlineSymbologyProperties (LevelId* level, UInt32* color, UInt32* weight, Int32* style, ElementHandleCR element, UInt32 index, bool isCap) const
    {
    MlineSymbology const*   mlSymb = NULL;
    if (isCap)
        mlSymb = _GetMlineCapSymbologyCP (element, static_cast <MultilineCapType> (index));
    else
        {
        MlineProfile const*     profile = _GetMlineProfileDefCP (element, index);
        if (NULL != profile)
            mlSymb = &profile->symb;
        }

    if (NULL == mlSymb)
        return false;

    DgnElementCP             elemCP = element.GetElementCP ();
    Symbology const*        elemSymb = &elemCP->GetSymbology();

    if (NULL != level)
        *level = (0 != mlSymb->level)    ? LevelId(mlSymb->level) : elemCP->GetLevel();
    if (NULL != color)
        *color = mlSymb->useColor        ? mlSymb->color     : elemSymb->color;
    if (NULL != weight)
        *weight = mlSymb->useWeight      ? mlSymb->weight    : elemSymb->weight;
    if (NULL != style)
        *style = mlSymb->useStyle        ? mlSymb->style     : elemSymb->style;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId MultilineHandler::_GetEffectiveSymbologyLevel (ElementHandleCR source, UInt32 index, bool isCap) const
    {
    LevelId level;
    bool result = GetEffectiveMlineSymbologyProperties (&level, NULL, NULL, NULL, source, index, isCap);
    BeAssert (result);
    return level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  MultilineHandler::_GetEffectiveSymbologyColor (ElementHandleCR source, UInt32 index, bool isCap) const
    {
    UInt32 color;
    bool result = GetEffectiveMlineSymbologyProperties (NULL, &color, NULL, NULL, source, index, isCap);
    BeAssert (result);
    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  MultilineHandler::_GetEffectiveSymbologyWeight (ElementHandleCR source, UInt32 index, bool isCap) const
    {
    UInt32 weight;
    bool result = GetEffectiveMlineSymbologyProperties (NULL, NULL, &weight, NULL, source, index, isCap);
    BeAssert (result);
    return weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
Int32   MultilineHandler::_GetEffectiveSymbologyStyle (ElementHandleCR source, UInt32 index, bool isCap) const
    {
    Int32 style;
    bool result = GetEffectiveMlineSymbologyProperties (NULL, NULL, NULL, &style, source, index, isCap);
    BeAssert (result);
    return style;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilinePointPtr MultilinePoint::CreateFromPoint (MlinePoint const & mlPoint)
    {
    MultilinePoint* newMlp = new MultilinePoint ();

    newMlp->FromElementData (mlPoint);
    return newMlp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilinePointPtr MultilinePoint::CreateFromPoint (DPoint3dCR point)
    {
    MultilinePoint* newMlp = new MultilinePoint ();

    newMlp->Init ();
    newMlp->m_point = point;
    return newMlp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilinePoint::Init ()
    {
    m_point.zero();
    m_associative = false;

    m_breakNo = 0;
    m_numBreaks = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilinePoint::FromElementData (MlinePoint const & mlPoint)
    {
    Init ();
    m_point = mlPoint.point;
    m_associative = mlPoint.flags.assoc;
    m_breakNo = mlPoint.breakNo;
    m_numBreaks = mlPoint.nBreaks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilinePoint::FromElementData (ElementHandleCR eh, UInt32 pointNum)
    {
    if (!MultilineHandler::IsMultilineElement (eh))
        return ERROR;

    if (pointNum >eh.GetElementCP()->ToMlineElm().nPoints)
        return ERROR;

    MlinePoint* mlPointStart = (MlinePoint*)(eh.GetElementCP()->ToMlineElm().profile + eh.GetElementCP()->ToMlineElm().nLines);
    FromElementData (mlPointStart[pointNum]);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilinePoint::ToElementData (MlinePoint& mlPoint) const
    {
    memset (&mlPoint, 0, sizeof(mlPoint));
    mlPoint.point = m_point;
    mlPoint.flags.assoc = m_associative;
    mlPoint.breakNo = m_breakNo;
    mlPoint.nBreaks = m_numBreaks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MultilinePoint::GetPoint () const
    {
    return m_point;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilinePoint::SetPoint (DPoint3dCR point)
    {
    m_point = point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilinePoint::IsAssociative () const
    {
    return m_associative;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilinePoint::SetAssociative (bool value)
    {
    m_associative = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 MultilinePoint::GetNumBreaks () const
    {
    return m_numBreaks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 MultilinePoint::GetBreakNumber () const
    {
    return m_breakNo;
    }
