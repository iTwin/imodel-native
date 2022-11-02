/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* Base class to make a class non-copyable.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class NonCopyableClass
{
private:
    NonCopyableClass(NonCopyableClass const&) = delete;
    NonCopyableClass& operator=(NonCopyableClass const&) = delete;

protected:
    NonCopyableClass() = default;
    ~NonCopyableClass() = default;
};

END_BENTLEY_NAMESPACE
