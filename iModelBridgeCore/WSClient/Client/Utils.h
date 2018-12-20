/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Utils.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <sstream>
struct StringUtils
    {
    //! Joins the values in the range using the specified delimiter.
    template<typename _InputIterator, typename _DelType>
    static Utf8String Join(_InputIterator first, _InputIterator last, _DelType delimeter)
        {
        std::stringstream sstr;
        if (first != last)
            {
            sstr << *(first++);
            for (; first != last; ++first)
                {
                sstr << delimeter;
                sstr << *first;
                }
            }
        return sstr.str().c_str();
        }
    };
