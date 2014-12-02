/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnFileIODeprecated.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_API_NAMESPACE

//! check the "deleted" flag on an element
inline bool elementRef_isDeleted (ElementRefP elemRef) {return  (NULL == elemRef) ? false : elemRef->IsDeleted();}

//__PUBLISH_SECTION_END__

END_BENTLEY_API_NAMESPACE
