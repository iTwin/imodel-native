/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewport.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD


//----------------------------------------------------------------------------------------
// Pointools have a limited number of viewport id so we track which one are available.
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct PtViewportIdPool
    {
    PtViewportIdPool() { m_nextId = 0; }

    static PtViewportIdPool& GetInstance();

    //! return -1 if we reach the viewport limit.
    int32_t GetId();

    void ReleaseId(int32_t id) {m_available.push_back(id);}

    std::list<int32_t> m_available;
    int32_t            m_nextId;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
PtViewportIdPool& PtViewportIdPool::GetInstance()
    {
    static PtViewportIdPool* ptViewport = new PtViewportIdPool();
    return *ptViewport;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
int32_t PtViewportIdPool::GetId()
    {
    if (m_available.empty())
        {
        if (m_nextId > PTVORTEX_MAX_VIEWPORTS)
            return -1; // no more 

        return m_nextId++;
        }

    int32_t id = m_available.back();
    m_available.pop_back();

    return id;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
PtViewport::PtViewport(int32_t id)
    :m_id(id)
    {
    int32_t vpId = PointCloudVortex::AddViewport(m_id, NULL);
    BeAssert(vpId == m_id);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
PtViewport::~PtViewport()
    {
    PointCloudVortex::RemoveViewport(m_id);

    PtViewportIdPool::GetInstance().ReleaseId(m_id);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
RefCountedPtr<PtViewport> PtViewport::Create()
    {
    int32_t id = PtViewportIdPool::GetInstance().GetId();
    if (-1 == id)
        return nullptr;

    return new PtViewport(id);
    }



