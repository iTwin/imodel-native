/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* Base class to make a class non-copyable.
* @bsiclass                                                     11/09
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
