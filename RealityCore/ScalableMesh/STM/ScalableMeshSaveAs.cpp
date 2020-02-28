/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMeshSaveAs.h>
#include "Stores/SMPublisher.h"
#include "ScalableMeshQuery.h"
#include "SMMeshIndex.h"

#include "ScalableMesh.h"
#include "ScalableMeshProgress.h"
#include "SMNodeGroup.h"
#include "Stores/SMStreamingDataStore.h"

#ifndef VANCOUVER_API
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#else
#include <DgnPlatform/Tools/ConfigurationManager.h>
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define SMSAVEASLOGNAME L"ScalableMesh::SMSaveAs"
#define SMSAVEAS_LOG (*NativeLogging::LoggingManager::GetLogger(SMSAVEASLOGNAME))

void PrepareClipsForSaveAs(ClipVectorPtr clips)
    {
    if (!clips.IsValid()) return;

    auto predicate = [](ClipPrimitivePtr i, ClipPrimitivePtr j)
        { // put boundary clips first
        if (!i->IsMask() && j->IsMask()) return true;
        return false;
        };

    // Ensure clip vector is sorted
    if (!std::is_sorted(clips->begin(), clips->end(), predicate))
        {
        std::sort(clips->begin(), clips->end(), predicate);
        }

    // Compute clip planes here as primitives are not thread safe
    for (auto& primitive : *clips) primitive->GetClipPlanes();
    }

StatusInt IScalableMeshSaveAs::DoSaveAs(const IScalableMeshPtr& source, const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress)
    {
    BeAssert(!"Deprecated. Use other version of DoSaveAs");
    auto transform = Transform::FromIdentity();
    return DoSaveAs(source, destination, clips, progress, transform);
    }

StatusInt IScalableMeshSaveAs::DoSaveAs(const IScalableMeshPtr& source, const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress, const Transform& transform)
{
    // Create Scalable Mesh at output path
    SaveAsNodeCreatorPtr scMeshDestination = new SaveAsNodeCreator(destination.c_str());
    if (!scMeshDestination.IsValid())
        return ERROR;

    IScalableMeshTextureInfoPtr textureInfo = nullptr;
    if (SUCCESS != source->GetTextureInfo(textureInfo))
        return ERROR;

    // Gather 3sm clips along with the other clips that are being passed
    bvector<uint64_t> clipIds;
    source->GetAllClipIds(clipIds);

    for(auto clipID : clipIds)
        {
        bvector<DPoint3d> clipData;
        if(source->GetClip(clipID, clipData))
            {
            //create a clipvector with infinite z dimensions for the polygons
            auto curvePtr = ICurvePrimitive::CreateLineString(clipData);
            CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
            clips->Append(*ClipVector::CreateFromCurveVector(*cv, 0.0, 0.1));
            }
        else
            {
            ClipVectorPtr clipVector;
            source->GetClip(clipID, clipVector);
            clips->Append(*clipVector);
            }
        SMNonDestructiveClipType clipType;
        source->GetClipType(clipID, clipType);
        clips->back()->SetIsMask(clipType == SMNonDestructiveClipType::Mask);
        }

    PrepareClipsForSaveAs(clips);

    // Set global parameters to the new 3sm (this will also create a new index)
    if (SUCCESS != scMeshDestination->SetGCS(source->GetGCS()))
        return ERROR;
    scMeshDestination->SetIsTerrain(source->IsTerrain());
    scMeshDestination->SetIsSingleFile(!source->IsCesium3DTiles());
    scMeshDestination->SetTextured(textureInfo->GetTextureType());

    //copy sources
    IDTMSourceCollection sourceCollection;
    source->LoadSources(sourceCollection);
    scMeshDestination->SaveSources(sourceCollection);

    { // Scope the publishing part for proper cleanup of parameters when finished
        SM3SMPublishParamsPtr smParams = new SM3SMPublishParams();

        smParams->SetSource(source.get());
        smParams->SetDestination(scMeshDestination);
        smParams->SetClips(clips);
        smParams->SetProgress(progress);
        smParams->SetSaveTextures(textureInfo->IsTextureAvailable() && !textureInfo->IsUsingBingMap());
        smParams->SetTransform(transform);

        auto smPublisher = IScalableMeshPublisher::Create(SMPublishType::THREESM);
        if (SUCCESS != smPublisher->Publish(smParams))
            return ERROR;
    }

    scMeshDestination = nullptr;

    return SUCCESS;
}


