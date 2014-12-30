/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeJStringUtilities.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeJStringUtilities.h>

/*--------------------------------------------------------------------------------------+
| See "modified UTF-8" pitfalls here: http://en.wikipedia.org/wiki/Java_Native_Interface
+--------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeJStringUtilities::InitWStringFromJString
(
JNIEnv*         env,
WStringR        wstring,
jstring         javaString
)
    {
    if (NULL != javaString)
        {
        const jchar* jcharString = env->GetStringChars (javaString, 0);
        const jsize stringLength = env->GetStringLength (javaString);

        BeStringUtilities::Utf16ToWChar (wstring, jcharString, stringLength);
        env->ReleaseStringChars (javaString, jcharString);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeJStringUtilities::InitUtf8StringFromJString
(
JNIEnv*         env,
Utf8StringR     utf8String,
jstring         javaString
)
    {
    // avoid "modified UTF-8 issues" by going through UTF-16
    if (NULL != javaString)
        {
        const jchar* jcharString = env->GetStringChars (javaString, 0);
        const int stringLength = env->GetStringLength (javaString);

        BentleyStatus status = BeStringUtilities::Utf16ToUtf8 (utf8String, jcharString, stringLength);
        BeAssert (SUCCESS == status);
    
        env->ReleaseStringChars (javaString, jcharString);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    06/11
+---------------+---------------+---------------+---------------+---------------+------*/
jstring         BeJStringUtilities::JStringFromWString
(
JNIEnv*         env,
WStringCR       wstring
)
    {
    Utf16Buffer bufferUtf16;

    if (SUCCESS == BeStringUtilities::WCharToUtf16 (bufferUtf16, wstring.c_str ()))
        return env->NewString ((const jchar*) bufferUtf16.data (), bufferUtf16.size () - 1);  // size includes the null terminator

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/11
+---------------+---------------+---------------+---------------+---------------+------*/
jstring         BeJStringUtilities::JStringFromWChar
(
JNIEnv*         env,
WCharCP         buffer
)
    {
    Utf16Buffer bufferUtf16;

    if (SUCCESS == BeStringUtilities::WCharToUtf16 (bufferUtf16, buffer))
        return env->NewString ((const jchar*) bufferUtf16.data (), bufferUtf16.size () - 1);  // size includes the null terminator

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
jstring         BeJStringUtilities::JStringFromUtf8
(
JNIEnv*         env,
Utf8CP          buffer
)
    {
    // avoid "modified UTF-8 issues" by going through WChar conversion first
    WString wstring;
    wstring.AssignUtf8 (buffer);

    return JStringFromWString (env, wstring);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
jstring         BeJStringUtilities::JStringFromUtf8String
(
JNIEnv*         env,
Utf8StringCR    utf8String
)
    {
    return JStringFromUtf8 (env, utf8String.c_str ());
    }
