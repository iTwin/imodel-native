/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/ValueFormat.r.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------
* This file contains ONLY typedefs and #defines specified by the Bentley Programming Style Guide
* and are used by .cpp, .r, and .mt files.
+--------------------------------------------------------------------------------------*/

//__PUBLISH_SECTION_START__

#include "Bentley.r.h"

BEGIN_BENTLEY_NAMESPACE

enum class PrecisionType
    {
    Decimal             = 0,
    Fractional          = 1,
    Scientific          = 2,
    };

//=======================================================================================
//! Used by various formatters to specify the format of non-integer values.
//! @bsiclass
//=======================================================================================
enum class PrecisionFormat
    {
    DecimalWhole            = 100,      //!< Ex. 30
    Decimal1Place           = 101,      //!< Ex. 30.1
    Decimal2Places          = 102,      //!< Ex. 30.12
    Decimal3Places          = 103,      //!< Ex. 30.123
    Decimal4Places          = 104,      //!< Ex. 30.1234
    Decimal5Places          = 105,      //!< Ex. 30.12345
    Decimal6Places          = 106,      //!< Ex. 30.123456
    Decimal7Places          = 107,      //!< Ex. 30.1234567
    Decimal8Places          = 108,      //!< Ex. 30.12345678
    FractionalWhole         = 200,      //!< Ex. 30
    FractionalHalf          = 201,      //!< Ex. 30 1/2
    FractionalQuarter       = 202,      //!< Ex. 30 1/4
    FractionalEighth        = 203,      //!< Ex. 30 1/8
    FractionalSixteenth     = 204,      //!< Ex. 30 1/16
    Fractional1_Over_32     = 205,      //!< Ex. 30 1/32
    Fractional1_Over_64     = 206,      //!< Ex. 30 1/64
    Fractional1_Over_128    = 207,      //!< Ex. 30 1/128
    Fractional1_Over_256    = 208,      //!< Ex. 30 1/256
    ScientificWhole         = 300,      //!< Ex. 3E+2
    Scientific1Place        = 301,      //!< Ex. 3.1E+2
    Scientific2Places       = 302,      //!< Ex. 3.12E+2
    Scientific3Places       = 303,      //!< Ex. 3.123E+2
    Scientific4Places       = 304,      //!< Ex. 3.1234E+2
    Scientific5Places       = 305,      //!< Ex. 3.12345E+2
    Scientific6Places       = 306,      //!< Ex. 3.123456E+2
    Scientific7Places       = 307,      //!< Ex. 3.1234567E+2
    Scientific8Places       = 308,      //!< Ex. 3.12345678E+2
    };

END_BENTLEY_NAMESPACE