bool Publish3DTile(IScalableMeshNodePtr& node, ISMDataStoreTypePtr<DRange3d>& pi_pDataStore, TransformCR transform, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const GeoCoordinates::BaseGCSCPtr sourceGCS, const GeoCoordinates::BaseGCSCPtr destinationGCS, IScalableMeshProgressPtr progress, bool outputTexture)
{
    assert(pi_pDataStore != nullptr);

    static double startTime = clock();
    static std::atomic<uint64_t> loadDataTime(0);
    static std::atomic<uint64_t> convertTime(0);
    static std::atomic<uint64_t> storeTime(0);
    static std::atomic<uint64_t> nbProcessedNodes(0);
    static std::atomic<uint64_t> nbNodes(0);

    if (progress != nullptr && progress->IsCanceled()) return false;

    if (!node->IsHeaderLoaded())
        node->LoadNodeHeader();

    if (node->GetLevel() == 0)
    {
        startTime = clock();
        loadDataTime = 0;
        convertTime = 0;
        storeTime = 0;
        nbProcessedNodes = 0;
        nbNodes = 0;
    }

    ++nbNodes;

    static uint64_t nbThreads = std::max((uint64_t)1, (uint64_t)(std::thread::hardware_concurrency() - 2));
    static const uint64_t maxQueueSize = /*std::max((uint64_t)m_SMIndex->m_totalNumNodes, (uint64_t)*/30000;//);

    typedef SMNodeDistributor<IScalableMeshNodePtr> Distribution_Type;
    static Distribution_Type::Ptr distributor = nullptr;

    if (node->GetLevel() == 0)
    {
        WString cfgVarValueStr;
        if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"SMPUBLISH_NUM_THREADS"))
            {
            int nT = std::stoi(cfgVarValueStr.c_str());
            if (nT >= 1) nbThreads = (uint64_t)nT;
            }

        SMSAVEAS_LOG.debugv("Publishing using [%I64d] threads", nbThreads);

        startTime = clock();
        bvector<DRange3d> ranges;
        bool allClipsAreMasks = true;

        if (clips.IsValid())
        {
            for (ClipPrimitivePtr const& primitive : *clips)
            {
                DRange3d        thisRange;
                if (primitive->GetRange(thisRange, nullptr, primitive->IsMask()))
                {
                    ranges.push_back(thisRange);
                }
                if (!primitive->IsMask())
                    allClipsAreMasks = false;
            }
        }

        //(*clips)[0]->GetRange(range, nullptr, true);
        auto nodeDataSaver = [pi_pDataStore, ranges, allClipsAreMasks, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, transform, outputTexture](IScalableMeshNodePtr& node)
        {
            if (progress != nullptr  && progress->IsCanceled()) return;

            std::wstringstream      threadIDStrStream;
            std::thread::id threadID = std::this_thread::get_id();
            threadIDStrStream << threadID;
            SMSAVEAS_LOG.debugv("Processing node [%I64d] in thread %ls", node->GetNodeId(), threadIDStrStream.str().c_str());

            bool hasMSClips = false;
            for (auto const& range : ranges)
            {
                if (!node->GetContentExtent().IsNull() && range.IntersectsWith(node->GetContentExtent()))
                    hasMSClips = true;
            }

            if (hasMSClips || allClipsAreMasks)
            {

                IScalableMeshNodePtr nodeP(node);
                bvector<Byte> cesiumData;
                IScalableMeshPublisherPtr cesiumPublisher = IScalableMeshPublisher::Create(SMPublishType::CESIUM);
                cesiumPublisher->Publish(nodeP, (hasMSClips ? clips : nullptr), coverageID, isClipBoundary, transform, cesiumData, outputTexture);

                ISMTileMeshDataStorePtr tileStore;

                auto smPtNode = (dynamic_cast<ScalableMeshNode<DPoint3d>*>(node.get()))->GetNodePtr();
                bool result = pi_pDataStore->GetNodeDataStore(tileStore, &smPtNode->m_nodeHeader);
                assert(result == true); // problem getting the tile mesh data store

                if (!cesiumData.empty())
                    tileStore->StoreBlock(&cesiumData, cesiumData.size(), smPtNode->GetBlockID());
                else
                    {
                    SMSAVEAS_LOG.errorv("No Cesium 3DTiles data generated for node [%I64d]", nodeP->GetNodeId());
                    }

                //// Store header
                //pi_pDataStore->StoreNodeHeader(&node->m_nodeHeader, node->GetBlockID());

                //{
                //std::lock_guard<mutex> clk(s_consoleMutex);
                //std::wcout << "[" << std::this_thread::get_id() << "] Done publishing --> " << node->m_nodeId << "    addr: "<< node <<"   ref count: " << node->GetRefCount() << std::endl;
                //}
            }

            node = nullptr;
            ++nbProcessedNodes;

            if (progress != nullptr)
            {
                // Report progress
                static_cast<ScalableMeshProgress*>(progress.get())->SetCurrentIteration(nbProcessedNodes);
            }
        };
        distributor = new Distribution_Type(nodeDataSaver, [](IScalableMeshNodePtr& node) {return true; }, nbThreads, maxQueueSize);
    }

    static auto loadChildExtentHelper = [](IScalableMeshNode* parent, IScalableMeshNode* child) ->void
    {

        auto smPtNode = dynamic_cast<ScalableMeshNode<DPoint3d>*>(parent)->GetNodePtr();
        // parent header needs child extent for the Cesium format
        if (child->GetPointCount() > 0 && child->GetNodeId() != 0)
        {
            auto childExtent = child->GetContentExtent();
            smPtNode->m_nodeHeader.m_childrenExtents[child->GetNodeId()] = childExtent;
        }
    };

