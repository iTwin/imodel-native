/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

static DRange3d const s_fullNpcRange =
    {
    {0.0, 0.0, 0.0},
    {1.0, 1.0, 1.0}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ViewContext()
    {
    m_purpose = DrawPurpose::NotSpecified;
    m_worldToNpc.InitIdentity();
    m_worldToView.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
    DgnElements& pool = m_dgndb->Elements();
    DgnElementCPtr el = allowLoad ? pool.GetElement(elementId) : pool.FindLoadedElement(elementId);
    if (!el.IsValid())
        return ERROR;

    GeometrySourceCP geomElem = el->ToGeometrySource();
    if (nullptr == geomElem)
        return ERROR;

    return VisitGeometry(*geomElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToNpc(DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    ViewToWorld(npcVec, screenVec, nPts);
    m_worldToNpc.M0.MultiplyAndRenormalize(npcVec, npcVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToView(DPoint3dP viewVec, DPoint3dCP npcVec, int nPts) const
    {
    NpcToWorld(viewVec, npcVec, nPts);
    WorldToView(viewVec, viewVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const
    {
    m_worldToNpc.M1.MultiplyAndRenormalize(worldPts, npcPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToNpc.M0.MultiplyAndRenormalize(npcPts, worldPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum ViewContext::GetFrustum()
    {
    Frustum frustum;
    DPoint3dP frustPts = frustum.GetPtsP();

    s_fullNpcRange.Get8Corners(frustPts);

    m_worldToNpc.M1.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
    frustum.FixOrder();
    return frustum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetDgnDb(DgnDbR dgnDb)
    {
    m_dgndb = &dgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.Multiply(viewPts, worldPts, nullptr, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.MultiplyAndRenormalize(viewPts, worldPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&t4dPt, worldPts+i, 1);

        t4dPt.GetProjectedXYZ(tPt);

        (viewPts+i)->x = (long) tPt.x;
        (viewPts+i)->y = (long) tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&t4dPt, worldPts+i, 1);

        t4dPt.GetProjectedXYZ(tPt);

        (viewPts+i)->x = tPt.x;
        (viewPts+i)->y = tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndNormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndRenormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Stroke(*this, pixelSize);

    // if we aborted, the graphic may not be complete, don't save it
    return WasAborted() ? nullptr : graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_OutputGeometry(GeometrySourceCR source)
    {
    if (!source.HasGeometry())
        return ERROR;

    double pixelSize = 0.0;

    Render::GraphicPtr graphic = _StrokeGeometry(source, pixelSize);
    if (!graphic.IsValid())
        return ERROR;

    _OutputGraphic(*graphic, &source);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams)
    {
    DgnGeometryPartCPtr partGeometry = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);

    if (!partGeometry.IsValid())
        return;

    GeometryStreamIO::Collection collection(partGeometry->GetGeometryStream().GetData(), partGeometry->GetGeometryStream().GetSize());

    auto partBuilder = graphic.CreateSubGraphic(subToGraphic);
    collection.Draw(*partBuilder, *this, geomParams, false, partGeometry.get());

    if (WasAborted()) // if we aborted, the graphic may not be complete, don't save it
        return;

    // NOTE: Need to cook GeometryParams to get GraphicParams, but we don't want to activate and bake into our Elem...
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    graphic.AddSubGraphic(*partBuilder->Finish(), subToGraphic, graphicParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_CookGeometryParams(GeometryParamsR geomParams, GraphicParamsR graphicParams)
    {
    geomParams.Resolve(*this);
    graphicParams.Cook(geomParams, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicBuilderR graphic)
    {
    GraphicParams graphicParams;

    _CookGeometryParams(geomParams, graphicParams);
    graphic.ActivateGraphicParams(graphicParams, &geomParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitGeometry(GeometrySourceCR source)
    {
    if (_CheckStop())
        return ERROR;

    return _OutputGeometry(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance)
    {
    if (m_volume.IsValid())
        {
        int nOutside = 0;

        for (int iPt=0; iPt < nPts; iPt++)
            {
            if (!m_volume->PointInside(worldPoints[iPt], tolerance))
                ++nOutside;
            }

        if (nOutside == nPts)
            return false;
        }

    if (m_ignoreViewRange)
        return true;

    return (FrustumPlanes::Contained::Outside != m_frustumPlanes.Contains(worldPoints, nPts, tolerance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsRangeVisible(DRange3dCR range, double tolerance)
    {
    Frustum box(range);

    return _AnyPointVisible(box.m_pts, 8, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsPointVisible(DPoint3dCR worldPoint, WantBoresite boresite, double tolerance)
    {
    if (WantBoresite::No == boresite)
        return _AnyPointVisible(&worldPoint, 1, tolerance);

    if (m_frustumPlanes.ContainsPoint(worldPoint, tolerance))
        return true;

    DVec3d worldZVec;
        {
        DPoint3d    zPoints[2];
        zPoints[0].Zero();
        zPoints[1].Init(0.0, 0.0, 1.0);
        NpcToWorld(zPoints, zPoints, 2);
        worldZVec.NormalizedDifference(zPoints[1], zPoints[0]);
        }

    return m_frustumPlanes.IntersectsRay(worldPoint, worldZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::MaterialPtr ViewContext::_GetMaterial(RenderMaterialId id) const
    {
    auto system = GetRenderSystem();
    return nullptr != system ? system->_GetMaterial(id, GetDgnDb()) : nullptr;
    }

static int getDefaultIndexedLineWidth(int index)
    {
    LIMIT_RANGE (0, 31, index);
    return index+1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicParams::Cook(GeometryParamsCR elParams, ViewContextR context)
    {
    Init();

    m_rasterWidth = getDefaultIndexedLineWidth(elParams.GetWeight());
    m_lineColor = m_fillColor = elParams.GetLineColor(); // NOTE: In case no fill is defined it should be set the same as line color...

    double netElemTransparency = elParams.GetNetTransparency();
    double netFillTransparency = elParams.GetNetFillTransparency();

    if (FillDisplay::Never != elParams.GetFillDisplay())
        {
        if (nullptr != elParams.GetGradient())
            {
            m_gradient = GradientSymb::Create();
            m_gradient->CopyFrom(*elParams.GetGradient());

            m_fillColor = ColorDef::White(); // Fill should be set to opaque white for gradient texture...

#if defined(TODO_ELEMENT_TILE)
            // We need to handle GradientSymb::Flags::Outline at render-time...
            if (0 == (m_gradient->GetFlags() & GradientSymb::Flags::Outline) && (FillDisplay::ByView != elParams.GetFillDisplay() || context.GetViewFlags().ShowFill()))
                {
                m_lineColor.SetAlpha(0xff); // Display checks for this to disable auto-outline...
                netElemTransparency = 0.0;  // Don't override the fully transparent outline below...
                }
#endif
            }
        else
            {
            m_fillColor = elParams.GetFillColor();

            bool outline = false;
            if (elParams.IsFillColorFromViewBackground(&outline))
                {
                // NOTE: Problem with white-on-white reversal...don't want this to apply to the background fill... :(
                if (ColorDef::White() == m_fillColor)
                    m_fillColor.SetRed(254);

                // Set line color the same as fill color if an outline fill isn't being drawn...
                if (!outline)
                    m_lineColor = m_fillColor;
                }
            }

        m_isFilled = true;
        m_isBlankingRegion = (FillDisplay::Blanking == elParams.GetFillDisplay());
        }

    if (elParams.GetMaterialId().IsValid())
        {
        m_material = context.GetMaterial(elParams.GetMaterialId());
        m_materialUVDetail = elParams.GetMaterialUVDetail();
        }

    if (0.0 != netElemTransparency)
        {
        Byte netTransparency = (Byte) (netElemTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_lineColor.SetAlpha(netTransparency);
        }

    if (0.0 != netFillTransparency)
        {
        Byte netTransparency = (Byte) (netFillTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_fillColor.SetAlpha(netTransparency);
        }

    if (nullptr != elParams.GetLineStyle())
        {
        LineStyleSymbCR lsSymb = elParams.GetLineStyle()->GetLineStyleSymb();

        if (nullptr != lsSymb.GetILineStyle())
            {
            if (lsSymb.UseLinePixels())
                {
                SetLinePixels((LinePixels) lsSymb.GetLinePixels());
                }
            else if (lsSymb.IsContinuous() && !lsSymb.GetUseStroker())
                {
                m_trueWidthStart = (lsSymb.HasOrgWidth() ? lsSymb.GetOriginWidth() : lsSymb.GetEndWidth());
                m_trueWidthEnd = (lsSymb.HasEndWidth() ? lsSymb.GetEndWidth() : lsSymb.GetOriginWidth());
                }
            else
                {
                double       textureWidth = 0.0;
                ILineStyleCP currLStyle = lsSymb.GetILineStyle();

                if (nullptr != currLStyle)
                    m_lineTexture = (const_cast<ILineStyleP> (currLStyle))->_GetTexture(textureWidth, context, elParams, false);

                if (m_lineTexture.IsValid())
                    {
                    textureWidth *= lsSymb.GetScale();
                    m_trueWidthStart = textureWidth;
                    m_trueWidthEnd = textureWidth;
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* compare two GraphicParams's to see if they're the same.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GraphicParams::operator==(GraphicParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_isFilled         != m_isFilled ||
        rhs.m_isBlankingRegion != m_isBlankingRegion ||
        rhs.m_linePixels       != m_linePixels ||
        rhs.m_rasterWidth      != m_rasterWidth ||
        rhs.m_lineColor        != m_lineColor ||
        rhs.m_fillColor        != m_fillColor ||
        rhs.m_trueWidthStart   != m_trueWidthStart ||
        rhs.m_trueWidthEnd     != m_trueWidthEnd ||
        rhs.m_lineTexture      != m_lineTexture ||
        rhs.m_material         != m_material)
        return false;

    if (!(rhs.m_gradient == m_gradient))
        return false;

    if (!rhs.m_materialUVDetail.IsEquivalent(m_materialUVDetail))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicParams::GraphicParams(GraphicParamsCR rhs)
    {
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_linePixels        = rhs.m_linePixels;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_trueWidthStart    = rhs.m_trueWidthStart;
    m_trueWidthEnd      = rhs.m_trueWidthEnd;
    m_lineTexture       = rhs.m_lineTexture;
    m_material          = rhs.m_material;
    m_materialUVDetail  = rhs.m_materialUVDetail;
    m_gradient          = rhs.m_gradient;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicParamsR GraphicParams::operator=(GraphicParamsCR rhs)
    {
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_linePixels        = rhs.m_linePixels;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_trueWidthStart    = rhs.m_trueWidthStart;
    m_trueWidthEnd      = rhs.m_trueWidthEnd;
    m_lineTexture       = rhs.m_lineTexture;
    m_material          = rhs.m_material;
    m_materialUVDetail  = rhs.m_materialUVDetail;
    m_gradient          = rhs.m_gradient;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryParams::GeometryParams(GeometryParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_resolved              = rhs.m_resolved;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_materialId            = rhs.m_materialId;
    m_materialUVDetail      = rhs.m_materialUVDetail;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_backgroundFill        = rhs.m_backgroundFill;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_geometryClass         = rhs.m_geometryClass;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryParamsR GeometryParams::operator=(GeometryParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_resolved              = rhs.m_resolved;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_materialId            = rhs.m_materialId;
    m_materialUVDetail      = rhs.m_materialUVDetail;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_backgroundFill        = rhs.m_backgroundFill;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_geometryClass         = rhs.m_geometryClass;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void GeometryParams::ResetAppearance()
    {
    AutoRestore<DgnCategoryId> saveCategory(&m_categoryId);
    AutoRestore<DgnSubCategoryId> saveSubCategory(&m_subCategoryId);

    *this = GeometryParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryParams::IsEquivalent(GeometryParamsCR other) const
    {
    if (this == &other)
        return true;

    if (m_categoryId != other.m_categoryId)
        return false;

    if (m_subCategoryId != other.m_subCategoryId)
        return false;

    if (m_geometryClass != other.m_geometryClass)
        return false;

    // Don't compare m_netPriority, compare the inputs: m_elmPriority + m_subCateogoryId.
    if (m_elmPriority != other.m_elmPriority)
        return false;

    // Don't compare m_netElmTransparency, compare the inputs: m_elmTransparency + m_subCateogoryId.
    if (m_elmTransparency != other.m_elmTransparency)
        return false;

    // Don't compare m_netFillTransparency, compare the inputs: m_fillTransparency + m_subCateogoryId.
    if (m_fillTransparency != other.m_fillTransparency)
        return false;

    // Don't compare m_lineColor unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_color != other.m_appearanceOverrides.m_color)
        return false;

    if (m_appearanceOverrides.m_color && (m_lineColor != other.m_lineColor))
        return false;

    // Don't compare m_weight unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_weight != other.m_appearanceOverrides.m_weight)
        return false;

    if (m_appearanceOverrides.m_weight && (m_weight != other.m_weight))
        return false;

    // Don't compare m_materialId unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_material != other.m_appearanceOverrides.m_material)
        return false;


    if (m_appearanceOverrides.m_material && (m_materialId != other.m_materialId))
        return false;

    if (!m_materialUVDetail.IsEquivalent(other.m_materialUVDetail))
        return false;

    // Don't compare m_styleInfo unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_style != other.m_appearanceOverrides.m_style)
        return false;

    if (m_appearanceOverrides.m_style)
        {
        if (m_styleInfo.IsValid() != other.m_styleInfo.IsValid())
            return false;

        if (m_styleInfo.IsValid())
            {
            // NOTE: Don't use == operator on LineStyleInfo, don't want to compare LineStyleSymb since it will differ between resolved/un-resolved params...
            if (m_styleInfo->GetStyleId() != other.m_styleInfo->GetStyleId())
                return false;

            LineStyleParamsCP thisParams = m_styleInfo->GetStyleParams();
            LineStyleParamsCP otherParams = other.m_styleInfo->GetStyleParams();

            if ((nullptr == thisParams) != (nullptr == otherParams))
                return false;

            if ((nullptr != thisParams) && !(*thisParams == *otherParams))
                return false;
            }
        }

    if (m_fillDisplay != other.m_fillDisplay)
        return false;

    if (FillDisplay::Never != m_fillDisplay)
        {
        if (m_appearanceOverrides.m_fill != other.m_appearanceOverrides.m_fill)
            return false;

        if (m_gradient.IsValid() != other.m_gradient.IsValid())
            return false;

        if (m_gradient.IsValid() && !(*m_gradient == *other.m_gradient))
            return false;

        // Don't compare m_fillColor unless sub-category appearance override is set...
        if (m_appearanceOverrides.m_fill)
            {
            if (m_backgroundFill != other.m_backgroundFill)
                return false;

            if (BackgroundFill::None == m_backgroundFill && m_fillColor != other.m_fillColor)
                return false;
            }
        }

    if (m_pattern.IsValid() != other.m_pattern.IsValid())
        return false;

    if (m_pattern.IsValid() && !(*m_pattern == *other.m_pattern))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::Resolve(DgnDbR dgnDb)
    {
    if (m_resolved)
        return; // Already resolved...

    BeAssert(m_subCategoryId.IsValid());
    if (!m_subCategoryId.IsValid())
        return;

    // Setup from SubCategory appearance...
    DgnSubCategoryCPtr subCat = DgnSubCategory::Get(dgnDb, m_subCategoryId);
    BeAssert(subCat.IsValid());
    if (!subCat.IsValid())
        return;

    auto appearance = subCat->GetAppearance();

    if (!m_appearanceOverrides.m_color)
        m_lineColor = appearance.GetColor();

    if (!m_appearanceOverrides.m_fill)
        m_fillColor = appearance.GetFillColor();
    else if (BackgroundFill::None != m_backgroundFill)
        m_fillColor = ColorDef::Black();

    if (!m_appearanceOverrides.m_weight)
        m_weight = appearance.GetWeight();

    if (!m_appearanceOverrides.m_style)
        m_styleInfo = LineStyleInfo::Create(appearance.GetStyle(), nullptr).get(); // WIP_LINESTYLE - Need LineStyleParams...

    if (!m_appearanceOverrides.m_material)
        m_materialId = appearance.GetRenderMaterial();

    // combine transparencies by multiplying the opaqueness.
    // A 50% transparent element on a 50% transparent category should give a 75% transparent result.
    // (1 - ((1 - .5) * (1 - .5))
    double categoryTransparency = appearance.GetTransparency();
    double categoryFillTransparency = appearance.GetFillTransparency();

    if (0.0 != categoryTransparency)
        {
        double elementOpaque = 1.0 - m_elmTransparency;
        double categoryOpaque = 1.0 - categoryTransparency;

        m_netElmTransparency = (1.0 - (elementOpaque * categoryOpaque));
        }

    if (0.0 != categoryFillTransparency)
        {
        double fillOpaque = 1.0 - m_fillTransparency;
        double categoryFillOpaque = 1.0 - categoryFillTransparency;

        m_netFillTransparency = (1.0 - (fillOpaque * categoryFillOpaque));
        }

    // SubCategory display priority is combined with element priority to compute net display priority.
    m_netPriority = m_elmPriority + appearance.GetDisplayPriority();

    if (m_styleInfo.IsValid() && nullptr == m_styleInfo->GetLineStyleSymb().GetILineStyle())
        m_styleInfo->Resolve(dgnDb);

    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::Resolve(ViewContextR context)
    {
    Resolve(context.GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::ApplyTransform(TransformCR transform, uint32_t options)
    {
    if (m_pattern.IsValid())
        m_pattern->ApplyTransform(transform, options);

    if (m_styleInfo.IsValid())
        {
        double oldScale = m_styleInfo->GetStyleParamsR().scale;
        m_styleInfo->GetStyleParamsR().ApplyTransform(transform, options);
        double newScale = m_styleInfo->GetStyleParamsR().scale;
        if (!DoubleOps::AlmostEqual(oldScale, newScale, 1.0e-5))
            {
            m_styleInfo->GetLineStyleSymbR().Init(nullptr);
            m_resolved = false; // max style width invalid, need to resolve with new scales...
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr ViewContext::_CreateTexture(Render::ImageCR image) const
    {
    Render::TexturePtr tx;
    auto sys = GetRenderSystem();
    if (nullptr != sys)
        tx = sys->_CreateTexture(image, GetDgnDb());

    return tx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr ViewContext::_CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp) const
    {
    Render::TexturePtr tx;
    auto sys = GetRenderSystem();
    if (nullptr != sys)
        tx = sys->_CreateTexture(source, bottomUp, GetDgnDb());

    return tx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsGeometryVisible(Render::GeometryParamsCR geomParams, DRange3dCP range)
    {
    ViewFlags   viewFlags = GetViewFlags();

    switch (geomParams.GetGeometryClass())
        {
        case DgnGeometryClass::Construction:
            if (!viewFlags.ShowConstructions())
                return false;
            break;

        case DgnGeometryClass::Dimension:
            if (!viewFlags.ShowDimensions())
                return false;
            break;

        case DgnGeometryClass::Pattern:
            if (!viewFlags.ShowPatterns())
                return false;
            break;
        }

    if (nullptr != range && !range->IsNull())
        {
        if (!IsRangeVisible(*range))
            return false; // Sub-graphic outside range...
        }

    return _IsSubCategoryVisible(geomParams.GetSubCategoryId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::_DepthFromDisplayPriority(int32_t priority) const
    {
    static constexpr int32_t maxPriority = (1<<23)-32;
    static constexpr double factor = ViewDefinition2d::Get2dFrustumDepth() / (double) (maxPriority+1);
    return factor * (double) priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum::Frustum(RTree3dValCR from)
    {
    m_pts[0].x = m_pts[2].x = m_pts[4].x = m_pts[6].x = from.m_minx;
    m_pts[1].x = m_pts[3].x = m_pts[5].x = m_pts[7].x = from.m_maxx;
    m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = from.m_miny;
    m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = from.m_maxy;
    m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = from.m_minz;
    m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = from.m_maxz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum::Frustum(DRange3dCR range)
    {
    m_pts[0].x = m_pts[2].x = m_pts[4].x = m_pts[6].x = range.low.x;
    m_pts[1].x = m_pts[3].x = m_pts[5].x = m_pts[7].x = range.high.x;
    m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = range.low.y;
    m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = range.high.y;
    m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = range.low.z;
    m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = range.high.z;
    }

/*---------------------------------------------------------------------------------**/ /**
* determine whether the points in the given polyhedron are in the correct order
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Frustum::HasMirror()
    {
    DPoint3dP polyhedron = GetPtsP();

    DVec3d u, v, w;
    u.DifferenceOf(polyhedron[NPC_001], polyhedron[NPC_000]);
    v.DifferenceOf(polyhedron[NPC_010], polyhedron[NPC_000]);
    w.DifferenceOf(polyhedron[NPC_100], polyhedron[NPC_000]);

    return u.TripleProduct(v, w) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Frustum::ScaleAboutCenter(double scale)
    {
    Frustum orig = *this;
    double f = 0.5 * (1.0 + scale);
    m_pts[NPC_000].Interpolate(orig.GetCorner(NPC_111), f, orig.GetCorner(NPC_000));
    m_pts[NPC_100].Interpolate(orig.GetCorner(NPC_011), f, orig.GetCorner(NPC_100));
    m_pts[NPC_010].Interpolate(orig.GetCorner(NPC_101), f, orig.GetCorner(NPC_010));
    m_pts[NPC_110].Interpolate(orig.GetCorner(NPC_001), f, orig.GetCorner(NPC_110));
    m_pts[NPC_001].Interpolate(orig.GetCorner(NPC_110), f, orig.GetCorner(NPC_001));
    m_pts[NPC_101].Interpolate(orig.GetCorner(NPC_010), f, orig.GetCorner(NPC_101));
    m_pts[NPC_011].Interpolate(orig.GetCorner(NPC_100), f, orig.GetCorner(NPC_011));
    m_pts[NPC_111].Interpolate(orig.GetCorner(NPC_000), f, orig.GetCorner(NPC_111));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DMap4d Frustum::ToDMap4d() const
    {
    DPoint3d org = GetCorner(NPC_LeftBottomRear);
    DVec3d xVec = DVec3d::FromStartEnd(org, GetCorner(NPC_RightBottomRear));
    DVec3d yVec = DVec3d::FromStartEnd(org, GetCorner(NPC_LeftTopRear));
    DVec3d zVec = DVec3d::FromStartEnd(org, GetCorner(NPC_LeftBottomFront));
    DMap4d map;
    bsiDMap4d_initFromVectorFrustum(&map, &org, &xVec, &yVec, &zVec, GetFraction());
    return map;
    }

/*---------------------------------------------------------------------------------**//**
* determine whether the points in the given polyhedron are in the correct order or are inside out. If not, reverse sense
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Frustum::FixOrder()
    {
    if (!HasMirror())
        return;

    // frustum has mirroring, reverse points
    DPoint3dP polyhedron = GetPtsP();
    for (int i = 0; i < 8; i += 2)
        {
        DPoint3d tmpPoint = polyhedron[i];
        polyhedron[i] = polyhedron[i+1];
        polyhedron[i+1] = tmpPoint;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromJson(BeJsConst val)
    {
    memset(this, 0, sizeof(*this));
    m_constructions = !val[json_noConstruct()].asBool();
    m_dimensions = !val[json_noDim()].asBool();
    m_patterns = !val[json_noPattern()].asBool();
    m_weights = !val[json_noWeight()].asBool();
    m_styles = !val[json_noStyle()].asBool();
    m_transparency = !val[json_noTransp()].asBool();
    m_fill = !val[json_noFill()].asBool();
    m_grid = val[json_grid()].asBool();
    m_acsTriad = val[json_acs()].asBool();
    m_textures = !val[json_noTexture()].asBool();
    m_materials = !val[json_noMaterial()].asBool();
    m_cameraLights = !val[json_noCameraLights()].asBool();
    m_sourceLights = !val[json_noSourceLights()].asBool();
    m_solarLight = !val[json_noSolarLight()].asBool();
    m_visibleEdges = val[json_visEdges()].asBool();
    m_hiddenEdges = val[json_hidEdges()].asBool();
    m_shadows = val[json_shadows()].asBool();
    m_noClipVolume = !val[json_clipVol()].asBool();
    m_monochrome = val[json_monochrome()].asBool();
    m_edgeMask = val[json_edgeMask()].asUInt();
    m_hLineMaterialColors = val[json_hlMatColors()].asBool();
    m_thematicDisplay = val[json_thematicDisplay()].asBool();
    m_wiremesh = val[json_wiremesh()].asBool();
    m_ambientOcclusion = val[json_ambientOcclusion()].asBool();
    m_backgroundMap = val[json_backgroundMap()].asBool();
    m_forceSurfaceDiscard = val[json_forceSurfaceDiscard()].asBool();
    m_noWhiteOnWhiteReversal = val[json_noWhiteOnWhiteReversal()].asBool();

    // Validate render mode. V8 converter only made sure to set everything above Phong to Smooth...
    uint32_t renderModeValue = val[json_renderMode()].asUInt();

    if (renderModeValue < (uint32_t)RenderMode::HiddenLine)
        m_renderMode = RenderMode::Wireframe;
    else if (renderModeValue > (uint32_t)RenderMode::SolidFill)
        m_renderMode = RenderMode::SmoothShade;
    else
        m_renderMode = RenderMode(renderModeValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::ToJson(BeJsValue val) const
    {
    if (!m_constructions) val[json_noConstruct()] = true;
    if (!m_dimensions) val[json_noDim()] = true;
    if (!m_patterns) val[json_noPattern()] = true;
    if (!m_weights) val[json_noWeight()] = true;
    if (!m_styles) val[json_noStyle()] = true;
    if (!m_transparency) val[json_noTransp()] = true;
    if (!m_fill) val[json_noFill()] = true;
    if (m_grid) val[json_grid()] = true;
    if (m_acsTriad) val[json_acs()] = true;
    if (!m_textures) val[json_noTexture()] = true;
    if (!m_materials) val[json_noMaterial()] = true;
    if (!m_cameraLights) val[json_noCameraLights()] = true;
    if (!m_sourceLights) val[json_noSourceLights()] = true;
    if (!m_solarLight) val[json_noSolarLight()] = true;
    if (m_visibleEdges) val[json_visEdges()] = true;
    if (m_hiddenEdges) val[json_hidEdges()] = true;
    if (m_shadows) val[json_shadows()] = true;
    if (!m_noClipVolume) val[json_clipVol()] = true;
    if (m_hLineMaterialColors) val[json_hlMatColors()] = true;
    if (m_monochrome) val[json_monochrome()] = true;
    if (m_backgroundMap) val[json_backgroundMap()] = true;
    if (m_thematicDisplay) val[json_thematicDisplay()] = true;
    if (m_wiremesh) val[json_wiremesh()] = true;
    if (m_ambientOcclusion) val[json_ambientOcclusion()] = true;
    if (m_edgeMask!=0) val[json_edgeMask()] = m_edgeMask;
    if (m_forceSurfaceDiscard) val[json_forceSurfaceDiscard()] = true;
    if (m_noWhiteOnWhiteReversal) val[json_noWhiteOnWhiteReversal()] = true;

    val[json_renderMode()] = (uint8_t) m_renderMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlagsOverrides::ViewFlagsOverrides(ViewFlags base) : m_present(0xffffffff), m_values(base)
    {
    // NB: A handful of flags (grid, acs) cannot be overridden on a per-branch basis...ignore.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlagsOverrides::Apply(ViewFlags& base) const
    {
    if (!AnyOverridden())
        return;

    if (IsPresent(kDimensions)) base.SetShowDimensions(m_values.ShowDimensions());
    if (IsPresent(kPatterns)) base.SetShowPatterns(m_values.ShowPatterns());
    if (IsPresent(kWeights)) base.SetShowWeights(m_values.ShowWeights());
    if (IsPresent(kStyles)) base.SetShowStyles(m_values.ShowStyles());
    if (IsPresent(kTransparency)) base.SetShowTransparency(m_values.ShowTransparency());
    if (IsPresent(kFill)) base.SetShowFill(m_values.ShowFill());
    if (IsPresent(kTextures)) base.SetShowTextures(m_values.ShowTextures());
    if (IsPresent(kMaterials)) base.SetShowMaterials(m_values.ShowMaterials());
    if (IsPresent(kVisibleEdges)) base.SetShowVisibleEdges(m_values.ShowVisibleEdges());
    if (IsPresent(kHiddenEdges)) base.SetShowHiddenEdges(m_values.ShowHiddenEdges());
    if (IsPresent(kShadows)) base.SetShowShadows(m_values.ShowShadows());
    if (IsPresent(kClipVolume)) base.SetShowClipVolume(m_values.ShowClipVolume());
    if (IsPresent(kBackgroundMap)) base.SetShowBackgroundMap(m_values.ShowBackgroundMap());
    if (IsPresent(kConstructions)) base.SetShowConstructions(m_values.ShowConstructions());
    if (IsPresent(kMonochrome)) base.SetMonochrome(m_values.IsMonochrome());
    if (IsPresent(kGeometryMap)) base.SetIgnoreGeometryMap(m_values.IgnoreGeometryMap());
    if (IsPresent(kHlineMaterialColors)) base.SetUseHlineMaterialColors(m_values.UseHlineMaterialColors());
    if (IsPresent(kEdgeMask)) base.SetEdgeMask(m_values.GetEdgeMask());
    if (IsPresent(kRenderMode)) base.SetRenderMode(m_values.GetRenderMode());
    if (IsPresent(kForceSurfaceDiscard)) base.SetForceSurfaceDiscard(m_values.ForceSurfaceDiscard());
    if (IsPresent(kWhiteOnWhiteReversal)) base.SetApplyWhiteOnWhiteReversal(m_values.ApplyWhiteOnWhiteReversal());
    if (IsPresent(kThematicDisplay)) base.SetThematicDisplay(m_values.GetThematicDisplay());
    if (IsPresent(kWiremesh)) base.SetWiremeshDisplay(m_values.GetWiremeshDisplay());

    if (IsPresent(kLighting))
        {
        auto lighting = m_values.ShowSolarLight() || m_values.ShowCameraLights() || m_values.ShowSourceLights();
        base.SetShowSolarLight(lighting);
        base.SetShowCameraLights(lighting);
        base.SetShowSourceLights(lighting);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlagsOverrides::ToJson(BeJsValue val) const
    {
    val.SetNull();
    auto setBoolean = [&](PresenceFlag flag, Utf8CP name, bool value)
        {
        if (IsPresent(flag))
            val[name] = value;
        };

    setBoolean(kDimensions, json_dimensions(), m_values.ShowDimensions());
    setBoolean(kPatterns, json_patterns(), m_values.ShowPatterns());
    setBoolean(kWeights, json_weights(), m_values.ShowWeights());
    setBoolean(kStyles, json_styles(), m_values.ShowStyles());
    setBoolean(kTransparency, json_transparency(), m_values.ShowTransparency());
    setBoolean(kFill, json_fill(), m_values.ShowFill());
    setBoolean(kTextures, json_textures(), m_values.ShowTextures());
    setBoolean(kMaterials, json_materials(), m_values.ShowMaterials());
    setBoolean(kLighting, json_lighting(), m_values.ShowSourceLights() || m_values.ShowCameraLights() || m_values.ShowSolarLight());
    setBoolean(kVisibleEdges, json_visibleEdges(), m_values.ShowVisibleEdges());
    setBoolean(kHiddenEdges, json_hiddenEdges(), m_values.ShowHiddenEdges());
    setBoolean(kShadows, json_shadows(), m_values.ShowShadows());
    setBoolean(kClipVolume, json_clipVolume(), m_values.ShowClipVolume());
    setBoolean(kConstructions, json_constructions(), m_values.ShowConstructions());
    setBoolean(kMonochrome, json_monochrome(), m_values.IsMonochrome());
    setBoolean(kGeometryMap, json_noGeometryMap(), m_values.IgnoreGeometryMap());
    setBoolean(kBackgroundMap, json_backgroundMap(), m_values.ShowBackgroundMap());
    setBoolean(kHlineMaterialColors, json_hLineMaterialColors(), m_values.UseHlineMaterialColors());
    setBoolean(kForceSurfaceDiscard, json_forceSurfaceDiscard(), m_values.ForceSurfaceDiscard());
    setBoolean(kWhiteOnWhiteReversal, json_whiteOnWhiteReversal(), m_values.ApplyWhiteOnWhiteReversal());
    setBoolean(kThematicDisplay, json_thematicDisplay(), m_values.GetThematicDisplay());
    setBoolean(kWiremesh, json_wiremesh(), m_values.GetWiremeshDisplay());

    if (IsPresent(kEdgeMask))
        val[json_edgeMask()] = m_values.GetEdgeMask();

    if (IsPresent(kRenderMode))
        val[json_renderMode()] = static_cast<int32_t>(m_values.GetRenderMode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlagsOverrides ViewFlagsOverrides::FromJson(BeJsConst val)
    {
    ViewFlagsOverrides ovrs;
    if (val.isNull() || !val.isObject())
        return ovrs;

    typedef void (ViewFlagsOverrides::*SetBoolean)(bool);
    auto setBoolean = [&](Utf8CP name, SetBoolean set)
        {
        if (val[name].isBool())
            ((ovrs).*(set))(val[name].asBool());
        };

    setBoolean(json_dimensions(), &ViewFlagsOverrides::SetShowDimensions);
    setBoolean(json_patterns(), &ViewFlagsOverrides::SetShowPatterns);
    setBoolean(json_weights(), &ViewFlagsOverrides::SetShowWeights);
    setBoolean(json_styles(), &ViewFlagsOverrides::SetShowStyles);
    setBoolean(json_transparency(), &ViewFlagsOverrides::SetShowTransparency);
    setBoolean(json_fill(), &ViewFlagsOverrides::SetShowFill);
    setBoolean(json_textures(), &ViewFlagsOverrides::SetShowTextures);
    setBoolean(json_materials(), &ViewFlagsOverrides::SetShowMaterials);
    setBoolean(json_lighting(), &ViewFlagsOverrides::SetApplyLighting);
    setBoolean(json_visibleEdges(), &ViewFlagsOverrides::SetShowVisibleEdges);
    setBoolean(json_hiddenEdges(), &ViewFlagsOverrides::SetShowHiddenEdges);
    setBoolean(json_shadows(), &ViewFlagsOverrides::SetShowShadows);
    setBoolean(json_clipVolume(), &ViewFlagsOverrides::SetShowClipVolume);
    setBoolean(json_constructions(), &ViewFlagsOverrides::SetShowConstructions);
    setBoolean(json_monochrome(), &ViewFlagsOverrides::SetMonochrome);
    setBoolean(json_noGeometryMap(), &ViewFlagsOverrides::SetIgnoreGeometryMap);
    setBoolean(json_backgroundMap(), &ViewFlagsOverrides::SetShowBackgroundMap);
    setBoolean(json_hLineMaterialColors(), &ViewFlagsOverrides::SetUseHlineMaterialColors);
    setBoolean(json_forceSurfaceDiscard(), &ViewFlagsOverrides::SetForceSurfaceDiscard);
    setBoolean(json_whiteOnWhiteReversal(), &ViewFlagsOverrides::SetApplyWhiteOnWhiteReversal);
    setBoolean(json_thematicDisplay(), &ViewFlagsOverrides::SetThematicDisplay);
    setBoolean(json_wiremesh(), &ViewFlagsOverrides::SetWiremeshDisplay);

    if (val[json_renderMode()].isNumeric())
        ovrs.SetRenderMode(static_cast<RenderMode>(val[json_renderMode()].asInt()));

    if (val[json_edgeMask()].isNumeric())
        ovrs.SetEdgeMask(val[json_edgeMask()].asInt());

    return ovrs;
    }

