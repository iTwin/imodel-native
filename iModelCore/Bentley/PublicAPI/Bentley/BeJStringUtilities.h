/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#if !defined (ANDROID)
    #error Android-only header!
#endif

#include <jni.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Convert to/from Android/JNI JStrings.
//! @ingroup GROUP_String
//! @ingroup GROUP_DgnClientFxAndroid
//  @bsiclass                                                   Shaun.Sewall     08/2011
//=======================================================================================
struct BeJStringUtilities : public NonCopyableClass
{
public:
    BENTLEYDLL_EXPORT static jstring JStringFromWString (JNIEnv*, WStringCR);
    BENTLEYDLL_EXPORT static jstring JStringFromWChar (JNIEnv*, WCharCP);
    BENTLEYDLL_EXPORT static jstring JStringFromUtf8 (JNIEnv*, Utf8CP);
    BENTLEYDLL_EXPORT static jstring JStringFromUtf8String (JNIEnv*, Utf8StringCR);
    BENTLEYDLL_EXPORT static void    InitWStringFromJString (JNIEnv*, WStringR, jstring);
    BENTLEYDLL_EXPORT static void    InitUtf8StringFromJString (JNIEnv*, Utf8StringR, jstring);
};

END_BENTLEY_NAMESPACE
