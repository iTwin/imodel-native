/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgHatchExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer)
    {
    // called from the model or a paper space
    auto& entity = context.GetEntityPtrR ();
    m_hatch = DwgDbHatch::Cast (entity.get());
    if (m_hatch == nullptr)
        return  BSIERROR;

    bool isFilled = m_hatch->IsSolidFill() || m_hatch->IsGradient();
    bool wantConversion = importer.GetOptions().IsFilledHatchAsFilledElement ();

    // for a filled hatch, extract hatch boundaries and create a pattern element, as toolkit draws it as very fine mesh
    if (isFilled && wantConversion)
        {
        m_toBimContext = &context;

        // ready to convert the hatch to geometry
        auto geometry = this->_ConvertToGeometry (entity.get(), !context.GetModel().Is3d(), importer, nullptr);
        if (geometry.IsValid())
            {
            auto status = this->CreateElementInModel (*geometry, importer);
            if (status == BSISUCCESS)
                return  status;
            }
        }

    // for a patterned hatch, we still prefer to let toolkit draw it, which gives us better graphics
    return importer._ImportEntity(context.GetElementResultsR(), context.GetElementInputsR());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr DwgHatchExt::_ConvertToGeometry (DwgDbEntityCP entity, bool is2D, DwgImporterR importer, IDwgDrawParametersP drawParams)
    {
    // called from block draw or above _ConvertToBim from importing model entity
    m_hatch = DwgDbHatch::Cast (entity);
    if (m_hatch == nullptr)
        return  nullptr;

    size_t  numLoops = m_hatch->GetLoopCount ();
    bool    wantConversion = importer.GetOptions().IsFilledHatchAsFilledElement ();
    if (numLoops < 1 || !wantConversion)
        return  nullptr;

    m_hatchNormal = m_hatch->GetNormal();
    m_scaleToMeters = importer.GetScaleToMeters ();
    m_hatchHandle = m_hatch->GetObjectId().ToUInt64 ();

    // compose element transform by hatch definition matrix with its elevation
    m_hatch->GetEcs (m_transform);
    if (!is2D)
        DwgHelper::ComposeTransformByExtrusion (m_transform, is2D ? 0.0 : m_hatch->GetElevation());
    m_isIdentityTransform = m_transform.IsIdentity ();

    auto paths = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
    if (!paths.IsValid())
        return  nullptr;

    for (size_t loopIndex=0; loopIndex < numLoops; loopIndex++)
        this->ConvertLoop (*paths, loopIndex);

    if (drawParams != nullptr)
        this->GetDrawParameters (*drawParams);

    // create a CurveVector geometry
    return GeometricPrimitive::Create(paths);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgHatchExt::ConvertLoop (CurveVectorR paths, size_t loopIndex)
    {
    if (!this->ShouldConvertLoop(loopIndex))
        return  BSISUCCESS;

    auto path = this->CreatePathFromLoop (loopIndex);
    if (!path.IsValid())
        return  BSIERROR;

    paths.Add (path);
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgHatchExt::ShouldConvertLoop (size_t loopIndex) const
    {
    int32_t loopType = (int32_t)m_hatch->GetLoopType(loopIndex);

    // Fill always ignores unclosed boundaries
    if (0 != (DwgDbHatch::LoopType::NotClosed & loopType))
        return  false;

    bool outermost = 0 != (DwgDbHatch::LoopType::Outermost & loopType);
    bool external = 0 != (DwgDbHatch::LoopType::External & loopType);
    if (!external)
        {
        auto hatchStyle = m_hatch->GetHatchStyle();
        switch (hatchStyle)
            {
            case DwgDbHatch::Style::Outer:
                if (!outermost)
                    return  false;
                break;
            case DwgDbHatch::Style::Ignore:
                return  false;
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVector::BoundaryType DwgHatchExt::GetBoundaryType (DwgDbHatch::LoopType loopType) const
    {
    bool external   = 0 != (DwgDbHatch::LoopType::External & loopType);
    bool textBox    = 0 != (DwgDbHatch::LoopType::Textbox & loopType);
    bool outermost  = 0 != (DwgDbHatch::LoopType::Outermost & loopType);
    bool textIsland = 0 != (DwgDbHatch::LoopType::TextIsland & loopType);
    bool notClosed  = 0 != (DwgDbHatch::LoopType::NotClosed & loopType);

    CurveVector::BoundaryType   boundaryType = CurveVector::BOUNDARY_TYPE_Outer;
    if ((!external && !outermost) || textBox)
        boundaryType = CurveVector::BOUNDARY_TYPE_Inner;

    /*---------------------------------------------------------------------------------------------------
    When a loop is not closed, it should be a part of a closed loop formed by a collection of open loops.
    Keep these "loops" as open loops to allow them to eventually form a closed loop.  There are cases in
    which the bit kNotClosed is erroneously set in a hatch which only has a single loop, TFS#69857 as such
    an example, we want to close the loop or otherwise our assoc handler would fail.
    ---------------------------------------------------------------------------------------------------*/
    if (notClosed && m_hatch->GetLoopCount() > 1)
        boundaryType = CurveVector::BOUNDARY_TYPE_Open;

    return  boundaryType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHatchExt::CreatePathFromLoop (size_t loopIndex)
    {
    auto loopType = m_hatch->GetLoopType (loopIndex);
    auto boundaryType = this->GetBoundaryType (loopType);
    auto path = CurveVector::Create (boundaryType);
    if (!path.IsValid())
        return  nullptr;

    if (0 != (loopType & DwgDbHatch::LoopType::Polyline))
        return this->CreatePathFromPolyline (loopIndex, boundaryType);
    else
        return this->CreatePathFromEdges (loopIndex, boundaryType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHatchExt::CreatePathFromPolyline (size_t loopIndex, CurveVector::BoundaryType boundaryType)
    {
    // the loop is a polyline
    DPoint2dArray points;
    DwgDbDoubleArray bulges;

    DwgDbStatus   status = m_hatch->GetLoop(loopIndex, points, bulges);
    if (status != DwgDbStatus::Success || points.empty())
        return  nullptr;

    CurveVectorPtr  path = CurveVector::Create (boundaryType);
    if (!path.IsValid())
        return  nullptr;

    double      bulge1 = bulges.empty() ? 0.0 : bulges[0];
    double      bulge0;
    DPoint2d    point1 = points[0];
    DPoint2d    point0;
    DPoint3dArray   collectedPoints;

    // Note: when the polyline is closed, RealDWG inserts a final point that is equal to the first point. That point is not stored in the file.
    size_t  nSegments = points.size() - 1;
    size_t  pointCount = 0;
    for (size_t iSegment = 1; iSegment <= nSegments; iSegment++)
        {
        point0 = point1;
        bulge0 = bulge1;

        point1 = points[iSegment];
        bulge1 = bulges.empty () ? 0.0 : bulges[iSegment];

        pointCount = collectedPoints.size ();
        if (DwgHelper::IsBulgeFactorValid(bulge0))
            {
            // create a bulge arc, break apart from previous lines, and start a new segment
            if (pointCount > 0)
                {
                if (!m_isIdentityTransform)
                    m_transform.Multiply (&collectedPoints.front(), (int)pointCount);

                auto previousSegs = ICurvePrimitive::CreateLineString (collectedPoints);
                if (previousSegs.IsValid())
                    path->Add (previousSegs);

                // break point from previous segments - a new segment will start after the bulge arc
                collectedPoints.clear ();
                }

            // create a bulge arc
            DEllipse3d  arc;
            DwgHelper::CreateArc2d (arc, point0, point1, bulge0);

            auto bulgeArc = ICurvePrimitive::CreateArc (arc);
            if (bulgeArc.IsValid())
                path->Add (bulgeArc);
            }
        else
            {
            // collect linear points
            if (pointCount == 0)
                collectedPoints.push_back (DPoint3d::From(point0));

            collectedPoints.push_back (DPoint3d::From(point1));
            }
        }

    pointCount = collectedPoints.size ();
    if (pointCount > 0)
        {
        if (!m_isIdentityTransform)
            m_transform.Multiply (&collectedPoints.front(), (int)pointCount);

        // Don't create degenerate loops (TR# 168301)
        size_t  kPoint = 1;
        for (; kPoint < pointCount; kPoint++)
            if (collectedPoints.at(kPoint-1).Distance(collectedPoints.at(kPoint)) > 1.e-6)
                break;

        if (kPoint == pointCount)
            {
            LOG_ENTITY.debugv ("Ignoring degenerate loop in hatch, ID=%llx", m_hatchHandle);
            }
        else
            {
            auto linestring = ICurvePrimitive::CreateLineString (collectedPoints);
            if (linestring.IsValid())
                path->Add (linestring);
            }
        }

    return  path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHatchExt::CreatePathFromEdges (size_t loopIndex, CurveVector::BoundaryType boundaryType)
    {
    // the loop consists of individual segments
    CurveVectorPtr  rawEdges;
    DwgDbStatus     status = m_hatch->GetLoop (loopIndex, rawEdges);
    if (status != DwgDbStatus::Success)
        return nullptr;

    CurveVectorPtr  path = CurveVector::Create (boundaryType);
    if (!path.IsValid())
        return nullptr;

    DPoint3d    currentStart, currentEnd, firstStart, firstEnd;
    firstStart.InitDisconnect ();
    firstEnd.InitDisconnect ();

    double  dwgTolerance = this->GetLoopTolerance ();
    double  bimTolerance = dwgTolerance * m_scaleToMeters;
    bool    gapFound = false;
    size_t  numEdges = rawEdges->size ();
    size_t  edgeIndex = 0;

    for (; edgeIndex < numEdges; edgeIndex++)
        {
        auto    edge = rawEdges->at(edgeIndex);
        if (!edge.IsValid() || !this->IsValidEdge(*edge, dwgTolerance))
            continue;

        DPoint3d    start, end;
        if (edge->GetStartEnd(start, end))
            {
            if (path->empty())
                {
                firstStart = start;
                firstEnd = end;
                }
            else if (this->IsRepetitive(*path, *edge, edgeIndex, dwgTolerance) || this->IsDangling(currentEnd, start, end, *rawEdges, edgeIndex, dwgTolerance))
                {
                // skip repetitive edges as our fill does not work well with these, TR 347482.
                // skip dangling edges as CloneWithGapsClosed can't seem to handle that well.
                continue;
                }
            else if (start.Distance(currentEnd) > dwgTolerance)
                {
                // there is a gap, but if it is the last edge and the loop is closed, skip the bad edge - for Takenaka TFS 191926.
                if (edgeIndex + 1 == numEdges && firstStart.Distance(currentEnd) < dwgTolerance)
                    continue;
                else
                    gapFound = true;
                }
            currentStart = start;
            currentEnd = end;

            if (!m_isIdentityTransform)
                edge->TransformInPlace (m_transform);
            path->Add (edge);
            }
        }

    if (path->empty())
        return  nullptr;

    // check overlap of the first and the last segments of the entire loop
    auto edge = path->front ();
    if (edgeIndex > 2 && path->size() > 1 && this->IsRepetitive(*path, *edge, edgeIndex, bimTolerance))
        {
        // remove the last segment
        path->pop_back ();
        }

    if (gapFound)
        this->ClosePath (path, bimTolerance);

    if (m_hatch->IsGradient())
        this->ValidatePathDirection (path, loopIndex);

    return  path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgHatchExt::IsValidEdge (ICurvePrimitiveCR edge, double sizeTolerance) const
    {
    // A small overlapping arc can cause CloneWithGapsClosed to create bad loop - letter C in TFS#16392.
    // For a valid(i.e. no overlapping) arc, a gap shall be created and then filled by a line segment.
    auto arc = edge.GetArcCP ();
    if (arc != nullptr)
        {
        double arcLength = arc->ArcLength ();
        if (arcLength <= sizeTolerance)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgHatchExt::ClosePath (CurveVectorPtr& path, double pointTolerance) const
    {
    CurveGapOptions gapOptions (pointTolerance, 0.001, 0.002);
    auto    path1 = path->CloneWithGapsClosed (gapOptions);
    double  maxGap = path->ReorderForSmallGaps ();

    path = path->CloneWithGapsClosed (gapOptions);

    double  reorderedLength = path->Length ();
    double  originalLength = path1->Length ();
    if (originalLength < reorderedLength)
        path = path1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgHatchExt::ValidatePathDirection (CurveVectorPtr& path, size_t loopIndex) const
    {
    // check loop normal for gragient fill
    uint32_t loopType = (uint32_t)m_hatch->GetLoopType (loopIndex);
    bool external   = 0 != (DwgDbHatch::LoopType::External & loopType);
    bool textBox    = 0 != (DwgDbHatch::LoopType::Textbox & loopType);
    bool textIsland = 0 != (DwgDbHatch::LoopType::TextIsland & loopType);
    if (!textBox && !textIsland && external)
        {
        DVec3d loopNormal;
        DPoint3d center;
        double area = 0.0;
        if (path->CentroidNormalArea(center, loopNormal, area) && m_hatchNormal.DotProduct(loopNormal) < 0.0)
            {
            // loop normal is opposing to the hatch normal, reverse loop direction
            path = path->CloneReversed ();
            return  true;
            }
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgHatchExt::IsRepetitive(CurveVectorR path, ICurvePrimitiveCR newCurve, size_t edgeNo, double tol) const
    {
    /*------------------------------------------------------------------------------------------------
    A trivial rejection of duplicated edges - two adjacent edges with coincident start and end points.
    What this method does not do is a loop algorithm that runs through entire input edges and re-chain
    them to produce a valid loop.  That is done in the geom lib when creating assoc region from the
    CurveVector.  Only miminim effort is made here for the sake of performance.
    ------------------------------------------------------------------------------------------------*/

    // start & end points of the new curve
    DPoint3d    start, end;
    if (path.empty() || !newCurve.GetStartEnd(start, end))
        return  false;

    // start & end points of the previous curve in the loop queue
    DPoint3d    lastStart, lastEnd;
    auto lastCurve = path.back ();
    if (!lastCurve.IsValid() || !lastCurve->GetStartEnd(lastStart, lastEnd))
        return  false;

    // previous curve type & new curve type:
    auto lastType = lastCurve->GetCurvePrimitiveType ();
    auto newType = newCurve.GetCurvePrimitiveType ();

    /*----------------------------------------------------------------------------------------------------------
    Check for potential repetitive elements based on current start-end points and last start-end points.  
    Exclude arcs as they may be a result from a buldge factor (both are arcs).  Exclude different start & end 
    curve type as they may not be true overlaps (a green leaf in TFS 15347).
    ----------------------------------------------------------------------------------------------------------*/
    if (lastType != newType || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == newType)
        return  false;

    if (start.Distance(lastStart) < tol && end.Distance(lastEnd) < tol)
        return  true;

    // check the points in reversed order only after the 2nd edge
    if (edgeNo < 2)
        return  false;

    if (start.Distance(lastEnd) < tol && end.Distance(lastStart) < tol)
        return  true;

    /*----------------------------------------------------------------------------------------------------------
    Before we declare that there is no repetition found between the two curves, we want to check splines for 
    either full overlaping or inclusive, as some hatch loops contain such bad edges that throw our parity code 
    out, the letter m of "mesa" in test file from TFS# 15347 for instance.
    ----------------------------------------------------------------------------------------------------------*/
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == newType)
        {
        DPoint3d    curvePoint;
        double      param = 0.0;
        if (lastCurve->ClosestPointBounded(start, param, curvePoint) && start.Distance(curvePoint) < tol &&
            lastCurve->ClosestPointBounded(end, param, curvePoint) && end.Distance(curvePoint) < tol)
            {
            DPoint3d    midPoints[2];
            if (newCurve.FractionToPoint(0.3, midPoints[0]) && newCurve.FractionToPoint(0.7, midPoints[1]) &&
                lastCurve->ClosestPointBounded(midPoints[0], param, curvePoint) && midPoints[0].Distance(curvePoint) < tol &&
                lastCurve->ClosestPointBounded(midPoints[1], param, curvePoint) && midPoints[1].Distance(curvePoint) < tol)
                {
                // all 4 points from the new curve overlap on last curve, keep the longer one and remove the shorter:
                double  lastLength = 0.0, newLength = 0.0;
                if (!lastCurve->FastLength(lastLength) || !newCurve.FastLength(newLength))
                    return  false;

                if (lastLength > newLength)
                    return  true;

                path.pop_back ();
                }
            }
        }

    /*----------------------------------------------------------------------------------------------------------
    This edge does not overlap with pervious edge, but check if it overlaps two previous adjacent edges as a case
    found in TFS 207477 which would otherwise fail on CloneWithGapsClosed.  To workaround this 3-edge overlapping
    case, check if this new edge folds back and completely overlaps 2 previous edges.  Do this only if all 3 
    edges are linear.
    ----------------------------------------------------------------------------------------------------------*/
    if (edgeNo > 2 && path.size() > 2 && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == newType && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == lastType &&
       (start.Distance(lastStart) < tol || start.Distance(lastEnd) < tol || end.Distance(lastEnd) < tol || end.Distance(lastStart) < tol))
        {
        DVec3d      newVector, lastVector;
        newVector.NormalizedDifference (end, start);
        lastVector.NormalizedDifference (lastEnd, lastStart);
        
        if (fabs(fabs(newVector.DotProduct(lastVector)) - 1.0) < 0.001)
            {
            // get the curve before the last curve
            lastCurve = *(path.end() - 2);
            if (lastCurve.IsValid() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == lastCurve->GetCurvePrimitiveType() &&
                lastCurve->GetStartEnd(lastStart, lastEnd))
                {
                // check the new edge's points to see if either one touchs the edge past two counts:
                if (start.Distance(lastStart) < tol || end.Distance(lastEnd) < tol ||
                    start.Distance(lastEnd) < tol || end.Distance(lastStart) < tol)
                    return  true;
                }
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgHatchExt::IsDangling (DPoint3dCR prevEnd, DPoint3dCR start, DPoint3dCR end, CurveVectorCR edges, size_t edgeNo, double tol) const
    {
    /*------------------------------------------------------------------------------------------------------
    Attempt to trivial reject a dangling edge - an edge with one point laying outside of the boundary loop,
    and the other point sharing with previous end and next start points.  TFS 157668.
    ------------------------------------------------------------------------------------------------------*/
    size_t  numEdges = edges.size ();
    if (numEdges < 4)
        return  false;

    DPoint3d    nextStart, nextEnd;
    size_t  nextEdgeNo = (edgeNo + 1) < numEdges ? (edgeNo + 1) : 0;

    auto checkEdge = edges.at (nextEdgeNo);
    if (!checkEdge.IsValid() || !checkEdge->GetStartEnd(nextStart, nextEnd))
        return  false;

    // check if next edge touches previous edge
    if (prevEnd.Distance(nextStart) < tol || prevEnd.Distance(nextEnd) < tol)
        {
        bool    touchedAtStart = prevEnd.Distance (start) < tol; 
        bool    touchedAtEnd = prevEnd.Distance (end) < tol;

        /*--------------------------------------------------------------------------------------------------
        If one point touches previous point but the other one does not, it is either a dangling edge or an
        edge overlapped with/repetive to next edge.  Reject either case.  Also reject a degenerated edge.
        Do not reject the case in which no points touch - valid edges can be randomly ordered in the edge
        list.
        --------------------------------------------------------------------------------------------------*/
        if (touchedAtStart != touchedAtEnd || touchedAtStart && touchedAtEnd)
            return  true;
        }
    
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
double DwgHatchExt::GetLoopTolerance () const
    {
    double  toleranceInDwg = 1.0e-8;

    DRange3d range;
    if (m_hatch->GetRange(range) == DwgDbStatus::Success)
        toleranceInDwg = 0.001 * range.DiagonalDistance();

    return  toleranceInDwg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgHatchExt::GetFillOrGradientColor (Render::GeometryParams& display) const
    {
    if (m_hatch->IsGradient())
        {
        DwgGiGradientFillPtr    dwgGradient = DwgGiGradientFill::CreateFrom (m_hatch);
        if (!dwgGradient.IsNull())
            {
            auto dgnGradient = GradientSymb::Create ();
            if (dgnGradient.IsValid())
                {
                DwgHelper::GetDgnGradientColor (*dgnGradient, *dwgGradient);
                display.SetGradient (dgnGradient.get());
                }
            }
        }
    else if (m_hatch->IsSolidFill())
        {
        auto dwgColor = m_hatch->GetBackgroundColor ();
        auto colorMethod = dwgColor.GetColorMethod ();

        ColorDef    dgnColor;
        switch (colorMethod)
            {
            case DwgCmEntityColor::Method::ByBlock:
                dgnColor = ColorDef::White ();
                break;
            case DwgCmEntityColor::Method::ByLayer:
                dgnColor = DwgHelper::GetColorDefFromLayer (m_hatch->GetLayerId());
                break;
            case DwgCmEntityColor::Method::ByACI:
                dgnColor = DwgHelper::GetColorDefFromACI (dwgColor.GetIndex());
                break;
            case DwgCmEntityColor::Method::ByColor:
                dgnColor = DwgHelper::GetColorDefFromTrueColor (dwgColor);
                break;
            default:
                // default to boundary color
                dgnColor = display.GetLineColor ();
                break;
            }

        display.SetFillColor (dgnColor);
        }
    else
        {
        BeAssert (false && "Expect only filled or gradient hatch");
        }

    display.SetFillDisplay (FillDisplay::Always);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgHatchExt::GetTransparency (Render::GeometryParams& display) const
    {
    auto layerId = m_hatch->GetLayerId();
    double  transparency = DwgHelper::GetTransparencyFromDwg (m_hatch->GetTransparency(), &layerId, nullptr);
    display.SetTransparency (transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgHatchExt::GetDrawParameters (IDwgDrawParametersR drawParams) const
    {
    // return draw params for the source hatch entity
    drawParams._SetFillType (DwgGiFillType::Always);
    drawParams._SetColor (m_hatch->GetEntityColor());

    if (m_hatch->IsGradient())
        {
        auto  gradient = DwgGiGradientFill::CreateFrom (m_hatch);
        if (!gradient.IsNull())
            drawParams._SetFill (gradient.get());
        }
    else if (m_hatch->IsSolidFill())
        {
        auto fillColor = m_hatch->GetBackgroundColor ();
        if (!fillColor.IsNone())
            drawParams._SetColor (fillColor.GetEntityColor());
        }

    drawParams._SetTransparency (m_hatch->GetTransparency());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgHatchExt::PlaceGeometry (GeometricPrimitiveR geometry, DPoint3dR placementPoint)
    {
    // for a model element, get a placement point and move the shape in reverse:
    if (nullptr != m_toBimContext)
        {
        Transform   moveToOrigin;
        moveToOrigin = m_toBimContext->GetTransform ();
        this->SetPlacementPoint (moveToOrigin, placementPoint);
        geometry.TransformInPlace (moveToOrigin);
        return  BSISUCCESS;
        }
    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgHatchExt::SetPlacementPoint (TransformR transform, DPoint3dR placementPoint)
    {
    // don't bother calculating the placement point when we only create geometry and no element
    if (nullptr == m_toBimContext)
        return  BSISUCCESS;

    // use the placement point from the first grip point, in model coordinates:
    placementPoint = DwgHelper::DefaultPlacementPoint (m_toBimContext->GetEntity());
    m_toBimContext->GetTransform().Multiply (placementPoint);

    // move the body in model to compensate the placement point:
    DPoint3d    translation;
    transform.GetTranslation (translation);

    translation.Subtract (placementPoint);
    transform.SetTranslation (translation);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgHatchExt::CreateElementInModel (GeometricPrimitiveR geometry, DwgImporterR importer)
    {
    // if the hatch is in a model, create an element and save to the model
    if (m_toBimContext == nullptr)
        return  BSIERROR;

    auto& inputs = m_toBimContext->GetElementInputsR ();
    DwgImporter::ElementCreateParams  params(inputs.GetTargetModelR());
    if (importer._GetElementCreateParams (params, inputs.GetTransform(), m_toBimContext->GetEntity()) != BSISUCCESS)
        return  BSIERROR;

    // calculate placement point for the hatch, move geometry as necessary
    DPoint3d    pointInModel;
    if (this->PlaceGeometry(geometry, pointInModel) != BSISUCCESS)
        pointInModel.Zero ();
    
    GeometryBuilderPtr  builder;
    if (params.GetModel().Is3d())
        builder = GeometryBuilder::Create (params.GetModelR(), params.GetCategoryId(), pointInModel);
    else
        builder = GeometryBuilder::Create (params.GetModelR(), params.GetCategoryId(), DPoint2d::From(pointInModel));
    if (!builder.IsValid())
        return  BSIERROR;

    Render::GeometryParams  display;
    display.SetCategoryId (params.GetCategoryId());
    display.SetSubCategoryId (params.GetSubCategoryId());
    display.SetGeometryClass (Render::DgnGeometryClass::Primary);
    display.SetLineColor (DwgHelper::GetColorDefFromEntity(m_toBimContext->GetEntity()));

    this->GetFillOrGradientColor (display);
    this->GetTransparency (display);

    builder->Append (display);
    builder->Append (geometry);

    ElementFactory  factory(m_toBimContext->GetElementResultsR(), m_toBimContext->GetElementInputsR(), params, importer);
    factory.SetGeometryBuilder (builder.get());

    return factory.CreateElement ();
    }
