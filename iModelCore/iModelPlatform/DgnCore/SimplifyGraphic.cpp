/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnRscFontStructures.h>

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SimplifyCurveCollector : IGeometryProcessor
{
protected:

CurveVectorPtr  m_curves;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    CurveVectorPtr childCurve = curves.Clone(); // NOTE: Don't apply local to world...want geometry in local coords of parent graphic...

    if (m_curves.IsNull())
        m_curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    m_curves->Add(childCurve);

    return true;
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector() {return m_curves;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(SimplifyGraphic const& graphic, ISolidPrimitiveCR geom, ViewContextR context)
    {
    SimplifyCurveCollector    processor;
    Render::GraphicBuilderPtr builder = new SimplifyGraphic(graphic.GetCreateParams(), processor, context);

    WireframeGeomUtil::Draw(geom, *builder, &context);

    return processor.GetCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(SimplifyGraphic const& graphic, MSBsplineSurfaceCR geom, ViewContextR context)
    {
    SimplifyCurveCollector    processor;
    Render::GraphicBuilderPtr builder = new SimplifyGraphic(graphic.GetCreateParams(), processor, context);

    WireframeGeomUtil::Draw(geom, *builder, &context);

    return processor.GetCurveVector();
    }

}; // SimplifyCurveCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SimplifyGraphic::SimplifyGraphic(Render::GraphicBuilder::CreateParams const& params, IGeometryProcessorR processor, ViewContextR context)
    : GraphicBuilder(params), m_processor(processor), m_context(context)
    {
    m_facetOptions = m_processor._GetFacetOptionsP();

    if (!m_facetOptions.IsValid())
        {
        m_facetOptions = IFacetOptions::Create();

        m_facetOptions->SetMaxPerFace(5000/*MAX_VERTICES*/);
        m_facetOptions->SetAngleTolerance(0.25 * Angle::Pi());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr SimplifyGraphic::_CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip) const
    {
    SimplifyGraphic* subGraphic = new SimplifyGraphic(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(), Transform::FromProduct(GetLocalToWorldTransform(), subToGraphic)), m_processor, m_context);

    subGraphic->m_currGraphicParams  = m_currGraphicParams;
    subGraphic->m_currGeometryParams = m_currGeometryParams;
    subGraphic->m_currGeomEntryId    = m_currGeomEntryId;
    subGraphic->m_currClip           = (nullptr != clip ? clip->Clone(&GetLocalToWorldTransform()) : nullptr);

    return subGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d SimplifyGraphic::GetLocalToView() const
    {
    DMatrix4d   localToWorld = DMatrix4d::From(GetLocalToWorldTransform());
    DMatrix4d   worldToView = m_context.GetWorldToView().M0;
    DMatrix4d   localToView;

    localToView.InitProduct(worldToView, localToWorld);

    return localToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d SimplifyGraphic::GetViewToLocal() const
    {
    Transform   worldToLocalTrans;

    worldToLocalTrans.InverseOf(GetLocalToWorldTransform());

    DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
    DMatrix4d   viewToWorld = m_context.GetWorldToView().M1;
    DMatrix4d   viewToLocal;

    viewToLocal.InitProduct(worldToLocal, viewToWorld);

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    GetLocalToView().Multiply(viewPts, localPts, nullptr, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    DMatrix4dCR localToView = GetLocalToView();

    localToView.MultiplyAffine(viewPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(GetLocalToWorldTransform());
    m_context.ViewToWorld(localPts, viewPts, nPts);
    worldToLocal.Multiply(localPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(GetLocalToWorldTransform());
    m_context.ViewToWorld(localPts, viewPts, nPts);
    worldToLocal.Multiply(localPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::IsRangeTotallyInside(DRange3dCR range) const
    {
    Frustum box(range);
    return m_context.GetFrustumPlanes().Contains(box.m_pts, 8) == FrustumPlanes::Contained::Inside;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::IsRangeTotallyInsideClip(DRange3dCR range) const
    {
    DPoint3d    corners[8];
    range.Get8Corners(corners);
    return ArePointsTotallyInsideClip(corners, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints) const
    {
    if (nullptr == GetCurrentClip())
        return true;

    for (ClipPrimitivePtr const& primitive : *GetCurrentClip())
        if (ClipPlaneContainment_StronglyInside != primitive->ClassifyPointContainment(points, nPoints))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints) const
    {
    if (nullptr == GetCurrentClip())
        return false;

    for (ClipPrimitivePtr const& primitive : *GetCurrentClip())
        if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyPointContainment(points, nPoints))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessAsLinearSegments(CurveVectorCR geom, bool filled)
    {
    bvector<DPoint3d> points;

    geom.AddStrokePoints(points, *GetScaledFacetOptions());
    m_processor._ProcessLinearSegments(&points.front(), points.size(), geom.IsAnyRegionType(), filled, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void processCurvePrimitives(IGeometryProcessorR processor, CurveVectorCR curves, bool filled, SimplifyGraphic& graphic)
    {
    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                {
                BeAssert(true && "Unexpected entry in region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            processCurvePrimitives(processor, *curve->GetChildCurveVectorCP (), filled && curves.IsUnionRegion(), graphic); // Don't pass filled when spewing parity region loops...
            }
        }
    else
        {
        bool isSingleEntry = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive());
        bool isClosed = curves.IsClosedPath();
        bool isOpen = curves.IsOpenPath();
        bool isComplex = ((isClosed || isOpen) && !isSingleEntry);

        for (ICurvePrimitivePtr curve : curves)
            {
            if (!curve.IsValid())
                continue;

            if (processor._ProcessCurvePrimitive(*curve, !isComplex && isClosed, !isComplex && filled, graphic)) // Don't pass filled when spewing primitives...
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType())
                processCurvePrimitives(processor, *curve->GetChildCurveVectorCP (), filled, graphic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessAsCurvePrimitives(CurveVectorCR geom, bool filled)
    {
    processCurvePrimitives(m_processor, geom, filled, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessCurveVector(CurveVectorCR geom, bool filled)
    {
    if (m_processor._ProcessCurveVector(geom, filled, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled) && geom.IsAnyRegionType()) // Can only facet regions...
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*GetScaledFacetOptions());
        builder->AddRegion(geom);

        m_processor._ProcessPolyface(builder->GetClientMeshR(), filled, *this);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessSolidPrimitive(ISolidPrimitiveCR geom)
    {
    if (m_processor._ProcessSolidPrimitive(geom, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled))
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*GetScaledFacetOptions());
        builder->AddSolidPrimitive(geom);

        m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        // NOTE: Can't use WireframeGeomUtil::CollectCurves because we require geometry relative to this graphic/context...
        CurveVectorPtr curves = SimplifyCurveCollector::Process(*this, geom, m_context);

        if (!curves.IsValid())
            return;

        m_processor._ProcessCurveVector(*curves, false, *this);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessSurface(MSBsplineSurfaceCR geom)
    {
    if (m_processor._ProcessSurface(geom, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled))
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*GetScaledFacetOptions());
        builder->Add(geom);

        m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        // NOTE: Can't use WireframeGeomUtil::CollectCurves because we require geometry relative to this graphic/context...
        CurveVectorPtr curves = SimplifyCurveCollector::Process(*this, geom, m_context);

        if (!curves.IsValid())
            return;

        m_processor._ProcessCurveVector(*curves, false, *this);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessPolyface(PolyfaceQueryCR geom, bool filled)
    {
    if (m_processor._ProcessPolyface(geom, filled, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        ProcessPolyfaceAsCurves(geom);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessPolyfaceAsCurves(PolyfaceQueryCR geom)
    {
    int const*  vertIndex = geom.GetPointIndexCP();
    size_t      numIndices = geom.GetPointIndexCount();
    DPoint3dCP  verts = geom.GetPointCP();
    int         polySize = geom.GetNumPerFace();
    int         thisIndex, prevIndex=0, firstIndex=0;
    size_t      thisFaceSize = 0;

    if (!vertIndex)
        return;

    for (size_t readIndex = 0; readIndex < numIndices; readIndex++)
        {
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            if (!thisFaceSize)
                {
                // remember first index in this face loop
                firstIndex = thisIndex;
                }
            else if (prevIndex > 0)
                {
                // draw visible edge (prevIndex, thisIndex)
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(thisIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));
                CurveVectorPtr curvePtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve);

                m_processor._ProcessCurveVector(*curvePtr, false, *this);
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
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(firstIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));
                CurveVectorPtr curvePtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve);

                m_processor._ProcessCurveVector(*curvePtr, false, *this);
                }

            thisFaceSize = 0;
            }

        if (0 == (readIndex % 100) && m_context.CheckStop())
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessBody(IBRepEntityCR geom)
    {
    if (m_processor._ProcessBody(geom, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    auto entityType = geom.GetEntityType();
    bool isSolidOrSheet = (IBRepEntity::EntityType::Sheet == entityType || IBRepEntity::EntityType::Solid == entityType);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled) && isSolidOrSheet)
        {
        ProcessBodyAsPolyface(geom);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessBodyAsPolyface(IBRepEntityCR entity)
    {
    IFacetOptionsPtr scaledFacetOptions = GetScaledFacetOptions();

    if (nullptr != entity.GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr> polyfaces;
        bvector<FaceAttachment> params;

        if (T_HOST.GetBRepGeometryAdmin()._FacetEntity(entity, polyfaces, params, *scaledFacetOptions))
            {
            for (size_t i = 0; i < polyfaces.size(); i++)
                {
                if (0 == polyfaces[i]->GetPointCount())
                    continue;

                GeometryParams faceParams;

                FaceAttachmentUtil::ToGeometryParams(params[i], faceParams, m_currGeometryParams);
                m_context.CookGeometryParams(faceParams, *this);
                m_processor._ProcessPolyface(*polyfaces[i], false, *this);
                }

            return;
            }
        }

    PolyfaceHeaderPtr meshPtr = T_HOST.GetBRepGeometryAdmin()._FacetEntity(entity, *scaledFacetOptions);

    if (!meshPtr.IsValid())
        return;

    m_processor._ProcessPolyface(*meshPtr, false, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessText(TextStringCR text)
    {
    if (m_processor._ProcessTextString(text, *this))
        return;

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(text, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Box & unhandled))
        {
        if (text.GetText().empty())
            return;

        DPoint3d points[5];

        text.ComputeBoundingShape(points);
        text.ComputeTransform().Multiply(points, _countof(points));

        Render::GraphicBuilderPtr graphic = _CreateSubGraphic(Transform::FromIdentity(), nullptr);
        SimplifyGraphic* sGraphic = static_cast<SimplifyGraphic*> (graphic.get());

        if (nullptr == sGraphic)
            return;

        CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, 5));
        sGraphic->ProcessCurveVector(*curve, false);
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        Render::GraphicBuilderPtr graphic = _CreateSubGraphic(text.ComputeTransform(), nullptr);
        SimplifyGraphic* sGraphic = static_cast<SimplifyGraphic*> (graphic.get());

        if (nullptr == sGraphic)
            return;

        auto        numGlyphs = text.GetNumGlyphs();
        DPoint3dCP  glyphOrigins = text.GetGlyphOrigins();
        DVec3d      xVector, yVector;
        DbGlyphCP const* glyphs = text.GetGlyphs();

        text.ComputeGlyphAxes(xVector, yVector);

        if (text.GetGlyphSymbology(sGraphic->m_currGeometryParams))
            m_context.CookGeometryParams(sGraphic->m_currGeometryParams, *graphic);

        Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xVector, yVector));
        for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
            {
            if (nullptr != glyphs[iGlyph])
                {
                CurveVectorPtr  curves = glyphs[iGlyph]->GetCurveVector ();
                if (curves.IsNull())
                    continue;
                curves->TransformInPlace (Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform));
                ProcessCurveVector(*curves, curves->IsAnyRegionType ());
                }
            }

        text.AddUnderline(*graphic); // NOTE: Issue with supporting bold resource fonts, don't want underline bolded...
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double zDepth)
    {
    for (int i=0; i<numPoints; i++, pts3d++, pts2d++)
        pts3d->Init(pts2d->x, pts2d->y, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddLineString(int numPoints, DPoint3dCP points)
    {
    m_processor._OnNewGeometry();

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLineString(points, numPoints));

    ProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddLineString2d(int numPoints, DPoint2dCP points, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddLineString(numPoints, &localPointsBuf3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPointString(int numPoints, DPoint3dCP points)
    {
    m_processor._OnNewGeometry();

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreatePointString(points, numPoints));

    ProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPointString2d(int numPoints, DPoint2dCP points, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddPointString(numPoints, &localPointsBuf3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddShape(int numPoints, DPoint3dCP points, bool filled)
    {
    m_processor._OnNewGeometry();

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, numPoints));

    ProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddShape(numPoints, &localPointsBuf3d[0], filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags)
    {
    m_processor._OnNewGeometry();

    if (AsThickenedLine::Yes == usageFlags) // represents thickened line...
        {
        int         nPt = 0;
        IndexedScopedArray<DPoint3d> tmpPtsP(numPoints+1);

        for (int iPtS1=0; iPtS1 < numPoints; iPtS1 = iPtS1+2)
            tmpPtsP[nPt++] = points[iPtS1];

        for (int iPtS2=numPoints-1; iPtS2 > 0; iPtS2 = iPtS2-2)
            tmpPtsP[nPt++] = points[iPtS2];

        tmpPtsP[nPt] = tmpPtsP[0]; // Add closure point...simplifies drop of extrude thickness...

        _AddShape(numPoints+1, tmpPtsP.GetDataCP(), true);
        return;
        }

    // spew triangles
    for (int iPt=0; iPt < numPoints-2; iPt++)
        _AddShape(3, &points[iPt], true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddTriStrip(numPoints, &localPointsBuf3d[0], usageFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled)
    {
    m_processor._OnNewGeometry();

    // NOTE: display closes arc ends and displays them filled (see outputCapArc for linestyle strokes)...
    CurveVectorPtr curve = CurveVector::Create((isEllipse || filled) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse));

    if (filled && !isEllipse && !ellipse.IsFullEllipse())
        {
        DSegment3d         segment;
        ICurvePrimitivePtr gapSegment;

        ellipse.EvaluateEndPoints(segment.point[1], segment.point[0]);
        gapSegment = ICurvePrimitive::CreateLine(segment);
        gapSegment->SetMarkerBit(ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
        curve->push_back(gapSegment);
        }

    ProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddArc(ellipse, isEllipse, filled);
        }
    else
        {
        auto ell = ellipse;
        ell.center.z = zDepth;
        _AddArc(ell, isEllipse, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    m_processor._OnNewGeometry();

    CurveVectorPtr curve = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(bcurve));

    ProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurve bs;
        bs.CopyFrom(bcurve);
        size_t nPoles = bs.GetNumPoles();
        DPoint3d* poles = bs.GetPoleP();
        for (size_t i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurve(bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddCurveVector(CurveVectorCR curves, bool isFilled)
    {
    m_processor._OnNewGeometry();

    ProcessCurveVector(curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddCurveVector(curves, isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVector(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddSolidPrimitive(ISolidPrimitiveCR geom)
    {
    m_processor._OnNewGeometry();

    ProcessSolidPrimitive(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineSurface(MSBsplineSurfaceCR geom)
    {
    m_processor._OnNewGeometry();

    ProcessSurface(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPolyface(PolyfaceQueryCR geom, bool filled)
    {
    m_processor._OnNewGeometry();

    // See if we need to modify this polyface to conform to the processor's facet options...
    IGeometryProcessor::IncompatiblePolyfacePreference pref = m_processor._GetIncompatiblePolyfacePreference(geom, *this);

    if (IGeometryProcessor::IncompatiblePolyfacePreference::Original != pref)
        {
        size_t maxPerFace;

        if ((m_facetOptions->GetNormalsRequired() && 0 == geom.GetNormalCount()) ||
            (m_facetOptions->GetParamsRequired() && (0 == geom.GetParamCount() || 0 == geom.GetFaceCount())) ||
            (m_facetOptions->GetEdgeChainsRequired() && 0 == geom.GetEdgeChainCount()) ||
            (m_facetOptions->GetConvexFacetsRequired() && !geom.HasConvexFacets()) ||
            (geom.GetNumFacet(maxPerFace) > 0 && (int) maxPerFace > m_facetOptions->GetMaxPerFace()))
            {
            if (IGeometryProcessor::IncompatiblePolyfacePreference::Ignore == pref)
                return;

            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*GetScaledFacetOptions());

            builder->AddPolyface(geom);
            ProcessPolyface(builder->GetClientMeshR(), filled);
            return;
            }
        }

    ProcessPolyface(geom, filled);
    }

/*---------------------------------------------------------------------------------**//**


* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBody(IBRepEntityCR geom)
    {
    m_processor._OnNewGeometry();

    ProcessBody(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTextString(TextStringCR text)
    {
    m_processor._OnNewGeometry();

    ProcessText(text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTextString2d(TextStringCR text, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddTextString(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextString(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::AddImage(ImageGraphicCR img)
    {
    m_processor._OnNewGeometry();
    ProcessImage(img);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::AddImage2d(ImageGraphicCR input, double zDepth)
    {
    auto img = input.Clone();
    zDepth = m_processor._AdjustZDepth(zDepth);
    img->SetZ(zDepth);
    AddImage(*img);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessImage(ImageGraphicCR img)
    {
    if (m_processor._ProcessImage(img, *this))
        return;

    // NOTE: We currently have no way to forward the texture image - if processor cares about it they must implement _ProcessImage()
    auto mask = IGeometryProcessor::UnhandledPreference::Box | IGeometryProcessor::UnhandledPreference::Curve;
    auto unhandled = m_processor._GetUnhandledPreference(img, *this);
    if (IGeometryProcessor::UnhandledPreference::Ignore == (mask & unhandled))
        return;

    ProcessCurveVector(*img.ToCurveVector(), true);
    if (img.HasBorder())
        ProcessCurveVector(*img.ToCurveVector(CurveVector::BOUNDARY_TYPE_Open), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP clip)
    {
    // Nothing to do...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams)
    {
    m_currGraphicParams = graphicParams;

    if (nullptr != geomParams)
        m_currGeometryParams = *geomParams;
    else
        m_currGeometryParams = GeometryParams();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GeometryProcessorContext : NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

IGeometryProcessorR    m_processor;

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    {
    return new SimplifyGraphic(params, m_processor, *this);
    }

public:

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryProcessorContext(IGeometryProcessorR processor) : m_processor(processor)
    {
    m_purpose = m_processor._GetProcessPurpose();
    m_wantMaterials = true; // Setup material in GeometryParams in case processor needs it...do we still need to do this on DgnDbCR???
    }

}; // GeometryProcessorContext

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryProcessor::Process(IGeometryProcessorR processor, DgnDbR dgnDb)
    {
    GeometryProcessorContext context(processor);

    context.SetDgnDb(dgnDb);
    processor._OutputGraphics(context);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryProcessor::Process(IGeometryProcessorR processor, GeometrySourceCR source)
    {
    GeometryProcessorContext context(processor);

    context.SetDgnDb(source.GetSourceDgnDb());
    context.VisitGeometry(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr SimplifyGraphic::_Finish()
    {
    m_isOpen = false;
    return new Base(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::_WantStrokeLineStyle(LineStyleSymbCR symb, IFacetOptionsPtr& options)
    {
    return m_processor._DoLineStyleStroke(symb, options, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::_WantStrokePattern(PatternParamsCR pattern)
    {
    return m_processor._DoPatternStroke(pattern, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr SimplifyGraphic::GetScaledFacetOptions() const
    {
    double scale = 1.0;
    bool isRigidScale = GetLocalToWorldTransform().IsRigidScale(scale);
    BeAssert(isRigidScale); // Brien says we should only get uniform scale through this path
    UNUSED_VARIABLE(isRigidScale);
    if (0 == BeNumerical::Compare(1.0, scale))
        return m_facetOptions;

    IFacetOptionsPtr scaledFacetOptions = m_facetOptions->Clone();
    scaledFacetOptions->SetToleranceDistanceScale(1.0 / scale);
    return scaledFacetOptions;
    }


