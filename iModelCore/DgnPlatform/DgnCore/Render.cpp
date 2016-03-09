/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Render.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

BEGIN_UNNAMED_NAMESPACE
    static Render::Queue* s_renderQueue = nullptr;
    static int s_gps;
    static int s_sceneTarget;
    static int s_progressiveTarget;
END_UNNAMED_NAMESPACE

#define RENDER_LOGGING 1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::VerifyRenderThread() {DgnDb::VerifyRenderThread();}
void Render::Target::Debug::SaveGPS(int gps) {s_gps=gps; Show();}
void Render::Target::Debug::SaveSceneTarget(int val) {s_sceneTarget=val; Show();}
void Render::Target::Debug::SaveProgressiveTarget(int val) {s_progressiveTarget=val; Show();}
void Render::Target::Debug::Show()
    {
#if defined(RENDER_LOGGING) 
    NativeLogging::LoggingManager::GetLogger("GPS")->debugv("GPS=%d, Scene=%d, PD=%d", s_gps, s_sceneTarget, s_progressiveTarget);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::RecordFrameTime(GraphicList& scene, double seconds, bool isFromProgressiveDisplay) 
    {
    uint32_t count = scene.GetCount();
    if (0 == count)
        return;

    if (seconds < .0001)
        seconds = .0001;

    uint32_t gps = (uint32_t) ((double) count / seconds);
    Render::Target::Debug::SaveGPS(gps);

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

    m_tasks.push_back(&task);
    mux.unlock();      // release lock before notify so other thread will start immediately vs. "hurry up and wait" problem
    m_cv.notify_all(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::Queue::HasPending(Task::Operation op)
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
        THREADLOG.errorv("task=%s, elapsed=%lf", _GetName(), m_elapsedTime);
    else if (m_elapsedTime>.125)
        THREADLOG.warningv("task=%s, elapsed=%lf", _GetName(), m_elapsedTime);
    else
        THREADLOG.debugv("task=%s, elapsed=%lf", _GetName(), m_elapsedTime);
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
    if (nullptr != s_renderQueue)
        {
        BeAssert(false);
        return;
        }

    s_renderQueue = new Render::Queue();

    // create the rendering thread
    BeThreadUtilities::StartNewThread(300*1024, Render::Queue::Main, s_renderQueue); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Plan::Plan(DgnViewportCR vp)
    {
    m_viewFlags = vp.GetViewFlags();
    m_is3d      = vp.Is3dView();
    m_frustum   = vp.GetFrustum(DgnCoordSystem::World, true);
    m_bgColor   = vp.GetBackgroundColor();
    m_fraction  = vp.GetFrustumFraction();
    m_aaLines   = vp.WantAntiAliasLines();
    m_aaText    = vp.WantAntiAliasText();
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
    graphic.Close(); 
    m_list.push_back(Node(graphic, ovr, ovrFlags));
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
FrustumPlanes::Contained FrustumPlanes::Contains(DPoint3dCP points, int nPts) const
    {
    BeAssert(IsValid());

    bool allInside = true;
    for (ClipPlaneCR plane : m_planes)
        {
        int nOutside = 0;
        for (int j=0; j < nPts; ++j)
            {
            if (plane.EvaluatePoint(points[j]) < 1.0E-8)
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
