/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/DependencyManagerInternal.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_API_NAMESPACE
DGNPLATFORM_EXPORT bool     dependency_setPlaceholderFieldParentDependencies (EditElementHandleR eh);
DGNPLATFORM_EXPORT DgnModelP dependency_getNonDgnModelIfNecessary (ElementRefP oldRootRef, DgnModelP oldRootModel);
END_BENTLEY_API_NAMESPACE
