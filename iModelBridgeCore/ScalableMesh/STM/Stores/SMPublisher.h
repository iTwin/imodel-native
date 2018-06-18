/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\IScalableMesh.h>
#include <ScalableMesh\IScalableMeshProgress.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <ScalableMesh\IScalableMeshPublisher.h>
#include "../SMNodeGroup.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define MAX_QUEUE_SIZE 30000

struct SM3SMPublishParams;
typedef RefCountedPtr<SM3SMPublishParams> SM3SMPublishParamsPtr;

struct SM3SMPublishParams : virtual public IScalableMeshPublishParams
    {
    private:

        IScalableMeshPtr m_source = nullptr;
        IScalableMeshNodeCreatorPtr m_destination = nullptr;
        ClipVectorPtr m_clips = nullptr;
        IScalableMeshProgressPtr m_progress = nullptr;
        bool m_saveTextures = false;
        Transform m_transform;

    public:

        IScalableMeshPtr GetSource() { return m_source;}

        IScalableMeshNodeCreatorPtr GetDestination() { return m_destination;}

        ClipVectorPtr GetClips() { return m_clips; }

        IScalableMeshProgressPtr GetProgress() { return m_progress; }

        Transform GetTransform() { return m_transform; }

        bool SaveTextures() { return m_saveTextures; }

        void SetSource(IScalableMeshPtr source) { m_source = source; }

        void SetDestination(IScalableMeshNodeCreatorPtr destination) { m_destination = destination; }

        void SetClips(ClipVectorPtr clips) { m_clips = clips; }

        void SetProgress(IScalableMeshProgressPtr progress) { m_progress = progress; }

        void SetSaveTextures(const bool saveTextures) { m_saveTextures = saveTextures; }

        void SetTransform(const Transform& transform) { m_transform = transform; }

    };

struct SM3SMPublisher : virtual public IScalableMeshPublisher
    {
    private:
        typedef std::pair<std::promise<IScalableMeshNodeEditPtr>, std::shared_future<IScalableMeshNodeEditPtr>> SMNodeEditPromise;
        typedef std::shared_ptr<SMNodeEditPromise> SMNodeEditPromisePtr;
        struct LocalThreadPublishInfo
            {
            SM3SMPublisher* m_publisher;
            IScalableMeshNodePtr m_source;
            SMNodeEditPromisePtr m_destParentFuture;
            SMNodeEditPromisePtr m_destNodePromise;
            Transform            m_transform;
            };

        struct ClipRangeInfo
            {
            ClipPrimitivePtr m_clip = nullptr;
            DRange3d m_range;
            };

        typedef SMNodeDistributor<LocalThreadPublishInfo> SMPublishThreadPool;
        typedef SMPublishThreadPool::Ptr SMPublishThreadPoolPtr;

        static SMPublishThreadPoolPtr s_publishThreadPool;

        static SMPublishThreadPoolPtr GetPublishThreadPool();

        bvector<ClipRangeInfo> m_clipRanges;
        SM3SMPublishParamsPtr m_params = nullptr;
        std::atomic<uint64_t> m_numPublishedNodes = {0};
        SMPublishThreadPoolPtr m_publishThreadPool = nullptr;
        std::mutex m_newNodeMtx;

    protected:

        StatusInt _Publish(IScalableMeshPublishParamsPtr params) override;
        void _Publish(IScalableMeshNodePtr node, const Transform& tranform, bvector<Byte>& outData, bool outputTexture) override {}
        void _Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData, bool outputTexture) override {}
		void _ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smHeader) override {}
		void _ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smMasterHeader) override {}



    private:

        bool      IsNodeClippedOut(IScalableMeshNodePtr sourceNode);
        StatusInt ProcessNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& destNode, const Transform& transform);
        StatusInt SetNodeMeshData(IScalableMeshNodePtr sourceNode, IScalableMeshMeshPtr mesh, IScalableMeshNodeEditPtr& destNode, const Transform& transform);
        StatusInt PublishRecursive(IScalableMeshNodePtr sourceNode, SMNodeEditPromisePtr parentDestNode, const Transform& transform);
        StatusInt CreateAndAddNewNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& newDestNode);

        SMNodeEditPromisePtr AddWorkItem(IScalableMeshNodePtr source, SMNodeEditPromisePtr currentPromise, const Transform& transform);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE