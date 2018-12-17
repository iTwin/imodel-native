/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/js/BeJavaScriptJs.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
