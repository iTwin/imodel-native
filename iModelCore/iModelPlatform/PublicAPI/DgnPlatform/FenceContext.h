/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/SimplifyGraphic.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FenceGeometryProcessor : IGeometryProcessor
{
IFacetOptionsPtr    m_facetOptions;

bool                m_allowOverlaps;
bool                m_hasOverlaps;

bool                m_earlyDecision;
bool                m_currentAccept;
bool                m_accept;
bool                m_firstAccept;

ClipPlaneSet        m_worldClipPlanes;
ClipPlaneSet        m_worldClipMasks;
ClipPlaneSet        m_localClipPlanes;
ClipPlaneSet        m_localClipMasks;
Transform           m_localToWorld;

DrawPurpose _GetProcessPurpose() const override {return DrawPurpose::FenceAccept;}
IFacetOptionsP _GetFacetOptionsP() override;

UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override {return UnhandledPreference::Box;}

bool ProcessPoints(DPoint3dCP, int numPoints, SimplifyGraphic&);

bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic&) override;
bool _ProcessCurveVector(CurveVectorCR, bool isFilled, SimplifyGraphic&) override;
bool _ProcessSolidPrimitive(ISolidPrimitiveCR, SimplifyGraphic&) override;
bool _ProcessSurface(MSBsplineSurfaceCR, SimplifyGraphic&) override;
bool _ProcessPolyface(PolyfaceQueryCR, bool isFilled, SimplifyGraphic&) override;
bool _ProcessBody(IBRepEntityCR, SimplifyGraphic&) override;

void InitCurrentAccept();
void UpdateCurrentAccept(ClipPlaneContainment);
void CheckCurrentAccept();
bool CheckStop();

bool HasOverlaps() const {return m_hasOverlaps;}
bool AllowOverlaps() const {return m_allowOverlaps;}
bool IsInsideMode() const {return !m_allowOverlaps;}

void SetWorldClipPlanes(ClipVectorCR);
void UpdateLocalClipPlanes(TransformCR localToWorld);
ClipPlaneContainment CheckRange (DRange3dCR localRange, TransformCR localToWorld);
ClipPlaneContainment OnNewGeometrySource(GeometrySourceCR source);

FenceGeometryProcessor(ClipVectorCR clip, bool allowOverlaps)
    {
    m_allowOverlaps = allowOverlaps;
    SetWorldClipPlanes(clip);
    InitCurrentAccept();
    }

}; // FenceGeometryProcessor

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FenceContext : NullContext
{
    DEFINE_T_SUPER(NullContext);

struct Request {
    BeJsConst m_value;
    Request(BeJsConst val) : m_value(val) {}
    BE_JSON_NAME(candidates)
    BE_JSON_NAME(clip)
    BE_JSON_NAME(allowOverlaps)
    BE_JSON_NAME(viewFlags)
    BE_JSON_NAME(offSubCategories)

    bool IsValid() const {return m_value.isMember(json_clip()) && m_value.isMember(json_candidates());}
    ClipVectorPtr GetClip() const {return ClipVector::FromJson(m_value[json_clip()]);}
    bool GetAllowOverlaps() const {return m_value.isMember(json_allowOverlaps()) ? m_value[json_allowOverlaps()].asBool() : false;}
    Render::ViewFlags GetViewFlags() const {Render::ViewFlags viewFlags; viewFlags.SetShowConstructions(true); if (m_value.isMember(json_viewFlags())) viewFlags.FromJson(m_value[json_viewFlags()]); return viewFlags;}

    bvector<DgnElementId> GetCandidates() const {
        bvector<DgnElementId> elements;
        auto candidates = m_value[json_candidates()];
        if (candidates.isNull() || !candidates.isArray())
            return elements;
        uint32_t nEntries = (uint32_t) candidates.size();
        for (uint32_t i=0; i < nEntries; i++) {
            DgnElementId elemId;
            elemId.FromJson(candidates[i]);
            elements.push_back(elemId);
        }
    return elements;
    }

    DgnElementIdSet GetOffSubCategories() const {
        DgnElementIdSet elements;
        auto offSubCategories = m_value[json_offSubCategories()];
        if (offSubCategories.isNull() || !offSubCategories.isArray())
            return elements;
        uint32_t nEntries = (uint32_t) offSubCategories.size();
        for (uint32_t i=0; i < nEntries; i++) {
            DgnElementId elemId;
            elemId.FromJson(offSubCategories[i]);
            elements.insert(elemId);
        }
    return elements;
    }
};

struct Response {
    BeJsValue m_value;
    Response(BeJsValue val) : m_value(val) {}
    BE_JSON_NAME(status)
    BE_JSON_NAME(candidatesContainment)
    BE_JSON_NAME(numInside)
    BE_JSON_NAME(numOutside)
    BE_JSON_NAME(numOverlap)

    void SetStatus(BentleyStatus val) {m_value[json_status()] = (uint32_t) val;}
    void SetNumInside(uint32_t val) {m_value[json_numInside()] = val;}
    void SetNumOutside(uint32_t val) {m_value[json_numOutside()] = val;}
    void SetNumOverlap(uint32_t val) {m_value[json_numOverlap()] = val;}

    void SetCandiatesStatus(bvector<ClipPlaneContainment> containments) {
        for (ClipPlaneContainment status : containments)
            m_value[json_candidatesContainment()].appendValue() = (uint32_t) status;
    }
};

FenceGeometryProcessor& m_processor;
typedef bmap<DgnGeometryPartId, ElementAlignedBox3d> PartRangeMap;
PartRangeMap m_partRanges;
DgnElementIdSet m_offSubCategories;

FenceContext(FenceGeometryProcessor& processor) : m_processor(processor)
    {
    m_purpose = processor._GetProcessPurpose();
    }

ElementAlignedBox3d GetCachedPartRange(DgnGeometryPartId partId)
    {
    PartRangeMap::const_iterator found = m_partRanges.find(partId);

    if (found != m_partRanges.end())
        return found->second;

    DgnGeometryPartCPtr partGeometry = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);
    ElementAlignedBox3d partRange = partGeometry.IsValid() ? partGeometry->GetBoundingBox() : ElementAlignedBox3d();

    m_partRanges[partId] = partRange;

    return partRange;
    }

