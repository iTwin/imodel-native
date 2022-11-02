/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <GeomSerialization/GeomLibsFlatBufferApi.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityCPtr BRepHandle::ObtainEntity() const
    {
    if (m_entity.IsValid())
        return m_entity;

    if (m_flatBufferData.empty())
        return nullptr;

    m_entity = GeometryStreamIO::Reader::ReadBRepEntity(m_flatBufferData.data());
    m_flatBufferData.clear();
    return m_entity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepFacetRequest::Scope::~Scope()
    {
    BeMutexHolder lock(m_req.GetConditionVariable().GetMutex());
    m_req.m_finished.store(true);
    lock.unlock();
    m_req.m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepFacetRequest::BRepFacetRequest(BRepHandleR handle, IFacetOptionsCR facetOptions, ICancellableP requestor)
    : m_handle(&handle), m_brepOptions(facetOptions, handle)
    {
    m_facetOptions = facetOptions.Clone();
    auto tol = BRepFacetOptions::kUncurvedToleranceLog2 == m_brepOptions.m_toleranceLog2 ? ComputeMinimumChordTolerance(handle.GetEntityRange()) : pow(2.0, m_brepOptions.m_toleranceLog2);
    m_facetOptions->SetChordTolerance(tol);
    m_facetOptions->SetBRepIgnoredFeatureSize(tol * 2.5); // s_minRangeBoxSize from Tile.cpp...

    m_requestors.insert(requestor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequest::ProduceFacets()
    {
    Scope scope(*this);

    if (IsCanceled() || IsFinished())
        return;

    auto entity = m_handle->ObtainEntity();
    if (entity.IsNull())
        return;

    // Some tests run this on main thread. Real code runs it on facetter thread only.
    if (DgnDb::ThreadId::Client != DgnDb::GetThreadId())
        entity->ChangePartition();

    try
        {
        if (nullptr == entity->GetFaceMaterialAttachments())
            {
            auto polyface = T_HOST.GetBRepGeometryAdmin()._FacetEntity(*entity, *m_facetOptions);
            if (polyface.IsValid() && polyface->HasFacets())
                m_facets.m_polyfaces.push_back(polyface);
            }
        else if (!T_HOST.GetBRepGeometryAdmin()._FacetEntity(*entity, m_facets.m_polyfaces, m_facets.m_faceAttachments, *m_facetOptions))
            {
            m_facets.Clear();
            }
        } catch (...) {
            m_facets.Clear();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequest::UpdateRequestors()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto size = m_requestors.size();
    auto iter = m_requestors.begin();
    while (iter != m_requestors.end())
        {
        auto cancel = const_cast<ICancellableP>(*iter);
        if (ICancellable::IsCanceled(cancel))
            iter = m_requestors.erase(iter);
        else
            ++iter;
        }

    if (m_requestors.size() != size)
        {
        lock.unlock();
        m_cv.notify_all();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepFacetRequestPtr BRepFacetRequestQueue::Get(BRepHandleR handle, IFacetOptionsCR facetOptions, ICancellableP requestor)
    {
    BRepFacetRequestPtr request;
    BRepFacetOptions brepOptions(facetOptions, handle);

    BeMutexHolder lock(m_cv.GetMutex());
    if (m_active.IsValid() && m_active->Matches(handle.GetId(), brepOptions))
        {
        request = m_active;
        }
    else
        {
        auto iter = std::find_if(m_queue.begin(), m_queue.end(), [&](BRepFacetRequestPtr& p) { return p->Matches(handle.GetId(), brepOptions); });
        if (iter != m_queue.end())
            request = *iter;
        }

    if (request.IsValid())
        {
        request->AddRequestor(requestor);
        }
    else
        {
        request = BRepFacetRequest::Create(handle, facetOptions, requestor);
        m_queue.push_back(request);
        }

    // Take this opportunity to check for cancellations.
    UpdateRequestors();

    lock.unlock();
    m_cv.notify_all();
    return request;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequestQueue::UpdateRequestors()
    {
    auto erased = std::remove_if(m_queue.begin(), m_queue.end(), [&](BRepFacetRequestPtr& p) {
        p->UpdateRequestors();
        return p.get() != m_active.get() && p->IsCanceled();
    });

    if (erased != m_queue.end())
        {
        // Assume caller will notify_all?
        m_queue.erase(erased, m_queue.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequestQueue::WaitForWork()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    while (!IsTerminating() && m_queue.empty())
        m_cv.InfiniteWait(lock);

    if (!IsTerminating())
        {
        m_active = m_queue.front();
        m_queue.pop_front();
        UpdateRequestors();
        }

    lock.unlock();
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequestQueue::Process()
    {
    RefCountedPtr<IRefCounted> outerMark(T_HOST.GetBRepGeometryAdmin()._CreateWorkerThreadOuterMark());

    while (!IsTerminating())
        {
        WaitForWork();
        if (IsTerminating())
            break;

        BeAssert(m_active.IsValid());
        m_active->ProduceFacets();

        BeMutexHolder lock(m_cv.GetMutex());
        m_active = nullptr;
        lock.unlock();
        m_cv.notify_all();
        }

    BeMutexHolder lock(m_cv.GetMutex());
    m_active = nullptr;
    m_queue.clear();
    outerMark = nullptr; // destroy the mark so that its tls is freed before we are terminated
    m_state.store(State::Terminated);

    lock.unlock();
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL BRepFacetRequestQueue::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("BRepFacetRequestQueue");

    reinterpret_cast<BRepFacetRequestQueue*>(arg)->Process();
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequestQueue::Start()
    {
    if (State::NotStarted == m_state.load())
        {
        m_state.store(State::Running);
        BeThreadUtilities::StartNewThread(BRepFacetRequestQueue::Main, this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* The queue is a member of a Db::AppDataPtr. When the last reference to the app data
* is released, the destructor runs. This can happen on any thread except the facetter thread.
* The facetter thread never acquires a ref-counted ptr to the queue.
* It will generally happen on the main thread, because the DgnDb waits for all tile requests
* to be cancelled and releases all tile trees before destroying the Db.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepFacetRequestQueue::Stop()
    {
    BeMutexHolder lock(m_cv.GetMutex());

    if (!IsRunning())
        return;

    m_state.store(State::Terminating);
    lock.unlock();
    m_cv.notify_all();

    lock.lock();
    while (!IsTerminated())
        m_cv.InfiniteWait(lock);

    lock.unlock();
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepService::BRepService(IBRepInfoCacheUPtr&& cache, bool startThread) : m_cache(std::move(cache))
    {
    if (startThread)
        m_queue.Start();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepHandlePtr BRepService::GetBRepHandle(BRepHandleIdCR id, IBRepDataSupplierCR supplier, ICancellableP cancel)
    {
    return m_handles.Obtain(id, [&]() -> BRepHandlePtr
        {
        if (ICancellable::IsCanceled(cancel))
            return nullptr;

        auto data = supplier.SupplyData();
        BeAssert(nullptr != data.m_bytes);
        auto metadata = m_cache->LoadMetadata(id);
        if (metadata.IsValid())
            return new BRepHandle(id, metadata.Value(), data, *this);

        auto entity = GeometryStreamIO::Reader::ReadBRepEntity(data.m_bytes);
        if (entity.IsNull())
            return nullptr;

        BRepHandlePtr handle(new BRepHandle(id, *entity, *this));
        m_cache->StoreMetadata(id, handle->GetMetadata());
        return handle;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BRepFacets BRepService::ProduceFacets(BRepHandleR handle, IFacetOptionsR facetOptions, ICancellableP cancel)
    {
    BRepFacetOptions brepOptions(facetOptions, handle);
    auto cached = m_cache->LoadFacets(handle.GetId(), brepOptions);
    if (cached.IsValid())
      return cached.Value();

    auto request = m_queue.Get(handle, facetOptions, cancel);
    BeAssert(request.IsValid());

    BRepFacets facets;
    if (ICancellable::IsCanceled(cancel))
        return facets;

    BeMutexHolder lock(request->GetConditionVariable().GetMutex());
    while (!request->IsFinished() && !ICancellable::IsCanceled(cancel))
        request->GetConditionVariable().InfiniteWait(lock);

    if (!request->IsFinished() || ICancellable::IsCanceled(cancel))
        return facets;

    facets = request->GetFacets();
    m_cache->StoreFacets(handle.GetId(), brepOptions, facets);
    return facets.Clone();
    }

