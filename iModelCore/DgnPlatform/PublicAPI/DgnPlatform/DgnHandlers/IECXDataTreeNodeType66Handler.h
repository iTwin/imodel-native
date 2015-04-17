/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IECXDataTreeNodeType66Handler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

DGNPLATFORM_TYPEDEFS (IECXDataTreeNodeType66Handler)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* This interface defines a ECXDataTreeNodeHandlerType66 object. There can only be one
* of these active at a time. The object should be implemented, created and registered
* from Bentley.in ECXDataTreeMgr. This handler serves as a type 66 sub-type handler
* for data prior to Beijing and as an Element handler for data created from Beijing forward.
* In DgnElement/Type66Handlers.cpp the function mdlSystem_setECXDataTreeNodeType66Handler
* uses this interface to register the type 66 sub-type handler.
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IECXDataTreeNodeType66Handler : Handler, ITransactionHandler
{
virtual int     GetApplicationSignature () = 0;
virtual void    _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override = 0;

virtual void    _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override = 0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