#if 0
    if (m_pSubNodeNoSplit != nullptr)
    {
        assert(GetNumberOfSubNodesOnSplit() == 1 || GetNumberOfSubNodesOnSplit() == 4);
        if (!static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->Publish3DTile(pi_pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture)) return false;
        loadChildExtentHelper(this, this->m_pSubNodeNoSplit.GetPtr());
        //disconnectChildHelper(this->m_pSubNodeNoSplit.GetPtr());
        //this->m_pSubNodeNoSplit = nullptr;
    }
    else
    {
        assert(GetNumberOfSubNodesOnSplit() > 1 || this->IsLeaf());
        if (!this->IsLeaf())
        {
            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
            {
                assert(this->m_apSubNodes[indexNode] != nullptr); // A sub node will be skipped
                if (this->m_apSubNodes[indexNode] != nullptr)
                {
                    if (!static_cast<SMMeshIndexNode<DPoint3d, DRange3d>*>(&*(this->m_apSubNodes[indexNode]))->Publish3DTile(pi_pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture)) return false;
                    loadChildExtentHelper(this, this->m_apSubNodes[indexNode].GetPtr());
                    //disconnectChildHelper(this->m_apSubNodes[indexNode].GetPtr());
                    //this->m_apSubNodes[indexNode] = nullptr;
                }
            }
        }
    }

