/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnItem.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#ifdef WIP_ITEM_HANDLER
//=======================================================================================
//! Base class for ElementItem handlers (which follow the singleton pattern).
//! Many handler methods take the ElementItemKey type so as not to require the loading
//! of a ElementItem into memory.
//! @note Domain developers are expected to subclass ElementItemHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementItemHandler : DgnDomain::Handler
{
    HANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ElementItem, ElementItemHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

//__PUBLISH_SECTION_END__
    friend struct ElementGeomTableHandler;

//__PUBLISH_SECTION_START__
public:

    virtual ElementItemHandlerP _ToElementItemHandler() override {return this;}     //!< dynamic_cast this Handler to a ElementItemHandler

    DGNPLATFORM_EXPORT DgnClassId GetItemClassId(DgnDbR db);

    DGNPLATFORM_EXPORT ElementItemHandler* GetItemHandler(DgnDbR db, DgnClassId itemClassId);
};
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
