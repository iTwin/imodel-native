#include "SMPublisher.h"
#include "ScalableMeshPCH.h"
#include "SMCesiumPublisher.h"
#include "SMPublisher.h"
#include "..\ScalableMeshProgress.h"
#include "..\ScalableMeshQuery.h"
#include <ScalableMesh/IScalableMeshPolicy.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH
	
#pragma optimize("", off)

inline const GCS& GetDefaultGCS()
    {
    static const GCS DEFAULT_GCS(GetGCSFactory().Create(GeoCoords::Unit::GetMeter()));
    return DEFAULT_GCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
SaveAsNodeCreator::SaveAsNodeCreator(const WChar* scmFileName)
    : IScalableMeshNodeCreator::Impl(scmFileName)
    {
    LoadFromFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
SaveAsNodeCreator::~SaveAsNodeCreator()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SaveAsNodeCreator::SetGCS(const GeoCoords::GCS& gcs)
    {
    if(0 != m_scmPtr.get())
        return m_scmPtr->SetGCS(gcs);

    // Do not permit setting null GCS. Use default when it happens.
    m_gcs = (gcs.IsNull()) ? GetDefaultGCS() : gcs;
    m_gcsDirty = true;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshNodeEditPtr SaveAsNodeCreator::FindParentNodeFor(IScalableMeshNodePtr sourceNode, StatusInt& status)
    {
    status = SUCCESS;
    SMStatus smStatus = S_SUCCESS;
    IScalableMeshNodePtr currentNode = sourceNode->GetParentNode();
    auto foundNode = m_pDataIndex->FindLoadedNode(currentNode->GetNodeId());
    struct ChildInfo {
        int64_t id;
        DRange3d extent;
        };
    bvector<ChildInfo> childToInsert;
    while(!foundNode)
        {
        childToInsert.push_back({ currentNode->GetNodeId(), currentNode->GetNodeExtent() });
        currentNode = currentNode->GetParentNode();
        if(currentNode == nullptr) 
            break; // we have reached root node
        foundNode = m_pDataIndex->FindLoadedNode(currentNode->GetNodeId());
        }

    if(!foundNode)
        {
        BeAssert(false); // Parent node could not be found! This shouldn't happen, dangling nodes are not possible...
        status = ERROR;
        return nullptr;
        }

    // Dig down from found parent node and create (empty) intermediary nodes between this node and 
    // the node we want to add. This is to ensure we keep original index structure in the new index.
    IScalableMeshNodeEditPtr parentNode = new ScalableMeshNodeEdit<PointType>(foundNode);
    for(bvector<ChildInfo>::reverse_iterator childInfoPtr = childToInsert.rbegin(); childInfoPtr != childToInsert.rend(); ++childInfoPtr)
        {
        parentNode = AddChildNode(parentNode, childInfoPtr->extent, smStatus, false, childInfoPtr->id);
        if(smStatus != S_SUCCESS)
            {
            status = ERROR;
            break;
            }
        }
    return parentNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshPublishParamsPtr IScalableMeshPublishParams::Create(const SMPublishType& type)
    {
    switch (type)
        {
        case SMPublishType::THREESM:
        {
        return new SM3SMPublishParams();
        }
        default:
        {
        return nullptr;
        }
        }
    }

void IScalableMeshPublisher::ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smHeader)
    {
    _ExtractPublishNodeHeader(nodePtr, smHeader);
    }

void IScalableMeshPublisher::ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smMasterHeader)
    {
    _ExtractPublishMasterHeader(smPtr, smMasterHeader);
    }

IScalableMeshPublisherPtr IScalableMeshPublisher::Create(const SMPublishType& type)
    {
    switch (type)
        {
        case SMPublishType::CESIUM:
        {
        return new SMCesiumPublisher();
        }
        case SMPublishType::THREESM:
        {
        return new SM3SMPublisher();
        }
        default:
        {
        return nullptr;
        }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshPublisher::Publish(IScalableMeshPublishParamsPtr params)
    {
    return _Publish(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshPublisher::Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData, bool outputTexture)
    {
    return _Publish(node, transform, outData, outputTexture);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshPublisher::Publish(IScalableMeshNodePtr node, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const Transform& transform, bvector<Byte>& outData, bool outputTexture)
    {
    return _Publish(node, clips, coverageID, isClipBoundary, transform, outData, outputTexture);
    }



SM3SMPublisher::SMPublishThreadPoolPtr SM3SMPublisher::s_publishThreadPool = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
SM3SMPublisher::SMPublishThreadPoolPtr SM3SMPublisher::GetPublishThreadPool()
    {
    if (s_publishThreadPool == nullptr)
        {
        // Setup multithreaded publishing
        static const uint64_t nbThreads = std::max((uint64_t)1, (uint64_t)(std::thread::hardware_concurrency() - 2));
		//static const uint64_t nbThreads = 1;
        typedef std::function<void(LocalThreadPublishInfo&)> work_func_type;
		typedef std::function<bool(LocalThreadPublishInfo&)> pred_func_type;
        work_func_type func = [](LocalThreadPublishInfo& info)
            {
            BeAssert(info.m_publisher != nullptr && info.m_destParentFuture != nullptr && info.m_destNodePromise != nullptr);
            try
                {
                auto meshData = info.m_publisher->ExtractMeshData(info.m_source, info.m_transform);

                // Wait for the parent node to be ready
                auto parentDestNode = info.m_destParentFuture->second.get();

                // Create new destination node
                IScalableMeshNodeEditPtr newDestNode = nullptr;
                auto ret = info.m_publisher->ProcessNode(info.m_source, parentDestNode, newDestNode, meshData, info.m_transform);
                BeAssert(SUCCESS == ret);

                // Fulfill the promise with the newly created node
                info.m_destNodePromise->first.set_value(newDestNode);
                }
            catch (/*std::error_code& e*/...)
                {
                //std::cout << e.message() << std::endl;
                BeAssert(false); // Error processing node for publishing
                }
            };
		pred_func_type is_ready_func = [](LocalThreadPublishInfo& info)
		{
			return info.m_destParentFuture->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		};
        s_publishThreadPool = new SMPublishThreadPool(func, is_ready_func, nbThreads, MAX_QUEUE_SIZE);
        }

    return s_publishThreadPool;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SM3SMPublisher::_Publish(IScalableMeshPublishParamsPtr params)
    {
    m_params = dynamic_pointer_cast<SM3SMPublishParams, IScalableMeshPublishParams>(params);
    if (!m_params.IsValid())
        {
        BeAssert(false); // Parameters must be set
        return ERROR;
        }

    auto source = m_params->GetSource();
    auto destination = m_params->GetDestination();

    auto progress = m_params->GetProgress();
    if (progress != nullptr)
        {
        progress->ProgressStep() = ScalableMeshStep::STEP_SAVEAS_3SM;
        progress->ProgressStepIndex() = 1;
        progress->Progress() = 0.0f;
        }

    m_publishThreadPool = GetPublishThreadPool();

    // Create null promise for the root node and add it to the processing queue to kick-start the process
    SMNodeEditPromisePtr nullPromise = std::make_shared<SMNodeEditPromise>(SMNodeEditPromise());
    nullPromise->first.set_value(nullptr);
    nullPromise->second = nullPromise->first.get_future().share();
    
    auto transform = m_params->GetTransform();

    struct NodeInfo
        {
        IScalableMeshNodePtr currentNode;
        SMNodeEditPromisePtr currentDestNodePromise;
        };

    SMNodeEditPromisePtr rootPromise = AddWorkItem(source->GetRootNode(), nullPromise, transform);

    std::queue<NodeInfo> nodesToProcess;
    nodesToProcess.push(NodeInfo{ source->GetRootNode(), rootPromise });
    while(!nodesToProcess.empty())
        {
        if(progress != nullptr && progress->IsCanceled()) return SUCCESS;

        NodeInfo sourceInfo = nodesToProcess.front();
        IScalableMeshNodePtr sourceNode = sourceInfo.currentNode;

        bmap<uint64_t, SMNodeEditPromisePtr> meshNodePromises;
        uint64_t childID = 0;

        // Traverse using breadth first to prevent threads from being blocked as much as when traversing depth first (because we are using a FIFO queue)
        for(auto childNode : sourceNode->GetChildrenNodes())
            {
            meshNodePromises[childID++] = AddWorkItem(childNode, sourceInfo.currentDestNodePromise, transform);
            }

        childID = 0;
        for(auto childNode : sourceNode->GetChildrenNodes())
            {
            nodesToProcess.push(NodeInfo{ childNode, meshNodePromises[childID++] });

            //if(SUCCESS != PublishRecursive(childNode, meshNodePromises[childID++], transform))
            //    return ERROR;
            }

        if(progress != nullptr)
            {
            // Report progress
            static_cast<ScalableMeshProgress*>(progress.get())->SetCurrentIteration(m_numPublishedNodes);
            progress->UpdateListeners();
            }
        nodesToProcess.pop();
        }

    //if (SUCCESS != PublishRecursive(source->GetRootNode(), rootPromise, transform))
    //    return ERROR;

    if (progress != nullptr)
        {
        m_publishThreadPool->WaitUntilFinished([this, &progress]()
            {
            // Report progress
            static_cast<ScalableMeshProgress*>(progress.get())->SetCurrentIteration(m_numPublishedNodes);
            progress->UpdateListeners();
            return progress->GetProgress() >= 1.f;
            });
        }
    else
        {
        // Help loading data in memory
        m_publishThreadPool->WaitUntilFinished();
        //bool finishedLoadingData = false;
        //m_publishThreadPool->WaitUntilFinished([this, &finishedLoadingData] ()
        //    {
        //    if (!finishedLoadingData) 
        //        {
        //        std::unique_lock<std::mutex> lock(*m_publishThreadPool.GetPtr());
        //        auto item = m_publishThreadPool->begin(), next = item;
        //        for(; item != m_publishThreadPool->end(); item = next++)
        //            {
        //            auto distance = std::distance(item, m_publishThreadPool->end());
        //            distance;
        //            lock.unlock();
        //            ExtractMeshData(item->m_source, item->m_transform);
        //            if (item->m_source != nullptr) item->m_source->GetTextureCompressed();
        //            lock.lock();
        //            if(next->m_source == nullptr) next = m_publishThreadPool->begin();
        //            }
        //        }
        //    finishedLoadingData = true;
        //    return true;
        //    });
        }

    if (progress != nullptr)
        {
        // Report progress finished
        progress->Progress() = 1.0f;
        progress->UpdateListeners();
        }

    if (!m_publishThreadPool->empty())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SM3SMPublisher::SetNodeMeshData(IScalableMeshNodePtr sourceNode, IScalableMeshMeshPtr mesh, IScalableMeshNodeEditPtr& destNode, const Transform& transform)
    {
    auto extent = sourceNode->GetNodeExtent();
    if (!transform.IsIdentity())
        {
        DPoint3d box[8];
        extent.Get8Corners(box);
        for (size_t i = 0; i < 8; ++i)
            transform.Multiply(box[i]);
        extent = DRange3d::From(box, 8);
        }

    if (SUCCESS != destNode->SetNodeExtent(extent))
        return ERROR;

    auto contentExtent = sourceNode->GetContentExtent();
    if (!transform.IsIdentity())
        {
        DPoint3d box[8];
        contentExtent.Get8Corners(box);
        for (size_t i = 0; i < 8; ++i)
            transform.Multiply(box[i]);
        contentExtent = DRange3d::From(box, 8);
        }

    if (SUCCESS != destNode->SetContentExtent(contentExtent))
        return ERROR;

    float geometricResolution = 0.0f, textureResolution = 0.0f;
    sourceNode->GetResolutions(geometricResolution, textureResolution);
    if (SUCCESS != destNode->SetResolution(geometricResolution, textureResolution))
        return ERROR;

    bool arePoints3D = sourceNode->ArePoints3d();
    if (SUCCESS != destNode->SetArePoints3d(arePoints3D))
        return ERROR;

    if (mesh != nullptr)
        {
        auto polyfaceQuery = mesh->GetPolyfaceQuery();

        bvector<DPoint3d> points(polyfaceQuery->GetPointCount());
        bvector<int32_t> indices(polyfaceQuery->GetPointIndexCount());
        bvector<DPoint2d> uv(polyfaceQuery->GetParamCount());
        bvector<int32_t> uvIndices(polyfaceQuery->GetPointIndexCount());

        if (!points.empty()) memcpy(points.data(), polyfaceQuery->GetPointCP(), points.size() * sizeof(DPoint3d));
        if (!indices.empty()) memcpy(indices.data(), polyfaceQuery->GetPointIndexCP(), indices.size() * sizeof(int32_t));
        if (!uv.empty()) memcpy(uv.data(), polyfaceQuery->GetParamCP(), uv.size() * sizeof(DPoint2d));
        if (!uvIndices.empty() && nullptr != polyfaceQuery->GetParamIndexCP()) memcpy(uvIndices.data(), polyfaceQuery->GetParamIndexCP(), uvIndices.size() * sizeof(int32_t));

        if (!points.empty() && !indices.empty())
            {

            if (!transform.IsIdentity())
                {
                contentExtent = DRange3d::From(points);
                }
            auto texture = sourceNode->GetTextureCompressed();
            sourceNode->ClearCachedData();
            if (texture.IsValid() && texture->GetSize() > 0)
                {
                int64_t newTextureID = -1;
                if (m_params->SaveTextures())
                    {
                    auto dimension = texture->GetDimension();
                    newTextureID = m_params->GetDestination()->AddTextureCompressed(dimension.x, dimension.y, (int)texture->GetNOfChannels(), texture->GetData(), texture->GetSize());
                    }
                if (SUCCESS != destNode->AddTexturedMesh(points, indices, uv, uvIndices, 1, newTextureID))
                    return ERROR;
                }
            else
                {
                if (SUCCESS != destNode->AddMesh(points.data(), points.size(), indices.data(), indices.size()))
                    return ERROR;
                }
            }
        }

    if (SUCCESS != destNode->SetContentExtent(contentExtent))
        return ERROR;

    destNode->ClearCachedData();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   12/18
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshMeshPtr SM3SMPublisher::ExtractMeshData(IScalableMeshNodePtr sourceNode, const Transform& transform)
    {
    if(sourceNode == nullptr || sourceNode->GetPointCount() == 0)
        return nullptr;

    auto meshFlags = IScalableMeshMeshFlags::Create(true /*loadTexture*/, false /*loadGraph*/);
    //meshFlags->SetSaveToCache(true);

    IScalableMeshMeshPtr meshData = sourceNode->GetMeshUnderClip2(meshFlags, m_params->GetClips(), -1, false);

    if(!transform.IsIdentity()) meshData->SetTransform(transform);

    sourceNode->ClearCachedData();

    return meshData;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SM3SMPublisher::ProcessNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& destNode, IScalableMeshMeshPtr meshData, const Transform& transform)
    {
    // Update progress info to indicate a node has been processed from the queue
    ++m_numPublishedNodes;

    if (sourceNode != nullptr && (sourceNode->GetPointCount() > 0))
        {
        if (meshData.IsValid() && meshData != nullptr && meshData->GetPolyfaceQuery()->GetPointCount() > 0)
            {
            if (sourceNode->GetLevel() != 0 && parentDestNode == nullptr)
                {
                // The parent node is clipped out but not this child node. Find suitable parent node from existing destination nodes.
                StatusInt status = SUCCESS;
                parentDestNode = m_params->GetDestination()->FindParentNodeFor(sourceNode, status);
                if (parentDestNode == nullptr && status != SUCCESS)
                    return ERROR; // Couldn't find a parent node
                }

            if (SUCCESS != CreateAndAddNewNode(sourceNode, parentDestNode, destNode))
                return ERROR;

            if (SUCCESS != SetNodeMeshData(sourceNode, meshData, destNode, transform))
                return ERROR;
            }
        }
    else if (sourceNode->GetLevel() == 0 && parentDestNode == nullptr)
        {
        // Add root node even if it is empty...
        if (SUCCESS != CreateAndAddNewNode(sourceNode, nullptr, destNode))
            return ERROR;

        if (SUCCESS != SetNodeMeshData(sourceNode, nullptr, destNode, transform))
            return ERROR;
        }
    else if (parentDestNode != nullptr)
        {
        // Create new (empty) node so that children nodes can effectively be created/added as well
        if (SUCCESS != CreateAndAddNewNode(sourceNode, parentDestNode, destNode))
            return ERROR;

        if (SUCCESS != SetNodeMeshData(sourceNode, nullptr, destNode, transform))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SM3SMPublisher::PublishRecursive(IScalableMeshNodePtr sourceNode, SMNodeEditPromisePtr currentDestNodePromise, const Transform& transform)
    {
    auto progress = m_params->GetProgress();
    if (progress != nullptr && progress->IsCanceled()) return SUCCESS;

    bmap<uint64_t, SMNodeEditPromisePtr> meshNodePromises;
    uint64_t childID = 0;

    // Traverse using breadth first to prevent threads from being blocked as much as when traversing depth first (because we are using a FIFO queue)
    for (auto childNode : sourceNode->GetChildrenNodes())
        {
        meshNodePromises[childID++] = AddWorkItem(childNode, currentDestNodePromise, transform);
        }

    childID = 0;
    for (auto childNode : sourceNode->GetChildrenNodes())
        {
        if (SUCCESS != PublishRecursive(childNode, meshNodePromises[childID++], transform))
            return ERROR;
        }

    if (progress != nullptr)
        {
        // Report progress
        static_cast<ScalableMeshProgress*>(progress.get())->SetCurrentIteration(m_numPublishedNodes);
        progress->UpdateListeners();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SM3SMPublisher::CreateAndAddNewNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& newDestNode)
    {
    StatusInt status = SUCCESS;
    SMStatus smStatus = S_SUCCESS;
    auto extent = sourceNode->GetNodeExtent();
    auto nodeId = sourceNode->GetNodeId();
    {
    std::lock_guard<std::mutex> lock(m_newNodeMtx);
    newDestNode = m_params->GetDestination()->AddChildNode(parentDestNode, extent, smStatus, false, nodeId);
    }
    if (SUCCESS != status || newDestNode == nullptr || smStatus == S_ERROR)
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
SM3SMPublisher::SMNodeEditPromisePtr SM3SMPublisher::AddWorkItem(IScalableMeshNodePtr source, SMNodeEditPromisePtr currentPromise, const Transform& transform)
    {
    SMNodeEditPromisePtr newNodePromise = std::make_shared<SMNodeEditPromise>(SMNodeEditPromise());
    newNodePromise->second = newNodePromise->first.get_future().share();

    // Skip empty leaf nodes and nodes that are entirely clipped
    if ((source->ArePointsFullResolution() && source->GetPointCount() == 0) || IsNodeClippedOut(source))
        {
        // Don't need to clone data for this node, it can be ignored so just update progress information
        ++m_numPublishedNodes;

        // Fulfill the promise with invalid node in case a thread is waiting for it
        newNodePromise->first.set_value(nullptr);
        return newNodePromise;
        }

    m_publishThreadPool->AddWorkItem(LocalThreadPublishInfo{ this, source, currentPromise, newNodePromise, transform });

    return newNodePromise;
    }

bool SM3SMPublisher::IsNodeClippedOut(IScalableMeshNodePtr sourceNode)
    {
    //auto range = sourceNode->GetContentExtent();
    //DPoint3d center = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    //double radius = -range.DiagonalDistance() * 0.5;
    //for(auto clip : *m_params->GetClips())
    //    {
    //    if(clip->IsMask() && clip->SphereInside(center, radius))
    //        return true;
    //    }
    return false;
#if 0 
    if (sourceRange.IsNull() || sourceRange.IsEmpty()) return true;
    for (auto const& clipRangeInfo : m_clipRanges)
        {
        BeAssert(clipRangeInfo.m_clip != nullptr);
        if (clipRangeInfo.m_clip->IsMask())
            {
            if (sourceRange.IsContained(clipRangeInfo.m_range))
                {
                bvector<DPoint3d> intersectPoints(12);
                for (auto const& convexPlanes : *clipRangeInfo.m_clip->GetMaskPlanes())
                    {
                    for (auto const& plane : convexPlanes)
                        {
                        const auto& dplane3d = plane.GetDPlane3d();
                        int nbIntersectPoints = 0;
                        bsiDRange3d_intersectPlane(intersectPoints.data(), &nbIntersectPoints, 12, &sourceRange, &dplane3d.origin, &dplane3d.normal, false);
                        if (nbIntersectPoints > 0)
                            return false;
                        }
                    }

                // No intersection, check for inside/outside (for clip mask, inside means outside of the clip polygon
                if (clipRangeInfo.m_clip->PointInside(sourceRange.low))
                    return false;

                return true;
                }
            }
        else if (!sourceRange.IntersectsWith(clipRangeInfo.m_range))
            return true;
        }
    return false;
#endif
    }


