/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/NonCopyableClass.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
        NonCopyableClass (NonCopyableClass const&);
        NonCopyableClass& operator= (NonCopyableClass const&);

    protected:
        NonCopyableClass() {}
        ~NonCopyableClass() {}
    };

END_BENTLEY_NAMESPACE
