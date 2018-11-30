/*--------------------------------------------------------------------------------------+
|
|     $Source: IPointCloudFileEdit.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* Published methods that wrap around the virtual methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPointCloudFileEdit::SetMetaTag(WStringCR tagName, WStringCR tagValue) { return _SetMetaTag(tagName, tagValue); }
StatusInt       IPointCloudFileEdit::WriteMetaTags() { return _WriteMetaTags(); }

