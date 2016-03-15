/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/NetworkIndicator.mm $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#if !defined (__APPLE__)
    #error This file is only intended for iOS compilands!
#endif
#include "NetworkIndicator.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void NetworkIndicator::SetVisible (bool visible)
    {
    [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:visible];
    }