#endif

    for (auto& child : node->GetChildrenNodes())
    {
        if (!Publish3DTile(child, pi_pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture)) return false;
        loadChildExtentHelper(node.get(), child.get());
    }

    IScalableMeshNodePtr ptr(node);
    if (node->GetPointCount() > 2)
    {

        distributor->AddWorkItem(std::move(ptr)/*, false*/);
    }

    if (node->GetLevel() == 0)
        {
        bool printStats = false;
        WString cfgVarValueStr;
        if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"SMPUBLISH_PRINT_STATS"))
            printStats = true;
        if (printStats)
            {
            SMSAVEAS_LOG.infov("Time to process tree: %f", (clock() - startTime) / CLOCKS_PER_SEC);
            }
        while (progress != nullptr && !distributor->empty())
            {
            progress->UpdateListeners();
            if (progress->IsCanceled()) break;
            }
        distributor = nullptr; // join queue threads
        if (printStats)
            {
            auto tTime = (double)(clock() - startTime) / CLOCKS_PER_SEC;
            SMSAVEAS_LOG.infov("Total time: %f", tTime);
            SMSAVEAS_LOG.infov("Time to convert data : %f", (double)convertTime.load() / CLOCKS_PER_SEC / nbThreads);
            SMSAVEAS_LOG.infov("Time to store data: %f", (double)storeTime.load() / CLOCKS_PER_SEC / nbThreads);
            SMSAVEAS_LOG.infov("Number processed nodes: %I64d", nbProcessedNodes.load());
            SMSAVEAS_LOG.infov("Total number of nodes: %I64d", nbNodes.load());
            SMSAVEAS_LOG.infov("Convert speed (nodes/sec): %f", nbProcessedNodes.load() / tTime);
            }
        }
    return true;
}

