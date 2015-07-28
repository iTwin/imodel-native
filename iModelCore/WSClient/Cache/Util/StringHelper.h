/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/StringHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <Bentley/WString.h>
#include <string>
#include <sstream>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringHelper
    {
    //! Join values to list seperated by delimeter values. Delimeter is usually Utf8CP string or Utf8Char.
    template<typename _InputIterator, typename _Delim> static  Utf8String Join(_InputIterator first, _InputIterator last, _Delim delimeter)
        {
        if (first == last)
            return "";

        std::stringstream sstr;

        sstr << *first;
        ++first;

        for (; first != last; ++first)
            {
            sstr << delimeter;
            sstr << *first;
            }

        return Utf8String(sstr.str().c_str());
        }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
