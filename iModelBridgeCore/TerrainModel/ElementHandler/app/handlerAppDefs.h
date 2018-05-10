/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/app/handlerAppDefs.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Mstn/MdlApi/rtypes.r.h>
#include <Mstn/MdlApi/rscdefs.r.h>

enum TerrainModelAppMessageLists
    {
    TerrainModelAppMessageList_Default = 1,
    };


enum TerrainModelAppMessages
    {
    MESSAGE_HandleMoveStart = 1,
    MESSAGE_FromParent,
    };