StatusInt Publish3DTiles(SMMeshIndex<DPoint3d,DRange3d>* index, const WString& path, ClipVectorPtr clips, const uint64_t& coverageID, const GeoCoordinates::BaseGCSCPtr sourceGCS, bool outputTexture,ScalableMeshProgress* progress)
{
    if (progress != nullptr)
    {
        progress->ProgressStep() = ScalableMeshStep::STEP_GENERATE_3DTILES_HEADERS;
        progress->ProgressStepIndex() = 1;
        progress->Progress() = 0.0f;
    }

    bool isClipBoundary = false;
    if (coverageID != -2 && coverageID != -1)
    {
        for (const auto& diffSet : *static_cast<SMMeshIndexNode<DPoint3d, DRange3d>*>(index->GetRootNode().GetPtr())->GetDiffSetPtr())
        {
            if (diffSet.clientID == coverageID)
            {
                bvector<bvector<DPoint3d>> polys;
                polys.push_back(bvector<DPoint3d>());
                SMClipGeometryType geom;
                SMNonDestructiveClipType type;
                bool isActive;
                index->GetClipRegistry()->GetClipWithParameters(diffSet.clientID, polys.back(), geom, type, isActive);
                if (type == SMNonDestructiveClipType::Boundary)
                {
                    isClipBoundary = true;
                }
            }
        }
    }

    // Create store for 3DTiles output
    typedef SMStreamingStore<DRange3d>::SMStreamingSettings StreamingSettingsType;
    SMStreamingStore<DRange3d>::SMStreamingSettingsPtr settings = new StreamingSettingsType();
    settings->m_url = Utf8String(path.c_str());
    settings->m_location = StreamingSettingsType::ServerLocation::LOCAL;
    settings->m_dataType = StreamingSettingsType::DataType::CESIUM3DTILES;
    settings->m_commMethod = StreamingSettingsType::CommMethod::CURL;
    settings->m_isPublishing = true;
    ISMDataStoreTypePtr<DRange3d>     pDataStore(
#ifndef VANCOUVER_API
        new SMStreamingStore<DRange3d>(settings, nullptr)
#else
        SMStreamingStore<DRange3d>::Create(settings, nullptr)
#endif
    );

    // Register 3DTiles index to the store
    pDataStore->Register(index->m_smID);

    // Destination coordinates will be ECEF
    // Create a WGS84 GCS to convert the WGS84 Lat/Long to ECEF/XYZ
    WString warningMsg;
    StatusInt warning;
    auto destinationGCS = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
    destinationGCS->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary

    auto ecefTrans = Transform::FromIdentity();
    Transform toMeterTransform = Transform::FromIdentity();
    if(sourceGCS.IsValid())
        {
        GeoCoords::Unit unit(GeoCoords::GetUnitFor(*sourceGCS));

        toMeterTransform = Transform::FromRowValues(unit.GetRatioToBase(), 0, 0, 0.0,
                                                    0, unit.GetRatioToBase(), 0, 0.0,
                                                    0, 0, unit.GetRatioToBase(), 0.0);

        destinationGCS->SetVerticalDatumCode(sourceGCS->GetVerticalDatumCode());


        DRange3d range = index->GetContentExtent();
        DPoint3d origin = DPoint3d::FromInterpolate(range.low, .5, range.high);

        GeoPoint originLatLong, yLatLong, tempLatLong;
        sourceGCS->LatLongFromCartesian(originLatLong, origin);
        sourceGCS->LatLongFromCartesian(yLatLong, DPoint3d::FromSumOf(origin, DPoint3d::From(0.0, 1.0, 0.0)));         // Arbitrarily 10 meters in Y direction will be used to calculate y axis of transform.

        tempLatLong = originLatLong;
        sourceGCS->LatLongFromLatLong(originLatLong, tempLatLong, *destinationGCS);
        tempLatLong = yLatLong;
        sourceGCS->LatLongFromLatLong(yLatLong, tempLatLong, *destinationGCS);

        DPoint3d ecefOrigin, ecefY;
        destinationGCS->XYZFromLatLong(ecefOrigin, originLatLong);
        destinationGCS->XYZFromLatLong(ecefY, yLatLong);

        RotMatrix rMatrix = RotMatrix::FromIdentity();
        DVec3d zVector, yVector;
        zVector.Normalize((DVec3dCR)ecefOrigin);
        yVector.NormalizedDifference(ecefY, ecefOrigin);

        rMatrix.SetColumn(yVector, 1);
        rMatrix.SetColumn(zVector, 2);
        rMatrix.SquareAndNormalizeColumns(rMatrix, 1, 2);

        ecefTrans = Transform::From(rMatrix, ecefOrigin);

        toMeterTransform.Multiply(origin);

        auto dbToTile = Transform::From(DPoint3d::From(-origin.x, -origin.y, -origin.z));
        ecefTrans = Transform::FromProduct(ecefTrans, dbToTile);
        }

    SMIndexMasterHeader<DRange3d> oldMasterHeader;
    index->GetDataStore()->LoadMasterHeader(&oldMasterHeader, sizeof(oldMasterHeader));

    // Force multi file, in case the originating dataset is single file (result is intended for multi file anyway)
    oldMasterHeader.m_singleFile = false;

    SMGroupGlobalParameters::Ptr groupParameters = SMGroupGlobalParameters::Create(SMGroupGlobalParameters::StrategyType::CESIUM, static_cast<SMStreamingStore<DRange3d>*>(pDataStore.get())->GetDataSourceAccount(), DataSource::SessionName());
    SMGroupCache::Ptr groupCache = nullptr;
    SMNodeGroupPtr rootNodeGroup = SMNodeGroup::Create(groupParameters, groupCache, path, 0, nullptr);

    rootNodeGroup->SetMaxGroupDepth(index->GetDepth() % s_max_group_depth + 1);

    auto strategy =rootNodeGroup->GetStrategy<DRange3d>();
    strategy->SetOldMasterHeader(oldMasterHeader);
    strategy->SetClipInfo(coverageID, isClipBoundary);
    //strategy->SetSourceAndDestinationGCS(sourceGCS, destinationGCS);
    strategy->SetTransform(toMeterTransform);
    strategy->SetRootTransform(ecefTrans);
    strategy->AddGroup(rootNodeGroup.get());
    strategy->SetPublisherInfo(ScalableMeshModuleInfo());
    index->SetRootNodeGroup(rootNodeGroup);

    SMSAVEAS_LOG.debug("Publishing index...");

    // Saving groups isn't parallelized therefore we run it in a single separate thread so that we can properly update the listener with the progress
    std::thread saveGroupsThread([index, strategy, rootNodeGroup, progress]()
    {
        index->GetRootNode()->SaveGroupedNodeHeaders(rootNodeGroup, progress);

        // Handle all open groups
        strategy->SaveAllOpenGroups(false/*saveRoot*/);
        if (progress != nullptr) progress->Progress() = 1.0f;
    });

    while (progress != nullptr && !progress->IsCanceled() && progress->Progress() != 1.0)
    {
        progress->UpdateListeners();
    }

    saveGroupsThread.join();

    strategy->Clear();


    if (progress != nullptr && progress->IsCanceled()) return SUCCESS;

    if (progress != nullptr)
    {
        progress->ProgressStep() = ScalableMeshStep::STEP_CONVERT_3DTILES_DATA;
        progress->ProgressStepIndex() = 2;
        progress->Progress() = 0.0f;
    }

    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> nodePtr = dynamic_cast<SMPointIndexNode<DPoint3d, DRange3d>*>(index->GetRootNode().GetPtr());
    IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
        new ScalableMeshNode<DPoint3d>(nodePtr)
#else
        ScalableMeshNode<DPoint3d>::CreateItem(nodePtr)
#endif
    );

    PrepareClipsForSaveAs(clips);

    SMSAVEAS_LOG.debug("Publishing nodes...");

    Publish3DTile(nodeP, pDataStore, toMeterTransform, clips, coverageID, isClipBoundary, nullptr/*sourceGCS*/, nullptr/*destinationGCS*/, progress, outputTexture);

    if (progress != nullptr) progress->Progress() = 1.0f;

    return SUCCESS;
}

