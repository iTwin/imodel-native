/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Memory/Definitions.h>
#include <ScalableMesh/Foundations/Exception.h>

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE

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


END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
