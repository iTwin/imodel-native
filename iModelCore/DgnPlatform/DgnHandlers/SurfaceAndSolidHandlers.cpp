/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/SurfaceAndSolidHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define TOLERANCE_ExtrusionSkew 5.0 // Maximum deviation from parallel (in UORs) <- TOO BIG!!!

/*----------------------------------------------------------------------+
|                                                                       |
|   Surface/Solid Types                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
#define SURFTYPE_PROJECTION             0
#define SURFTYPE_REVOLUTION             8
#define SOLIDTYPE_REVOLUTION            1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool SurfaceOrSolidHandler::IsSurfaceOfRevolution (ElementHandleCR thisElm, bool* isCapped)
    {
    DgnElementCP el = thisElm.GetElementCP();

    if (SOLID_ELM != el->GetLegacyType() && SURFACE_ELM != el->GetLegacyType())
        return false;

    if (isCapped)
        *isCapped = (SOLID_ELM == el->GetLegacyType());

    return (SURFTYPE_REVOLUTION == el->ToSurface().surftype || SOLIDTYPE_REVOLUTION == el->ToSurface().surftype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool SurfaceOrSolidHandler::IsSurfaceOfProjection (ElementHandleCR thisElm, bool* isCapped)
    {
    DgnElementCP el = thisElm.GetElementCP();

    if (SOLID_ELM != el->GetLegacyType() && SURFACE_ELM != el->GetLegacyType())
        return false;
    
    if (isCapped)
        *isCapped = (SOLID_ELM == el->GetLegacyType());

    return (SURFTYPE_PROJECTION == el->ToSurface().surftype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool isPossibleSurfaceOfRevolution (ElementHandleCR eh)
    {
    if (SOLID_ELM != eh.GetLegacyType() && SURFACE_ELM != eh.GetLegacyType())
        return false;

    // Any surftype other than SURFTYPE_PROJECTION is fair game...try to find a rule arc...
    return (SURFTYPE_PROJECTION != eh.GetElementCP ()->ToSurface().surftype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int      countBoundaries (ElementHandleCR eh)
    {
    if (BSPLINE_CURVE_ELM == eh.GetLegacyType())
        {
        int     count = 1;

        for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
            count++;

        return count;
        }

    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isRuleClassElement (DgnElementCP el)
    {
    if (!el->IsGraphic())
        return false;

    return (DgnElementClass::PrimaryRule == static_cast<DgnElementClass>(el->GetElementClass()) || DgnElementClass::ConstructionRule == static_cast<DgnElementClass>(el->GetElementClass()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt testRuleArc
(
ElementHandleCR thisElm,
Arc_3d          *ruleP,
Arc_3d          *intmRuleP,
bool            *haveRuleP,
bool            *haveIntmRuleP,
double          *totalSweepP,
int             *needAxisOfRevP,
DPoint3d        *axisRevP,
double          *distAP,
DPoint3d        *pt0P,
DPoint3d        *pt1P
)
    {
    DgnElementCP  elP = thisElm.GetElementCP();

    if (!isRuleClassElement (elP))
        {
        // found primary elements
        if (*haveIntmRuleP)
            {
            *totalSweepP += intmRuleP->sweepAngle;

            if (!*haveRuleP)
                {
                *ruleP = *intmRuleP;
                *haveRuleP = true;
                }

            *haveIntmRuleP = false;
            }

        *needAxisOfRevP = (*needAxisOfRevP <= 1) ? 0 : 3;
        }
    else
        {
        // found rule element...must be arc
        if (ARC_ELM != elP->GetLegacyType())
            return ERROR;

        if (*needAxisOfRevP)
            {
            double      dist0, dist1;

            if (3 == *needAxisOfRevP)
                {
                *pt0P = *pt1P = elP->ToArc_3d().origin;
                *distAP = 0.0;

                axisRevP->x = axisRevP->y = axisRevP->z = 0.0;

                --(*needAxisOfRevP);
                }
            else if (*needAxisOfRevP < 3)
                {
                dist0 = elP->ToArc_3d().origin.distance (pt0P);
                dist1 = elP->ToArc_3d().origin.distance (pt1P);
                DEllipse3d ellipse;
                ellipse.initFromDGNFields3d (
                                &elP->ToArc_3d().origin,
                                const_cast <double *> (elP->ToArc_3d().quat),
                                NULL, NULL,
                                elP->ToArc_3d().primary,
                                elP->ToArc_3d().secondary,
                                &elP->ToArc_3d().startAngle, &elP->ToArc_3d().sweepAngle
                                );
                if (dist0 > dist1)
                    {
                    if (dist0 > *distAP)
                        {
                        *pt1P = elP->ToArc_3d().origin;
                        *distAP = dist0;

                        *needAxisOfRevP = 1;
                        }
                    }
                else if (dist1 > *distAP)
                    {
                    *pt0P = elP->ToArc_3d().origin;
                    *distAP = dist1;

                    *needAxisOfRevP = 1;
                    }
                }
            }

        if (elP->ToArc_3d().primary && elP->ToArc_3d().secondary)
            {
            if (!*haveIntmRuleP ||
                ((elP->ToArc_3d().primary + elP->ToArc_3d().secondary) >
                 (intmRuleP->primary + intmRuleP->secondary)))
                {
                memcpy (intmRuleP, elP, sizeof (*intmRuleP));
                *haveIntmRuleP = true;
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt extractRuleArcData
(
ChildElemIter*  thisElm,        /* => list of elems */
double          *r0,            /* <= primary radius for rule arc */
double          *r1,            /* <= secondary radius for rule arc */
DPoint3d        *rCenter,       /* <= rule arc center */
DVec3d          *rm0,           /* <= primary axis for rule arc */
DVec3d          *rm1,           /* <= secondary axis for rule arc */
DVec3d          *axisRev,       /* <= rotation axis for surface */
double          *rStart,        /* <= start angle for rule arc */
double          *rSweep         /* <= sweep angle for rule arc */
)
    {
    int         needAxisOfRev = 4;
    double      totalSweep = 0.0, distA;
    DPoint3d    pt0, pt1;
    bool        haveRule = false, haveIntmRule = false;
    Arc_3d      rule, intmRule;
    
    memset (&intmRule, 0, sizeof intmRule); // Memsets just to quiet compiler warning about uninitialized
    memset (&rule, 0, sizeof intmRule);

    /* NOTE: Explaination of values for needAxisOfRev...

       4 => need primary element
       3 => need 1st rule origin
       2 => need 2nd rule
       1 => just looking for better
    */

    for (ChildElemIter curr (thisElm->ToNext()); curr.IsValid(); curr = curr.ToNext())
        {
        // In case we need to use alloca to get element do work in another function...
        if (SUCCESS != testRuleArc (curr, &rule, &intmRule, &haveRule, &haveIntmRule, &totalSweep, &needAxisOfRev, axisRev, &distA, &pt0, &pt1))
            return ERROR;
        }

    if (haveIntmRule)
        {
        totalSweep += intmRule.sweepAngle;

        if (!haveRule)
            rule = intmRule;
        }
    else if (!haveRule)
        {
        // found no suitable rule elements
        return ERROR;
        }

    if (rSweep)
        *rSweep = totalSweep;

    if (rStart)
        *rStart = rule.startAngle;

    if (rCenter)
        *rCenter = rule.origin;

    if (r0)
        *r0 = rule.primary;

    if (r1)
        *r1 = rule.secondary;

    RotMatrix       mtx;

    mtx.InitTransposedFromQuaternionWXYZ ( rule.quat);

    if (rm0)
        mtx.getColumn (rm0, 0);

    if (rm1)
        mtx.getColumn (rm1, 1);

    if (axisRev)
        {
        if (needAxisOfRev > 1)
            mtx.getColumn (axisRev, 2);
        else
            axisRev->normalizedDifference (&pt1, &pt0);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool searchForRuleArc (ElementHandleCR eh, DEllipse3dR ruleArc)
    {
    if (!isPossibleSurfaceOfRevolution (eh))
        return false;

    // NOTE: Any surftype other than SURFTYPE_PROJECTION is fair game...try to find a rule arc...
    ChildElemIter  tmpIter (eh, ExposeChildrenReason::Count);

    if (!tmpIter.IsValid ())
        return false;

    double      sweep;
    DVec3d      rPrimary, rSecondary, rAxis;
    DPoint3d    center;

    if (SUCCESS != extractRuleArcData (&tmpIter, NULL, NULL, &center, &rPrimary, &rSecondary, &rAxis, NULL, &sweep))
        return false;

    RotMatrix   rMatrix;

    rMatrix.InitFromColumnVectors (rPrimary, rSecondary, rAxis);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 0, 1); // Remove skew...

    DVec3d      vector0, vector90;

    rMatrix.GetColumn (vector0, 0);
    rMatrix.GetColumn (vector90, 1);

    ruleArc.InitFromVectors (center, vector0, vector90, 0.0, sweep);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::IsBlock (ElementHandleCR eh, double& length, double& width, double& height, DPoint3dR center, RotMatrixR rMatrix)
    {
    if (!IsSurfaceOfProjection (eh))
        return false;
    
    ISolidPrimitivePtr  primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (eh);
    DgnBoxDetail        detail;

    if (!primitive.IsValid () || !primitive->TryGetDgnBoxDetail (detail))
        return false;

    if (!DoubleOps::AlmostEqual (detail.m_baseX, detail.m_topX) || !DoubleOps::AlmostEqual (detail.m_baseY, detail.m_topY))
        return false; // Reject scale...

    DVec3d  vectorZ, normal;

    height = vectorZ.NormalizedDifference (detail.m_topOrigin, detail.m_baseOrigin);
    normal.NormalizedCrossProduct (detail.m_vectorX, detail.m_vectorY);

    if (fabs (fabs (normal.DotProduct (vectorZ)) - 1.0) > mgds_fc_epsilon)
        return false; // Reject skew...

    length = detail.m_baseX;
    width  = detail.m_baseY;

    rMatrix.InitFromColumnVectors (detail.m_vectorX, detail.m_vectorY, vectorZ);
    center = DPoint3d::FromProduct (detail.m_baseOrigin, rMatrix, detail.m_baseX * 0.5, detail.m_baseY * 0.5, height * 0.5);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::IsCone (ElementHandleCR eh, double& baseRadius, double& topRadius, DPoint3dR basePt, DPoint3dR topPt, RotMatrixR rMatrix)
    {
    if (!IsSurfaceOfProjection (eh))
        return false;
    
    ISolidPrimitivePtr  primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (eh);
    DgnConeDetail       detail;

    if (!primitive.IsValid () || !primitive->TryGetDgnConeDetail (detail))
        return false;

    bool  capped;

    return detail.IsCircular (basePt, topPt, rMatrix, baseRadius, topRadius, capped);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::IsSphere (ElementHandleCR eh, double& radius, DPoint3dR center, RotMatrixR rMatrix)
    {
    if (!IsSurfaceOfRevolution (eh))
        return false;

    ISolidPrimitivePtr  primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (eh);
    DgnSphereDetail     detail;

    if (!primitive.IsValid () || !primitive->TryGetDgnSphereDetail (detail))
        return false;

    return detail.IsTrueSphere (center, rMatrix, radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::IsTorus (ElementHandleCR eh, double& eRadius, double& tRadius, double& sweep, DPoint3dR center, RotMatrixR rMatrix)
    {
    if (!IsSurfaceOfRevolution (eh))
        return false;

    ISolidPrimitivePtr  primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (eh);
    DgnTorusPipeDetail  detail;

    if (!primitive.IsValid () || !primitive->TryGetDgnTorusPipeDetail (detail))
        return false;
    
    eRadius = detail.m_minorRadius;
    tRadius = detail.m_majorRadius;
    sweep   = detail.m_sweepAngle;
    center  = detail.m_center;

    rMatrix.InitFrom2Vectors (detail.m_vectorX, detail.m_vectorY);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 0, 1);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::GetProjectionParameters (ElementHandleCR source, CurveVectorPtr& profile, DVec3dR direction, double& distance, bool ignoreSkew)
    {
    if (!IsSurfaceOfProjection (source))
        return ERROR;
    
    ISolidPrimitivePtr   primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (source, false); // Don't simplify!!!
    DgnRuledSweepDetail  detail;
    
    if (!primitive.IsValid () || !primitive->TryGetDgnRuledSweepDetail (detail))
        return ERROR;

    if (2 != detail.m_sectionCurves.size ())
        return ERROR;

    DVec3d  extrusionVector;

    if (!detail.GetSectionCurveTranslation (extrusionVector, 0, 1))
        return ERROR; // Reject scale and twist...

    distance = direction.Normalize (extrusionVector);
    profile  = detail.m_sectionCurves.front ();

    if (ignoreSkew)
        return SUCCESS;

    double      area;
    DVec3d      normal, startCross;
    DPoint3d    centroid;

    if (!profile->CentroidNormalArea (centroid, normal, area))
        {
        switch (profile->HasSingleCurvePrimitive ())        
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                // NOTE: Old api didn't bother to test extruded line for skew...
                DSegment3dCP  segment = profile->front ()->GetLineCP ();
                DPoint3d      offset[2];
                double        closeParam;

                offset[0].SumOf (segment->point[0], extrusionVector);
                offset[1].SumOf (segment->point[1], extrusionVector);

                if (!DRay3d::FromOriginAndTarget (offset[0], offset[1]).ProjectPointUnbounded (offset[0], closeParam, segment->point[0]))
                    return ERROR;

                normal.NormalizedDifference (offset[0], segment->point[0]);
                break;
                }

            default:
                {
                DRange3d    localRange;
                Transform   localToWorld, worldToLocal;

                // NOTE: Old api always rejected non-planar profiles even if ignoring skew...
                if (!profile->IsPlanar (localToWorld, worldToLocal, localRange))
                    return ERROR;

                normal.Init (0.0, 0.0, 1.0);
                localToWorld.MultiplyMatrixOnly (normal);
                normal.Normalize ();
                break;
                }
            }
        }

    // Use the cross product to detect skew. Magnitude is max deviation from normal.
    startCross.CrossProduct (extrusionVector, normal);

    return (startCross.Magnitude () < TOLERANCE_ExtrusionSkew) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::ExtractProjectionParameters
(
ElementHandleCR     eh,
EditElementHandleR  profileEeh,
DVec3dR             direction,
double&             distance,
bool                ignoreSkew
)
    {
    CurveVectorPtr  profile;

    if (SUCCESS != GetProjectionParameters (eh, profile, direction, distance, ignoreSkew))
        return ERROR;

    if (SUCCESS != DraftingElementSchema::ToElement (profileEeh, *profile, NULL, true, *eh.GetDgnModelP ()))
        return ERROR;

    ElementPropertiesSetter::ApplyTemplate (profileEeh, eh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::SetProjectionParameters (EditElementHandleR eeh, CurveVectorCR profile, DVec3dCR direction, double distance)
    {
    bool        isCapped;

    if (!SurfaceOrSolidHandler::IsSurfaceOfProjection (eeh, &isCapped))
        return ERROR;

    ChildElemIter   firstChildEh (eeh, ExposeChildrenReason::Count);

    if (!firstChildEh.IsValid ())
        return ERROR;

    EditElementHandle  profileEeh;

    if (SUCCESS != DraftingElementSchema::ToElement (profileEeh, profile, &firstChildEh, true, *eeh.GetDgnModelP ()))
        return ERROR;

    DVec3d      extrudeVector = direction;
    DPoint3d    origin;

    origin.Zero ();
    extrudeVector.Scale (distance);

    EditElementHandle  newEeh;

    if (SUCCESS != SurfaceOrSolidHandler::CreateProjectionElement (newEeh, &eeh, profileEeh, origin, extrudeVector, NULL, isCapped, *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::GetRevolutionParameters (ElementHandleCR source, CurveVectorPtr& profile, DPoint3dR center, DVec3dR axis, double& sweep)
    {
    if (!isPossibleSurfaceOfRevolution (source))
        return ERROR;

    ISolidPrimitivePtr        primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (source, false); // Don't simplify!!!
    DgnRotationalSweepDetail  detail;

    if (!primitive.IsValid () || !primitive->TryGetDgnRotationalSweepDetail (detail))
        return ERROR;

    if (!detail.TryGetRotationAxis (center, axis, sweep))
        return ERROR;

    profile = detail.m_baseCurve;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::ExtractRevolutionParameters
(
ElementHandleCR     eh,
EditElementHandleR  profileEeh,
DPoint3dR           center,
DVec3dR             axis,
double&             sweep,
bool                ignoreNonStandardForms
)
    {
    if (ignoreNonStandardForms && !SurfaceOrSolidHandler::IsSurfaceOfRevolution (eh))
        return ERROR;

    CurveVectorPtr  profile;

    if (SUCCESS != GetRevolutionParameters (eh, profile, center, axis, sweep))
        return ERROR;

    if (SUCCESS != DraftingElementSchema::ToElement (profileEeh, *profile, NULL, true, *eh.GetDgnModelP ()))
        return ERROR;

    ElementPropertiesSetter::ApplyTemplate (profileEeh, eh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::SetRevolutionParameters (EditElementHandleR eeh, CurveVectorCR profile, DPoint3dCR center, DVec3dCR axis, double sweep, size_t numProfileRules)
    {
    bool        isCapped;

    if (!SurfaceOrSolidHandler::IsSurfaceOfRevolution (eeh, &isCapped))
        return ERROR;

    ChildElemIter   firstChildEh (eeh, ExposeChildrenReason::Count);

    if (!firstChildEh.IsValid ())
        return ERROR;

    EditElementHandle  profileEeh;

    if (SUCCESS != DraftingElementSchema::ToElement (profileEeh, profile, &firstChildEh, true, *eeh.GetDgnModelP ()))
        return ERROR;

    EditElementHandle  newEeh;

    if (SUCCESS != SurfaceOrSolidHandler::CreateRevolutionElement (newEeh, &eeh, profileEeh, center, axis, sweep, isCapped, *eeh.GetDgnModelP (), numProfileRules))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::GetProfiles (ElementHandleCR sourceEh, bvector<CurveVectorPtr>& profiles)
    {
    // Don't check el.surf.surftype to allow profiles to be extracted from unknown types created by applications...
    if (SOLID_ELM != sourceEh.GetLegacyType() && SURFACE_ELM != sourceEh.GetLegacyType())
        return ERROR;

    ChildElemIter  childEh (sourceEh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    int                      nBoundElms = sourceEh.GetElementCP ()->ToSurface().boundelms + 1;
    int                      boundElmsProcessed = 0;
    CurveVectorPtr           currentLoop;
    bvector<CurveVectorPtr>  currentProfileLoops;

    // Step through all of the children...
    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (isRuleClassElement (childEh.GetElementCP ()))
            continue;

        CurveVectorPtr  childCurve = ICurvePathQuery::ElementToCurveVector (childEh);

        if (childCurve.IsNull ())
            continue;

        if (childCurve->IsClosedPath ())
            {
            // Flush current loop, this is a new/complete loop...
            if (!currentLoop.IsNull ())
                {
                currentProfileLoops.push_back (currentLoop);
                currentLoop = NULL;
                }

            currentProfileLoops.push_back (childCurve);
            }
        else if (currentLoop.IsNull ())
            {
            currentLoop = childCurve;
            }
        else
            {
            DPoint3d    loopStartPt, loopEndPt, childStartPt;

            currentLoop->GetStartEnd (loopStartPt, loopEndPt);
            childCurve->GetStartPoint (childStartPt);

            // NOTE: Must test for parity region loop closure using 2 uor tolerance for historial reasons...YUCK!
            if (childStartPt.IsEqual (loopEndPt, 2.0) || !loopStartPt.IsEqual (loopEndPt, 2.0))
                {
                // Should be a single primitive since non-bcurve headers aren't stored, just to be safe...
                for (ICurvePrimitivePtr tmpCurve: *childCurve)
                    {
                    if (tmpCurve.IsNull ())
                        continue;

                    currentLoop->push_back (tmpCurve);
                    }
                }
            else
                {
                // Flush current loop, this is the start of a new loop...
                currentProfileLoops.push_back (currentLoop);
                currentLoop = childCurve;
                }
            }

        boundElmsProcessed += countBoundaries (childEh);

        if (boundElmsProcessed < nBoundElms)
            continue;

        // Flush current loop, could be a single open profile or a closed profile with gaps...
        if (!currentLoop.IsNull ())
            {
            currentProfileLoops.push_back (currentLoop);
            currentLoop = NULL;
            }

        switch (currentProfileLoops.size ())
            {
            case 0:
                return ERROR;

            case 1:
                {
                CurveVectorPtr  tmpCurve = currentProfileLoops.front ();
                DPoint3d        pts[2];

                // Return closed path (not a physically closed open path) for solids...uses horrible/historical 2 uor closure check for chains...
                static double s_closureTolerance = 2.0;
                if (SOLID_ELM == sourceEh.GetLegacyType() && tmpCurve->IsOpenPath () && tmpCurve->GetStartEnd (pts[0], pts[1]) && pts[0].IsEqual (pts[1], 1 == tmpCurve->size () ? 1.0e-8 : s_closureTolerance))
                    tmpCurve->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Outer);

                profiles.push_back (tmpCurve);
                break;
                }

            default:
                {
                CurveVectorPtr  regionProfile = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);

                for (CurveVectorPtr tmpCurve: currentProfileLoops)
                    {
                    // Outer loop is first followed by inner loops...
                    tmpCurve->SetBoundaryType (tmpCurve == currentProfileLoops.front () ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Inner);
                    regionProfile->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*tmpCurve));
                    }

                profiles.push_back (regionProfile);
                break;
                }
            }

        currentProfileLoops.clear ();
        boundElmsProcessed = 0;
        }

    return (profiles.size () > 1 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::ExtractProfiles (ElementHandleCR eh, ElementAgendaR agenda)
    {
    bvector<CurveVectorPtr>  profiles;

    if (SUCCESS != GetProfiles (eh, profiles))
        return ERROR;

    for (CurveVectorPtr curve: profiles)
        {
        EditElementHandle  eeh;

        if (SUCCESS != DraftingElementSchema::ToElement (eeh, *curve, NULL, true, *eh.GetDgnModelP ()))
            return ERROR;

        // Setup profile symbology...
        ElementPropertiesSetter::ApplyTemplate (eeh, eh);

        agenda.Insert (eeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SurfaceOrSolidHandler::_GetSolidPrimitive (ElementHandleCR eh, ISolidPrimitivePtr& primitive)
    {
    bvector<CurveVectorPtr> profiles;

    if (SUCCESS != GetProfiles (eh, profiles))
        return ERROR;

    DEllipse3d  ruleArc;

    if (searchForRuleArc (eh, ruleArc)) // Surface type tolerant check that just searches for a rule arc...
        {
        DgnRotationalSweepDetail  detail (profiles.front (), ruleArc.center, DVec3d::FromNormalizedCrossProduct (ruleArc.vector0, ruleArc.vector90), ruleArc.sweep, SOLID_ELM == eh.GetLegacyType());

        detail.SetVRuleCount (GetNumProfileRules (&eh)); // Set v rule count for wireframe display based on components...

        primitive = ISolidPrimitive::CreateDgnRotationalSweep (detail);
        }
    else
        {
        DgnRuledSweepDetail  detail (profiles, SOLID_ELM == eh.GetLegacyType());

        primitive = ISolidPrimitive::CreateDgnRuledSweep (detail);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SurfaceOrSolidHandler::_SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive)
    {
    DgnConeDetail  coneDetail;

    // NOTE: Simplify can change ruled sweeps into cones; set should preserve element type...
    if (primitive.TryGetDgnConeDetail (coneDetail) && 0.0 != coneDetail.m_radiusA && 0.0 != coneDetail.m_radiusB)
        {
        DEllipse3d  ellipseA, ellipseB;

        if (coneDetail.FractionToSection (0.0, ellipseA) && coneDetail.FractionToSection (1.0, ellipseB))
            {
            CurveVectorPtr curveA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            CurveVectorPtr curveB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

            curveA->push_back (ICurvePrimitive::CreateArc (ellipseA));
            curveB->push_back (ICurvePrimitive::CreateArc (ellipseB));

            DgnRuledSweepDetail sweepDetail (curveA, curveB, coneDetail.m_capped);
            ISolidPrimitivePtr  sweepPrimitive = ISolidPrimitive::CreateDgnRuledSweep (sweepDetail);

            return _SetSolidPrimitive (eeh, *sweepPrimitive);
            }
        }

    EditElementHandle   newEeh;

    if (SUCCESS != DraftingElementSchema::ToElement (newEeh, primitive, &eeh, *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SurfaceOrSolidHandler::_ApplyTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    if (IsSurfaceOfRevolution (elemHandle))
        {
        StatusInt   status;
        bool        wasHandled;

        if (SUCCESS != (status = ElementUtil::NonUniformScaleAsBsplineSurf (wasHandled, elemHandle, trans)) || wasHandled)
            return status;
        }

    return T_Super::_ApplyTransform (elemHandle, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SurfaceOrSolidHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    // Stretch not supported for surface/solid of revolution...
    if (IsSurfaceOfRevolution (elemHandle))
        return SUCCESS;

    return T_Super::_OnFenceStretch (elemHandle, transform, fp, options);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SurfWFCollector : IElementGraphicsProcessor
{
protected:

ViewContextP        m_context;
Transform           m_currentTransform;
ElementAgendaR      m_agenda;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
explicit SurfWFCollector (ElementAgendaR agenda) : m_agenda (agenda) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceContext (ViewContextR context) override {m_context = &context;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsBody (bool isCurved) const override {return false;} // Output only edges...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurvePrimitive (ICurvePrimitiveCR curve, bool isClosed, bool isFilled) override
    {
    BeAssert (NULL != m_context->GetCurrentElement ());
    
    ICurvePrimitivePtr tmpCurve = curve.Clone ();

    if (!m_currentTransform.IsIdentity ())
        tmpCurve->TransformInPlace (m_currentTransform);

    EditElementHandle eeh;

    // Output segments for linestrings...
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == tmpCurve->GetCurvePrimitiveType ())
        {
        bvector<DPoint3d> const& points = *tmpCurve->GetLineStringCP ();

        for (size_t i = 0; i < points.size ()-1; ++i)
            {
            DSegment3d  segment = DSegment3d::From (points[i], points[i+1]);

            // NOTE: Always create 3d elements, don't use dimension of current model...
            if (SUCCESS == LineHandler::CreateLineElement (eeh, NULL, segment, true, *m_context->GetCurrentElement ()->GetDgnModelP ()))
                m_agenda.Insert (eeh);
            }
        }
    else
        {
        // NOTE: Always create 3d elements, don't use dimension of current model...
        if (SUCCESS == DraftingElementSchema::ToElement (eeh, *tmpCurve, NULL, true, *m_context->GetCurrentElement ()->GetDgnModelP ()))
            m_agenda.Insert (eeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override {return ERROR;} // Output primitives...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessSolidPrimitive (ISolidPrimitiveCR primitive) override {return ERROR;} // Output edges...

}; // SurfWFCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void simplifyFaceGeometry (IGeometryPtr& faceGeom)
    {
    if (!faceGeom.IsValid ())
        return;

    CurveVectorPtr  faceCurves = faceGeom->GetAsCurveVector ();

    if (faceCurves.IsValid ())
        {
        // Prefer simple shapes and linestrings to complex shapes...
        faceCurves->ConsolidateAdjacentPrimitives ();
        return;
        }

    ISolidPrimitivePtr  faceSurface = faceGeom->GetAsISolidPrimitive ();

    if (!faceSurface.IsValid ())
        return;

    if (SolidPrimitiveType_DgnRuledSweep != faceSurface->GetSolidPrimitiveType () || faceSurface->HasCurvedFaceOrEdge ())
        return;

    DgnRuledSweepDetail detail;
    
    // Simplify lateral face geometry that can be represented as a shape instead of a ruled sweep...
    if (!faceSurface->TryGetDgnRuledSweepDetail (detail) || 2 != detail.m_sectionCurves.size () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line != detail.m_sectionCurves.front ()->HasSingleCurvePrimitive ())
        return;

    DPoint3d    shapePts[5];

    if (!detail.m_sectionCurves.front ()->GetStartEnd (shapePts[0], shapePts[1]) || !detail.m_sectionCurves.back ()->GetStartEnd (shapePts[3], shapePts[2]))
        return;

    shapePts[4] = shapePts[0];
    faceGeom = IGeometry::Create (CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString (shapePts, 5)));
    }
            
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SurfaceOrSolidHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Solids & geometry.GetOptions ()))
        return ERROR;

    if (DropGeometry::SOLID_Wireframe == geometry.GetSolidsOptions ())
        {
        ElementAgenda   wfGeom;
        SurfWFCollector edges (wfGeom);

        ElementGraphicsOutput::Process (edges, eh); // Collect wire geometry based on what is displayed, not components...

        if (0 == wfGeom.GetCount ())
            return ERROR;

        EditElementHandleP curr = wfGeom.GetFirstP ();
        EditElementHandleP end  = curr + wfGeom.GetCount ();

        for (; curr < end; curr++)
            {
            ElementPropertiesSetter::ApplyTemplate (*curr, eh);
            dropGeom.Insert (*curr);
            }

        return SUCCESS;
        }

    ISolidPrimitivePtr  primitive;

    if (SUCCESS != _GetSolidPrimitive (eh, primitive))
        return ERROR;

    bvector <SolidLocationDetail::FaceIndices> faceIndices;

    primitive->GetFaceIndices (faceIndices);

    if (0 == faceIndices.size ())
        return ERROR;

    for (SolidLocationDetail::FaceIndices const& thisFace: faceIndices)
        {
        IGeometryPtr  faceGeom = primitive->GetFace (thisFace);

        if (!faceGeom.IsValid ())
            continue;

        simplifyFaceGeometry (faceGeom);

        EditElementHandle  faceEeh;

        if (SUCCESS != DraftingElementSchema::ToElement (faceEeh, *faceGeom, NULL, *eh.GetDgnModelP ()))
            continue;

        ElementPropertiesSetter::ApplyTemplate (faceEeh, eh);
        dropGeom.Insert (faceEeh);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SurfaceOrSolidHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    SnapPathP   snap = context->GetSnapPath ();

    if (SnapMode::Origin == context->GetSnapMode ())
        {
        DPoint3d       hitPoint;
        ElementHandle  eh (snap->GetPathElem (snapPathIndex));

        GetRangeCenter (eh, hitPoint);

        context->ElmLocalToWorld (hitPoint);
        context->SetSnapInfo (snapPathIndex, SnapMode::Origin, context->GetSnapSprite (SnapMode::Origin), hitPoint, true, false);

        return SnapStatus::Success;
        }

    // NOTE: Draw doesn't visit components anymore to push them onto the path...
    if (snap->GetCount () > snapPathIndex+1)
        return context->DoSnapUsingNextInPath (snapPathIndex);

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceOrSolidHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    ChildElemIter   firstChild (thisElm, ExposeChildrenReason::Count);
    DisplayHandlerP dHandler = (firstChild.IsValid () ? firstChild.GetDisplayHandler() : NULL);

    if (!dHandler) // Bad element, nothing to do but ignore it...
        {
        T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);

        return;
        }

    // Type 18/19 use their first child for their display params
    dHandler->GetElemDisplayParams (firstChild, params, wantMaterials);

#ifdef WIP_VANCOUVER_MERGE // material
    if (wantMaterials)   // look for attachment for top level element if none found yet
        {
        if (params.GetMaterialUVDetailP ())
            params.GetMaterialUVDetailP ()->SetElementHandle (thisElm);
        if (NULL == params.GetMaterial ())
            params.SetMaterial (MaterialManager::GetManagerR ().FindMaterialAttachment (NULL, thisElm, *thisElm.GetDgnModelP (), false), true);
        }
#endif

    params.SetIsRenderable (true);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  StrokeSurfacePrimitive : IStrokeForCache
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize = 0.0) override
    {
    BeAssert(NULL != dh.GetElementHandleCP());
    ISolidPrimitivePtr  primitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (*dh.GetElementHandleCP());

    if (!primitive.IsValid ())
        return;

    context.GetIDrawGeom().DrawSolidPrimitive (*primitive);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceOrSolidHandler::DrawSurface (ElementHandleCR thisElm, ViewContextR context)
    {
    StrokeSurfacePrimitive  stroker;

    context.DrawCached (thisElm, stroker, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceOrSolidHandler::_GetOrientation (ElementHandleCR thisElm, RotMatrixR rMatrix)
    {
    ChildElemIter   tmpIter (thisElm, ExposeChildrenReason::Count);

    if (!tmpIter.IsValid ())
        {
        T_Super::_GetOrientation (thisElm, rMatrix);

        return;
        }

    DgnElementCP     el = thisElm.GetElementCP();

    if (SURFTYPE_PROJECTION != el->ToSurface().surftype)
        {
        DVec3d      rm0, rm1, axisRev;

        if (SUCCESS == extractRuleArcData (&tmpIter, NULL, NULL, NULL, &rm0, &rm1, &axisRev, NULL, NULL))
            {
            rMatrix.initFromColumnVectors (&rm0, &rm1, &axisRev);
            rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);

            return;
            }
        }

    do
        {
        DgnElementCP     boundEl = tmpIter.GetElementCP();

        if (!isRuleClassElement (boundEl))
            {
            DisplayHandlerP dHandler = tmpIter.GetDisplayHandler();

            if (dHandler)
                {
                dHandler->GetOrientation (tmpIter, rMatrix);

                return;
                }
            }

        tmpIter = tmpIter.ToNext();

        } while (tmpIter.IsValid ());

    T_Super::_GetOrientation (thisElm, rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceOrSolidHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    EditElementHandle  cellEeh;

    NormalCellHeaderHandler::CreateOrphanCellElement (cellEeh, SURFACE_ELM == eeh.GetLegacyType() ? L"From Surface" : L"From Solid", true, *eeh.GetDgnModelP ());

    for (ChildElemIter childEh (eeh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        MSElementDescrPtr tmpEdP = childEh.GetElementDescrCP ()->Duplicate();

        EditElementHandle tmpEeh (tmpEdP.get(), false);

        NormalCellHeaderHandler::AddChildElement (cellEeh, tmpEeh);
        }

    NormalCellHeaderHandler::AddChildComplete (cellEeh);
    eeh.ReplaceElementDescr (cellEeh.ExtractElementDescr().get());

    eeh.GetHandler().ConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return false; // NOTE: Vancouver will no longer display linestyle in wireframe...this is because QV is drawing the surface edges/silhouettes now.

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_SURFACE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    DrawSurface (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            SolidHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_SOLID_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SolidHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    DrawSurface (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct
    {
    EditElementHandleP  surfEeh;
    bool                isExtrusion;
    bool                isSolid;
    Transform           transform;
    void                (*transformFunc) (TransformR, void*); // function to transform profile
    DPoint3d            distance;
    double              revAngle; // angle to revolve element
    RotMatrix           viewRMatrix; // for calculating rule arcs
    void                (*ruleFunc) (void*, ElementHandleCR); // function to add rule elements

    } SurfaceParams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     countNonHeaderElements (int& numElems, ElementHandleCR eh)
    {
    // Ignore all complex headers except B-spline curve headers
    if (!eh.GetElementCP ()->IsComplexHeaderType () || BSPLINE_CURVE_ELM == eh.GetLegacyType())
        {
        numElems += countBoundaries (eh);

        return;
        }

    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        countNonHeaderElements (numElems, childEh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     appendComponents (SurfaceParams* sp, ElementHandleCR eh)
    {
    // Ignore all complex headers except B-spline curve headers
    if (!eh.GetElementCP()->IsComplexHeaderType() || BSPLINE_CURVE_ELM == eh.GetLegacyType())
        {
        EditElementHandle  tmpEeh;

        tmpEeh.Duplicate (eh);
        sp->surfEeh->GetElementDescrP()->AddComponent(*tmpEeh.ExtractElementDescr().get());

        return;
        }

    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        appendComponents (sp, childEh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Ferguson   04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static MSElementDescrVec::const_iterator findReferenceToFirstBoundaryElement (int numBounds, MSElementDescrVecCR components)
    {
    int i=0;
    for (auto boundary = components.rbegin(); boundary != components.rend(); ++boundary)
        {
        if (!isRuleClassElement (&(*boundary)->Element()))
            {
            if ((i += countBoundaries (ElementHandle(boundary->get(), false))) >= numBounds)
                return &*boundary;
            }
        }

    BeAssert(false);
    return components.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void extractBoundaryElements (EditElementHandleR boundEeh, ElementHandleCR eh)
    {
    boundEeh.SetElementDescr (new MSElementDescr(*eh.GetElementCP (), *eh.GetDgnModelP ()), false);

     int numBounds = eh.GetElementDescrCP()->Element().ToSurface().boundelms + 1;
    auto components = eh.GetElementDescrCP()->Components();
    auto bound = findReferenceToFirstBoundaryElement (numBounds, components);

    // Now create output boundary
    for (int i=0; ((bound != components.end()) && i < numBounds); i += countBoundaries(ElementHandle(bound->get(), false)), ++bound)
        {
        boundEeh.GetElementDescrP()->AddComponent(*bound->get()->Duplicate(true, false));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     extrudeElementTransform (TransformR transform, void* params)
    {
    SurfaceParams*  sp = (SurfaceParams*) (params);
    Transform       offset;

    offset.InitIdentity ();
    offset.SetTranslation (sp->distance);
    transform.InitProduct (sp->transform, offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     revolveElementTransform (TransformR transform, void* params)
    {
    SurfaceParams*  sp = (SurfaceParams*) (params);

    transform = sp->transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementClass  getRuleClass (ElementHandleCR eh)
    {
    return (DgnElementClass::Construction == static_cast<DgnElementClass>(eh.GetElementCP ()->GetElementClass()) ? DgnElementClass::ConstructionRule : DgnElementClass::PrimaryRule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void     projectElementAddRuleLines (SurfaceParams* sp, ElementHandleCR eh1, ElementHandleCR eh2)
    {
    if (eh1.GetLegacyType() != eh2.GetLegacyType())
        return; // Profiles don't match...

    CurveVectorPtr curves1 = ICurvePathQuery::ElementToCurveVector (eh1);
    CurveVectorPtr curves2 = ICurvePathQuery::ElementToCurveVector (eh2);

    if (!curves1.IsValid () || !curves2.IsValid ())
        return;

    DgnRuledSweepDetail detail (curves1, curves2, sp->isSolid);
    bvector<DSegment3d> rules;
    bvector<bool>       interior;

    WireframeGeomUtil::CollectRules (detail, rules, interior);

    ChildElemIter       childEh (*sp->surfEeh, ExposeChildrenReason::Count);

    for (size_t iRule = 0; iRule < rules.size (); ++iRule)
        {
        EditElementHandle  ruleEeh;

        // NOTE: Use firstElem for template, never a cell header (i.e. valid level) and clean of linkages...
        LineHandler::CreateLineElement (ruleEeh, &childEh, rules.at (iRule), true, *sp->surfEeh->GetDgnModelP ());
        ruleEeh.GetElementP()->SetElementClass(getRuleClass (*sp->surfEeh));

        sp->surfEeh->GetElementDescrP()->AddComponent(*ruleEeh.ExtractElementDescr().get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     extrudeElementAddRuleLines (void* params, ElementHandleCR eh)
    {
    bvector<CurveVectorPtr>  profiles;

    SurfaceOrSolidHandler::GetProfiles (eh, profiles); // NOTE: eh is always 18/19 header. We don't check status because we don't have both end caps...

    if (0 == profiles.size ())
        return;

    SurfaceParams*  sp = (SurfaceParams*) (params);
    DVec3d          extrudeVector;
    DPoint3d        startPt, endPt;
    Transform       transform;

    sp->transformFunc (transform, sp);
    profiles.front ()->GetStartPoint (startPt);
    transform.Multiply (endPt, startPt);
    extrudeVector.DifferenceOf (endPt, startPt);

    DgnExtrusionDetail  detail (profiles.front (), extrudeVector, sp->isSolid); // NOTE: We couldn't have used ElementToSolidPrimitive as we don't have rules yet!
    bvector<DSegment3d> rules;
    bvector<bool>       interior;

    WireframeGeomUtil::CollectRules (detail, rules, interior);

    ChildElemIter   childEh (eh, ExposeChildrenReason::Count);

    for (size_t iRule = 0; iRule < rules.size (); ++iRule)
        {
        EditElementHandle  ruleEeh;

        // NOTE: Because of scale/twist we can't use rule segment directly...
        transform.Multiply (rules.at (iRule).point[1], rules.at (iRule).point[0]);

        // NOTE: Use firstElem for template, never a cell header (i.e. valid level) and clean of linkages...
        LineHandler::CreateLineElement (ruleEeh, &childEh, rules.at (iRule), true, *sp->surfEeh->GetDgnModelP ());
        ruleEeh.GetElementP()->SetElementClass(getRuleClass (*sp->surfEeh));

        sp->surfEeh->GetElementDescrP()->AddComponent(*ruleEeh.ExtractElementDescr().get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     revolveElementAddRuleArcs (void* params, ElementHandleCR eh)
    {
    bvector<CurveVectorPtr>  profiles;

    SurfaceOrSolidHandler::GetProfiles (eh, profiles); // NOTE: eh is always 18/19 header. We don't check status because we don't have both end caps...

    if (0 == profiles.size ())
        return;

    SurfaceParams*  sp = (SurfaceParams*) (params);
    DVec3d          axis;
    DPoint3d        center = sp->distance;

    sp->viewRMatrix.GetRow (axis, 2);

    DgnRotationalSweepDetail detail (profiles.front (), center, axis, sp->revAngle, sp->isSolid); // NOTE: We couldn't have used ElementToSolidPrimitive as we don't have rules yet!
    bvector<DEllipse3d>      rules;
    bvector<bool>            interior;

    WireframeGeomUtil::CollectRules (detail, rules, interior);

    ChildElemIter   childEh (eh, ExposeChildrenReason::Count);

    for (size_t iRule = 0; iRule < rules.size (); ++iRule)
        {
        EditElementHandle  ruleEeh;

        // NOTE: Use firstElem for template, never a cell header (i.e. valid level) and clean of linkages...
        ArcHandler::CreateArcElement (ruleEeh, &childEh, rules.at (iRule), true, *sp->surfEeh->GetDgnModelP ());
        ruleEeh.GetElementP ()->SetElementClass(getRuleClass (*sp->surfEeh));

        sp->surfEeh->GetElementDescrP()->AddComponent(*ruleEeh.ExtractElementDescr().get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     cleanProfileElements (MSElementDescrP edP)
    {
    DgnElementP  elmP = &edP->ElementR();

    elmP->InvalidateElementId();
    elmP->SetSizeWords(elmP->GetAttributeOffset());

    if (elmP->IsGraphic())
        {
        // Make sure this is not a rule class element for some reason...
        if (DgnElementClass::PrimaryRule == static_cast<DgnElementClass>(elmP->GetElementClass()))
            elmP->SetElementClass(DgnElementClass::Primary);
        else if (DgnElementClass::ConstructionRule == static_cast<DgnElementClass>(elmP->GetElementClass()))
            elmP->SetElementClass(DgnElementClass::Construction);
        }

    for (auto& child : edP->Components())
        cleanProfileElements (child.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   completeSurface (SurfaceParams* sp)
    {
    if (!sp->surfEeh->IsValid ())
        return ERROR;

    sp->surfEeh->GetElementDescrP ()->Validate ();

    // Enforce basic/uniform symbology from solid header (either from defaults or template) since profile could have anything...
    ElementPropertiesSetter remapper;

    DgnElementCP hdrElmP = sp->surfEeh->GetElementCP ();

    remapper.SetLevel (LevelId(hdrElmP->GetLevel()));
    remapper.SetColor (hdrElmP->GetSymbology().color);
    remapper.SetWeight (hdrElmP->GetSymbology().weight);
    remapper.SetLinestyle (hdrElmP->GetSymbology().style, NULL);
    remapper.SetElementClass ((DgnElementClass) hdrElmP->GetElementClass());

    Display_attribute   attribute;

    if (mdlElement_displayAttributePresent (hdrElmP, TRANSPARENCY_ATTRIBUTE, &attribute))
        remapper.SetTransparency (attribute.attr_data.transparency.transparency);
    else
        remapper.SetTransparency (0.0);

    remapper.Apply (*sp->surfEeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     continueSurface (SurfaceParams* sp)
    {
    EditElementHandle  boundEeh;

    // Get the last set of boundary elements from the surface
    extractBoundaryElements (boundEeh, *sp->surfEeh);

    EditElementHandle  tmpEeh;
    Transform          transform;

    // Copy the boundary elements, transform and re-add them
    tmpEeh.Duplicate (boundEeh);
    sp->transformFunc (transform, sp);
    tmpEeh.GetHandler().ApplyTransform (tmpEeh, TransformInfo (transform));
    appendComponents (sp, tmpEeh);

    // Create rule elements and append them
    sp->ruleFunc (sp, boundEeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   createSurfaceFromElement
(
ElementHandleCR profileEh, 
ElementHandleCP templateEh, 
SurfaceParams*  sp,
DgnModelR    modelRef
)
    {
    if (SURFACE_ELM == profileEh.GetLegacyType() || SOLID_ELM == profileEh.GetLegacyType())
        {
        sp->surfEeh->Duplicate (profileEh); 
        }
    else
        {
        int     nBoundElms = 0;

        // NEEDSWORK: Verify that profileEh has only acceptable element types...
        countNonHeaderElements (nBoundElms, profileEh);

        if (nBoundElms > MAX_VERTICES)
            return ERROR;//MDLERR_TOOMANYSURFACEELMS;

        SurfaceOrSolidHandler::CreateSurfaceOrSolidHeaderElement (*sp->surfEeh, templateEh, sp->isExtrusion, sp->isSolid, nBoundElms, modelRef);

#ifdef WIP_VANCOUVER_MERGE // constraint2d
        // Copy 2D constraints from profile to header.
        Constraint2dExtension*      constraint2dExtension;

        if (NULL != (constraint2dExtension = Constraint2dExtension::Cast (profileEh.GetHandler ())))
            constraint2dExtension->CopyConstraintData (*sp->surfEeh, profileEh);
#endif

        // Add initial profile elements to surface (copy w/o xattributes and strip linkages!)
        MSElementDescrPtr tmpEdP = profileEh.GetElementDescrCP()->Duplicate(false, false);
        cleanProfileElements(tmpEdP.get());

        EditElementHandle tmpEeh (tmpEdP.get(), false);

        appendComponents (sp, tmpEeh);
        }

    continueSurface (sp);

    return completeSurface (sp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            SurfaceOrSolidHandler::CreateSurfaceOrSolidHeaderElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
bool                isExtrusion,
bool                isSolid,
int                 nBoundElms,
DgnModelR        modelRef
)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, isSolid ? SOLID_ELM : SURFACE_ELM, in->GetLevel(), false, ElementUtil::ELEMDIM_3d);

        if (SOLID_ELM != in->GetLegacyType() && SURFACE_ELM != in->GetLegacyType())
            memset (out.ToSurfaceR().reserved, 0, sizeof (out.ToSurface().reserved));
        }
    else
        {
        memset (&out, 0, sizeof (Surface));
        ElementUtil::SetRequiredFields (out, isSolid ? SOLID_ELM : SURFACE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, ElementUtil::ELEMDIM_3d);
        }

    out.SetSnappable(true); // non snappable
    out.SetPlanar(true); // non planar

    out.ToSurfaceR().surftype = (isExtrusion ? SURFTYPE_PROJECTION : SURFTYPE_REVOLUTION);

    if (nBoundElms > 0)
        out.ToSurfaceR().boundelms = nBoundElms-1;

    int         elmSize = sizeof (Surface);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    if (in)
        {
        // Remove known/unwanted linkages copied from template...leave material on header...
        mdlElement_displayAttributeRemove (&out, FILL_ATTRIBUTE);
        mdlElement_displayAttributeRemove (&out, GRADIENT_ATTRIBUTE);
        elemUtil_deleteLinkage (&out, LINKAGEID_Thickness);
        PatternLinkageUtil::DeleteFromElement (out, -1);

        Display_attribute   attribute;

        // YUCK: Place sphere tool didn't add transparency linkage to hdr in V8i, it was only found on components...
        if ((SOLID_ELM == in->GetLegacyType() || SURFACE_ELM == in->GetLegacyType()) && !mdlElement_displayAttributePresent (in, TRANSPARENCY_ATTRIBUTE, &attribute))
            {
            ChildElemIter  childTemplateEh (*templateEh, ExposeChildrenReason::Count);

            if (childTemplateEh.IsValid () && mdlElement_displayAttributePresent (childTemplateEh.GetElementCP (), TRANSPARENCY_ATTRIBUTE, &attribute))
                mdlElement_displayAttributeAdd (&out, &attribute);
            }
        }

    ElementUtil::InitScanRangeForUnion (out.GetRangeR(), true);
    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SurfaceOrSolidHandler::IsValidProfileType (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    // Surface/Solid only supports specific element types...
    switch (eh.GetLegacyType())
        {
        case SURFACE_ELM:
        case SOLID_ELM:
            return true; // continuation surface...

        case LINE_ELM:
        case LINE_STRING_ELM:
        case SHAPE_ELM:
        case CURVE_ELM:
        case CMPLX_STRING_ELM:
        case CMPLX_SHAPE_ELM:
        case ELLIPSE_ELM:
        case ARC_ELM:
        case BSPLINE_CURVE_ELM:
            return true;

        case CELL_HEADER_ELM:
            return GroupedHoleHandler::IsGroupedHole (eh);

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isClosedCurve (ElementHandleCR eh)
    {
    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    return (pathCurve.IsValid () ? pathCurve->IsClosedPath () : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::CreateProjectionElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh, 
ElementAgendaR     profiles, 
bool               preferSolid,
DgnModelR       modelRef
)
    {
    int     nBoundElms = 0;

    EditElementHandleP curr = profiles.GetFirstP ();
    EditElementHandleP end  = curr + profiles.GetCount ();

    for (; curr < end; curr++)
        {
        if (!IsValidProfileType (*curr))
            return ERROR;

        int     nProfileBoundElms = 0;

        // NEEDSWORK: Verify that profileEh has only acceptable element types...
        countNonHeaderElements (nProfileBoundElms, *curr);

        if (nProfileBoundElms > MAX_VERTICES)
            return ERROR; // MDLERR_TOOMANYSURFACEELMS;
                                                    
        if (0 != nBoundElms && nProfileBoundElms != nBoundElms)
            return ERROR; // Profiles don't match...

        nBoundElms = nProfileBoundElms;
        }

    SurfaceParams   sp;

    memset (&sp, 0, sizeof (sp));

    sp.surfEeh     = &eeh;
    sp.isExtrusion = true;
    sp.isSolid     = preferSolid && (isClosedCurve (*profiles.GetFirstP ()) || GroupedHoleHandler::IsGroupedHole (*profiles.GetFirstP ()));

    SurfaceOrSolidHandler::CreateSurfaceOrSolidHeaderElement (*sp.surfEeh, templateEh, sp.isExtrusion, sp.isSolid, nBoundElms, modelRef);

    EditElementHandleP last = NULL;

    curr = profiles.GetFirstP ();
    end  = curr + profiles.GetCount ();

    for (; curr < end; curr++)
        {
        // Add initial profile elements to surface (copy w/o xattributes and strip linkages!)
        MSElementDescrPtr tmpEdP = curr->GetElementDescrCP()->Duplicate(false, false);
        cleanProfileElements (tmpEdP.get());

        EditElementHandle  tmpEeh (tmpEdP.get(), false);
        appendComponents (&sp, tmpEeh);

        if (last)
            projectElementAddRuleLines (&sp, *last, *curr);

        last = curr;
        }

    return completeSurface (&sp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::CreateProjectionElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh, 
ElementHandleCR     profileEh, 
DPoint3dCR          origin, 
DVec3dCR            extrudeVector, 
TransformCP         transform,
bool                preferSolid,
DgnModelR        modelRef
)
    {
    if (!IsValidProfileType (profileEh))
        return ERROR;

    SurfaceParams   sp;

    memset (&sp, 0, sizeof (sp));

    sp.surfEeh          = &eeh;
    sp.isExtrusion      = true;
    sp.ruleFunc         = extrudeElementAddRuleLines;
    sp.transformFunc    = extrudeElementTransform;

    sp.distance.init (&extrudeVector);

    if (transform)
        sp.transform = *transform;
    else
        sp.transform.initIdentity ();

    (sp.transform).SetFixedPoint (origin);

    sp.isSolid = preferSolid && (isClosedCurve (profileEh) || GroupedHoleHandler::IsGroupedHole (profileEh));

    return createSurfaceFromElement (profileEh, templateEh, &sp, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createRevolutionElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh, 
ElementHandleCR     profileEh, 
DPoint3dCR          center, 
DVec3dCR            axis,
double              sweepAngle, 
bool                preferSolid,
DgnModelR        modelRef
)
    {
    RotMatrix   rMatrix;

    rMatrix.initFrom1Vector (&axis, 2, true);
    rMatrix.transpose ();

    SurfaceParams   sp;

    memset (&sp, 0, sizeof (sp));

    sp.surfEeh          = &eeh;
    sp.isExtrusion      = false;
    sp.ruleFunc         = revolveElementAddRuleArcs;
    sp.transformFunc    = revolveElementTransform;

    sp.distance         = center;
    sp.revAngle         = sweepAngle;
    sp.viewRMatrix      = rMatrix;

    RotMatrix   invRMatrix, tmpRMatrix;

    invRMatrix.InverseOf(rMatrix);
    tmpRMatrix.InitFromPrincipleAxisRotations(rMatrix,  0.0,  0.0,  sweepAngle);
    tmpRMatrix.InitProduct(invRMatrix, tmpRMatrix);
    (sp.transform).InitFrom (tmpRMatrix);

    (sp.transform).SetFixedPoint (center);

    sp.isSolid = preferSolid && (isClosedCurve (profileEh) || GroupedHoleHandler::IsGroupedHole (profileEh));

    return createSurfaceFromElement (profileEh, templateEh, &sp, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
size_t   SurfaceOrSolidHandler::GetNumProfileRules (ElementHandleCP eh)
    {
    if (!eh || !eh->IsValid ())
        return 0;

    if (SOLID_ELM != eh->GetLegacyType() && SURFACE_ELM != eh->GetLegacyType())
        return 0;

    int         elmsPerBoundary;

    if (0 == (elmsPerBoundary = eh->GetElementCP ()->ToSurface().boundelms + 1))
        return 0;

    int         numBoundElms = 0;

    for (ChildElemIter childEh (*eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (isRuleClassElement (childEh.GetElementCP ()))
            continue;

        numBoundElms++;

        if (BSPLINE_CURVE_ELM != childEh.GetLegacyType())
            continue;

        for (ChildElemIter curveChildEh (childEh, ExposeChildrenReason::Count); curveChildEh.IsValid (); curveChildEh = curveChildEh.ToNext ())
            numBoundElms++;
        }

    size_t      numProfileRules = (numBoundElms / elmsPerBoundary);

    return (numProfileRules > 0) ? numProfileRules-1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SurfaceOrSolidHandler::CreateRevolutionElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh, 
ElementHandleCR     profileEh, 
DPoint3dCR          center, 
DVec3dCR            axis,
double              sweepAngle, 
bool                preferSolid,
DgnModelR        modelRef,
size_t              numProfileRules
)
    {
    if (!IsValidProfileType (profileEh))
        return ERROR;

    BentleyStatus   status = BSIERROR;

    if (0 == numProfileRules)
        numProfileRules = GetNumProfileRules (templateEh); // Get current from a solid/surface template...

    if (numProfileRules > 1)
        {
        double  stepAngle = sweepAngle / numProfileRules;

        for (size_t i=0; i < numProfileRules; i++)
            if (SUCCESS != (status = createRevolutionElement (eeh, templateEh, i == 0 ? profileEh : eeh, center, axis, stepAngle, preferSolid, modelRef)))
                break;
        }
    else
        {
        status = createRevolutionElement (eeh, templateEh, profileEh, center, axis, sweepAngle, preferSolid, modelRef);
        }

    return status;
    }

