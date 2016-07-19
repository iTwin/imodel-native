/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/NetworkIndicator.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

//=======================================================================================
//  @bsiclass                                               Beneditas.Lipnickas 02/2014
//=======================================================================================
struct NetworkIndicator
    {
    BEHTTP_EXPORT static void SetVisible(bool visible);
    };

END_BENTLEY_HTTP_NAMESPACE
