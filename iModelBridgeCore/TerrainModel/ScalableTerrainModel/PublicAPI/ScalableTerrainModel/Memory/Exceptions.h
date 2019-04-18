/*--------------------------------------------------------------------------------------+
|    $RCSfile: Exceptions.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/26 18:47:31 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Memory/Definitions.h>
#include <ScalableTerrainModel/Foundations/Exception.h>

BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE

/*
 * Packet exceptions
 */
struct BadPacketCastException : public ExceptionMixinBase<BadPacketCastException>
    {
    explicit        BadPacketCastException                     ()   :   super_class(L"Bad packet cast error!") {}
    };

struct UndefinedPacketCastException : public ExceptionMixinBase<UndefinedPacketCastException>
    {
    explicit        UndefinedPacketCastException               ()   :   super_class(L"Undefined packet cast error!") {}
    };

struct ReadOnlyPacketException : public ExceptionMixinBase<ReadOnlyPacketException>
    {
    explicit        ReadOnlyPacketException                    ()   :   super_class(L"Trying to edit a read only packet!") {}
    };


END_BENTLEY_MRDTM_MEMORY_NAMESPACE