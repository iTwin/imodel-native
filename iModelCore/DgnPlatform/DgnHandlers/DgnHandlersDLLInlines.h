/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlersDLLInlines.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
/* This file exists to allow non-virtual-interface methods to be inlined in
/* the production builds of Ustation.dll, but non-inlined for debug builds (otherwise you can't step into them).
+--------------------------------------------------------------------------------------*/

#if !defined (__DGNPLATFORM_BUILD__)
    #error This file is only valid inside DgnPlatform.dll
#endif

DG_INLINE WString   IEcPropertyHandler::GetEcPropertiesClassName (ElementHandleCR eh) {return _GetEcPropertiesClassName (eh);}
DG_INLINE StatusInt IEcPropertyHandler::GetEcProperties (T_EcCategories& t, ElementHandleCR eh) {return _GetEcProperties (t, eh);}
DG_INLINE bool      IEcPropertyHandler::IsPropertyReadOnly (ElementHandleCR eh, uint32_t propId, size_t arrayIndex) {return _IsPropertyReadOnly (eh, propId, arrayIndex);}
DG_INLINE bool      IEcPropertyHandler::IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName) {return _IsNullProperty (enabler, className, propName);}
DG_INLINE IsNullReturnType IEcPropertyHandler::IsNullProperty (ElementHandleCR eh, uint32_t propId, size_t arrayIndex) {return _IsNullProperty (eh, propId, arrayIndex);}