bool IsElementVisible(GeometrySourceCR source)
    {
    if (m_offSubCategories.empty() && m_viewflags.ShowConstructions() && m_viewflags.ShowDimensions() && m_viewflags.ShowPatterns())
        return true;

    // Don't accept an element fully inside clip unless sub-category and geometry class are visible...
    GeometryStreamIO::Collection collection(source.GetGeometryStream().GetData(), source.GetGeometryStream().GetSize());
    GeometryStreamIO::Reader reader(source.GetSourceDgnDb());
    Render::GeometryParams geomParams(source.GetCategoryId());

    for (auto const& egOp : collection)
        {
        if (egOp.IsGeometryOp())
            {
            if (!IsGeometryVisible(geomParams, nullptr))
                continue;

            return true;
            }

        if (GeometryStreamIO::OpCode::BasicSymbology == egOp.m_opCode)
            reader.Get(egOp, geomParams); // Only care about sub-category and geometry class changes...
        }

    return false;
    }

bool _IsSubCategoryVisible(DgnSubCategoryId subCategoryId) override
    {
    return (m_offSubCategories.empty() || !m_offSubCategories.Contains(subCategoryId));
    }

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    {
    return new SimplifyGraphic(params, m_processor, *this);
    }

void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams) override
    {
    if (!_IsSubCategoryVisible(geomParams.GetSubCategoryId()))
        return; // Parts don't allow sub-category changes...

    Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    ElementAlignedBox3d range = GetCachedPartRange(partId);

    if (range.IsNull())
        return;

    ClipPlaneContainment status = m_processor.CheckRange(range, partToWorld);

    if (ClipPlaneContainment_Ambiguous != status)
        {
        m_processor.UpdateCurrentAccept(status);
        return;
        }

    return T_Super::_AddSubGraphic(graphic, partId, subToGraphic, geomParams);
    }

StatusInt _OutputGeometry(GeometrySourceCR source) override
    {
    ClipPlaneContainment status = m_processor.OnNewGeometrySource(source);

    if (ClipPlaneContainment_Ambiguous != status)
        {
        if (ClipPlaneContainment_StronglyInside == status)
            m_processor.m_accept = IsElementVisible(source);

        m_processor.m_earlyDecision = true;

        return SUCCESS;
        }

    return T_Super::_OutputGeometry(source);
    }

//! Query the containment status as a json value.
DGNPLATFORM_EXPORT static void DoClassify(BeJsValue out, BeJsConst input, DgnDbR db, ICancellablePtr cancel = nullptr);

}; // FenceContext

END_BENTLEY_DGN_NAMESPACE
