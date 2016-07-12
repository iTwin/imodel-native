/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Credentials.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/Credentials.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Credentials::Clear (Utf8StringR string)
    {
    size_t length = string.length ();
    for (size_t i = 0; i < length; i++)
        {
        string[i] = 0;
        }
    }
