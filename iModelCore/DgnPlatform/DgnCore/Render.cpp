/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Render.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

BEGIN_UNNAMED_NAMESPACE
    static Render::Queue* s_renderQueue = nullptr;

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::VerifyRenderThread() {DgnDb::VerifyRenderThread();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BSIRect Render::Target::SetAspectRatio(BSIRectCR requestedRect, double targetAspectRatio)
    {
    BSIRect rect = requestedRect;

    if (targetAspectRatio >= 1.0)
        {
        double requestedWidth = rect.Width();
        double requiredWidth = rect.Height() * targetAspectRatio;
        double adj = requiredWidth - requestedWidth;
        rect.Inset((int)(-adj/2.0), 0);
        }
    else
        {
        double requestedHeight = rect.Height();
        double requiredHeight = rect.Width() / targetAspectRatio;
        double adj = requiredHeight - requestedHeight;
        rect.Inset(0, (int)(-adj/2.0));
        }

    return rect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Queue::AddTask(Task& task)
    {
    DgnDb::VerifyClientThread();
    BeMutexHolder mux(m_cv.GetMutex());

    // see whether the new task should replace any existing tasks
    for (auto entry=m_tasks.begin(); entry != m_tasks.end();)
        {
        if ((task.GetTarget() == (*entry)->GetTarget()) && task._Replaces(**entry))
            {
            (*entry)->m_outcome = Render::Task::Outcome::Abandoned;
            entry = m_tasks.erase(entry);

            }
        else
            ++entry;
        }

    task._OnQueued();

    // Put it after all existing tasks of equal or higher priority (lower values mean higher priority).
    auto entry=m_tasks.begin();
    for (; entry != m_tasks.end(); ++entry)
        {
        if ((*entry)->GetPriority().m_value > task.GetPriority().m_value)
            break;
        }

    m_tasks.insert(entry, &task);

    mux.unlock();      // release lock before notify so other thread will start immediately vs. "hurry up and wait" problem
    m_cv.notify_all(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::Queue::IsIdle() const
    {
    DgnDb::VerifyClientThread();
    BeMutexHolder holder(m_cv.GetMutex());
    return m_tasks.empty() && !m_currTask.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::Queue::HasPending(Task::Operation op) const
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    for (auto entry : m_tasks)
        {
        if (entry->GetOperation() == op)
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::Queue::HasActiveOrPending(Task::Operation op, Target* target) const
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    if (m_currTask.IsValid() && m_currTask->GetOperation()==op && (nullptr == target || m_currTask->GetTarget()==target))
        return true;

    for (auto entry : m_tasks)
        {
        if (entry->GetOperation() == op && (nullptr == target || entry->GetTarget()==target))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Queue::WaitForIdle()
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    while (m_currTask.IsValid() || !m_tasks.empty())
        m_cv.InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Queue::WaitForWork()
    {
    BeMutexHolder holder(m_cv.GetMutex());
    while (m_tasks.empty())
        m_cv.InfiniteWait(holder);

    m_currTask = m_tasks.front();
    m_tasks.pop_front();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Task::Perform(StopWatch& timer)
    {
    m_outcome = Task::Outcome::Started;
    timer.Start();
    m_outcome = _Process(timer);
    m_elapsedTime = timer.GetCurrentSeconds();

    if (m_elapsedTime>.5)
        ERROR_PRINTF("[%d] task=%s, elapsed=%lf", m_target.IsValid() ? m_target->GetId() : 0, _GetName(), m_elapsedTime);
    else if (m_elapsedTime>.125)
        WARN_PRINTF("[%d] task=%s, elapsed=%lf", m_target.IsValid() ? m_target->GetId() : 0, _GetName(), m_elapsedTime);
    else
        {
//        DEBUG_PRINTF("[%d] task=%s, elapsed=%lf", m_target.IsValid() ? m_target->GetId() : 0, _GetName(), m_elapsedTime);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Queue::Process()
    {
    StopWatch timer(false);

    static bool s_go = true;
    while (s_go) // this is to quiet the compiler complaining that the main never returns
        {
        WaitForWork();
        m_currTask->Perform(timer);

        BeMutexHolder holder(m_cv.GetMutex());
        m_currTask = nullptr; // change with mutex held
        holder.unlock();      // release lock before notify so other thread will start immediately vs. "hurry up and wait" problem
        m_cv.notify_all();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL Render::Queue::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("Render"); // for debugging only
    DgnDb::SetThreadId(DgnDb::ThreadId::Render);

    ((Render::Queue*)arg)->Process(); // this never returns
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::StartRenderThread()
    {
    DgnDb::VerifyClientThread();
    if (nullptr != s_renderQueue)
        {
        BeAssert(false);
        return;
        }

    s_renderQueue = new Render::Queue();

    // create the rendering thread
    BeThreadUtilities::StartNewThread(Render::Queue::Main, s_renderQueue); 
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct DestroyTargetTask : Render::SceneTask
{
    Utf8CP _GetName() const override {return "Destroy Target";}
    Outcome _Process(StopWatch& timer) override {m_target->_OnDestroy(); return Outcome::Finished;}
    DestroyTargetTask(Render::Target& target) : SceneTask(&target, Operation::DestroyTarget, Priority::Highest()) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::DestroyNow()
    {
    DgnViewport::RenderQueue().AddAndWait(*new DestroyTargetTask(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetRenderTarget(Target* newTarget)
    {
    DgnDb::VerifyClientThread();
    if (m_renderTarget.IsValid())
        m_renderTarget->DestroyNow();

    m_renderTarget = newTarget; 
    if (newTarget)
        newTarget->SetMinimumFrameRate(m_minimumFrameRate);

    // Feature symbology is per-Target - will need to be updated for new Target (now, or possibly later if newTarget=nullptr)
    if (m_viewController.IsValid())
        m_viewController->SetFeatureOverridesDirty();

    m_sync.InvalidateFirstDrawComplete();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2016
//---------------------------------------------------------------------------------------
uint32_t DgnViewport::SetMinimumTargetFrameRate(uint32_t frameRate)
    {
    m_minimumFrameRate = frameRate;
    if (m_renderTarget.IsValid())
        m_minimumFrameRate = m_renderTarget->SetMinimumFrameRate(m_minimumFrameRate);

    return m_minimumFrameRate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Plan::Plan(DgnViewportCR vp)
    {
    m_is3d      = vp.Is3dView();
    m_frustum   = vp.GetFrustum();
    m_fraction  = vp.GetFrustumFraction();
    m_aaLines   = vp.WantAntiAliasLines();
    m_aaText    = vp.WantAntiAliasText();
    m_hiliteColor = vp.GetHiliteColor();

    auto& controller = vp.GetViewControllerR();
    m_activeVolume = controller.GetActiveVolume();

    auto& def = controller.GetViewDefinitionR();
    auto& style = def.GetDisplayStyle();
    m_bgColor   = style.GetBackgroundColor();
    m_monoColor = style.GetMonochromeColor();
    m_viewFlags = style.GetViewFlags();

    auto style3d = style.ToDisplayStyle3dP();
    if (style3d)
        {
        m_hline = style3d->GetHiddenLineParams();
        auto spatial = controller._ToSpatialView();
        if (spatial)
            m_lights = spatial->GetLights();
        else
            {
            BeAssert(false); // somehow we have a 3d style on a 2d controller???
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Queue& DgnViewport::RenderQueue() 
    {
    BeAssert(nullptr != s_renderQueue);
    return *s_renderQueue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FrustumPlanes::Init(FrustumCR frustum)
    {
    m_isValid = true;
    ClipUtil::RangePlanesFromFrustum(m_planes, frustum, true, true, 1.0E-6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
FrustumPlanes::Contained FrustumPlanes::Contains(DPoint3dCP points, int nPts, double tolerance) const
    {
    BeAssert(IsValid());

    bool allInside = true;
    for (ClipPlaneCR plane : m_planes)
        {
        int nOutside = 0;
        for (int j=0; j < nPts; ++j)
            {
            if (plane.EvaluatePoint(points[j]) + tolerance < 0.0)
                {
                ++nOutside;
                allInside = false;
                }
            }

        if (nOutside == nPts)
            return Contained::Outside;
        }
    
    return allInside ? Contained::Inside : Contained::Partly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool FrustumPlanes::IntersectsRay(DPoint3dCR origin, DVec3dCR direction)
    {
    double tFar  =  1e37;
    double tNear = -1e37;

    for (ClipPlaneCR plane : m_planes)
        {
        double vD = plane.DotProduct(direction), vN = plane.EvaluatePoint(origin);

        if (0.0 == vD)
            {
            // Ray is parallel... No need to continue testing if outside halfspace.
            if (vN < 0.0)
                return false;
            }
        else
            {
            double      rayDistance = -vN / vD;

            if (vD < 0.0)
                {
                if (rayDistance < tFar)
                   tFar = rayDistance;
                }
            else
                {
                if (rayDistance > tNear)
                    tNear = rayDistance;
                }
            }
        } 

    return tNear <= tFar;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void toDPoints(T& dpts, QPoint3dCP qpts, QPoint3d::ParamsCR qparams, int32_t numPoints)
    {
    dpts.resize(numPoints);
    for (int32_t i = 0; i < numPoints; i++)
        dpts[i] = qpts[i].UnquantizeAsVector(qparams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void toNormals(BlockedVectorDVec3dR normals, OctEncodedNormalCP encoded, int32_t count)
    {
    normals.resize(count);
    for (int32_t i = 0; i < count; i++)
        normals[i] = encoded[i].Decode();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshArgs::ToPolyface() const
    {
    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_numIndices);
    uint32_t const* pIndex = m_vertIndex;
    uint32_t const* pEnd = pIndex + m_numIndices;
    int* pOut = &pointIndex.front();

    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (nullptr != m_points)
        toDPoints(polyFace->Point(), m_points, m_pointParams, m_numPoints);

    if (nullptr != m_normals)
        toNormals(polyFace->Normal(), m_normals, m_numPoints);

    if (nullptr != m_textureUV)
        {
        polyFace->Param().resize(m_numPoints);
        floatToDouble(&polyFace->Param().front().x, &m_textureUV->x, 2 * m_numPoints);
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
double TileSizeAdjuster::Update(Render::TargetCR target, double frameTime)
    {
    double modifier = target.GetTileSizeModifier();
    if (m_numFrames >= m_frameWindow)
        {
        modifier = Compute(target, modifier);
        Reset();
        }

    Record(frameTime);
    return modifier;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileSizeAdjuster::Record(double frameTime)
    {
    BeAssert(m_numFrames < m_frameWindow);
    double sum = m_averageFrameTime * m_numFrames;
    ++m_numFrames;
    m_averageFrameTime = (sum + frameTime) / m_numFrames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
double TileSizeAdjuster::Compute(Render::TargetCR target, double curMod) const
    {
#define NO_TILESIZE_ADJUSTMENT
#if defined(NO_TILESIZE_ADJUSTMENT)
    return curMod;
#else
    BeAssert(m_numFrames == m_frameWindow);
    BeAssert(0 < target.GetMinimumFrameRate());

    double maxFrameTime = 1.0 / target.GetMinimumFrameRate();
    uint32_t observedFps = static_cast<uint32_t>(1.0 / m_averageFrameTime);
    double minMod = 1.0, maxMod = target.GetMaximumTileSizeModifier();
    double step = (maxMod-minMod)/8.0;
    if (m_averageFrameTime <= maxFrameTime)
        {
        // If we're doing *really* well (at least 5 fps better than minimum), decrease the tile size modifier a bit
        double newMod = curMod;
        if (5 <= observedFps - target.GetMinimumFrameRate())
            newMod -= step;

        return std::max(newMod, minMod);
        }

    // For every 5 fps below minimum, increase the tilesize modifier by another chunk
    step *= ((target.GetMinimumFrameRate() - observedFps) / 5) + 1;

    double newMod = curMod + step;
    newMod = std::min(newMod, maxMod);
    if (newMod > curMod)
        {
        WARN_PRINTF("Increasing tilesize mod to %f (avgFrameTime=%f av fps=%f)", newMod, m_averageFrameTime, 1.0/m_averageFrameTime);
        }

    return newMod;
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Render::Material::Trans2x3::GetTransform() const
    {
    Transform transform = Transform::FromIdentity();

    for (size_t i=0; i<2; ++i)
        {
        for (size_t j=0; j<2; ++j)
            transform.form3d[i][j] = m_val[i][j];
        }

    transform.form3d[0][3] = m_val[0][2];
    transform.form3d[1][3] = m_val[1][2];
    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::Style::ToJson() const
    {
    Json::Value val;
    val[json_ovrColor()] = m_ovrColor;
    val[json_color()] = m_color.GetValue();
    val[json_pattern()] = (Json::UInt32) m_pattern;
    val[json_width()] = (Json::UInt32) m_width;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::Style::FromJson(JsonValueCR val)
    {
    m_ovrColor = val[json_ovrColor()].asBool(m_ovrColor);
    m_color = ColorDef(val[json_color()].asUInt(m_color.GetValue()));
    m_pattern = (LinePixels) val[json_pattern()].asUInt((uint32_t) m_pattern);
    m_width = val[json_width()].asUInt(m_width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::ToJson() const
    {
    HiddenLineParams defaults;
    Json::Value val;

    if (m_visible != defaults.m_visible) val[json_visible()] = m_visible.ToJson();
    if (m_hidden != defaults.m_hidden) val[json_hidden()] = m_hidden.ToJson();
    if (m_transparencyThreshold != defaults.m_transparencyThreshold) val[json_transThreshold()] = m_transparencyThreshold;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::HiddenLineParams Render::HiddenLineParams::FromJson(JsonValueCR val)
    {
    HiddenLineParams params;

    if (val.isObject())
        {
        params.m_visible.FromJson(val[json_visible()]);
        params.m_hidden.FromJson(val[json_hidden()]);
        params.m_transparencyThreshold = val[json_transThreshold()].asDouble(params.m_transparencyThreshold);
        }
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Feature::operator<(FeatureCR rhs) const
    {
    if (GetElementId() != rhs.GetElementId())
        return GetElementId() < rhs.GetElementId();
    else if (GetSubCategoryId() != rhs.GetSubCategoryId())
        return GetSubCategoryId() < rhs.GetSubCategoryId();
    else if (GetClass() != rhs.GetClass())
        return static_cast<uint8_t>(GetClass()) < static_cast<uint8_t>(rhs.GetClass());
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureSymbologyOverrides::FeatureSymbologyOverrides(ViewControllerCR view) : m_alwaysDrawn(view.GetAlwaysDrawn()),
    m_neverDrawn(view.IsAlwaysDrawnExclusive() ? DgnElementIdSet() : view.GetNeverDrawn()), m_alwaysDrawnExclusive(view.IsAlwaysDrawnExclusive())
    {
    DgnDb::VerifyClientThread();

    if (!m_alwaysDrawnExclusive)
        {
        auto const& undisplayed = view.GetDgnDb().Elements().GetUndisplayedSet();
        m_neverDrawn.insert(undisplayed.begin(), undisplayed.end());
        }

    ViewFlags vf = view.GetViewFlags();
    m_constructions = vf.ShowConstructions();
    m_dimensions = vf.ShowDimensions();
    m_patterns = vf.ShowPatterns();
    m_lineWeights = vf.ShowWeights();

    // Features are defined by subcategory, which only implies category...
    // A subcategory is visible if it belongs to a viewed category and its appearance's visibility flag is set
    static constexpr Utf8CP ecsql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE InVirtualSet(?, Parent.Id)";
    CachedECSqlStatementPtr stmt = view.GetDgnDb().GetPreparedECSqlStatement(ecsql);
    stmt->BindVirtualSet(1, view.GetViewedCategories());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto subcatId = stmt->GetValueId<DgnSubCategoryId>(0);
        auto appearance = view.GetSubCategoryAppearance(subcatId);
        if (appearance.IsVisible())
            {
            m_visibleSubCategories.insert(subcatId);
            auto ovr = view.GetSubCategoryOverride(subcatId);
            if (ovr.IsAnyOverridden())
                {
                Appearance app;
                app.InitFrom(ovr);
                if (app.OverridesSymbology())
                    m_subcategoryOverrides.Insert(subcatId, app);
                }
            }
        }

    view.AddFeatureOverrides(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_AddFeatureOverrides(FeatureSymbologyOverrides& overrides) const
    {
    // NB: DgnDisplay will register an ISelectionEvents listener to mark view controller's feature symbology dirty when selection set changes...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureSymbologyOverrides::Appearance::InitFrom(DgnSubCategory::Override const& ovr)
    {
    Init();
    uint32_t weight;
    if (ovr.GetWeight(weight))
        SetWeight(weight);

    double trans;
    if (ovr.GetTransparency(trans))
        SetTransparency(trans);

    ColorDef rgb;
    if (ovr.GetColor(rgb))
        SetRgb(rgb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureSymbologyOverrides::Appearance FeatureSymbologyOverrides::Appearance::Extend(Appearance const& base) const
    {
    Appearance app = base;
    if (m_flags.m_rgb && !app.m_flags.m_rgb)
        app.SetRgb(GetRgb());

    if (m_flags.m_alpha && !app.m_flags.m_alpha)
        app.SetAlpha(GetAlpha());

    if (m_flags.m_weight && !app.m_flags.m_weight)
        app.SetWeight(GetWeight());

    if (m_flags.m_linePixels && !app.m_flags.m_linePixels)
        app.SetLinePixels(GetLinePixels());

    return app;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureSymbologyOverrides::IsFeatureVisible(FeatureCR feat) const
    {
    auto elemId = feat.GetElementId();
    bool alwaysDrawn = elemId.IsValid() && m_alwaysDrawn.end() != m_alwaysDrawn.find(elemId);
    if (alwaysDrawn || m_alwaysDrawnExclusive)
        return alwaysDrawn;

    if (elemId.IsValid() && m_neverDrawn.end() != m_neverDrawn.find(elemId))
        return false;

    if (feat.GetSubCategoryId().IsValid() && !IsSubCategoryVisible(feat.GetSubCategoryId()))
        return false;

    return IsClassVisible(feat.GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureSymbologyOverrides::GetAppearance(Appearance& app, FeatureCR feat) const
    {
    app.Init();

    // Is the element visible?
    auto elemId = feat.GetElementId();
    bool alwaysDrawn = false;
    bool haveElemOverrides = false;
    if (elemId.IsValid())
        {
        alwaysDrawn = m_alwaysDrawn.end() != m_alwaysDrawn.find(elemId);
        if (!alwaysDrawn)
            {
            if (m_alwaysDrawnExclusive)
                return false;
            else if (m_neverDrawn.end() != m_neverDrawn.find(elemId))
                return false;
            }

        // Element overrides take precedence
        auto elemIter = m_elementOverrides.find(elemId);
        haveElemOverrides = m_elementOverrides.end() != elemIter;
        if (haveElemOverrides)
            app = elemIter->second;
        }

    auto subcatId = feat.GetSubCategoryId();
    if (subcatId.IsValid())
        {
        if (!alwaysDrawn && !IsSubCategoryVisible(subcatId))
            return false;

        auto subcatIter = m_subcategoryOverrides.find(subcatId);
        if (m_subcategoryOverrides.end() != subcatIter)
            app = subcatIter->second.Extend(app);
        }

    if (!haveElemOverrides)
        app = m_defaultOverrides.Extend(app);

    bool visible = alwaysDrawn || IsClassVisible(feat.GetClass());
    if (visible && app.OverridesAlpha())
        visible = app.GetAlpha() < 0xff; // don't bother rendering something with full transparency...

    if (!m_lineWeights)
        app.SetWeight(1);

    return visible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureSymbologyOverrides::IsClassVisible(DgnGeometryClass geomClass) const
    {
    switch (geomClass)
        {
        case DgnGeometryClass::Construction:    return m_constructions;
        case DgnGeometryClass::Dimension:       return m_dimensions;
        case DgnGeometryClass::Pattern:         return m_patterns;
        default:                                return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureSymbologyOverrides::IsSubCategoryVisible(DgnSubCategoryId subcatId) const
    {
    return m_visibleSubCategories.end() != m_visibleSubCategories.find(subcatId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureSymbologyOverrides::OverrideSubCategory(DgnSubCategoryId id, Appearance app)
    {
    if (!id.IsValid() || !IsSubCategoryVisible(id))
        return;

    auto iter = m_subcategoryOverrides.find(id);
    if (iter != m_subcategoryOverrides.end())
        {
        if (!app.OverridesSymbology())
            m_subcategoryOverrides.erase(iter);
        else
            iter->second = app;
        }
    else
        {
        m_subcategoryOverrides.Insert(id, app);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureSymbologyOverrides::OverrideElement(DgnElementId id, Appearance app)
    {
    if (!id.IsValid() || m_neverDrawn.end() != m_neverDrawn.find(id))
        return;

    auto iter = m_elementOverrides.find(id);
    if (iter != m_elementOverrides.end())
        {
        if (!app.OverridesSymbology())
            m_elementOverrides.erase(iter);
        else
            iter->second = app;
        }
    else
        {
        m_elementOverrides.Insert(id, app);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdge::MeshEdge(uint32_t index0, uint32_t index1)
    {
    if (index0 < index1)
        {
        m_indices[0] = index0;
        m_indices[1] = index1;
        }
    else
        {
        m_indices[0] = index1;
        m_indices[1] = index0;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshEdge::operator < (MeshEdge const& rhs) const
    {
    return m_indices[0] == rhs.m_indices[0] ? (m_indices[1] < rhs.m_indices[1]) :  (m_indices[0] < rhs.m_indices[0]);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshEdgeArgs::Init(MeshEdgesCR meshEdges, QPoint3dCP points, QPoint3d::ParamsCR qparams, bool isPlanar)
    {
    if (meshEdges.m_visible.empty())
        return false;

    m_points        = points;
    m_pointParams   = qparams;
    m_edges         = meshEdges.m_visible.data();
    m_numEdges      = meshEdges.m_visible.size();
    m_isPlanar      = isPlanar;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SilhouetteEdgeArgs::Init(MeshEdgesCR meshEdges, QPoint3dCP points, QPoint3d::ParamsCR params)
    {
    if (meshEdges.m_silhouette.empty())
        return false;

    m_points        = points;
    m_pointParams   = params;
    m_edges         = meshEdges.m_silhouette.data();
    m_numEdges      = meshEdges.m_silhouette.size();
    m_normals       = meshEdges.m_silhouetteNormals.data();
    m_isPlanar      = false;

    return true;
    }

#if !defined(NDEBUG)
/*---------------------------------------------------------------------------------**//**
* This lives here because (1) annoying out-of-sync headers on rebuild and (2) want to
* catch the actual delta when assertion triggered. Only used in non-optimized builds.
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void OctEncodedNormal::VerifyNormalized(DVec3dCR vec)
    {
    auto magSq = vec.MagnitudeSquared();
    bool normalized = DoubleOps::WithinTolerance(magSq, 1.0, 0.001);
    BeAssert(normalized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void OctEncodedNormal::VerifyEncoded(uint16_t val, DVec3dCR in)
    {
    OctEncodedNormal enc = OctEncodedNormal::From(val);
    DVec3d out = enc.Decode();
    bool vecEqual = in.IsEqual(out, 0.05);
    BeAssert(vecEqual);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilder::CreateParams GraphicBuilder::CreateParams::World(DgnViewportR vp, TransformCR tf)
    {
    return World(vp.GetViewController().GetDgnDb(), tf, &vp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilder::CreateParams GraphicBuilder::CreateParams::View(DgnViewportR vp, TransformCR tf)
    {
    return View(vp.GetViewController().GetDgnDb(), tf, &vp);
    }

