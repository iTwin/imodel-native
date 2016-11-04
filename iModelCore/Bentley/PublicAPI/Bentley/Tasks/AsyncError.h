/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Tasks/AsyncError.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/WString.h>


BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
//! Class for generic error description
struct EXPORT_VTABLE_ATTRIBUTE AsyncError
    {
    protected:
        Utf8String m_message;
        Utf8String m_description;

    public:
        BENTLEYDLL_EXPORT AsyncError ();
        BENTLEYDLL_EXPORT AsyncError (Utf8StringCR message);
        BENTLEYDLL_EXPORT AsyncError (Utf8StringCR message, Utf8StringCR description);
        BENTLEYDLL_EXPORT virtual ~AsyncError ();

        BENTLEYDLL_EXPORT virtual Utf8String GetMessage () const;
        BENTLEYDLL_EXPORT virtual Utf8String GetDescription () const;
    };

typedef const AsyncError& AsyncErrorCR;
typedef AsyncError& AsyncErrorR;

END_BENTLEY_TASKS_NAMESPACE
