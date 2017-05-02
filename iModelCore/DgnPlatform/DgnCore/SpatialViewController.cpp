/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SpatialViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/BeSystemInfo.h>
#include <DgnPlatform/ElementTileTree.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum::Frustum(RTree3dValCR from)
    {                                                                  //       7+------+6
    m_pts[0].x = m_pts[3].x = m_pts[4].x = m_pts[7].x = from.m_minx;   //       /|     /|
    m_pts[1].x = m_pts[2].x = m_pts[5].x = m_pts[6].x = from.m_maxx;   //      / |    / |
    m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = from.m_miny;   //     / 4+---/--+5
    m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = from.m_maxy;   //   3+------+2 /    y   z
    m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = from.m_minz;   //    | /    | /     |  /
    m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = from.m_maxz;   //    |/     |/      |/
    }                                                                  //   0+------+1      *---x

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum::Frustum(DRange3dCR range)
    {
    m_pts[0].x = m_pts[3].x = m_pts[4].x = m_pts[7].x = range.low.x;
    m_pts[1].x = m_pts[2].x = m_pts[5].x = m_pts[6].x = range.high.x;
    m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = range.low.y;
    m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = range.high.y;
    m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = range.low.z;
    m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = range.high.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewController::SpatialViewController(SpatialViewDefinitionCR def) : T_Super(def)
    {
    m_viewSQL = "SELECT e.Id FROM " BIS_TABLE(BIS_CLASS_Element) " AS e, " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g "
                "WHERE g.ElementId=e.Id AND InVirtualSet(@vset,e.ModelId,g.CategoryId) AND e.Id=@elId";
    }

/*---------------------------------------------------------------------------------**//**
* SpatialViews decorate the copyright messages from reality models, if any.
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_DrawDecorations(DecorateContextR context)
    {
    DrawGroundPlane(context);

    if (m_copyrightMsgs.empty())
        return;

    DgnViewportCR vp = *context.GetViewport();

    static double const TEXT_HEIGHT_INCHES = 0.1;
    double textHeight = vp.PixelsFromInches(TEXT_HEIGHT_INCHES);
    double padding = (textHeight / 2.0);
    
    TextString textString;
    textString.GetStyleR().SetFont(DgnFontManager::GetDecoratorFont());
    textString.GetStyleR().SetSize(textHeight);
    textString.SetOrientation(RotMatrix::FromScaleFactors(1.0, -1.0, 1.0)); // y is flipped in view coords

    BSIRect viewRect = vp.GetViewRect();
    DPoint3d textBottomRight = DPoint3d::From(viewRect.Right() - padding, viewRect.Bottom() - padding);
    DRange2d runningTextBounds = DRange2d::NullRange();

    // Always draw text in black, then create a white blanking region behind it so that it's always visible.
    Render::GraphicBuilderPtr graphic = context.CreateGraphic();
    graphic->SetSymbology(ColorDef::Black(), ColorDef::Black(), 0);

    for (Utf8StringCR msg : m_copyrightMsgs)
        {
        textString.SetText(msg.c_str());
        textString.SetOriginFromJustificationOrigin(textBottomRight, TextString::HorizontalJustification::Right, TextString::VerticalJustification::Bottom);
        graphic->AddTextString(textString);
        
        // Text's range is a tight box around the ascent of the characters. Give it some padding for aesthetics.
        DRange2d textRange = textString.GetRange();
        textRange.low.x -= padding;
        textRange.low.y -= padding;
        textRange.high.x += padding;
        textRange.high.y += padding;

        // Accumulate the screen range to draw the unioned blanking region below.
        DRange2d screenTextRange;
        textString.ComputeTransform().Multiply(&screenTextRange.low, &textRange.low, 2);
        runningTextBounds.Extend(screenTextRange);
                                                                            
        // Advance up a line with some spacing for subsequent notices.
        textBottomRight.y -= (textHeight + padding);
        }

    static int const FILL_TRANSPARENCY = 128;
    ColorDef bgColor = vp.MakeColorTransparency(ColorDef::White(), FILL_TRANSPARENCY);
    graphic->SetBlankingFill(bgColor);
    
    DPoint3d textShape[4];
    textShape[0].Init(runningTextBounds.low);
    textShape[1].Init(runningTextBounds.low.x, runningTextBounds.high.y);
    textShape[2].Init(runningTextBounds.high.x, runningTextBounds.high.y);
    textShape[3].Init(runningTextBounds.high.x, runningTextBounds.low.y);
    graphic->AddShape(_countof(textShape), textShape, true);

    context.AddViewOverlay(*graphic->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d SpatialViewController::_GetViewedExtents(DgnViewportCR vp) const
    {
    AxisAlignedBox3d box = GetDgnDb().GeoLocation().GetProjectExtents();
    box.Extend(GetGroundExtents(vp));
    return box;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialViewController::_CreateScene(RenderContextR context)
    {
    DgnDb::VerifyClientThread();

#if defined(ELEMENT_TILE_PROFILE_RANGE_INDEX)
    static bool s_filledRangeIndices = false;
    if (!s_filledRangeIndices)
        {
        StopWatch profileTimer(true);
        uint32_t numFilled = 0;
        for (auto modelId : GetViewedModels())
            {
            auto model = GetDgnDb().Models().Get<GeometricModel>(modelId);
            if (model.IsValid())
                {
                model->FillRangeIndex();
                ++numFilled;
                }
            }

        double rangeIndexTime = profileTimer.GetCurrentSeconds();
        s_filledRangeIndices = true;
        Utf8PrintfString msg("Filled %u range indices in %f seconds", numFilled, rangeIndexTime);
        NotifyMessageDetails details(OutputMessagePriority::Info, msg.c_str());
        T_HOST.GetNotificationAdmin()._OutputMessage(details);
        }
#endif

#if defined(ELEMENT_TILE_PROFILE_CREATE_ROOT)
    if (!m_allRootsLoaded)
        {
        StopWatch rootsTimer(true);
        uint32_t numCreated = 0;
        for (auto modelId : GetViewedModels())
            {
            auto model = GetDgnDb().Models().Get<GeometricModel3d>(modelId);
            TileTree::RootPtr modelRoot;
            if (model.IsValid())
                {
                modelRoot = model->GetTileTree(context.GetTargetR().GetSystem());
                ++numCreated;
                }

            m_roots.Insert(modelId, modelRoot);
            }

        Utf8PrintfString msg("Loaded %u roots in %f seconds", numCreated, rootsTimer.GetCurrentSeconds());
        NotifyMessageDetails details(OutputMessagePriority::Info, msg.c_str());
        T_HOST.GetNotificationAdmin()._OutputMessage(details);
        m_allRootsLoaded = true;
        }
#endif

    StopWatch timer(true);

    DrawSkyBox(context);

    if (!m_allRootsLoaded)
        {
        // NB: The UpdatePlan's 'timeout' exists for scene creation...is not handled by context.CheckStop()...
        // ###TODO_ELEMENT_TILE: UpdatePlan is on RenderListContext...
        auto const& plan = static_cast<RenderListContext&>(context).GetUpdatePlan().GetQuery();
        uint64_t endTime = plan.GetTimeout() ? (BeTimeUtilities::QueryMillisecondsCounter() + plan.GetTimeout()) : 0;

        // Create as many tile trees as we can within the allotted time...
        bool timedOut = false;
        for (auto modelId : GetViewedModels())
            {
            auto iter = m_roots.find(modelId);
            if (m_roots.end() == iter)
                {
                auto model = GetDgnDb().Models().Get<GeometricModel3d>(modelId);
                TileTree::RootPtr modelRoot;
                if (model.IsValid())
                    {
                    modelRoot = model->GetTileTree(&context.GetTargetR().GetSystem());
                    Utf8String message = model->GetCopyrightMessage();
                    if (!message.empty()) // skip emptry strings.
                        m_copyrightMsgs.insert(message);
                    }

                m_roots.Insert(modelId, modelRoot);

                if (endTime && (BeTimeUtilities::QueryMillisecondsCounter() > endTime))
                    {
                    DEBUG_PRINTF("CreateScene aborted");
                    timedOut = true;
                    break;
                    }
                }
            }

        m_allRootsLoaded = !timedOut;
        }

    // Always draw all the tile trees we currently have...
    // NB: We assert that m_roots will contain ONLY models that are in our viewed models list (it may not yet contain ALL of them though)
    for (auto pair : m_roots)
        if (pair.second.IsValid())
            pair.second->DrawInView(context);

    //DEBUG_PRINTF("CreateScene: %f", timer.GetCurrentSeconds());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::SetAlwaysDrawn(DgnElementIdSet const& newSet, bool exclusive)
    {
    RequestAbort(true);
    m_noQuery = exclusive;
    m_special.m_always = newSet; // NB: copies values
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ClearAlwaysDrawn()
    {
    RequestAbort(true);
    m_special.m_always.clear();
    m_noQuery = false;
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::SetNeverDrawn(DgnElementIdSet const& newSet)
    {
    RequestAbort(true);
    m_special.m_never = newSet; // NB: copies values
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ClearNeverDrawn()
    {
    RequestAbort(true);
    m_special.m_never.clear();
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::SetViewFlags(Render::ViewFlags newFlags)
    {
    auto oldFlags = GetViewFlags();
    bool dirty = oldFlags.ShowConstructions() != newFlags.ShowConstructions()
              || oldFlags.ShowDimensions() != newFlags.ShowDimensions()
              || oldFlags.ShowPatterns() != newFlags.ShowPatterns();
    if (dirty)
        SetFeatureSymbologyDirty();

    m_definition->GetDisplayStyle().SetViewFlags(newFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::DropSubCategoryOverride(DgnSubCategoryId id)
    {
    m_definition->GetDisplayStyle().DropSubCategoryOverride(id);
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::OverrideSubCategory(DgnSubCategoryId id, DgnSubCategory::Override const& ovr)
    {
    m_definition->GetDisplayStyle().OverrideSubCategory(id, ovr);
    SetFeatureSymbologyDirty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    auto& models = GetSpatialViewDefinition().GetModelSelector().GetModelsR();
    if (onOff == models.Contains(modelId))
        return;

    RequestAbort(true);
    if (onOff)
        {
        models.insert(modelId);
        }
    else
        {
        models.erase(modelId);
        m_roots.erase(m_roots.find(modelId));
        }

    m_allRootsLoaded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_OnCategoryChange(bool singleEnabled)
    {
    T_Super::_OnCategoryChange(singleEnabled);
    RequestAbort(true);

    // Category stuff is baked into the tiles (for now) - throw them away
    // ###TODO_ELEMENT_TILES: If we were informed about the delta in the set of viewed categories, we could throw away only those tiles affected.
    m_roots.clear();
    m_allRootsLoaded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::RequestAbort(bool wait)
    {
    // ###TODO_ELEMENT_TILE: This is generally called when the criteria defining the scene have changed. e.g., we turned off a category or changed the sets of always/never drawn elemets.
    // In tile-based rendering we will want to invalidate tiles - though we'd probably also like to change how these criteria are defined and applied.
    DgnDb::VerifyClientThread();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewController::ElementsQuery::TestElement(DgnElementId elId)
    {
    if (IsNever(elId))
        return false;

    m_viewStmt->BindId(m_idCol, elId);
    bool stat = (BE_SQLITE_ROW == m_viewStmt->Step());
    m_viewStmt->Reset();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* virtual method bound to the "InVirtualSet(@vset,e.ModelId,g.CategoryId)" SQL function for view query.
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewController::_IsInSet(int nVals, DbValue const* vals) const
    {
    BeAssert(nVals == 2);   // we need ModelId and Category

    // check that both the model is on and the category is on.
    return GetViewedModels().Contains(DgnModelId(vals[0].GetValueUInt64())) && GetViewedCategories().Contains(DgnCategoryId(vals[1].GetValueUInt64()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_DrawView(ViewContextR context)
    {
    if (m_activeVolume.IsValid())
        context.SetActiveVolume(*m_activeVolume);

    _VisitAllElements(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_PickTerrain(PickContextR context)
    {
    auto& models = GetDgnDb().Models();
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = models.GetModel(modelId);
        if (!model.IsValid())
            continue;

        auto geomModel = model->ToGeometricModel3d();
        if (nullptr != geomModel)
            geomModel->_PickTerrainGraphics(context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::SceneLightsPtr SpatialViewController::GetLights() const
    {
    DgnDb::VerifyClientThread();
    if (m_lights.IsValid())
        return m_lights;
        
    Render::TargetP target = m_vp->GetRenderTarget();
    if (nullptr == target)
        return nullptr;

    auto& displayStle = GetSpatialViewDefinition().GetDisplayStyle3d();
    m_lights = displayStle.CreateSceneLights(*target); // lighting setup for the scene

    if (!displayStle.GetViewFlags().ShowSourceLights())
        return m_lights;

    auto& models = GetDgnDb().Models();
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = models.GetModel(modelId);
        if (!model.IsValid())
            continue;

        auto spatialModel = model->ToSpatialModelP();
        if (nullptr != spatialModel)
            spatialModel->AddLights(*m_lights, *target);
        }

    return m_lights;
    }

/*---------------------------------------------------------------------------------**//**
* Visit all of the elements in a SpatialViewController. This is used for picking, etc.
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_VisitAllElements(ViewContextR context)
    {
    QueryResults results;
    RangeQuery rangeQuery(*this, context.GetFrustum(), *context.GetViewport(), UpdatePlan::Query(), &results); // NOTE: the context may have a smaller frustum than the view
    rangeQuery.Start(*this);

    if (m_noQuery)
        {
        for (auto& thisScore : results.m_scores)
            {
            if (rangeQuery.TestElement(thisScore.second))
                context.VisitElement(thisScore.second);

            if (context.CheckStop())
                return;
            }

        return;
        }

    // the range tree will return all elements in the volume. Filter them by the view criteria
    DgnElementId thisId;
    while ((thisId = rangeQuery.StepRtree()).IsValid())
        {
        if (rangeQuery.TestElement(thisId))
            context.VisitElement(thisId);

        if (context.CheckStop())
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**

    m_pending.clear();

* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::RangeQuery::AddAlwaysDrawn(SpatialViewControllerCR view)
    {
    if (!HasAlwaysList() || nullptr==m_results)
        return;

    DgnElements& pool = view.GetDgnDb().Elements();
    for (auto const& curr : m_special->m_always)
        {
        DgnElementCPtr el = pool.GetElement(curr);
        GeometrySourceCP geom = el.IsValid() ? el->ToGeometrySource() : nullptr;
        if (nullptr == geom || !geom->HasGeometry())
            continue;

        Frustum box(geom->CalculateRange3d());
        if (RTreeMatchFunction::Within::Outside != (RTreeMatchFunction::Within) m_planes.Contains(box))
            m_results->m_scores.Insert(10.0, curr); // value just has to be higher than max occlusion score which is really 2.0
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::RangeQuery::DoQuery()
    {
    StopWatch watch(true);

    DEBUG_PRINTF("Query started, target=%d", m_plan.GetTargetNumElements());
    Start(m_view);

    uint64_t endTime = m_plan.GetTimeout().count() ? (BeTimeUtilities::QueryMillisecondsCounter() + m_plan.GetTimeout().count()) : 0;

    m_minScore = 0.0;
    m_hitLimit = m_plan.GetTargetNumElements();

    if (endTime && m_hitLimit > (m_view.m_queryElementPerSecond * m_plan.GetTimeout().count()))
        {
        m_hitLimit = (uint32_t) (m_view.m_queryElementPerSecond * m_plan.GetTimeout().count());
        DEBUG_PRINTF("limiting to %d", m_hitLimit);
        }

    BeAssert(m_hitLimit>0);

    while (true)
        {
        while (m_view.m_loading)
            {
            DEBUG_PRINTF("pause, loading");
            BeThreadUtilities::BeSleep(20);
            }

        DgnElementId thisId = StepRtree();
        if (!thisId.IsValid())
            break;

        BeAssert(m_lastId==thisId.GetValueUnchecked());

        if (TestElement(thisId) && !IsAlways(thisId))
            {
            if (++m_count > m_hitLimit)
                {
                SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
                m_results->m_scores.erase(m_results->m_scores.begin());
                m_results->m_incomplete = true;
                m_count = m_hitLimit;
                }

            m_results->m_scores.Insert(m_lastScore, thisId);
            m_minScore = m_results->m_scores.begin()->first;
            }

        if (endTime && (BeTimeUtilities::QueryMillisecondsCounter() > endTime))
            {
            ERROR_PRINTF("Query timeout");
            m_results->m_incomplete = true;
            break;
            }
        };

    if (m_count >= m_hitLimit)
        m_view.m_queryElementPerSecond = m_results->GetCount() / watch.GetCurrentSeconds();

    DEBUG_PRINTF("Query completed, total=%d, progressive=%d, time=%f, eps=%f", m_results->GetCount(), m_results->m_incomplete, watch.GetCurrentSeconds(), m_view.m_queryElementPerSecond);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
RTreeMatchFunction::Within SpatialViewController::SpatialQuery::TestVolume(FrustumCR box, RTree3dValCP coords)
    {
    if (!m_boundingRange.Intersects(*coords) || !SkewTest(coords)) // quick test for entirely outside axis-aligned bounding range
        return RTreeMatchFunction::Within::Outside;

    auto rangeTest = (RTreeMatchFunction::Within) m_planes.Contains(box);
    if (rangeTest == RTreeMatchFunction::Within::Outside || !m_activeVolume.IsValid())
        return rangeTest;

    auto volumeTest = m_activeVolume->ClassifyPointContainment(box.m_pts, 8);
    if (ClipPlaneContainment_StronglyOutside == volumeTest)
        return RTreeMatchFunction::Within::Outside; // overlaps outer bounding range but not inside clip planes

    if (ClipPlaneContainment_Ambiguous == volumeTest)
        return RTreeMatchFunction::Within::Partly; // make sure we keep testing

    return rangeTest;
    }

/*---------------------------------------------------------------------------------**//**
* callback function for "MATCH DGN_rTree(?1)" against the spatial index
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
int SpatialViewController::RangeQuery::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    RTree3dValCP coords = (RTree3dValCP) info.m_coords;
    Frustum box(*coords);

    RTreeMatchFunction::Within rangeTest;
    if (info.m_parentWithin == RTreeMatchFunction::Within::Inside)
        rangeTest = RTreeMatchFunction::Within::Inside; // if parent is contained, we're contained and don't need to test
    else
        rangeTest = TestVolume(box, coords);

    if (!ComputeOcclusionScore(m_lastScore, box))
        return BE_SQLITE_OK; // eliminated by LOD filter

    BeAssert(m_lastScore>=0.0);
    BeAssert(m_minScore>=0.0);
    if (m_hitLimit && (m_count >= m_hitLimit && m_lastScore <= m_minScore))
        return BE_SQLITE_OK; // this one is smaller than the smallest entry we already have, skip it (and children).

    m_lastId = info.m_rowid;  // for debugging - make sure we get entries immediately after we score them.

    // for leaf nodes (elements), we must return a score of 0 so they will be immediately returned. That's necessary since we're keeping the
    // score in this object and we otherwise would not be able to correlate elements with their score.
    // For depth-first traversals, we return values such that we decend down to the bottom of one branch before we look at the next node. Otherwise
    // traversals take too much memory (the only reason it works for non-depth-first traversals is that we start eliminating nodes on size criteria
    // to choose the n-best values).
    info.m_score = info.m_level==0 ? 0.0 : m_depthFirst ? (info.m_maxLevel - info.m_level - m_lastScore) : (info.m_level - m_lastScore);
    info.m_within = rangeTest;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* Set the size of a filter that eliminates elements smaller than a given size (in pixels). To enable this filter, you must also call SetTestLOD.
* For range queries, we only enable this after we've found our maximum number of hits.
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::RangeQuery::SetSizeFilter(DgnViewportCR vp, double size)
    {
    BSIRect screenRect = vp.GetViewRect();
    if (screenRect.Width() > 0 && screenRect.Height() > 0)
        {
        double width  = size/screenRect.Width();
        double height = size/screenRect.Height();

        m_lodFilterNPCArea = width * width + height * height;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewController::RangeQuery::RangeQuery(SpatialViewControllerCR view, FrustumCR frustum, DgnViewportCR vp, UpdatePlan::Query const& plan, QueryResults* results) :
        SpatialQuery(&view.m_special, view.GetActiveVolume().get()), m_view(view), m_plan(plan), m_results(results)
    {
    m_count = 0;
    m_localToNpc = vp.GetWorldToNpcMap()->M0;
    m_cameraOn   = vp.IsCameraOn();

    if (m_cameraOn)
        {
        m_cameraPosition = vp.GetCamera().GetEyePoint();
        }
    else
        {
        DPoint3d viewDirection[2] = {{0,0,0}, {0.0, 0.0, 1.0}};
        DPoint3d viewDirRoot[2];
        vp.NpcToWorld(viewDirRoot, viewDirection, 2);
        viewDirRoot[1].Subtract(viewDirRoot[0]);

        m_orthogonalProjectionIndex = ((viewDirRoot[1].x < 0.0) ? 1  : 0) +
                                      ((viewDirRoot[1].x > 0.0) ? 2  : 0) +
                                      ((viewDirRoot[1].y < 0.0) ? 4  : 0) +
                                      ((viewDirRoot[1].y > 0.0) ? 8  : 0) +
                                      ((viewDirRoot[1].z < 0.0) ? 16 : 0) +
                                      ((viewDirRoot[1].z > 0.0) ? 32 : 0);
        }

    SetFrustum(frustum);
    AddAlwaysDrawn(view);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewController::RangeQuery::ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn)
    {
    npcOut.x = (m_localToNpc.coff[0][0] * localIn.x + m_localToNpc.coff[0][1] * localIn.y + m_localToNpc.coff[0][2] * localIn.z + m_localToNpc.coff[0][3]);
    npcOut.y = (m_localToNpc.coff[1][0] * localIn.x + m_localToNpc.coff[1][1] * localIn.y + m_localToNpc.coff[1][2] * localIn.z + m_localToNpc.coff[1][3]);
    npcOut.z = (m_localToNpc.coff[2][0] * localIn.x + m_localToNpc.coff[2][1] * localIn.y + m_localToNpc.coff[2][2] * localIn.z + m_localToNpc.coff[2][3]);

    if (!m_cameraOn)
        return true;

    double w = m_localToNpc.coff[3][0] * localIn.x + m_localToNpc.coff[3][1] * localIn.y + m_localToNpc.coff[3][2] * localIn.z + m_localToNpc.coff[3][3];
    if (w < 1.0E-5)
        return false;

    npcOut.x /= w;
    npcOut.y /= w;
    npcOut.z /= w;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* compute the size in "NPC area squared"
* Algorithm by: Dieter Schmalstieg and Erik Pojar - ACM Transactions on Graphics.
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewController::RangeQuery::ComputeOcclusionScore(double& score, FrustumCR box)
    {
    // Note - This routine is VERY time critical - Most of the calls to the geomlib
    // functions have been replaced with inline code as VTune had showed them as bottlenecks.

    static const short s_indexList[43][7] =
    {
        { 0, 3, 7, 6, 5, 1,   6}, // 0 inside    (arbitrarily default to front, top, right.
        { 0, 4, 7, 3,-1,-1,   4}, // 1 left
        { 1, 2, 6, 5,-1,-1,   4}, // 2 right
        {-1,-1,-1,-1,-1,-1,   0}, // 3 -
        { 0, 1, 5, 4,-1,-1,   4}, // 4 bottom
        { 0, 1, 5, 4, 7, 3,   6}, // 5 bottom, left
        { 0, 1, 2, 6, 5, 4,   6}, // 6 bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, // 7 -
        { 2, 3, 7, 6,-1,-1,   4}, // 8 top
        { 0, 4, 7, 6, 2, 3,   6}, // 9 top, left
        { 1, 2, 3, 7, 6, 5,   6}, //10 top, right
        {-1,-1,-1,-1,-1,-1,   0}, //11 -
        {-1,-1,-1,-1,-1,-1,   0}, //12 -
        {-1,-1,-1,-1,-1,-1,   0}, //13 -
        {-1,-1,-1,-1,-1,-1,   0}, //14 -
        {-1,-1,-1,-1,-1,-1,   0}, //15 -
        { 0, 3, 2, 1,-1,-1,   4}, //16 front
        { 0, 4, 7, 3, 2, 1,   6}, //17 front, left
        { 0, 3, 2, 6, 5, 1,   6}, //18 front, right
        {-1,-1,-1,-1,-1,-1,   0}, //19 -
        { 0, 3, 2, 1, 5, 4,   6}, //20 front, bottom
        { 1, 5, 4, 7, 3, 2,   6}, //21 front, bottom, left
        { 0, 3, 2, 6, 5, 4,   6}, //22 front, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //23 -
        { 0, 3, 7, 6, 2, 1,   6}, //24 front, top
        { 0, 4, 7, 6, 2, 1,   6}, //25 front, top, left
        { 0, 3, 7, 6, 5, 1,   6}, //26 front, top, right
        {-1,-1,-1,-1,-1,-1,   0}, //27 -
        {-1,-1,-1,-1,-1,-1,   0}, //28 -
        {-1,-1,-1,-1,-1,-1,   0}, //29 -
        {-1,-1,-1,-1,-1,-1,   0}, //30 -
        {-1,-1,-1,-1,-1,-1,   0}, //31 -
        { 4, 5, 6, 7,-1,-1,   4}, //32 back
        { 0, 4, 5, 6, 7, 3,   6}, //33 back, left
        { 1, 2, 6, 7, 4, 5,   6}, //34 back, right
        {-1,-1,-1,-1,-1,-1,   0}, //35 -
        { 0, 1, 5, 6, 7, 4,   6}, //36 back, bottom
        { 0, 1, 5, 6, 7, 3,   6}, //37 back, bottom, left
        { 0, 1, 2, 6, 7, 4,   6}, //38 back, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //39 -
        { 2, 3, 7, 4, 5, 6,   6}, //40 back, top
        { 0, 4, 5, 6, 2, 3,   6}, //41 back, top, left
        { 1, 2, 3, 7, 4, 5,   6}, //42 back, top, right
    };

    uint32_t projectionIndex;
    if (m_cameraOn)
        {
        projectionIndex = ((m_cameraPosition.x < box.m_pts[0].x) ? 1  : 0) +
                          ((m_cameraPosition.x > box.m_pts[1].x) ? 2  : 0) +
                          ((m_cameraPosition.y < box.m_pts[0].y) ? 4  : 0) +
                          ((m_cameraPosition.y > box.m_pts[2].y) ? 8  : 0) +
                          ((m_cameraPosition.z < box.m_pts[0].z) ? 16 : 0) +       // Zs reversed for right-handed system.
                          ((m_cameraPosition.z > box.m_pts[4].z) ? 32 : 0);

        if (0 == projectionIndex)
            {
            score = 1.0;
            return true;
            }
        }
    else
        {
        projectionIndex = m_orthogonalProjectionIndex;
        }

    BeAssert(projectionIndex <= 42);
    if (projectionIndex > 42)
        {
        BeAssert(false);
        return false;
        }

    uint32_t nVertices= s_indexList[projectionIndex][6];
    DPoint3d    npcVertices[6];
    if (0 == nVertices)
        {
        BeAssert(false);
        return false;
        }

    uint32_t npcComputedMask = 0;
    if (m_testLOD && 0.0 != m_lodFilterNPCArea)
        {
        int        diagonalVertex = nVertices/2;
        DPoint3dR  diagonalNPC    = npcVertices[diagonalVertex];

        if (!ComputeNPC(npcVertices[0], box.m_pts[s_indexList[projectionIndex][0]]) ||
            !ComputeNPC(diagonalNPC,    box.m_pts[s_indexList[projectionIndex][diagonalVertex]]))
            {
            score = 2.0;  // range spans eye plane
            return true;
            }

        DPoint3dR   npcCorner = npcVertices[nVertices/2];
        DPoint2d    extent = {npcCorner.x - npcVertices[0].x, npcCorner.y - npcVertices[0].y};

        if ((extent.x * extent.x + extent.y * extent.y) < m_lodFilterNPCArea)
            return false;

        npcComputedMask |= 1;
        npcComputedMask |= (1 << diagonalVertex);
        }

    double zTotal = 0.0;
    for (uint32_t i=0, mask = 0x0001; i<nVertices; i++, mask <<= 1)
        {
        int cornerIndex = s_indexList[projectionIndex][i];
        if (0 == (mask & npcComputedMask) && !ComputeNPC(npcVertices[i], box.m_pts[cornerIndex]))
            {
            score = 2.0;  // range spans eye plane
            return true;
            }

        zTotal += npcVertices[i].z;
        }

    if (zTotal < 0.0)
        zTotal = 0.0;

    score = (npcVertices[nVertices-1].x - npcVertices[0].x) * (npcVertices[nVertices-1].y + npcVertices[0].y);
    for (uint32_t i=0; i<nVertices-1; ++i)
        score += (npcVertices[i].x - npcVertices[i+1].x) * (npcVertices[i].y + npcVertices[i+1].y);

    // an area of 0.0 means that we have a line. Recalculate using length/1000 (assuming width of 1000th of npc)
    if (score == 0.0)
        score = npcVertices[0].DistanceSquaredXY(npcVertices[2]) * .001;

    // Multiply area by the Z total value (0 is the back of the view) so that the score is roughly
    // equivalent to the area swept by the range through the view which makes things closer to the eye more likely to display first.
    // Also by scoring based on swept volume we are proportional to occluded volume.
    score *= .5 * (zTotal / (double) nVertices);

    if (score < 0.0)
        {
//        BeAssert(false);
        score = 0.0;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void exchangeAndNegate(double& dbl1, double& dbl2)
    {
    double temp = -dbl1;
    dbl1 = -dbl2;
    dbl2 = temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewController::SpatialQuery::SkewTest(RTree3dValCP testRange)
    {
    if (!m_doSkewTest || testRange->Intersects(m_backFace))
        return true;

    DVec3d skVector = m_viewVec;
    DPoint3d dlo, dhi;

    dlo.x = testRange->m_minx - m_backFace.m_maxx;
    dlo.y = testRange->m_miny - m_backFace.m_maxy;
    dhi.x = testRange->m_maxx - m_backFace.m_minx;
    dhi.y = testRange->m_maxy - m_backFace.m_miny;

    if (skVector.x < 0.0)
        {
        skVector.x = - skVector.x;
        exchangeAndNegate(dlo.x, dhi.x);
        }

    if (skVector.y < 0.0)
        {
        skVector.y = - skVector.y;
        exchangeAndNegate(dlo.y, dhi.y);
        }

    // Check the projection of the element's xhigh to the plane where ylow of the element is equal to yhigh of the skewrange
    double va1 = dlo.x * skVector.y;
    double vb2 = dhi.y * skVector.x;
    if (va1 > vb2)
        return false;

    // Check the projection of the element's xlow to the plane where yhigh of the element is equal to ylow of the skewrange
    double vb1 = dlo.y * skVector.x;
    double va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return false;

    // now we need the Z stuff
    dlo.z = testRange->m_minz - m_backFace.m_maxz;
    dhi.z = testRange->m_maxz - m_backFace.m_minz;
    if (skVector.z < 0.0)
        {
        skVector.z = - skVector.z;
        exchangeAndNegate(dlo.z, dhi.z);
        }

    // project onto either the xz or yz plane
    if (va1 > vb1)
        {
        double va3 = dlo.x * skVector.z;
        double vc2 = dhi.z * skVector.x;
        if (va3 > vc2)
            return false;
        }
    else
        {
        double vb3 = dlo.y * skVector.z;
        double vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return false;
        }

    // project onto the other plane
    if (va2 < vb2)
        {
        double va4 = dhi.x * skVector.z;
        double vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return false;
        }
    else
        {
        double vb4 = dhi.y * skVector.z;
        double vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::SpatialQuery::SetFrustum(FrustumCR frustum)
    {
    DRange3d range = frustum.ToRange();
    m_boundingRange.FromRange(range);

    range.InitFrom(frustum.GetPts(), 4);
    m_backFace.FromRange(range);

    // get unit bvector from front plane to back plane
    m_viewVec = DVec3d::FromStartEndNormalize(*frustum.GetPts(), *(frustum.GetPts()+4));

    // check to see if it's worthwhile using skew scan (skew vector not along one of the three major axes)
    int alongAxes = (fabs(m_viewVec.x) < 1e-8) + (fabs(m_viewVec.y) < 1e-8) + (fabs(m_viewVec.z) < 1e-8);
    m_doSkewTest = alongAxes<2;

    m_planes.Init(frustum);
    }

/*---------------------------------------------------------------------------------**//**
* this method must be called from the client thread.
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::SpatialQuery::Start(SpatialViewControllerCR view)
    {
    view.GetDgnDb().GetCachedStatement(m_rangeStmt, "SELECT ElementId FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId MATCH DGN_rTree(?1)");
    m_rangeStmt->BindInt64(1, (uint64_t) this);

    ElementsQuery::Start(view);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::ElementsQuery::Start(SpatialViewControllerCR view)
    {
    view.GetDgnDb().GetCachedStatement(m_viewStmt, view.m_viewSQL.c_str());
    int vSetCol = m_viewStmt->GetParameterIndex("@vset");
    BeAssert(0 != vSetCol);
    m_viewStmt->BindVirtualSet(vSetCol, view);

    m_idCol = m_viewStmt->GetParameterIndex("@elId");
    BeAssert(0 != m_idCol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId SpatialViewController::SpatialQuery::StepRtree()
    {
    auto rc=m_rangeStmt->Step();
    return (rc != BE_SQLITE_ROW) ? DgnElementId() : m_rangeStmt->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricModelP SpatialViewController::_GetTargetModel() const 
    {
#if defined (NEEDS_WORK_TARGET_MODEL)
#endif
    DgnModelId model = *GetViewedModels().begin();
    return GetDgnDb().Models().Get<GeometricModel>(model).get();
    }

