/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IModifyElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* Interface for modifying elements. Can be supplied to ElementAgenda::ModifyAgenda to
* apply an operation to each member of the agenda.
\ingroup MstnTool
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModifyElement
    {
    //! Call to determine whether original element is being modified or copied.
    virtual bool _WantMakeCopy () const = 0;
    //! Call to set copy option.
    virtual void _SetWantMakeCopy (bool) = 0;
#if defined (NOT_NOW_WIP_REMOVE_ELEMENTHANDLE)
    //! Called to perform element modification and update model. Implementation should call _OnElementModify
    //! and then add, replace, or delete the element based on a successful return status.
    //! @return SUCCESS if element was updated.
    virtual StatusInt _DoOperationForModify (EditElementHandleR) = 0;
    //! Called to apply element modification. This method should not update the model when returning SUCCESS.
    //! @return SUCCESS if modify was ok and element should be replaced, deleted, or added/copied.
    virtual StatusInt _OnElementModify (EditElementHandleR) = 0;
#endif
    //! Called to initialize abort processing for ElementAgenda::ModifyAgenda.
    virtual void _ResetStop () = 0;
    //! Called to abort processing for ElementAgenda::ModifyAgenda.
    virtual bool _CheckStop () = 0;
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
