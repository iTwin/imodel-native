/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/TileLoaderQueue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma  once
#include <ScalableMesh\AutomaticGroundDetection\GroundDetectionManager.h>
#include "PointCloudQuadTree.h"


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct TileLoadMessage
    {
    typedef enum
        {
        MESSAGE_LOAD_TILE,
        MESSAGE_SAVE_TILE,
        MESSAGE_BLANK
        } MessageTag;
    MessageTag tag;
    PointCloudQuadNode* tile;

    TileLoadMessage() : tag(MESSAGE_BLANK), tile(nullptr) {};
    TileLoadMessage(MessageTag msgTag, PointCloudQuadNode* msgData) : tag(msgTag), tile(msgData) {};
    void Process();
    };

typedef std::function<bool()> TileLoaderStop;

class TileLoaderQueue
    {
    private:
        std::atomic<int> tail;
        std::atomic<int> head;
        std::vector<TileLoadMessage> messageQueue;
        TileLoaderStop stopFunction;

    public:
        const size_t MAX_N_MESSAGES = 4000;
        std::atomic<bool> needResize;
        TileLoaderQueue();
        //We have an explicit copy constructor because atomic is non-copyable.
        TileLoaderQueue(const TileLoaderQueue& queue);
        void Run();
        void SetStop(TileLoaderStop stopFunc);
        void Send(TileLoadMessage& m);
        TileLoadMessage& Receive();
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
