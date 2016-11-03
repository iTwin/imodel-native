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
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::CheckValid() const
    {
    if (!GetViewId().IsValid())
        return DgnDbStatus::ViewNotFound;
    if (!GetModel()->IsSheetModel())
        return DgnDbStatus::WrongModel;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnInsert()
    {
    DgnDbStatus status = CheckValid();
    if (DgnDbStatus::Success != status)
        return status;
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnUpdate(DgnElementCR el)
    {
    DgnDbStatus status = CheckValid();
    if (DgnDbStatus::Success != status)
        return status;
    return T_Super::_OnUpdate(el);
    }
