/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandlerManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

#include    "../DgnPlatform.h"
#include    "ElementHandle.h"
#include    <Bentley/WString.h>
#include    "ITxnManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct     IEnumerateAvailableHandlers;

/*=================================================================================**//**
 @addtogroup RegisteringElementHandlers

An element handler must be a singleton. The same handler will be used to process many different elements.
Therefore, and element handler cannot have any state of its own that is specific to any one element.
<p>
Given this, how can you cache computed information for a specific information? A persistent element is
identified by an ElementRefP. You can attach your application data to an ElementRefP by subclassing from ElementRefAppData
and attaching your object via ElementRef::AddAppData.
Your element handler methods must be careful to check for elements that are non-persistent or are modified.
A non-persistent element will have no ElementRefP. A modified element may have an ElementRefP, but its data may not
correspond to the information that you have cached for it. In those cases, you must re-compute the information.
You can detect these cases by calling ElementHandle::IsPersistent(). If this returns false, then you have a temporary
or modified element; if it returns true, then you have a persistent object that you might have cached.

    <h3> Handler Ownership and Lifetime </h3>

\li A handler DLL owns the handler instances that it registers.
    ElementHandlerManager::RegisterHandler does not take ownership of a handler instance. ElementHandlerManager does
    not delete handler instances.
\li Once registered, a handler must not be unregistered or deleted while elements based on the handler exist.
    This means that the handler must be intact while any dependent DGN file is open. A handler DLL can unload
    if all dependent DGN files are closed. It is up to the handler DLL to check this condition before unloading.
    ElementHandlerManager does not unregister handlers or unload handler DLLs. ElementHandlerManager does not
    keep track of which DGN files depend on which handlers.
\li ElementHandlerManager does not currently notify handlers when DgnPlatform is terminated.

@beginGroup
*/

/*=================================================================================**//**
  Abstract base class for a class that can load the DLL that implements an element Handler or XAttributeHandler.

  DgnPlatform relies on the host implementation of ElementHandlerLoader to locate a DLL for a given element handler ID, load the DLL, and then
  call the appropriate function in the DLL to register the handler object.
+===============+===============+===============+===============+===============+======*/
struct ElementHandlerLoader : DgnHost::IHostObject
{
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

protected:
    void    ReadLodRegistry ();
    void*   GetHandlerDll   (HandlerId const&);
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}

public:

//! Loads the DLL that implements the specified element Handler, if necessary. Invokes the DLL's registerElementHandler function,
//! asking it to register a Handler singleton for this ID.
//! @Remarks FREE_THREADING
//! @param          hid         The ID of the handler that is required.
//! @return non-zero status code if the handler could not be found
DGNPLATFORM_EXPORT virtual StatusInt _LoadElementHandler (ElementHandlerId const& hid);

//! Loads the DLL that implements the specified XAttributeHandler, if necessary. Invokes the DLL's registerXAttributeHandler function,
//! asking it to register a Handler singleton for this ID.
//! @Remarks FREE_THREADING
//! @param          hid         The ID of the handler that is required.
//! @return non-zero status code if the handler could not be found
DGNPLATFORM_EXPORT virtual StatusInt _LoadXAttributeHandler (XAttributeHandlerId const& hid);

//! Handler loading errors that may be reported by DgnPlatform
enum HandlerLoadError
{
    HANDLER_LOADER_ERROR_NotFound,      //!< The handler was not found
};

//! Called when a handler load error occurs
//! @Remarks FREE_THREADING
//! @param          err         The error that has occurred.
//! @param          details     Information about the error, if available.
//! @param          handlerId   If the error concerns a a particular handler (element or XAttribute), handlerId identifies the handler.
virtual void _OnError (HandlerLoadError err, WCharCP details, HandlerId const& handlerId) {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  ElementHandlerManager 
{
public:
    static void OnHostInitialize ();
};

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)
