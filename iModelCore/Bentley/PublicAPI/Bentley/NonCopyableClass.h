/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/NonCopyableClass.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
