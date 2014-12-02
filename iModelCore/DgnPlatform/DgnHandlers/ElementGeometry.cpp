/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ElementGeometry.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createClosedLoop (EditElementHandleR eeh, bool is3d, DgnModelR modelRef)
    {
    EditElementHandle   chainEeh;

    ChainHeaderHandler::CreateChainHeaderElement (chainEeh, NULL, true, is3d, modelRef);

    if (SUCCESS != ChainHeaderHandler::AddComponentElement (chainEeh, eeh) ||
        SUCCESS != ChainHeaderHandler::AddComponentComplete (chainEeh))
        return ERROR;

    eeh.SetElementDescr (chainEeh.ExtractElementDescr().get(), false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus curvePrimitiveToElement_resolvedCurve (EditElementHandleR eeh, ICurvePrimitiveR curvePrimitive, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    EditElementHandle   newEeh;

    switch (curvePrimitive.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d segment = *curvePrimitive.GetLineCP ();

            if (SUCCESS != LineHandler::CreateLineElement (newEeh, NULL, segment, is3d, modelRef))
                return ERROR;

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = curvePrimitive.GetLineStringCP ();

            if (eeh.IsValid ())
                {
                size_t  iPoint = 0, totalPoints = points->size ();

                if (totalPoints < 2)
                    return ERROR;

                do
                    {
                    size_t  pointsRemaining = (totalPoints - iPoint);
                    size_t  pointsToProcess = pointsRemaining > MAX_VERTICES ? MAX_VERTICES : pointsRemaining;

                    if (2 == pointsToProcess)
                        {
                        DSegment3d segment;

                        segment.Init (points->at (iPoint), points->at (iPoint+1));

                        if (SUCCESS != LineHandler::CreateLineElement (newEeh, NULL, segment, is3d, modelRef))
                            return ERROR;
                        }
                    else
                        {
                        if (SUCCESS != LineStringHandler::CreateLineStringElement (newEeh, NULL, &points->at (iPoint), pointsToProcess, is3d, modelRef))
                            return ERROR;
                        }

                    if (SUCCESS != ChainHeaderHandler::AddComponentElement (eeh, newEeh))
                        return ERROR;

                    iPoint += pointsToProcess-1; // Duplicate last point...

                    } while (iPoint < totalPoints-1);

                }
            else if (curves.IsClosedPath ())
                {
                if (SUCCESS != ShapeHandler::CreateShapeElement (newEeh, NULL, &points->front (), points->size (), is3d, modelRef))
                    return ERROR;
                }
            else if (2 == points->size ())
                {
                DSegment3d segment;

                segment.Init (points->at (0), points->at (1));

                if (SUCCESS != LineHandler::CreateLineElement (newEeh, NULL, segment, is3d, modelRef))
                    return ERROR;
                }
            else
                {
                if (SUCCESS != LineStringHandler::CreateLineStringElement (newEeh, NULL, &points->front (), points->size (), is3d, modelRef))
                    return ERROR;
                }

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d ellipse = *curvePrimitive.GetArcCP ();

            if (curves.IsClosedPath () && !eeh.IsValid ())
                {
                if (SUCCESS != EllipseHandler::CreateEllipseElement (newEeh, NULL, ellipse, is3d, modelRef))
                    return ERROR;
                }
            else
                {
                if (SUCCESS != ArcHandler::CreateArcElement (newEeh, NULL, ellipse, is3d, modelRef))
                    return ERROR;
                }

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            {
            MSBsplineCurveCP bCurve = curvePrimitive.GetBsplineCurveCP ();

            if (BSPLINE_STATUS_Success != BSplineCurveHandler::CreateBSplineCurveElement (newEeh, NULL, *bCurve, is3d, modelRef))
                return ERROR;

            // NOTE: Nested chains are invalid, create will return a chain when bCurve.params.numPoles > MAX_POLES!
            if (!eeh.IsValid () || BSPLINE_CURVE_ELM == newEeh.GetLegacyType())
                {
                if (curves.IsClosedPath () && !eeh.IsValid () && !bCurve->params.closed) // Pole-based curve closure doesn't match loop closure...
                    if (SUCCESS != createClosedLoop (newEeh, is3d, modelRef))
                        return ERROR;
                break;
                }

            // Chain was created and we're inside chain create already...just add components to output chain...
            for (ChildElemIter childEh (newEeh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
                {
                EditElementHandle   tmpEeh (childEh, true);

                if (SUCCESS != ChainHeaderHandler::AddComponentElement (eeh, tmpEeh))
                    return ERROR;
                }

            newEeh.Invalidate ();
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            {
            MSInterpolationCurveCP fitCurve = curvePrimitive.GetInterpolationCurveCP ();

            if (BSPLINE_STATUS_Success != BSplineCurveHandler::CreateBSplineCurveElement (newEeh, NULL, *fitCurve, is3d, modelRef))
                return ERROR;

            if (curves.IsClosedPath () && !eeh.IsValid () && !fitCurve->params.isPeriodic) // Non-rational interpolation curve is always open...
                if (SUCCESS != createClosedLoop (newEeh, is3d, modelRef))
                    return ERROR;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            {
            bvector<DPoint3d> const* points = curvePrimitive.GetAkimaCurveCP ();

            if (SUCCESS != CurveHandler::CreateCurveElement (newEeh, NULL, &points->front (), points->size (), is3d, modelRef))
                return ERROR;

            if (curves.IsClosedPath () && !eeh.IsValid ()) // Akima curve is always open...
                if (SUCCESS != createClosedLoop (newEeh, is3d, modelRef))
                    return ERROR;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            DSpiral2dPlacementCP  sp = curvePrimitive.GetSpiralPlacementCP ();
            TransitionSpiralData  data (*sp->spiral, sp->frame, sp->fractionA, sp->fractionB);

            if (BSPLINE_STATUS_Success != SpiralCurveHandler::CreateSpiralCurveElement (newEeh, NULL, data, is3d, modelRef))
                return ERROR;

            if (curves.IsClosedPath () && !eeh.IsValid ()) // Spiral curve is always open...
                if (SUCCESS != createClosedLoop (newEeh, is3d, modelRef))
                    return ERROR;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = curvePrimitive.GetPointStringCP ();

            if (SUCCESS != PointStringHandler::CreatePointStringElement (newEeh, NULL, &points->front (), NULL, points->size (), true, is3d, modelRef))
                return ERROR;

            break;
            }

        default:
            return ERROR;
        }

    if (eeh.IsValid ())
        return newEeh.IsValid () ? ChainHeaderHandler::AddComponentElement (eeh, newEeh) : SUCCESS;

    eeh.SetElementDescr (newEeh.ExtractElementDescr().get(), false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus curvePrimitiveToElement (EditElementHandleR eeh, ICurvePrimitiveR curvePrimitive, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve == curvePrimitive.GetCurvePrimitiveType ())
        {
        ICurvePrimitivePtr resolvedCurve = curvePrimitive.CloneDereferenced (true, true); // Allow extension. Resolve through entire chain of partial curves.

        if (!resolvedCurve.IsValid ())
            return ERROR;

        return curvePrimitiveToElement_resolvedCurve (eeh, *resolvedCurve, curves, is3d, modelRef);
        }

    return curvePrimitiveToElement_resolvedCurve (eeh, curvePrimitive, curves, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createRegionElement (EditElementHandleR eeh, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (CurveVector::BOUNDARY_TYPE_UnionRegion != curves.GetBoundaryType ())
        return ERROR;

    ElementAgenda   regionAgenda;

    for (ICurvePrimitivePtr curvePrimitive : curves)
        {
        if (curvePrimitive.IsNull ())
            continue;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curvePrimitive->GetCurvePrimitiveType ())
            return ERROR; // Each loop must be a child curve bvector...

        EditElementHandle   tmpEeh;

        if (SUCCESS != DraftingElementSchema::ToElement (tmpEeh, *curvePrimitive->GetChildCurveVectorCP (), NULL, is3d, modelRef))
            return ERROR;

        regionAgenda.Insert (tmpEeh);
        }

    RegionParams    params;

    params.SetType (RegionType::Union);

    return AssocRegionCellHeaderHandler::CreateAssocRegionElement (eeh, regionAgenda, NULL, 0, NULL, 0, params, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createGroupedHoleElement (EditElementHandleR eeh, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (CurveVector::BOUNDARY_TYPE_ParityRegion != curves.GetBoundaryType ())
        return ERROR;

    EditElementHandle   solidEeh;
    ElementAgenda       holeAgenda;

    for (ICurvePrimitivePtr curvePrimitive : curves)
        {
        if (curvePrimitive.IsNull ())
            continue;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curvePrimitive->GetCurvePrimitiveType ())
            return ERROR; // Each loop must be a child curve bvector...

        switch (curvePrimitive->GetChildCurveVectorCP ()->GetBoundaryType ())
            {
            case CurveVector::BOUNDARY_TYPE_Outer:
                {
                if (solidEeh.IsValid ())
                    return ERROR; // Can only have one solid loop!

                if (SUCCESS != DraftingElementSchema::ToElement (solidEeh, *curvePrimitive->GetChildCurveVectorCP (), NULL, is3d, modelRef))
                    return ERROR;

                break;
                }

            case CurveVector::BOUNDARY_TYPE_Inner:
                {
                EditElementHandle   holeEeh;

                if (SUCCESS != DraftingElementSchema::ToElement (holeEeh, *curvePrimitive->GetChildCurveVectorCP (), NULL, is3d, modelRef))
                    return ERROR;

                holeAgenda.Insert (holeEeh);
                break;
                }

            default:
                return ERROR;
            }
        }

    return GroupedHoleHandler::CreateGroupedHoleElement (eeh, solidEeh, holeAgenda);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createPathElement (EditElementHandleR eeh, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (!(curves.IsClosedPath () || curves.IsOpenPath ()))
        return ERROR;

    ChainHeaderHandler::CreateChainHeaderElement (eeh, NULL, curves.IsClosedPath (), is3d, modelRef);

    for (ICurvePrimitivePtr curvePrimitive : curves)
        {
        if (curvePrimitive.IsNull ())
            continue;

        // Ignore gaps in the middle of chain...gaps and end could have come from partial copy...
        if (curvePrimitive->GetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve) && !(curvePrimitive == curves.front () || curvePrimitive == curves.back ()))
            continue;

        if (SUCCESS != curvePrimitiveToElement (eeh, *curvePrimitive, curves, is3d, modelRef))
            return ERROR;
        }

    return ChainHeaderHandler::AddComponentComplete (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createPrimitiveElement (EditElementHandleR eeh, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive ())
        return ERROR;

    EditElementHandle   newEeh; // NOTE: curvePrimitiveToElement assumes it's creating shape/chain if eeh is valid...

    if (SUCCESS != curvePrimitiveToElement (newEeh, *curves.front (), curves, is3d, modelRef))
        return ERROR;

    eeh.SetElementDescr (newEeh.ExtractElementDescr().get(), false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createGroupCellElement (EditElementHandleR eeh, CurveVectorCR curves, bool is3d, DgnModelR modelRef)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curves.HasSingleCurvePrimitive ())
        return ERROR;

    ElementAgenda   agenda;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (curve.IsNull ())
            continue;

        EditElementHandle   newEeh;

        if (SUCCESS != DraftingElementSchema::ToElement (newEeh, *curve, NULL, is3d, modelRef))
            return ERROR;

        agenda.Insert (newEeh);
        }

    return NormalCellHeaderHandler::CreateGroupCellElement (eeh, agenda);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DraftingElementSchema::ToElement (EditElementHandleR eeh, CurveVectorCR curves, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    EditElementHandle   newEeh; // NOTE: Preserve eeh in case it's also supplied as the template...

    switch (curves.GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            if (SUCCESS == createRegionElement (newEeh, curves, is3d, modelRef))
                break;

            return ERROR;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            if (curves.size () == 1)
                { // Just one loop !!!
                CurveVectorPtr child = curves[0]->GetChildCurveVectorP ();
                if (child.IsValid () 
                    && (  SUCCESS == createPrimitiveElement (newEeh, *child, is3d, modelRef)
                       || SUCCESS == createPathElement (newEeh, *child, is3d, modelRef)
                       )
                   )
                    break;
                }
            else  // multiple loops
                {
                if (SUCCESS == createGroupedHoleElement (newEeh, curves, is3d, modelRef))
                    break;
                }            
            return ERROR;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            if (SUCCESS == createGroupCellElement (newEeh, curves, is3d, modelRef))
                break;

            // FALL THROUGH: PointString...
            }

        default:
            {
            if (SUCCESS == createPrimitiveElement (newEeh, curves, is3d, modelRef) ||
                SUCCESS == createPathElement (newEeh, curves, is3d, modelRef))
                break;

            return ERROR;
            }
        }

    // Now use handler interfaces to make the new element match the template...
    if (templateEh)
        ElementPropertiesSetter::ApplyTemplate (newEeh, *templateEh);

    eeh.SetElementDescr (newEeh.ExtractElementDescr().get(), false);

    return SUCCESS;
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DraftingElementSchema::ToElement (EditElementHandleR eeh, ICurvePrimitiveCR curve, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve.GetCurvePrimitiveType ())
        return ToElement (eeh, *curve.GetChildCurveVectorCP (), templateEh, is3d, modelRef);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    curveVector->push_back (curve.Clone ());

    return ToElement (eeh, *curveVector, templateEh, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DraftingElementSchema::ToElements (ElementAgendaR agenda, CurveVectorCR curves, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef)
    {
    if (CurveVector::BOUNDARY_TYPE_None == curves.GetBoundaryType () && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString != curves.HasSingleCurvePrimitive ())
        {
        size_t  oldCount = agenda.GetCount ();

        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull ())
                continue;

            EditElementHandle   eeh;

            if (SUCCESS != ToElement (eeh, *curve, templateEh, is3d, modelRef))
                return ERROR;

            agenda.Insert (eeh);
            }

        return (agenda.GetCount () > oldCount ? SUCCESS : ERROR);
        }

    EditElementHandle   eeh;

    if (SUCCESS != ToElement (eeh, curves, templateEh, is3d, modelRef))
        return ERROR;

    agenda.Insert (eeh);

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void setPositiveSweep (DVec3dR axis, double& sweepRadians)
    {
    // Update to positive sweep and return rule count
    if (sweepRadians < 0.0)
        {
        sweepRadians = -sweepRadians;
        axis.Negate ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnCone (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnConeDetail  coneData;

    if (!solid.TryGetDgnConeDetail (coneData))
        return ERROR;

    DPoint3d    centerA, centerB;
    RotMatrix   rotMatrix;
    double      radiusA, radiusB;
    bool        capped;

    if (!coneData.IsCircular (centerA, centerB, rotMatrix, radiusA, radiusB, capped))
        return ERROR;

    return ConeHandler::CreateConeElement (eeh, templateEh, radiusB, radiusA, centerB, centerA, rotMatrix, capped, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnTorus (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnTorusPipeDetail  torusData;

    if (!solid.TryGetDgnTorusPipeDetail (torusData))
        return ERROR;
    
    DPoint3d    center;
    DVec3d      axis;
    double      sweepRadians;

    if (!torusData.TryGetRotationAxis (center, axis, sweepRadians))
        return ERROR;

    DEllipse3d          minorHoop = torusData.VFractionToUSectionDEllipse3d (0.0);
    EditElementHandle   meridianElement;
    ElementHandle       profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    if (SUCCESS != EllipseHandler::CreateEllipseElement (meridianElement, profileTemplate.IsValid () ? &profileTemplate : NULL, minorHoop, true, modelRef))
        return ERROR;

    bool        capped = solid.GetCapped ();
    size_t      numRules = DgnRotationalSweepDetail::ComputeVRuleCount (sweepRadians);

    setPositiveSweep (axis, sweepRadians);

    return SurfaceOrSolidHandler::CreateRevolutionElement (eeh, templateEh, meridianElement, center, axis, sweepRadians, capped, modelRef, numRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnSphere (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnSphereDetail  detail;

    if (!solid.TryGetDgnSphereDetail (detail))
        return ERROR;

    DPoint3d    center;
    double      sweepRadians;
    DVec3d      zAxis;

    if (!detail.TryGetRotationAxis (center, zAxis, sweepRadians))
        return ERROR;

    DEllipse3d          meridianEllipse = detail.UFractionToVSectionDEllipse3d (0.0);
    EditElementHandle   meridianElement;
    ElementHandle       profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    if (SUCCESS != ArcHandler::CreateArcElement (meridianElement, profileTemplate.IsValid () ? &profileTemplate : NULL, meridianEllipse, true, modelRef))
        return ERROR;

    bool        capped = solid.GetCapped ();
    size_t      numRules = DgnRotationalSweepDetail::ComputeVRuleCount (sweepRadians);

    setPositiveSweep (zAxis, sweepRadians);

    double      tmpRadius;
    DPoint3d    tmpCenter;
    RotMatrix   tmpAxes;

    // To create a solid sphere (Type 19) profile needs to be a closed element.
    if (capped && detail.IsTrueSphere (tmpCenter, tmpAxes, tmpRadius))
        {
        DSegment3d          segment;
        EditElementHandle   closedEeh, lineEeh;

        meridianEllipse.EvaluateEndPoints (segment.point[1], segment.point[0]);
        LineHandler::CreateLineElement (lineEeh, profileTemplate.IsValid () ? &profileTemplate : NULL, segment, true, modelRef);

        ChainHeaderHandler::CreateChainHeaderElement (closedEeh, profileTemplate.IsValid () ? &profileTemplate : NULL, true, true, modelRef);
        ChainHeaderHandler::AddComponentElement (closedEeh, meridianElement);
        ChainHeaderHandler::AddComponentElement (closedEeh, lineEeh);
        ChainHeaderHandler::AddComponentComplete (closedEeh);

        meridianElement.SetElementDescr (closedEeh.ExtractElementDescr ().get (), false);
        }

    return SurfaceOrSolidHandler::CreateRevolutionElement (eeh, templateEh, meridianElement, center, zAxis, sweepRadians, capped, modelRef, numRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnBox (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnBoxDetail  boxData;

    if (!solid.TryGetDgnBoxDetail (boxData))
        return ERROR;

    bvector<DPoint3d>   corners;

    boxData.GetCorners (corners);

    DPoint3d  baseRectangle[5];

    baseRectangle[0] = corners[0];
    baseRectangle[1] = corners[1];
    baseRectangle[2] = corners[3];
    baseRectangle[3] = corners[2];
    baseRectangle[4] = corners[0];

    DPoint3d  topRectangle[5];

    topRectangle[0] = corners[4];
    topRectangle[1] = corners[5];
    topRectangle[2] = corners[7];
    topRectangle[3] = corners[6];
    topRectangle[4] = corners[4];

    EditElementHandle   baseElement, topElement;
    ElementHandle       profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    if (SUCCESS != ShapeHandler::CreateShapeElement (baseElement, profileTemplate.IsValid () ? &profileTemplate : NULL, baseRectangle, 5, true, modelRef))
        return ERROR;

    if (SUCCESS != ShapeHandler::CreateShapeElement (topElement, profileTemplate.IsValid () ? &profileTemplate : NULL, topRectangle, 5, true, modelRef))
        return ERROR;
    
    ElementAgenda   profiles;

    profiles.Insert (baseElement);
    profiles.Insert (topElement);

    bool    capped = solid.GetCapped ();

    return SurfaceOrSolidHandler::CreateProjectionElement (eeh, templateEh, profiles, capped, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnExtrusion (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnExtrusionDetail  detail;

    if (!solid.TryGetDgnExtrusionDetail (detail))
        return ERROR;

    EditElementHandle   profileElement;
    ElementHandle       profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    if (SUCCESS != DraftingElementSchema::ToElement (profileElement, *detail.m_baseCurve, profileTemplate.IsValid () ? &profileTemplate : NULL, true, modelRef))
        return ERROR;

    bool        capped = solid.GetCapped ();
    DVec3d      extrusion = detail.m_extrusionVector;
    DPoint3d    origin;

    detail.m_baseCurve->GetStartPoint (origin); // humbug. The origin arg for CreateProjectionElement is unused, but make it accurate.

    return SurfaceOrSolidHandler::CreateProjectionElement (eeh, templateEh, profileElement, origin, extrusion, NULL, capped, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnRotationalSweep (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnRotationalSweepDetail  detail;

    if (!solid.TryGetDgnRotationalSweepDetail (detail))
        return ERROR;

    DPoint3d    center;
    DVec3d      axis;
    double      sweepRadians;

    if (!detail.TryGetRotationAxis (center, axis, sweepRadians))
        return ERROR;

    EditElementHandle   profileElement;
    ElementHandle       profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    if (SUCCESS != DraftingElementSchema::ToElement (profileElement, *detail.m_baseCurve, profileTemplate.IsValid () ? &profileTemplate : NULL, true, modelRef))
        return ERROR;

    bool    capped = solid.GetCapped ();
    size_t  numRules = detail.GetVRuleCount ();

    setPositiveSweep (axis, sweepRadians);

    return SurfaceOrSolidHandler::CreateRevolutionElement (eeh, templateEh, profileElement, center, axis, sweepRadians, capped, modelRef, numRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus elementFromDgnRuledSweep (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    DgnRuledSweepDetail  detail;
    
    if (!solid.TryGetDgnRuledSweepDetail (detail))
        return ERROR;

    ElementAgenda  agenda;
    ElementHandle  profileTemplate;

    if (templateEh)
        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (profileTemplate, *templateEh); // Don't use a surface/solid hdr as profile template...

    for (CurveVectorPtr profileCurve: detail.m_sectionCurves)
        {
        EditElementHandle   profileElement;

        if (SUCCESS == DraftingElementSchema::ToElement (profileElement, *profileCurve, profileTemplate.IsValid () ? &profileTemplate : NULL, true, modelRef))
            agenda.Insert (profileElement);
        }
        
    bool    capped = solid.GetCapped ();

    return SurfaceOrSolidHandler::CreateProjectionElement (eeh, templateEh, agenda, capped, modelRef);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToElement (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    switch (solid.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            return elementFromDgnTorus (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnCone:
            return elementFromDgnCone (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnBox:
            return elementFromDgnBox (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnSphere:
            return elementFromDgnSphere (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnExtrusion:
            return elementFromDgnExtrusion (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnRotationalSweep:
            return elementFromDgnRotationalSweep (eeh, solid, templateEh, modelRef);

        case SolidPrimitiveType_DgnRuledSweep:
            return elementFromDgnRuledSweep (eeh, solid, templateEh, modelRef);

        default:
            return ERROR;
        }
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToElement (EditElementHandleR eeh, MSBsplineSurfaceCR surface, ElementHandleCP templateEh, DgnModelR modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return (BSPLINE_STATUS_Success == BSplineSurfaceHandler::CreateBSplineSurfaceElement (eeh, templateEh, surface, modelRef) ? SUCCESS : ERROR);
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToElement (EditElementHandleR eeh, PolyfaceQueryCR meshData, ElementHandleCP templateEh, DgnModelR modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return MeshHeaderHandler::CreateMeshElement (eeh, templateEh, meshData, modelRef.Is3d (), modelRef);
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToElement (EditElementHandleR eeh, ISolidKernelEntityCR entityData, ElementHandleCP templateEh, DgnModelR modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return BrepCellHeaderHandler::CreateBRepCellElement (eeh, templateEh, entityData, modelRef);
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToElement (EditElementHandleR eeh, IGeometryCR geomData, ElementHandleCP templateEh, DgnModelR modelRef)
    {
    switch (geomData.GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = geomData.GetAsICurvePrimitive ();

            return DraftingElementSchema::ToElement (eeh, *curvePrimitive, templateEh, modelRef.Is3d (), modelRef);        
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = geomData.GetAsCurveVector ();

            return DraftingElementSchema::ToElement (eeh, *curveVector, templateEh, modelRef.Is3d (), modelRef);
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = geomData.GetAsISolidPrimitive ();

            return DraftingElementSchema::ToElement (eeh, *solidPrimitive, templateEh, modelRef);
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = geomData.GetAsMSBsplineSurface ();

            return DraftingElementSchema::ToElement (eeh, *bSurface, templateEh, modelRef);
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = geomData.GetAsPolyfaceHeader ();

            return DraftingElementSchema::ToElement (eeh, *polyface, templateEh, modelRef);
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshProcessor : public IElementGraphicsProcessor
{
bvector <PolyfaceHeaderPtr> &m_output;
IFacetOptionsP m_options;
Transform           m_currentTransform;

MeshProcessor (bvector<PolyfaceHeaderPtr> &output, IFacetOptionsP options)
    : m_output (output), m_options (options)
    {
    }

virtual IFacetOptionsP _GetFacetOptionsP () { return m_options;}
virtual bool _ProcessAsFacets (bool isPolyface) const override {return true;}
virtual bool _ProcessAsBody (bool isCurved) const {return false;}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                           
 +---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                           
 +---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessFacets (PolyfaceQueryCR facets, bool isFilled = false) override
    {
    PolyfaceHeaderPtr header = PolyfaceHeader::New ();
    header->CopyFrom (facets);
    header->Transform(m_currentTransform);
    m_output.push_back (header);
    return SUCCESS;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IMeshQuery::ElementToApproximateFacets
(
ElementHandleCR source,
bvector<PolyfaceHeaderPtr> &output,
IFacetOptionsP options
)
    {
    output.clear ();
    MeshProcessor processor (output, options);
    ElementGraphicsOutput::Process (processor, source);
    return output.size () > 0 ? SUCCESS : ERROR;
    }
