/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

    public:

        IScalableMeshPtr GetSource() { return m_source;}

        IScalableMeshNodeCreatorPtr GetDestination() { return m_destination;}

        ClipVectorPtr GetClips() { return m_clips; }

        IScalableMeshProgressPtr GetProgress() { return m_progress; }

        bool SaveTextures() { return m_saveTextures; }

        void SetSource(IScalableMeshPtr source) { m_source = source; }

        void SetDestination(IScalableMeshNodeCreatorPtr destination) { m_destination = destination; }

        void SetClips(ClipVectorPtr clips) { m_clips = clips; }

        void SetProgress(IScalableMeshProgressPtr progress) { m_progress = progress; }

        void SetSaveTextures(const bool saveTextures) { m_saveTextures = saveTextures; }

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
        std::atomic<uint64_t> m_numPublishedNodes = 0;
        SMPublishThreadPoolPtr m_publishThreadPool = nullptr;

    protected:

        StatusInt _Publish(IScalableMeshPublishParamsPtr params) override;
        void _Publish(IScalableMeshNodePtr node, const Transform& tranform, bvector<Byte>& outData, bool outputTexture) override {}
        void _Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData, bool outputTexture) override {}

    private:

        bool      IsNodeClippedOut(IScalableMeshNodePtr sourceNode);
        StatusInt ProcessNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& destNode);
        StatusInt SetNodeMeshData(IScalableMeshNodePtr sourceNode, IScalableMeshMeshPtr mesh, IScalableMeshNodeEditPtr& destNode);
        StatusInt PublishRecursive(IScalableMeshNodePtr sourceNode, SMNodeEditPromisePtr parentDestNode);
        StatusInt CreateAndAddNewNode(IScalableMeshNodePtr sourceNode, IScalableMeshNodeEditPtr parentDestNode, IScalableMeshNodeEditPtr& newDestNode);

        SMNodeEditPromisePtr AddWorkItem(IScalableMeshNodePtr source, SMNodeEditPromisePtr currentPromise);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE