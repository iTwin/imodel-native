/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SimplifyViewDrawGeom.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnRscFontStructures.h>

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/07
+===============+===============+===============+===============+===============+======*/
struct Dgn::SimplifyDrawUnClippedProcessor
    {
    virtual StatusInt _ProcessUnClipped() { return ERROR; }

    }; // SimplifyDrawUnClippedProcessor

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FacetClipper : PolyfaceQuery::IClipToPlaneSetOutput
{
private:

    SimplifyViewDrawGeom&   m_output;
    ClipVectorCP            m_clip;
    bool                    m_triangulate;
    bool                    m_filled;
    
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
FacetClipper(SimplifyViewDrawGeom& output, bool filled) : m_output(output), m_filled(filled)
    {
    m_clip = output.PerformClip() ? output.GetCurrClip() : NULL;
    m_triangulate = output.GetFacetOptions()->GetMaxPerFace() <= 3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ProcessPolyface(PolyfaceQueryCR polyfaceQuery) 
    {
    if (NULL != m_clip)
        return m_clip->ClipPolyface(polyfaceQuery, *this, m_triangulate);
        
    return OutputPolyface(polyfaceQuery);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ProcessDisposablePolyface(PolyfaceHeaderR polyfaceHeader) 
    {
    return (NULL != m_clip) ? m_clip->ClipPolyface(polyfaceHeader, *this, m_triangulate) : OutputPolyface(polyfaceHeader);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override 
    {
    return OutputPolyface(polyfaceQuery);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override 
    {
    return OutputPolyface(polyfaceHeader);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt OutputPolyface(PolyfaceQueryCR polyfaceQuery)
    {
    return m_output.ProcessGeometryMapOrFacetSet(polyfaceQuery, m_filled);
    }

}; // FacetClipper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void setDefaultFacetOptions(IFacetOptionsP options, double chordTolerance, bool addNormals, bool addParams)
    {
    options->SetMaxPerFace(5000/*MAX_VERTICES*/);
    options->SetChordTolerance(chordTolerance);
    options->SetAngleTolerance(0.25 * Angle::Pi());
    options->SetNormalsRequired(addNormals);
    options->SetParamsRequired(addParams);
    options->SetEdgeChainsRequired(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/07
+---------------+---------------+---------------+---------------+---------------+------*/
SimplifyViewDrawGeom::SimplifyViewDrawGeom(bool addFacetNormals, bool addFacetParams)
    {
    m_context = NULL;

    m_viewFlags.InitDefaults();
    m_viewFlags.styles = false; // don't want linestyles for range calculation - they're added later.

    m_defaultFacetOptions = IFacetOptions::New();
    setDefaultFacetOptions(m_defaultFacetOptions.get(), 0.0, addFacetNormals, addFacetParams);

    m_overrideMatSymb.SetFlags(MATSYMB_OVERRIDE_None);

    m_inPatternDraw   = false;
    m_inSymbolDraw    = false;
    m_inTextDraw      = false;
    m_inThicknessDraw = false;
    m_elementTransformStackIndex = 0;

    m_processingMaterialGeometryMap = false;
    m_elementTransformStackIndex = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorCP SimplifyViewDrawGeom::GetCurrClip() 
    {
    if (NULL == m_context)
        {
        BeAssert(false);
        return NULL;

        }
    return m_context->GetTransformClipStack().GetDrawGeomClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SimplifyViewDrawGeom::ProcessGeometryMapOrFacetSet(PolyfaceQueryCR polyfaceQuery, bool filled)
    {
#ifdef NEEDS_WORK_GEOMETRY_MAPS
    if (SUCCESS != ProcessGeometryMap(polyfaceQuery) &&
        SUCCESS == ProcessTextureOutlines(polyfaceQuery))
        return SUCCESS;
#endif

    return ProcessFacetSet(polyfaceQuery, filled);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyViewDrawGeom::PerformClip()
    {
    return _DoClipping() && NULL != GetCurrClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IPolyfaceConstructionPtr SimplifyViewDrawGeom::GetPolyfaceBuilder()
    {
    _GetFacetOptions()->SetToleranceDistanceScale(1.0 / m_context->GetTransformClipStack().GetTransformScale());
    return IPolyfaceConstruction::New(*GetFacetOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void processCurvePrimitives(SimplifyViewDrawGeom& drawGeom, CurveVectorCR curves, bool filled)
    {
    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                {
                BeAssert(true && "Unexpected entry in region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            processCurvePrimitives(drawGeom, *curve->GetChildCurveVectorCP (), filled && curves.IsUnionRegion()); // Don't pass filled when spewing parity region loops...
            }
        }
    else
        {
        bool    isSingleEntry = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive());
        bool    isClosed = curves.IsClosedPath();
        bool    isOpen = curves.IsOpenPath();
        bool    isComplex = ((isClosed || isOpen) && !isSingleEntry);

        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            if (SUCCESS == drawGeom.ProcessCurvePrimitive(*curve, !isComplex && isClosed, !isComplex && filled)) // Don't pass filled when spewing primitives...
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType())
                processCurvePrimitives(drawGeom, *curve->GetChildCurveVectorCP (), filled);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* points)
    {
    BeAssert(numX==1 && numY==1 && "TBD: march over tiles");

    DPoint3d    shapePoints[5];

    shapePoints[0] = shapePoints[4] = points[0];
    shapePoints[1] = points[1];
    shapePoints[2] = points[2];
    shapePoints[3] = points[3];

    _DrawShape3d(5, shapePoints, true, NULL);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SimplifyViewDrawGeom::CurveVectorOutputProcessor(CurveVectorCR curves, bool filled)
    {
    if (1 > curves.size() || SUCCESS == _ProcessCurveVector(curves, filled))
        return SUCCESS;

    if (_ProcessAsStrokes(curves.ContainsNonLinearPrimitive()))
        {
        if (curves.IsAnyRegionType() && _ProcessAsFacets(false))
            {
            IPolyfaceConstructionPtr  builder = GetPolyfaceBuilder();

            builder->AddRegion(curves);

            return (BentleyStatus) ProcessGeometryMapOrFacetSet(builder->GetClientMeshR (), filled);
            }

        bvector<DPoint3d>  points;

        curves.AddStrokePoints(points, *_GetFacetOptions());

        return (BentleyStatus) _ProcessLinearSegments(&points.front(), points.size(), curves.IsAnyRegionType(), filled);
        }

    processCurvePrimitives(*this, curves, filled);

    return SUCCESS;
    }

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct IntersectLocationDetail
    {
    size_t      m_index;
    double      m_fraction;

    IntersectLocationDetail(size_t index, double fraction) {m_index = index; m_fraction = fraction;}
    };
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int compareCurveIntersections(IntersectLocationDetail const* detail0, IntersectLocationDetail const* detail1)
    {
    if (detail0->m_index < detail1->m_index)
        {
        return -1;
        }
    else if (detail0->m_index > detail1->m_index)
        {
        return 1;
        }
    else
        {
        if (detail0->m_fraction < detail1->m_fraction)
            return -1;
        else if (detail0->m_fraction > detail1->m_fraction)
            return 1;
        else
            return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool computeInteriorPoint(DPoint3dR midPoint, CurveVectorCR curves, IntersectLocationDetail& startDetail, IntersectLocationDetail& endDetail)
    {
    size_t      midIndex;
    double      midFraction;

    if (startDetail.m_index == endDetail.m_index)
        {
        midIndex = startDetail.m_index;
        midFraction = (startDetail.m_fraction + endDetail.m_fraction) / 2.0;
        }
    else if (endDetail.m_index > startDetail.m_index+1)
        {
        midIndex = startDetail.m_index+1;
        midFraction = 0.5;
        }
    else
        {
        midIndex = endDetail.m_index;
        midFraction = 0.0;
        }

   return curves.at(midIndex)->FractionToPoint(midFraction, midPoint);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool clipAsOpenCurveVector(CurveVectorCR curves, ClipVectorCR clip, SimplifyViewDrawGeom* drawGeom)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        bool    clipped = false;

        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsNull() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType())
                clipped |= clipAsOpenCurveVector(*curve->GetChildCurveVectorCP (), clip, drawGeom);
            }

        return clipped;
        }

    DRange3d  curveRange;
                             
    if (!curves.GetRange(curveRange))
        return false;

    curveRange.Extend(1.0); // UORs.   

    bvector<IntersectLocationDetail> intersectDetails;

    for (ClipPrimitivePtr const& thisClip: clip)
        {
        DRange3d  clipRange;                                                                                             

        // Optimization for TR#300934. Don't intersect with planes from clips that are disjoint from the current GPA.
        if (thisClip->GetRange(clipRange, NULL, true) && !curveRange.IntersectsWith(clipRange))
            continue;

        ClipPlaneSetCP      clipPlaneSet;

        if (NULL != (clipPlaneSet = thisClip->GetMaskOrClipPlanes()))       // Use mask planes if they exist.
            {
            for (ConvexClipPlaneSetCR convexPlaneSet: *clipPlaneSet)
                {
                for (ClipPlaneCR plane: convexPlaneSet)
                    {
                    // NOTE: It would seem that it would not be necessary to calculate intersections with "interior" planes. However, we use 
                    //       that designation to mean any plane that should not generate cut geometry so "interior" is not really correct
                    //       and we need to intersect with these planes as well. Interior intersections do not really cause a problem as we 
                    //       discard them below if insidedness does not change. (RayB TR#244943)
                    bvector<CurveLocationDetailPair> intersections;
                
                    curves.AppendCurvePlaneIntersections(plane.GetDPlane3d(), intersections); // NOTE: Method calls clear in output vector!!!

                    // Get curve index for sorting, can disregard 2nd detail in pair as both should be identical...
                    for (CurveLocationDetailPair pair: intersections)
                        intersectDetails.push_back(IntersectLocationDetail(curves.CurveLocationDetailIndex(pair.detailA), pair.detailA.fraction));
                    }
                }
            }
        }

    if (0 == intersectDetails.size())
        {
        DPoint3d  testPoint;

        if (!curves.front()->FractionToPoint(0.5, testPoint))
            return false;

        return !clip.PointInside(testPoint, 1.0e-5);
        }

    qsort(&intersectDetails.front(), intersectDetails.size(), sizeof (IntersectLocationDetail), (int (*) (void const*, void const*)) compareCurveIntersections);

    // Add final point for last curve...
    intersectDetails.push_back(IntersectLocationDetail(curves.size()-1, 1.0));

    bool lastInside = false;
    IntersectLocationDetail insideStartDetail(0, 0.0);
    IntersectLocationDetail lastDetail(0, 0.0);

    for (IntersectLocationDetail thisDetail: intersectDetails)
        {
        if (thisDetail.m_index == lastDetail.m_index && (thisDetail.m_fraction - lastDetail.m_fraction) < 1.0e-4)
            continue;

        bool        thisInside = false;
        DPoint3d    midPoint;

        if (computeInteriorPoint(midPoint, curves, lastDetail, thisDetail))
            thisInside = clip.PointInside(midPoint, 1.0e-5);

        if (thisInside)
            {
            if (!lastInside)
                insideStartDetail = lastDetail;
            }
        else
            {
            if (lastInside)
                {
                CurveVectorPtr  partialCurve = curves.CloneBetweenDirectedFractions((int) insideStartDetail.m_index, insideStartDetail.m_fraction, (int) lastDetail.m_index, lastDetail.m_fraction, false);

                drawGeom->CurveVectorOutputProcessor(*partialCurve, false);
                }
            }

        lastDetail = thisDetail;
        lastInside = thisInside;
        }

    if (lastInside)
        {
        CurveVectorPtr  partialCurve = curves.CloneBetweenDirectedFractions((int) insideStartDetail.m_index, insideStartDetail.m_fraction, (int) lastDetail.m_index, lastDetail.m_fraction, false);
        
        drawGeom->CurveVectorOutputProcessor(*partialCurve, false);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool clipPointStringCurvePrimitive(ICurvePrimitiveCR primtive, ClipVectorCR clip, SimplifyViewDrawGeom* drawGeom)
    {
    bvector<DPoint3d> const* points = primtive.GetPointStringCP ();
    bvector<DPoint3d>        insidePts;

    for (size_t iPt = 0; iPt < points->size(); ++iPt)
        {
        if (!clip.PointInside(points->at(iPt), 1.0E-5))
            continue;

        insidePts.push_back(points->at(iPt));
        }

    if (insidePts.empty() || insidePts.size() == points->size())
        return false;

    CurveVectorPtr  tmpCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    
    tmpCurve->push_back(ICurvePrimitive::CreatePointString(&insidePts.front(), insidePts.size()));
    drawGeom->CurveVectorOutputProcessor(*tmpCurve, false);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool clipUnclassifiedCurveVector(CurveVectorCR curves, ClipVectorCR clip, SimplifyViewDrawGeom* drawGeom)
    {
    if (1 > curves.size())
        return false;

    bool    clipped = false;

    for (ICurvePrimitivePtr curve: curves)
        {
        if (curve.IsNull())
            continue;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                {
                clipped |= clipPointStringCurvePrimitive(*curve, clip, drawGeom);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                clipped |= clipAsOpenCurveVector(*curve->GetChildCurveVectorCP (), clip, drawGeom);
                break;
                }

            default:
                {
                clipped |= clipAsOpenCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), clip, drawGeom);
                break;
                }
            }
        }

    return clipped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessCurveVector(CurveVectorCR curves, bool filled)
    {
    CurveTopologyId::AddCurveVectorIds(curves, CurvePrimitiveId::Type_CurveVector, CurveTopologyId::FromCurveVector(), nullptr);

    if (!PerformClip())
        {
        CurveVectorOutputProcessor(curves, filled);
        return;
        }

    if (curves.IsAnyRegionType())
        {
        bool        containsCurves = curves.ContainsNonLinearPrimitive();

        if (_ProcessAsFacets(containsCurves) && _ProcessAsStrokes(containsCurves))
            {
            IPolyfaceConstructionPtr  builder = GetPolyfaceBuilder();

            builder->AddRegion(curves);
            FacetClipper(*this, filled).ProcessDisposablePolyface(builder->GetClientMeshR ());
            return;
            }
        else if (_ClipPreservesRegions())
            {
            bvector<CurveVectorPtr> insideCurves;

            if (SUCCESS == T_HOST.GetSolidsKernelAdmin()._ClipCurveVector(insideCurves, curves, *GetCurrClip(), m_context->GetCurrLocalToWorldTransformCP()))
                {
                for (CurveVectorPtr tmpCurves: insideCurves)
                    CurveVectorOutputProcessor(*tmpCurves, filled);

                return;
                }
            }
         }

    if (CurveVector::BOUNDARY_TYPE_None == curves.GetBoundaryType())
        {
        if (clipUnclassifiedCurveVector(curves, *GetCurrClip(), this))
            return;
        }
    else
        {
        if (clipAsOpenCurveVector(curves, *GetCurrClip(), this))
            return;
        }

    CurveVectorOutputProcessor(curves, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessFacetSet(PolyfaceQueryCR facets, bool filled)
    {
    FacetClipper(*this, filled).ProcessPolyface(facets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessBodyAsFacets(ISolidKernelEntityCR entity)
    {
    IFacetTopologyTablePtr  facetsPtr;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._FacetBody(facetsPtr, entity, *_GetFacetOptions()))
        return;

    T_FaceAttachmentsVec const* faceAttachmentsVec = (_GetFacetOptions()->GetIgnoreFaceMaterialAttachments() ? nullptr : facetsPtr->_GetFaceAttachmentsVec());

    if (faceAttachmentsVec)
        {
        bvector<PolyfaceHeaderPtr>             polyfaces;
        bmap<FaceAttachment, PolyfaceHeaderCP> uniqueFaceAttachments;
        bmap<int, PolyfaceHeaderCP>            faceToPolyfaces;
        T_FaceToSubElemIdMap const*            faceToSubElemIdMap = facetsPtr->_GetFaceToSubElemIdMap();

        for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap->begin(); curr != faceToSubElemIdMap->end(); ++curr)
            {
            FaceAttachment faceAttachment = faceAttachmentsVec->at(curr->second.second);
            bmap<FaceAttachment, PolyfaceHeaderCP>::iterator found = uniqueFaceAttachments.find(faceAttachment);

            if (found == uniqueFaceAttachments.end())
                {
                PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

                polyfaces.push_back(polyface);
                faceToPolyfaces[curr->first] = uniqueFaceAttachments[faceAttachment] = polyface.get();
                }
            else
                {
                faceToPolyfaces[curr->first] = found->second;
                }
            }

        if (SUCCESS == IFacetTopologyTable::ConvertToPolyfaces(polyfaces, faceToPolyfaces, *facetsPtr, *_GetFacetOptions()))
            {
            m_context->PushTransform(entity.GetEntityTransform());

            for (size_t i=0; i<polyfaces.size(); i++)
                {
                polyfaces[i]->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
                faceAttachmentsVec->at(i).ToElemDisplayParams(m_context->GetCurrentDisplayParams());
                m_context->CookDisplayParams();

                FacetClipper(*this, false).ProcessDisposablePolyface(*polyfaces[i]);
                }

            m_context->PopTransformClip();
            }

        return;
        }

    PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*polyface, *facetsPtr, *_GetFacetOptions()))
        return;

    polyface->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());

    m_context->PushTransform(entity.GetEntityTransform());
    FacetClipper(*this, false).ProcessDisposablePolyface(*polyface);
    m_context->PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessBody(ISolidKernelEntityCR entity, SimplifyDrawUnClippedProcessor* unclippedOutputProcessor)
    {
    if (_ProcessAsBody(T_HOST.GetSolidsKernelAdmin()._QueryEntityData(entity, DgnPlatformLib::Host::SolidsKernelAdmin::EntityQuery_HasCurvedFaceOrEdge)))
        {
        if (!PerformClip())
            {
            _ProcessBody(entity);
            }
        else
            {
            bool                           clipped;
            bvector<ISolidKernelEntityPtr> clippedBodies;

            T_HOST.GetSolidsKernelAdmin()._ClipBody(clippedBodies, clipped, entity, *GetCurrClip());

            if (clipped || NULL == unclippedOutputProcessor || SUCCESS != unclippedOutputProcessor->_ProcessUnClipped())
                {
                for (ISolidKernelEntityPtr entityOut: clippedBodies)
                    _ProcessBody(*entityOut);
                }
            }
        }
    else if (_ProcessAsFacets(false))
        {
        ClipAndProcessBodyAsFacets(entity);
        }
    else if (_ProcessAsWireframe())
        {
        WireframeGeomUtil::Draw(entity, *m_context);
        }    
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/07
+===============+===============+===============+===============+===============+======*/
struct UnClippedSurfaceProcessor : SimplifyDrawUnClippedProcessor
{
SimplifyViewDrawGeom*   m_drawGeom;
MSBsplineSurfaceCR      m_surface;

UnClippedSurfaceProcessor(SimplifyViewDrawGeom* drawGeom, MSBsplineSurfaceCR surface) : m_drawGeom(drawGeom), m_surface(surface) {}
                                                                                                        
virtual StatusInt _ProcessUnClipped() override {return m_drawGeom->ProcessSurface(m_surface);}

}; // UnClippedSurfaceProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessSurface(MSBsplineSurfaceCR surface)
    {
    // Give output a chance to handle un-clipped geometry directly...
    if (!PerformClip() && SUCCESS == _ProcessSurface(surface))
        return;

    ISolidKernelEntityPtr   entityPtr;
    bool                    processAsFacets = _ProcessAsFacets(false);

    // Parasolid is expensive - if it is planar bilinear, send it through as facets which will represent exactly.
    if ((!surface.IsPlanarBilinear() || !processAsFacets) && _ProcessAsBody(true) && 
        SUCCESS == T_HOST.GetSolidsKernelAdmin()._CreateBodyFromBSurface(entityPtr, surface))
        {
        UnClippedSurfaceProcessor proc(this, surface);

        ClipAndProcessBody(*entityPtr.get(), &proc);
        }
    else if (processAsFacets)
        {
        IPolyfaceConstructionPtr  builder = GetPolyfaceBuilder();

        builder->Add(surface);
        FacetClipper(*this, false).ProcessDisposablePolyface(builder->GetClientMeshR ());
        }
    else if (_ProcessAsWireframe())
        {
        WireframeGeomUtil::Draw(surface, *m_context);
        }
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/07
+===============+===============+===============+===============+===============+======*/
struct UnClippedSolidPrimitiveProcessor : SimplifyDrawUnClippedProcessor
{
SimplifyViewDrawGeom&   m_drawGeom;
ISolidPrimitiveCR       m_primitive;

UnClippedSolidPrimitiveProcessor(SimplifyViewDrawGeom& drawGeom, ISolidPrimitiveCR primitive) : m_drawGeom(drawGeom), m_primitive(primitive) {}

virtual StatusInt _ProcessUnClipped() override {return m_drawGeom.ProcessSolidPrimitive(m_primitive);}

}; // UnClippedSolidPrimitiveProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    // Give output a chance to handle un-clipped geometry directly...
    if (!PerformClip() && SUCCESS == ProcessSolidPrimitive(primitive))
        return;

    ISolidKernelEntityPtr  entityPtr;

    if (_ProcessAsBody(primitive.HasCurvedFaceOrEdge()) && SUCCESS == T_HOST.GetSolidsKernelAdmin()._CreateBodyFromSolidPrimitive(entityPtr, primitive))
        {
        UnClippedSolidPrimitiveProcessor  proc(*this, primitive);

        ClipAndProcessBody(*entityPtr.get(), &proc);
        }
    else if (_ProcessAsFacets(false))
        {
        IPolyfaceConstructionPtr  builder = GetPolyfaceBuilder();

        builder->AddSolidPrimitive(primitive);
        FacetClipper(*this, false).ProcessDisposablePolyface(builder->GetClientMeshR ());
        }
    else if (_ProcessAsWireframe())
        {
        WireframeGeomUtil::Draw(primitive, *m_context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool mightHaveHole(GPArrayCP gpa)
    {
    size_t  loopCount = 0;

    for (int loopStart = 0, loopEnd = 0; loopStart < gpa->GetCount(); loopStart = loopEnd+1)
        {
        GraphicsPoint const* gPt = gpa->GetConstPtr(loopStart);

        if (gPt && (HMASK_RSC_HOLE == (gPt->mask & HMASK_RSC_HOLE))) // NOTE: Mask value weirdness, hole shares bits with line/poly...
            return true;

        loopEnd = jmdlGraphicsPointArray_findMajorBreakAfter(gpa, loopStart);
        loopCount++;
        }

    return loopCount > 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPhysicallyClosed(ICurvePrimitiveCR primitive)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP ()->size() > 3 && primitive.GetLineStringCP ()->front().IsEqual(primitive.GetLineStringCP ()->back()));

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return primitive.GetArcCP ()->IsFullEllipse();

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return (primitive.GetProxyBsplineCurveCP ()->IsClosed());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessGlyph(DgnFontCR font, DgnGlyphCR glyph, DPoint3dCR glyphOffset)
    {
    GPArraySmartP  gpaText;

    if (SUCCESS != glyph.FillGpa(gpaText) || 0 == gpaText->GetCount())
        return;

    Transform   offsetTrans;

    offsetTrans.InitFrom(glyphOffset);

    DVec3d      zVec;
    DPoint3d    origin;
    Transform   scaledTrans, compoundTrans;

    zVec.Init(0.0, 0.0, 1.0);
    origin.Init(0.0, 0.0, 0.0);
    scaledTrans.InitFromOriginAndVectors(origin, m_textAxes[0], m_textAxes[1], zVec);
    compoundTrans.InitProduct(offsetTrans, scaledTrans);

    bool  isFilled = (0 != gpaText->GetArrayMask(HPOINT_ARRAYMASK_FILL));
    bool  isRscWithPossibleHoles = (isFilled && DgnFontType::Rsc == font.GetType() && mightHaveHole(gpaText));

    if (DgnFontType::TrueType == font.GetType() || isRscWithPossibleHoles)
        {
        CurveVectorPtr  curves = gpaText->CreateCurveVector();

        if (!curves.IsValid())
            return;
            
        curves->ConsolidateAdjacentPrimitives();
        curves->FixupXYOuterInner();
        curves->TransformInPlace(compoundTrans);

        ClipAndProcessCurveVector(*curves, isFilled);
        return;
        }

    // Create curve vector that is just a collection of curves and not an open/closed path or region...
    gpaText->Transform(&compoundTrans);

    BentleyStatus            status = SUCCESS;
    bvector<CurveVectorPtr>  glyphCurves;

    for (int i=0, count = gpaText->GetCount(); i < count && SUCCESS == status; )
        {
        bool                isPoly = (DgnFontType::Rsc == font.GetType() && 0 != (gpaText->GetConstPtr(i)->mask & HMASK_RSC_POLY));
        ICurvePrimitivePtr  primitive;

        switch (gpaText->GetCurveType(i))
            {
            case GPCurveType::LineString:
                {
                bvector<DPoint3d> points;

                if (SUCCESS != (status = gpaText->GetLineString(&i, points)))
                    break;

                primitive = ICurvePrimitive::CreateLineString(points);
                break;
                }

            case GPCurveType::Ellipse:
                {
                DEllipse3d  ellipse;

                if (SUCCESS != (status = gpaText->GetEllipse(&i, &ellipse)))
                    break;

                primitive = ICurvePrimitive::CreateArc(ellipse);
                break;
                }

            case GPCurveType::Bezier:
            case GPCurveType::BSpline:
                {
                MSBsplineCurve  bcurve;

                if (SUCCESS != (status = gpaText->GetBCurve(&i, &bcurve)))
                    break;

                primitive = ICurvePrimitive::CreateBsplineCurve(bcurve);
                bcurve.ReleaseMem();
                break;
                }

            default:
                {
                i++;
                break;
                }
            }

        if (!primitive.IsValid())
            continue;

        CurveVectorPtr  singleCurve = CurveVector::Create((isPoly && isPhysicallyClosed(*primitive)) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);

        singleCurve->push_back(primitive);
        glyphCurves.push_back(singleCurve);
        }

    size_t  nGlyphCurves = glyphCurves.size();

    if (0 == nGlyphCurves)
        return; // Empty glyph?!?

    CurveVectorPtr  curves;

    if (1 == nGlyphCurves)
        {
        curves = glyphCurves.front(); // NOTE: Glyph fill flag should be set correctly for this case...
        }
    else
        {
        size_t  nClosed = 0, nOpen = 0;

        for (CurveVectorPtr singleCurve: glyphCurves)
            {
            if (singleCurve->IsClosedPath())
                nClosed++;
            else
                nOpen++;
            }

        // NOTE: Create union region if all closed, create none for all open or mix...
        curves = CurveVector::Create((nClosed == nGlyphCurves) ? CurveVector::BOUNDARY_TYPE_UnionRegion : CurveVector::BOUNDARY_TYPE_None);

        for (CurveVectorPtr singleCurve: glyphCurves)
            {
            if (nOpen == nGlyphCurves)
                curves->push_back(singleCurve->front()); // NOTE: Flatten hierarchy, better to not have child vectors for disjoint collection of sticks...
            else
                curves->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*singleCurve));
            }

        if (0 != nClosed && 0 != nOpen)
            isFilled = true; // NOTE: Poly in mixed glyph treated as filled but glyph fill flag isn't set...
        }

    ClipAndProcessCurveVector(*curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessText(TextStringCR text, double* zDepth)
    {
    Transform drawTrans = text.ComputeTransform();
    m_context->PushTransform(drawTrans);

    // NOTE: Need text axes to compute gpa transform in _OnGlyphAnnounced...
    text.ComputeGlyphAxes(m_textAxes[0], m_textAxes[1]);
    
    DgnFontCR font = text.GetStyle().GetFont();
    auto numGlyphs = text.GetNumGlyphs();
    DgnGlyphCP const* glyphs = text.GetGlyphs();
    DPoint3dCP glyphOrigins = text.GetGlyphOrigins();

    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        ClipAndProcessGlyph(font, *glyphs[iGlyph], glyphOrigins[iGlyph]);

    m_context->PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessSymbol(IDisplaySymbol* symbolDefP, TransformCP transP, ClipPlaneSetP clipPlaneSetP, bool ignoreColor, bool ignoreWeight)
    {
    if (!_DoSymbolGeometry())
        return;

#ifdef POTENTIAL_OPTIMIZATION_FROM_SS3
    DRange3d    range;

    DataConvert::ScanRangeToDRange3d(range, edP->el.hdr.dhdr.range);

    // An optimization for patterns. - Any mask that is not in range of the current tile can not
    // effect the clip  - so cull these from the current mask.   We don't need to copy or restore
    // as the clip is popped below.   (TR# 300934).
    m_clipStack->CullDisjointMasks(range);
#endif

    BeAssert(!m_inSymbolDraw); // Can't have nested symbols...

    AutoRestore <bool> saveInSymbolDraw(&m_inSymbolDraw, true);
    AutoRestore <ElemMatSymb> saveOutputElemMatSymb(&m_currentMatSymb);
    AutoRestore <OvrMatSymb> saveOutputOvrMatSymb(&m_overrideMatSymb);
    AutoRestore <ElemMatSymb> saveContextElemMatSymb(m_context->GetElemMatSymb());
    AutoRestore <OvrMatSymb> saveContextOvrMatSymb(m_context->GetOverrideMatSymb());
    AutoRestore <ElemDisplayParams> saveContextDisplayParams(&m_context->GetCurrentDisplayParams());

#if defined (NEEDS_WORK_DGNITEM)
    m_context->GetDisplayParamsIgnores().Set(*m_context->GetCurrentDisplayParams(), true, ignoreColor, ignoreWeight); // NOTE: Symbol level is always inherited from base element...
#endif

    if (NULL != clipPlaneSetP)
        m_context->PushClipPlanes(*clipPlaneSetP);

    if (NULL != transP)
        m_context->PushTransform(*transP);

    symbolDefP->_Draw(*m_context);

    if (NULL != transP)
        m_context->PopTransformClip();

    if (NULL != clipPlaneSetP)
        m_context->PopTransformClip();

#if defined (NEEDS_WORK_DGNITEM)
    m_context->GetDisplayParamsIgnores().Clear();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double zDepth)
    {
    for (int i=0; i<numPoints; i++, pts3d++, pts2d++)
        pts3d->Init(pts2d->x, pts2d->y, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range)
    {
    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    
    curve->push_back(ICurvePrimitive::CreateLineString(points, numPoints));
    ClipAndProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, 0.0);
    _DrawLineString3d(numPoints, &localPointsBuf3d[0], NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range)
    {
    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    curve->push_back(ICurvePrimitive::CreatePointString(points, numPoints));
    ClipAndProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, 0.0);
    _DrawPointString3d(numPoints, &localPointsBuf3d[0], NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range)
    {
    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    curve->push_back(ICurvePrimitive::CreateLineString(points, numPoints));
    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, 0.0);
    _DrawShape3d(numPoints, &localPointsBuf3d[0], filled, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range)
    {
    if (1 == usageFlags) // represents thickened line...
        {
        int         nPt = 0;
        DPoint3dP   tmpPtsP = (DPoint3dP) _alloca((numPoints+1) * sizeof (DPoint3d));

        for (int iPtS1=0; iPtS1 < numPoints; iPtS1 = iPtS1+2)
            tmpPtsP[nPt++] = points[iPtS1];

        for (int iPtS2=numPoints-1; iPtS2 > 0; iPtS2 = iPtS2-2)
            tmpPtsP[nPt++] = points[iPtS2];

        tmpPtsP[nPt] = tmpPtsP[0]; // Add closure point...simplifies drop of extrude thickness...

        _DrawShape3d(numPoints+1, tmpPtsP, true, NULL);
        return;
        }

    // spew triangles
    for (int iPt=0; iPt < numPoints-2; iPt++)
        _DrawShape3d(3, &points[iPt], true, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range)
    {
    std::valarray<DPoint3d>  localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, 0.0);
    _DrawTriStrip3d(numPoints, &localPointsBuf3d[0], usageFlags, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range)
    {
    // NOTE: QVis closes arc ends and displays them filled (see outputCapArc for linestyle strokes)...
    CurveVectorPtr  curve = CurveVector::Create((isEllipse || filled) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    
    curve->push_back(ICurvePrimitive::CreateArc(ellipse));

    if (filled && !isEllipse && !ellipse.IsFullEllipse())
        {
        DSegment3d          segment;
        ICurvePrimitivePtr  gapSegment;

        ellipse.EvaluateEndPoints(segment.point[1], segment.point[0]);
        gapSegment = ICurvePrimitive::CreateLine(segment);
        gapSegment->SetMarkerBit(ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);

        curve->push_back(gapSegment);
        }

    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range)
    {
    _DrawArc3d(ellipse, isEllipse, filled, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    CurveVectorPtr  curve = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);

    curve->push_back(ICurvePrimitive::CreateBsplineCurve(bcurve));
    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double zDepth)
    {
    _DrawBSplineCurve(bcurve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawCurveVector(CurveVectorCR curves, bool isFilled)
    {
    ClipAndProcessCurveVector(curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth)
    {
    _DrawCurveVector(curves, isFilled); // Ignore zDepth...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    ClipAndProcessSolidPrimitive(primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawBSplineSurface(MSBsplineSurfaceCR surface)
    {
    ClipAndProcessSurface(surface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::ClipAndProcessFacetSetAsCurves(PolyfaceQueryCR meshData)
    {
    int const*  vertIndex   = meshData.GetPointIndexCP();
    size_t      numIndices  = meshData.GetPointIndexCount();
    DPoint3dCP  verts       = meshData.GetPointCP();
    int         polySize    = meshData.GetNumPerFace();
    int         thisIndex, prevIndex=0, firstIndex=0;
    size_t      thisFaceSize = 0;

    if (!vertIndex)
        return;

    for (size_t readIndex = 0; readIndex < numIndices; readIndex++)
        {    
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            // remember first index in this face loop
            if (!thisFaceSize)
                firstIndex = thisIndex;

            // draw visible edge (prevIndex, thisIndex)
            else if (prevIndex > 0)
                {
                int                 closeVertexId = (abs(prevIndex) - 1);
                int                 segmentVertexId = (abs(thisIndex) - 1);
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));
                CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type_PolyfaceEdge, CurveTopologyId(CurveTopologyId::Type_PolyfaceEdge, closeVertexId, segmentVertexId), nullptr);

                curve->SetId(newId.get());
                ClipAndProcessCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }

            prevIndex = thisIndex;
            thisFaceSize++;
            }

        // found end of face loop (found first pad/terminator or last index in fixed block)
        if (thisFaceSize && (!thisIndex || (polySize > 1 && polySize == thisFaceSize)))
            {
            // draw last visible edge (prevIndex, firstIndex)
            if (prevIndex > 0)
                {
                int                 closeVertexId = (abs(prevIndex) - 1);
                int                 segmentVertexId = (abs(firstIndex) - 1);
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));
                CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type_PolyfaceEdge, CurveTopologyId(CurveTopologyId::Type_PolyfaceEdge, closeVertexId, segmentVertexId), nullptr);

                curve->SetId(newId.get());
                ClipAndProcessCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }

            thisFaceSize = 0;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawPolyface(PolyfaceQueryCR meshData, bool filled)
    {
    if (_ProcessAsFacets(true))
        {
        size_t  maxPerFace;

        if ((GetFacetOptions()->GetNormalsRequired() && 0 == meshData.GetNormalCount()) ||
            (GetFacetOptions()->GetParamsRequired() && (0 == meshData.GetParamCount() || 0 == meshData.GetFaceCount())) ||
            (GetFacetOptions()->GetEdgeChainsRequired() && 0 == meshData.GetEdgeChainCount()) ||
            (GetFacetOptions()->GetConvexFacetsRequired() && !meshData.HasConvexFacets()) ||
            (meshData.GetNumFacet(maxPerFace)  > 0 && (int) maxPerFace > GetFacetOptions()->GetMaxPerFace()))
            {
            IPolyfaceConstructionPtr  builder = PolyfaceConstruction::New(*GetFacetOptions());

            builder->AddPolyface(meshData);
            FacetClipper(*this, filled).ProcessDisposablePolyface(builder->GetClientMeshR ());
            }
        else
            {
            ClipAndProcessFacetSet(meshData, filled);
            }

        return;
        }

    if (!_ProcessAsWireframe())
        return;

    ClipAndProcessFacetSetAsCurves(meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyViewDrawGeom::_DrawBody(ISolidKernelEntityCR entity, double pixelSize)
    {
    ClipAndProcessBody(entity, NULL);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawTextString(TextStringCR text, double* zDepth)
    {
    AutoRestore <bool>  saveInTextDraw(&m_inTextDraw, true);

    if (!_DoTextGeometry())
        {
        if (text.GetText().empty())
            return;
        
        DPoint3d points[5];
        text.ComputeBoundingShape(points);
        text.ComputeTransform().Multiply(points, _countof(points));

        _DrawShape3d(5, points, false, NULL);
        return;
        }

    ClipAndProcessText(text, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range)
    {
    DPoint3d    shapePoints[5];

    shapePoints[0] = shapePoints[4] = points[0];
    shapePoints[1] = points[1];
    shapePoints[2] = points[2];
    shapePoints[3] = points[3];

    _DrawShape3d(5, shapePoints, true, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range)
    {
    std::valarray<DPoint3d> localPointsBuf3d(4);

    copy2dTo3d(4, &localPointsBuf3d[0], points, 0.0);
    _DrawRaster(&localPointsBuf3d[0], pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawDgnOle(DgnOleDraw* ole)
    {

    // NEEDSWORK...Draw Shape...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_DrawPointCloud(PointCloudDraw* drawParams)
    {
    enum {MAX_POINTS_PER_BATCH = 300};

    uint32_t        numPoints = drawParams->GetNumPoints();

    if (0 == numPoints)
        return;

    DPoint3d    offsets;

    offsets.Init(0, 0, 0);

    bool        haveOffsets = drawParams->GetOrigin(&offsets);
    DPoint3dCP  dPoints = drawParams->GetDPoints();

    if (NULL != dPoints)
        {
        //  QVision does not support DPoint3d with offsets so QvOutput does not support it and there
        //  is no need to support it here.
        BeAssert(!haveOffsets);

        while (numPoints > 0)
            {
            if (m_context->CheckStop())
                return;
            
            uint32_t pointsThisIter = numPoints > MAX_POINTS_PER_BATCH ? MAX_POINTS_PER_BATCH: numPoints;

            _DrawPointString3d(pointsThisIter, dPoints, NULL);
            numPoints -= pointsThisIter;
            dPoints   += pointsThisIter;
            }

        return;
        }

    // Don't risk stack overflow to get points buffer
    uint32_t maxPointsPerIter = MAX_POINTS_PER_BATCH;

    if (numPoints < maxPointsPerIter)
        maxPointsPerIter = numPoints;

    DPoint3dP   pointBuffer = (DPoint3dP)_alloca(maxPointsPerIter * sizeof (*pointBuffer));

    // Convert float points to DPoints and add offset
    FPoint3dCP fPoints = drawParams->GetFPoints();
    FPoint3dCP currIn = fPoints;

    while (numPoints > 0)
        {
        if (m_context->CheckStop())
            return;

        uint32_t pointsThisIter = numPoints > maxPointsPerIter ? maxPointsPerIter : numPoints;

        for (DPoint3dP  curr = pointBuffer; curr < pointBuffer + pointsThisIter; curr++, currIn++)
            {
            curr->x = currIn->x + offsets.x;
            curr->y = currIn->y + offsets.y;
            curr->z = currIn->z + offsets.z;
            }

        _DrawPointString3d(pointsThisIter, pointBuffer, NULL);
        numPoints -= pointsThisIter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_ActivateMatSymb(ElemMatSymbCP matSymb)
    {
    BeAssert(m_context);

    m_currentMatSymb = *matSymb;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyViewDrawGeom::_ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb)
    {
    if (NULL == ovrMatSymb)
        m_overrideMatSymb.SetFlags(MATSYMB_OVERRIDE_None);
    else
        m_overrideMatSymb = *ovrMatSymb;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymbR     SimplifyViewDrawGeom::GetCurrentMatSymb(ElemMatSymbR matSymb)
    {
    matSymb = m_currentMatSymb;
    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_Color))
        matSymb.SetLineColor(ColorDef((m_overrideMatSymb.GetLineColor().GetValue() & 0xffffff) | (matSymb.GetLineColor().GetValue() & 0xff000000)));

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_ColorTransparency))
        matSymb.SetLineColor(ColorDef((matSymb.GetLineColor().GetValue() & 0xffffff) | (m_overrideMatSymb.GetLineColor().GetValue() & 0xff000000)));

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_FillColor))
        matSymb.SetFillColor(ColorDef((m_overrideMatSymb.GetFillColor().GetValue() & 0xffffff) | (matSymb.GetFillColor().GetValue() & 0xff000000)));

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_FillColorTransparency))
        matSymb.SetFillColor(ColorDef((matSymb.GetFillColor().GetValue() & 0xffffff) | (m_overrideMatSymb.GetFillColor().GetValue() & 0xff000000)));

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_Style))
        matSymb.SetRasterPattern(m_overrideMatSymb.GetRasterPattern());

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_RastWidth))
        matSymb.SetWidth(m_overrideMatSymb.GetWidth());

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_RenderMaterial))
        matSymb.SetMaterial(m_overrideMatSymb.GetMaterial().get());

    return matSymb;
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyViewDrawGeom::IsRangeTotallyInside(DRange3dCR range)
    {
    return ClipPlaneContainment_StronglyInside == m_context->GetTransformClipStack().ClassifyRange(range);
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyViewDrawGeom::IsRangeTotallyInsideClip(DRange3dCR range)
    { 
    DPoint3d    corners[8];
    
    range.Get8Corners(corners);

    return ArePointsTotallyInsideClip(corners, 8);
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyViewDrawGeom::ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints)
    { 
    if (NULL == GetCurrClip())
        return true;

    for (ClipPrimitivePtr const& primitive: *GetCurrClip())
        if (ClipPlaneContainment_StronglyInside != primitive->ClassifyPointContainment(points, nPoints))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyViewDrawGeom::ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints)
    { 
    if (NULL == GetCurrClip())
        return false;

    for (ClipPrimitivePtr const& primitive: *GetCurrClip())
        if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyPointContainment(points, nPoints))
            return true;

    return false;
    }

#ifdef NEEDS_WORK_GEOMETRY_MAPS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP SimplifyViewDrawGeom::GetCurrentMaterial() const
    {
    if (m_inTextDraw)
        return NULL;

    if (0 != (m_overrideMatSymb.GetFlags() & MATSYMB_OVERRIDE_RenderMaterial))
        return m_overrideMatSymb.GetMaterial();
    
    return m_currentMatSymb.GetMaterial();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCP SimplifyViewDrawGeom::GetCurrentGeometryMap() const
    {
    MaterialCP      material;
    MaterialMapCP   map;

    if (! m_processingMaterialGeometryMap  &&
        NULL != m_context->GetViewFlags() &&
        (NULL == m_context->GetCurrentCookedDisplayStyle() || !m_context->GetCurrentCookedDisplayStyle()->m_flags.m_ignoreGeometryMaps) &&
        NULL != (material = GetCurrentMaterial()) &&
        NULL != (map = material->GetGeometryMap()) &&
        map->IsEnabled() &&
        _ProduceMaterialGeometryMaps(*material, *map))
        return map;

    return NULL;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2013
+===============+===============+===============+===============+===============+======*/
struct      GeometryMapFacetOptionsMark
{
    SimplifyViewDrawGeom&       m_drawGeom;
    bool                        m_saveParamsRequired;

    ~GeometryMapFacetOptionsMark() { m_drawGeom.GetFacetOptions()->SetParamsRequired(m_saveParamsRequired); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryMapFacetOptionsMark(SimplifyViewDrawGeom& drawGeom) : m_drawGeom(drawGeom)
    { 
    m_saveParamsRequired = drawGeom.GetFacetOptions()->GetParamsRequired();
    drawGeom.GetFacetOptions()->SetParamsRequired(true);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static double calculateFacetParamArea(ElementProjectionInfo& projectionInfo, TransformCP currTrans, PolyfaceQueryCR facets, MaterialCR material, MaterialMapLayerCR layer)
    {
    double                  area = 0.0;
    bvector<DPoint2d>       facetParams;

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(facets); visitor->AdvanceToNextFace(); )
        {
        material.ComputeUVParams(facetParams, currTrans, projectionInfo, *visitor, layer);
        area += fabs(PolygonOps::Area(&facetParams[0], visitor->NumEdgesThisFace()));
        }

    return area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyViewDrawGeom::ProcessGeometryMap(PolyfaceQueryCR facets)
    {
    MaterialCP          material;
    MaterialMapCP       geometryMap;

    if (NULL == (material = GetCurrentMaterial()) ||
        NULL == (geometryMap = GetCurrentGeometryMap()))
        return ERROR;

    StatusInt               status;
    EditElementHandle       definitionEh;
    bool                    useCellColors;
    MaterialMapLayerCR      layer = geometryMap->GetLayers().GetTopLayer();

    if (SUCCESS != (status = material->GetGeometryMapDefinition(definitionEh, useCellColors)))
        return status;


    // Set the level same as the parent so the definition children are never eliminated on level/scan criteria.
    ElementPropertiesSetter     levelSetter;

    levelSetter.SetCategory(m_context->GetCurrentDisplayParams()->GetCategory());
    levelSetter.SetChangeEntireElement(true);

    levelSetter.Apply(definitionEh);

    bvector<DPoint2d>       facetParams;
    ElementProjectionInfo   projectionInfo;
    CookedDisplayStyleP     displayStyle = const_cast <CookedDisplayStyleP> (m_context->GetCurrentCookedDisplayStyle());        // Changed temporarily....
    bool                    colorFromMaterial = NULL != displayStyle && displayStyle->m_flags.m_hLineMaterialColors;

    if (useCellColors && colorFromMaterial)
         displayStyle->m_flags.m_hLineMaterialColors = false;         // Else an assigned material may cause material from color to be assigned to geometry map.
        
    AutoRestore <ViewContext::T_DrawMethod>     saveDrawMethod(&m_context->m_callDrawMethod, &ViewContext::DrawElementNormal);
    AutoRestore <bool>                          saveNoRangeTestOnComponents(&m_context->m_noRangeTestOnComponents, true);
    AutoRestore <bool>                          saveProcessingGeometryMap(&m_processingMaterialGeometryMap, true);
    AutoRestore <ElemMatSymb>                   saveOutputElemMatSymb(&m_currentMatSymb);
    AutoRestore <OvrMatSymb>                    saveOutputOvrMatSymb(&m_overrideMatSymb);
    AutoRestore <ElemMatSymb>                   saveContextElemMatSymb(m_context->GetElemMatSymb());
    AutoRestore <OvrMatSymb>                    saveContextOvrMatSymb(m_context->GetOverrideMatSymb());
    AutoRestore <ElemDisplayParams>             saveContextDisplayParams(m_context->GetCurrentDisplayParams());
    XGraphicsRecorder*                          xGraphicsRecorder = NULL;
    Transform                                   localToElement, elementToRoot;

    m_context->GetDisplayParamsIgnores().Set(*m_context->GetCurrentDisplayParams(), true, !useCellColors, false); // NOTE: Geometry map level is always inherited from base element...

    projectionInfo.CalculateForElement(GetCurrentElement(), SUCCESS == GetElementToRootTransform(elementToRoot) ? &elementToRoot : NULL, *material, geometryMap);

    // The parameter area is equivalent to the tile count.... 
    double          paramArea = calculateFacetParamArea(projectionInfo, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, facets, *material, layer);
    static double   s_minTileCount = (.01 * .01), s_maxTileCount = (1000.0 * 1000.0);
    
    if (paramArea < s_minTileCount || paramArea > s_maxTileCount)
        {
        //BeAssert (false);
        return ERROR;
        }

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(facets); visitor->AdvanceToNextFace(); )
        {
        if (ClipPlaneContainment_StronglyOutside != m_context->GetTransformClipStack().ClassifyPoints(visitor->GetPointCP(), visitor->NumEdgesThisFace()))
            {
            material->ComputeUVParams(facetParams, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, projectionInfo, *visitor, layer);
            ProcessGeometryMap(visitor->GetPointCP(), &facetParams[0], visitor->NumEdgesThisFace(), definitionEh, xGraphicsRecorder);
            }
        }

    DELETE_AND_CLEAR (xGraphicsRecorder);

    m_context->GetDisplayParamsIgnores().Clear();

    if (useCellColors && colorFromMaterial)
        displayStyle->m_flags.m_hLineMaterialColors = true;            // Restore if changed above.

    return SUCCESS;
    }

#define IS_SIGNIFICANT20(v)     (fabs(v) > 1.0e-20)

/*---------------------------------------------------------------------------------**//**
* solves linear system using Gaussian elimination with partial pivoting
* @bsimethod                                                    BJB             01/87
+---------------+---------------+---------------+---------------+---------------+------*/
////////////////////NEEDS_WORK... Find equivalent in geomlibs?  /////////////////////////////
static int solve_linear_system
(
double  *amatrix,       /* =>  matrix of coefficients */
int     dimensions,     /* =>  dimension of matrix */
double  *rhsides,       /* <=> input=right hand sides.  output=solutions */
int     numrhs          /* =>  number of right hand sides */
)
    {
    int         row, irow, column, i, j, pivotrow;
    double      *dp, *dp1, *dp2;
    double      temp, pivotel;

    for (row=0; row<dimensions; row++)
        {
        /* first find the largest element of column i and interchange
            that row with row row */
        for (j=pivotrow=row, dp=amatrix+(row*dimensions+row), temp = 0.0;
                j < dimensions;  j++, dp+=dimensions)
            {
            if (fabs(*dp) > temp)
                {
                temp = fabs(*dp);
                pivotrow = j;
                }
            }

        /* if we have an illconditioned matrix, quit */
        if (! IS_SIGNIFICANT20(temp)) return (1);
        else if (row != pivotrow)
            {
            /* interchange row and pivotrow of coefficient matrix & rhsides */
            for (j=row, dp=amatrix+(row*dimensions+j),
                      dp1=amatrix+(pivotrow*dimensions+j); j<dimensions;
                        j++, dp++, dp1++)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }

            for (j=0, dp=rhsides+row, dp1=rhsides+pivotrow; j<numrhs;
                    j++, dp+=dimensions, dp1+=dimensions)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }
            }

        /* now go through and do the elimination */
        for (irow=row+1, pivotel= *(amatrix+(row*dimensions+row));
                irow<dimensions; irow++)
            {
            for (column=row+1, dp=amatrix+(irow*dimensions+row),
                dp1=amatrix+(row*dimensions+column),
                temp= *dp++/pivotel; column<dimensions; column++, dp++, dp1++)
                {
                *dp -= temp * *dp1;
                }
            /* do the same operation on the right hand sides */
            for (column=0, dp=rhsides+irow, dp1=rhsides+row;
                 column<numrhs; column++, dp1+= dimensions, dp+=dimensions)
                {
                *dp -= temp * *dp1;
                }
            }
        }

    /* next go through and do the back substitution for all rh sides */
    for (i=0; i<numrhs; i++)
        {
        for (row=dimensions-1, dp=rhsides+i*dimensions+row; row>=0; row--, dp--)
            {
            for (j=dimensions-1, dp1=rhsides+i*dimensions+j,
                    dp2=amatrix+row*dimensions+j; j>row; j--, dp1--, dp2--)
                {
                *dp -= *dp1 * *dp2;
                }
            *dp /= *dp2;
            }
        }
    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt calculateParamToWorld(TransformR transform, DPoint2dCP params, DPoint3dCP points, size_t nPoints)
    {
    StatusInt   status;
    double      aMatrix[3][3];
    double      rhSides[3][3];

    /* a11 = sumxixi, a12 = sumxiyi, a14 = sumxi,
       a21 = sumxiyi, a22 = sumyiyi, a24 = sumyi,
       a41 = sumxi,   a42 = sumyi,   a44 = 1 */

    /* rhs11 = sumXixi, rhs12 = sumXiyi, rhs14 = sumXi,
       rhs21 = sumYixi, rhs22 = sumYiyi, rhs24 = sumYi,
       rhs31 = sumZixi, rhs32 = sumZiyi, rhs34 = sumZi */

    memset(aMatrix, 0, sizeof (aMatrix));
    memset(rhSides, 0, sizeof (rhSides));

    DRange3d        pointRange;

    pointRange.InitFrom(points, (int) nPoints);
    
    for (size_t i=0; i<nPoints; i++)
        {
        DPoint3d            point;

        point.DifferenceOf(points[i], pointRange.low);

        aMatrix[0][0] += params[i].x * params[i].x;
        aMatrix[0][1] += params[i].x * params[i].y;
        aMatrix[0][2] += params[i].x;
        aMatrix[1][1] += params[i].y * params[i].y;
        aMatrix[1][2] += params[i].y;

        rhSides[0][0] += point.x * params[i].x;
        rhSides[0][1] += point.x * params[i].y;
        rhSides[0][2] += point.x;
        rhSides[1][0] += point.y * params[i].x;
        rhSides[1][1] += point.y * params[i].y;
        rhSides[1][2] += point.y;
        rhSides[2][0] += point.z * params[i].x;
        rhSides[2][1] += point.z * params[i].y;
        rhSides[2][2] += point.z;

        }                                                                                                                                                                   
    aMatrix[1][0] = aMatrix[0][1];
    aMatrix[2][0] = aMatrix[0][2];
    aMatrix[2][1] = aMatrix[1][2];
    aMatrix[2][2] = (double) nPoints;

    if (SUCCESS == (status = solve_linear_system((double *)aMatrix, 3, (double *)rhSides, 3)))
        {
        DVec3d      xColumn, yColumn, zColumn;
        RotMatrix   rMatrix;

        xColumn.x = rhSides[0][0];
        xColumn.y = rhSides[1][0];
        xColumn.z = rhSides[2][0];

        yColumn.x = rhSides[0][1];
        yColumn.y = rhSides[1][1];
        yColumn.z = rhSides[2][1];
        zColumn.NormalizedCrossProduct(xColumn, yColumn);

        rMatrix.InitFromColumnVectors(xColumn, yColumn, zColumn);

        transform.InitFrom(rMatrix);
        transform.form3d[0][3] = rhSides[0][2] + pointRange.low.x;
        transform.form3d[1][3] = rhSides[1][2] + pointRange.low.y;
        transform.form3d[2][3] = rhSides[2][2] + pointRange.low.z;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double computePolygonNormal(DVec3dR normal, DPoint3dCP pXYZ, size_t numXYZ)
    {
    DVec3d      uVec, vVec, wVec;

    normal.Zero();
    uVec.DifferenceOf(pXYZ[1], pXYZ[0]);
    for (size_t i = 2 ; i < numXYZ; i++)
        {
        vVec.DifferenceOf(pXYZ[ i], pXYZ[0]);
        wVec.CrossProduct(uVec, vVec);
        normal.add(&wVec);
        uVec = vVec;
        }
    return normal.Normalize();
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2010
+===============+===============+===============+===============+===============+======*/
struct FacetOutlineMeshGatherer : PolyfaceQuery::IClipToPlaneSetOutput
{
    IPolyfaceConstructionR  m_builder;
    TransformCR             m_transform;
        
    FacetOutlineMeshGatherer(IPolyfaceConstructionR builder, TransformCR transform) : m_builder(builder), m_transform(transform) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override
    {
    PolyfaceHeaderPtr   tempPolyface = PolyfaceHeader::New();

    tempPolyface->CopyFrom(polyfaceQuery);
    tempPolyface->Transform(m_transform);
    m_builder.AddPolyface(*tempPolyface);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override
    {
    polyfaceHeader.Transform(m_transform);
    m_builder.AddPolyface(polyfaceHeader);

    return SUCCESS;
    }

}; // FacetOutlineMeshGatherer

static double   s_facetTileAreaMinimum   = (.01 * .01);      // Minimum portion of tile within a single 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyViewDrawGeom::ProcessFacetTextureOutlines(IPolyfaceConstructionR builder, DPoint3dCP points, DPoint2dCP params, bool const* edgeVisible, size_t nPoints, bvector<DPoint3d>& outlinePoints, bvector<int32_t>& outlineIndices)
    {
    DRange2d            paramRange;
    DVec2d              paramDelta;
    static double       s_minParamRange = 1.0E-3;

    paramRange.InitFrom(params, (int) nPoints);
    paramDelta.DifferenceOf(paramRange.high, paramRange.low);

    if (paramDelta.x < s_minParamRange || paramDelta.y < s_minParamRange)
        return ERROR;

    Transform           paramToWorld;
    DVec3d              normal;
    DRange2d            tileRange;

    computePolygonNormal(normal, points, nPoints);
    if (PolygonOps::Area(params, (int) nPoints) < s_facetTileAreaMinimum ||
        SUCCESS != calculateParamToWorld(paramToWorld, params, points, nPoints))
        return ERROR;

    tileRange.InitFrom(floor(paramRange.low.x), floor(paramRange.low.y), ceil(paramRange.high.x), ceil(paramRange.high.y));

    ConvexClipPlaneSet  convexPlanes;
    bool                clockwise = params[1].crossProductToPoints(&params[0], &params[2]) > 0.0;

    for (size_t i=0; i<nPoints; i++)
        {
        DVec2d          delta;
        if (0.0 != delta.NormalizedDifference(params[(i+1) % nPoints], params[i]))
            {
            DVec3d          normal;
            double          distance;

            normal.Init(clockwise ? delta.y : -delta.y, clockwise ? -delta.x : delta.x, 0.0);
            distance = normal.x * params[i].x + normal.y * params[i].y;

            convexPlanes.push_back(ClipPlane(normal, distance, !edgeVisible[i]));
            }
        }

    ClipPlaneSet                clipPlaneSet(convexPlanes);
    bvector <DPoint3d>          tileOutline(outlinePoints.size());
    PolyfaceQueryCarrier        tileFacets(0, true, outlineIndices.size(), outlinePoints.size(), &tileOutline[0], &outlineIndices[0]);
    FacetOutlineMeshGatherer    outputGatherer(builder, paramToWorld);
    T_ClipPlaneSets             planeSets;

    planeSets.push_back(clipPlaneSet);
    for (int i = (int) tileRange.low.x; i < (int) tileRange.high.x; i++)
        {
        for (int j = (int) tileRange.low.y; j < (int) tileRange.high.y; j++)  
            {
            for (size_t iPoint = 0; iPoint < outlinePoints.size(); iPoint++)
                tileOutline[iPoint].Init((double) i + outlinePoints[iPoint].x, (double) j + outlinePoints[iPoint].y, 0.0);

            tileFacets.ClipToPlaneSetIntersection(planeSets, outputGatherer, false);
            }
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyViewDrawGeom::ProcessTextureOutlines(PolyfaceQueryCR facets)
    {
    MaterialCP              material;
    MaterialMapCP           patternMap;
    size_t                  numFacets = facets.GetNumFacet();
    bvector<DPoint3d>       outlinePoints;

    if (0 == facets.GetParamCount() ||             // Prevents recursion.
        !_ProduceTextureOutlines() ||
        NULL == (material = GetCurrentMaterial()) ||
        NULL == (patternMap = material->GetSettings().GetMaps().GetMapCP (MaterialMap::MAPTYPE_Pattern)) ||
        SUCCESS != T_HOST.GetGraphicsAdmin()._ExtractTextureOutline(outlinePoints, *material))
        return ERROR;

    MaterialMapLayerCR      layer = patternMap->GetLayers().GetTopLayer();
    ElementProjectionInfo   projectionInfo;
    Transform               localToElement, elementToRoot;

    projectionInfo.CalculateForElement(GetCurrentElement(), SUCCESS == GetElementToRootTransform(elementToRoot) ? &elementToRoot : NULL, *material, patternMap);

    double          paramArea = calculateFacetParamArea(projectionInfo, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, facets, *material, layer);
    double          pointMagnitude = (double) outlinePoints.size() * paramArea * (double) numFacets;
    static double   s_maxPointMagnitude = 1.0E8;

    if (pointMagnitude > s_maxPointMagnitude)
        return ERROR;
    
    PolyfaceHeaderPtr   triangulatedFacets = PolyfaceHeader::New();
    bvector<int32_t>      triangulatedOutlineIndices;
    StatusInt           status;

    triangulatedFacets->CopyFrom(const_cast <PolyfaceQueryR> (facets));
    triangulatedFacets->Triangulate();

    if (SUCCESS != (status = vu_triangulateSpacePolygonExt2(&triangulatedOutlineIndices, NULL, NULL, NULL, NULL, &outlinePoints[0], (int) outlinePoints.size(), 1.0E-6, TRUE)))
        return status;

    bvector<DPoint2d>           facetParams;
    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::New(*_GetFacetOptions());

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*triangulatedFacets); visitor->AdvanceToNextFace(); )
        {
        size_t  nFacetPoints = visitor->NumEdgesThisFace();

        material->ComputeUVParams(facetParams, m_context->GetCurrLocalToWorldTransformCP(), projectionInfo, *visitor, layer);
        ProcessFacetTextureOutlines(*builder, visitor->GetPointCP(), &facetParams[0], visitor->GetVisibleCP(), nFacetPoints, outlinePoints, triangulatedOutlineIndices);
        }

    if (builder->GetClientMeshR().HasFacets())
        _ProcessFacetSet(builder->GetClientMeshR(), false);
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    SimplifyViewDrawGeom::StrokeGeometryMap(CurveVectorCR curves)
    {
    if (NULL == GetCurrentGeometryMap())
        return;

    GeometryMapFacetOptionsMark     facetOptionsMark(*this);
    IPolyfaceConstructionPtr        builder = GetPolyfaceBuilder();

    builder->AddRegion(curves);
    ProcessGeometryMap(builder->GetClientMeshR());
    }

#endif //NEEDS_WORK_GEOMETRY_MAPS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SimplifyViewDrawGeom::GetElementToWorldTransform(TransformR transform) { return m_context->GetTransformClipStack().GetTransformFromIndex(transform, m_elementTransformStackIndex); }
BentleyStatus SimplifyViewDrawGeom::GetLocalToElementTransform(TransformR transform) { return m_context->GetTransformClipStack().GetTransformFromTopToIndex(transform, m_elementTransformStackIndex); }























                                                                                                                                      