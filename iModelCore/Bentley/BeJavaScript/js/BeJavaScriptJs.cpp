/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeJavaScript/BeJavaScript.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
Utf8CP BeJsContext::_GetBeJavaScriptJsSource()
    {
    static const char source[] = ___BEJAVASCRIPTJS__;
    return source;
    }
