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
    static int s_gps;
    static int s_sceneTarget;
    static int s_progressiveTarget;
    static double s_frameRateGoal;

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::VerifyRenderThread() {DgnDb::VerifyRenderThread();}
void Render::Target::Debug::SaveGPS(int gps, double fr) {s_gps=gps; s_frameRateGoal=fr; Show();}
void Render::Target::Debug::SaveSceneTarget(int val) {s_sceneTarget=val; Show();}
void Render::Target::Debug::SaveProgressiveTarget(int val) {s_progressiveTarget=val; Show();}
void Render::Target::Debug::RecordGraphicsStats()
    {
    if (IDisplayMetricsHandler::IsRecorderActive())
        DisplayMetricsHandler::RecordGraphicsStats(s_gps, s_sceneTarget, s_progressiveTarget, s_frameRateGoal);
    }
void Render::Target::Debug::Show()
    {
#if defined (DEBUG_LOGGING) 
    NativeLogging::LoggingManager::GetLogger("GPS")->debugv("GPS=%d, Scene=%d, PD=%d, FR=%lf", s_gps, s_sceneTarget, s_progressiveTarget, s_frameRateGoal);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::_RecordFrameTime(uint32_t count, double seconds, bool isFromProgressiveDisplay) 
    {
    if (0 == count)
        return;

    if (seconds < .00001)
        seconds = .00001;

    uint32_t gps = (uint32_t) ((double) count / seconds);
    Render::Target::Debug::SaveGPS(gps, m_frameRateGoal);
    if (!isFromProgressiveDisplay)
        Render::Target::Debug::RecordGraphicsStats();

    // Typically GPS increases as progressive display continues. We cannot let CreateScene graphics
    // be affected by the progressive display rate.  
    //
    // The GPS from the beginning of progressive is likely to be less
    // than the GPS from the end of progressive display. Therefore,
    // we reset the progressive display value by saving the create scene value.
    m_graphicsPerSecondNonScene.store(gps);
    if (!isFromProgressiveDisplay)
        m_graphicsPerSecondScene.store(gps);
    }

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
bool Render::Queue::HasActiveOrPending(Task::Operation op) const
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    if (m_currTask.IsValid() && m_currTask->GetOperation()==op)
        return true;

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

    if (m_elapsedTime>.125 && IDisplayMetricsHandler::IsRecorderActive())
        DisplayMetricsHandler::RecordError(Utf8PrintfString("[%d] task=%s, elapsed=%d", m_target.IsValid() ? m_target->GetId() : 0, _GetName(), (int)(m_elapsedTime*1000)).c_str());

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

    auto& controller = vp.GetViewController();
    m_activeVolume = controller.GetActiveVolume();

    auto& def = controller.GetViewDefinition();
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
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Graphic* GraphicSet::Find(DgnViewportCR vp, double metersPerPixel) const
    {
    for (auto graphic : m_graphics)
        {
        if (graphic->IsValidFor(vp, metersPerPixel))
            return graphic.get();
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicSet::DropFor(DgnViewportCR vp)
    {
    // NB: Note there can be more than one Graphic for a viewport
    for (auto it=m_graphics.begin(); it!=m_graphics.end(); )
        {
        if ((*it)->IsSpecificToViewport(vp))
            it = m_graphics.erase(it);
        else
            ++it;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicSet::Drop(Render::Graphic& graphic)
    {
    auto size = m_graphics.erase(&graphic);
    UNUSED_VARIABLE(size);
    BeAssert(0 != size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicList::Add(Graphic& graphic, void* ovr, uint32_t ovrFlags) 
    {
    graphic.EnsureClosed();
    m_list.push_back(Node(graphic, ovr, ovrFlags));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicList::Drop(Graphic& graphic) 
    {
    for (auto it=m_list.begin(); it!=m_list.end(); ++it)
        {
        if (it->m_ptr.get() == &graphic)
            {
            m_list.erase(it);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicList::ChangeOverride(Graphic& graphic, void* ovr, uint32_t ovrFlags) 
    {
    for (auto it=m_list.begin(); it!=m_list.end(); ++it)
        {
        if (it->m_ptr.get() == &graphic)
            {
            it->m_overrides = ovr;
            it->m_ovrFlags = ovrFlags;
            return;
            }
        }
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr GraphicBuilder::TriMeshArgs::ToPolyface() const
    {
    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_numIndices);
    int32_t const* pIndex = m_vertIndex;
    int32_t const* pEnd = pIndex + m_numIndices;
    int32_t* pOut = &pointIndex.front();

    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (nullptr != m_points)
        {
        polyFace->Point().resize(m_numPoints);
        floatToDouble(&polyFace->Point().front().x, &m_points->x, 3 * m_numPoints);
        }

    if (nullptr != m_normals)
        {
        polyFace->Normal().resize(m_numPoints);
        floatToDouble(&polyFace->Normal().front().x, &m_normals->x, 3 * m_numPoints);
        polyFace->NormalIndex() = pointIndex;
        }

    if (nullptr != m_textureUV)
        {
        polyFace->Param().resize(m_numPoints);
        floatToDouble(&polyFace->Param().front().x, &m_textureUV->x, 2 * m_numPoints);
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr GraphicBuilder::CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip) const
    {
    return m_builder->_CreateSubGraphic(subToGraphic, clip);
    }

#ifdef FRAMERATE_DEBUG
#   define FRAMERATE_DEBUG_PRINTF DEBUG_PRINTF
#   define FRAMERATE_WARN_PRINTF WARN_PRINTF
#else
#   define FRAMERATE_DEBUG_PRINTF(...)
#   define FRAMERATE_WARN_PRINTF(...)
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Render::FrameRateAdjuster::AdjustFrameRate(Render::TargetCR target, double lowestScore)
    {
    double frameRateGoal = target.GetFrameRateGoal();

    // We have to have enough draw events before we can tell how well we are doing.
    if (m_drawCount < 10)
        return frameRateGoal;

    double successPct = (m_drawCount - m_abortCount) / (double)m_drawCount;

    auto viewRect = target.GetDevice()->GetWindow()->_GetViewRect();
    double pixelsPerNpc = viewRect.Width(); // use pixels across as an approximation. Maybe we should measure the diagonal?
    double pixelsPerInch = target.GetDevice()->PixelsFromInches(1.0);
    double inchesPerNpc = pixelsPerNpc / pixelsPerInch; // inches/NPC = pixels/NPC * inches/pixel
    
    double smallestRangeDrawnNpc = sqrt(lowestScore);

    // from here on, all measurements are in inches
    double smallestRangeDrawn = smallestRangeDrawnNpc * inchesPerNpc;   // size of the diagonal of the smallest range returned by the query

    static const double FINE_ELEMENT_RES = 1 / 16.0; 

    FRAMERATE_DEBUG_PRINTF("[%d] frameRateGoal=%lf smallestRangeDrawn=%lf successPct=%lf", target.GetId(), frameRateGoal, smallestRangeDrawn, successPct);

    static volatile double s_longTermSuccessRate = 0.80;

    if ((m_drawCount >= 10 * frameRateGoal) && (successPct > s_longTermSuccessRate))
        {
        // After a long string of successes, reset the stats. Otherwise, we won't notice when aborts start happening again.
        Reset();
        FRAMERATE_DEBUG_PRINTF("Reset stats");
        return frameRateGoal;
        }

    static volatile double s_shortTermSuccessRate = 0.95;

    if (successPct <= s_shortTermSuccessRate)
        {
        Reset();

        // We have been failing too often to draw the whole scene in the time allotted for a frame. 
        // About all we can do is allow more time per frame. I can only hope that the update planner does not increase the element count!!
        if (frameRateGoal > target.GetMinimumFrameRate())
            {
            --frameRateGoal;
            FRAMERATE_WARN_PRINTF("ABORTS TOO MUCH => -frameRateGoal -> %lf (smallestRangeDrawn=%lf)", frameRateGoal, smallestRangeDrawn);
            }

        return frameRateGoal;
        }

    // If we got here, we know that we have been able draw all elements in the scene in the time allotted at least most of the time.
    if (0 == (m_drawCount % (int) frameRateGoal))
        {
        // If we have been succeeding for 1 second or more, then maybe we should increase the frame rate.
        // That would have the benefit of making everything smoother. However, that would also have the effect of making the update planner reduce
        // the number of elements per scene, that is, increase dropout. That would be OK if we are currently drawing too many small elements anyway.
        // If we are not drawing small elements, then don't increase the frame rate, as the rejected elements would be too noticable.
        if (smallestRangeDrawn < FINE_ELEMENT_RES)
            {
            Reset();
            if (frameRateGoal < FRAME_RATE_MAX)
                {
                ++frameRateGoal;
                FRAMERATE_WARN_PRINTF("SUCCESS @ FINE => +frameRateGoal -> %lf (smallestRangeDrawn=%lf)", frameRateGoal, smallestRangeDrawn);
                }
            }
        }

    return frameRateGoal;
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
    m_pattern = (GraphicParams::LinePixels) val[json_pattern()].asUInt((uint32_t) m_pattern);
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

