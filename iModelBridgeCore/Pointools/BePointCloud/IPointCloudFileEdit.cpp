/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* Published methods that wrap around the virtual methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IPointCloudFileEdit::SetMetaTag(WStringCR tagName, WStringCR tagValue) { return _SetMetaTag(tagName, tagValue); }
StatusInt       IPointCloudFileEdit::WriteMetaTags() { return _WriteMetaTags(); }