bool s_stream_from_wsg;
bool s_stream_from_grouped_store;
bool s_is_virtual_grouping;

StatusInt IScalableMeshSaveAs::Generate3DTiles(const IScalableMeshPtr& meshP, const WString& outContainerName, const Transform& transform, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId)
    {
    if (!meshP.IsValid()) return ERROR;

    StatusInt status;

    auto mesh = static_cast<ScalableMesh<DPoint3d>*>(meshP.get());
    WString path;
    if (server == SMCloudServerType::Azure)
        {
        // Setup streaming stores to use Azure
        //s_stream_from_disk = false;
        s_stream_from_wsg = false;

        path += outContainerName + L"/" + outDatasetName;
        }
    else if (server == SMCloudServerType::WSG)
        {
        // Setup streaming stores to use WSG
        //s_stream_from_disk = false;
        s_stream_from_wsg = true;

        path += outContainerName + L"~2F" + outDatasetName;
        }
    else if (server == SMCloudServerType::LocalDiskCURL)
        {
        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        s_stream_using_curl = true;

        const auto smFileName = BeFileName(mesh->GetPath());
        path += BEFILENAME(GetDirectoryName, smFileName);
        path += L"cloud\\";
        path += BEFILENAME(GetFileNameWithoutExtension, smFileName);
        }
    else
        {
        assert(server == SMCloudServerType::LocalDisk);

        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        path = outContainerName;
        }

    //s_stream_from_grouped_store = false;
    mesh->GetMainIndexP()->m_progress = progress;
    bool hasCoverages = false;
    bvector<SMNodeGroupPtr> coverageTilesets;

    if (coverageId == (uint64_t)-1)
        {
        // Generate 3DTiles tilesets for all coverages
        bvector<uint64_t> ids;
        mesh->GetMainIndexP()->GetClipRegistry()->GetAllCoverageIds(ids);
        hasCoverages = !ids.empty();
        for (auto coverageID : ids)
            {
            Utf8String coverageName;
            mesh->GetMainIndexP()->GetClipRegistry()->GetCoverageName(coverageID, coverageName);

            BeFileName coverageFileName(coverageName.c_str());
            if (BeFileName::DoesPathExist(coverageFileName))
                {
                // Ensure that coverage path is formatted correctly (e.g. remove redundant double backslashes such as \\\\)
                WString coverageFullPathName;
                BeFileName::BeGetFullPathName(coverageFullPathName, coverageFileName.c_str());

                IScalableMeshPtr coverageMeshPtr = nullptr;
                if ((coverageMeshPtr = IScalableMesh::GetFor(coverageFullPathName.c_str(), meshP->GetEditFilesBasePath(), false, true, true, status)) == nullptr || status != SUCCESS)
                    {
                    BeAssert(false); // Error opening coverage 3sm
                    return status;
                    }

                auto coverageMesh = static_cast<ScalableMesh<DPoint3d>*>(coverageMeshPtr.get());

                // Create directory for coverage tileset output
                BeFileName coverageOutDir(path.c_str());
                coverageOutDir.AppendToPath(BeFileName::GetFileNameWithoutExtension(coverageFileName).c_str());
                coverageOutDir.AppendSeparator();
                if (!BeFileName::DoesPathExist(coverageOutDir) && (status = (StatusInt)BeFileName::CreateNewDirectory(coverageOutDir)) != SUCCESS)
                    {
                    BeAssert(false); // Could not create tileset output directory for coverage
                    return status;
                    }
                if ((status = Generate3DTiles(coverageMesh, coverageOutDir.c_str(), outDatasetName, server, nullptr /*no progress?*/, clips, coverageID)) != SUCCESS)
                    {
                    BeAssert(false); // Could not publish coverage
                    return status;
                    }
                auto coverageIndex = coverageMesh->GetMainIndexP();
                auto root = coverageIndex->GetRootNodeGroup();
                BeAssert(root.IsValid()); // Something wrong in the publish
                coverageTilesets.push_back(root);
                }
            }
        }


    IScalableMeshTextureInfoPtr textureInfo;

    status = meshP->GetTextureInfo(textureInfo);

    bool outputTexture = true;

    //BingMap texture MUST NOT be baked into the Cesium 3D tile data
    if (status != SUCCESS || textureInfo->IsUsingBingMap())
        outputTexture = false;

    status = Publish3DTiles((SMMeshIndex<DPoint3d, DRange3d>*)(mesh->GetMainIndexP().GetPtr()), path, clips, (uint64_t)(hasCoverages && coverageId == (uint64_t)-1 ? 0 : coverageId), meshP->GetGCS().GetGeoRef().GetBasePtr(), outputTexture, (ScalableMeshProgress*)(mesh->GetMainIndexP()->m_progress.get()));
    SMNodeGroupPtr rootTileset = mesh->GetMainIndexP()->GetRootNodeGroup();
    BeAssert(rootTileset.IsValid()); // something wrong in the publish


    for (auto& converageTileset : coverageTilesets)
        {
        // insert tileset as child tileset to the current tileset
        rootTileset->AppendChildGroup(converageTileset);
        converageTileset->Close<Extent3dType>();
        }

    WString wktStr;

    mesh->GetDbFile()->GetWkt(wktStr);



    rootTileset->GetParameters()->SetWellKnownText(wktStr);

    // Force save of root tileset and take into account coverages
    rootTileset->Close<Extent3dType>();
    return status;
        }
StatusInt IScalableMeshSaveAs::Generate3DTiles(const IScalableMeshPtr& meshP, const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId)
    {
    return IScalableMeshSaveAs::Generate3DTiles(meshP, outContainerName, Transform::FromIdentity(), outDatasetName, server, progress, clips, coverageId);
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
