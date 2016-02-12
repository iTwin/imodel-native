/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/TileLoaderQueue.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include "../STM/ImagePPHeaders.h"
#include "TileLoaderQueue.h"


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

TileLoaderQueue::TileLoaderQueue()
    {
    tail = head = 0;
    messageQueue.resize(MAX_N_MESSAGES);
    needResize = false;
    stopFunction = []() { return false; }; 
    }

TileLoaderQueue::TileLoaderQueue(const TileLoaderQueue& queue)
    {
    tail = head = 0;
    messageQueue.resize(MAX_N_MESSAGES);
    needResize = false;
    stopFunction = queue.stopFunction;
    }

void TileLoaderQueue::Run()
    {
    bool processed;
    while (!stopFunction() || tail > head)
        {
        processed = false;
        if (tail > head)
            {
            TileLoadMessage m = Receive();
            m.Process();
            processed = true;
            }
        if (needResize)
            {
            int h = head;
            int t = tail;
            std::rotate(messageQueue.begin(),messageQueue.begin() + h, messageQueue.end());
            head = 0;
            tail = t - h;
            needResize = false;
            }
        if(!processed) this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }

void TileLoaderQueue::SetStop(TileLoaderStop stopFunc)
    {
    stopFunction = stopFunc;
    }

void TileLoaderQueue::Send(TileLoadMessage& m)
    {
    while (needResize) { this_thread::sleep_for(std::chrono::microseconds(50));  }
    if (tail > MAX_N_MESSAGES - 10) needResize = true;
    else
        {
        messageQueue[tail++] = m;
        }
    }

TileLoadMessage& TileLoaderQueue::Receive()
    {
    ++head;
    return messageQueue.at(head);
    }

void TileLoadMessage::Process()
    {
    if (tag == MESSAGE_LOAD_TILE)
        {
        if(tile->m_pChannelBuffer == nullptr) tile->loadData(true);
        tile->isReady = true;
        }
    else if (tag == MESSAGE_SAVE_TILE)
        {
        tile->saveClassification();
        tile->unloadData();
        tile->isReady = false;
        }
    }
END_BENTLEY_SCALABLEMESH_NAMESPACE
