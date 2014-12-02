/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/AlongTextDependency.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/TextBlock/TextAPICommon.h>
#include <DgnPlatform/DgnHandlers/TextHandlers.h>
#if defined (NEEDS_WORK_DGNITEM)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    JohnFerguson      09/03
//=======================================================================================
struct AlongTextDependency : AlongTextLinkageData
{
    public: DGNPLATFORM_EXPORT  static  bool            IsRootValid                 (DisplayPath const & path);
    public:                             BentleyStatus   CreateDisplayPath           (DisplayPath& path, DgnModelR homeCache) const;
    public:                             bool            IsValid                     (DgnModelR homeCache) const;
    public:                             BentleyStatus   DoesElementMatchDependency  (ElementHandleCR, bool& matches) const;

}; // AlongTextDependency

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif
