/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/NetworkIndicator.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "NetworkIndicator.h"

USING_NAMESPACE_BENTLEY_HTTP

//---------------------------------------------------------------------------------------
// @bsimethod                                   Benediktas.Lipnickas            10/13
//---------------------------------------------------------------------------------------
void NetworkIndicator::SetVisible (bool visible)
    {
    // Do nothing on non-iOS platforms.
    }