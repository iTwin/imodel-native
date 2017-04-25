/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/PublicAPI/Licensing/Client.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
struct Client
{
public:
    LICENSING_EXPORT Client();

    LICENSING_EXPORT BentleyStatus TestMethod();
};

END_BENTLEY_LICENSING_NAMESPACE
