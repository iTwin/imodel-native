/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewAttachment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ViewAttachment.h>

#define PROP_ViewId         "ViewId"
#define PROP_Scale          "Scale"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(ViewAttachmentHandler);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnInsert()
    {
    return GetViewId().IsValid() ? T_Super::_OnInsert() : DgnDbStatus::ViewNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnUpdate(DgnElementCR el)
    {
    return GetViewId().IsValid() ? T_Super::_OnUpdate(el) : DgnDbStatus::ViewNotFound;
    }
