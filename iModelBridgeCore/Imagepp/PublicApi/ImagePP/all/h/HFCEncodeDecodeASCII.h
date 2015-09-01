//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCEncodeDecodeASCII.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCEncodeDecodeASCII
//---------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HFCEncodeDecodeASCII
    {
public:

    IMAGEPP_EXPORT static void EscapeToASCII(string&   pio_rString,
                                     bool     pi_UTF8String = false);
    IMAGEPP_EXPORT static void EscapeToASCII(WString&  pio_rString,
                                     bool     pi_UTF8String = false);

    IMAGEPP_EXPORT static void ASCIIToEscape(string&   pio_rString,
                                     bool     pi_UTF8String = false);
    IMAGEPP_EXPORT static void ASCIIToEscape(WString&  pio_rString,
                                     bool     pi_UTF8String = false);

    IMAGEPP_EXPORT static void ASCIIToEscapeComponent(string&   pio_rString,
                                              bool     pi_UTF8String = false);
    IMAGEPP_EXPORT static void ASCIIToEscapeComponent(WString&  pio_rString,
                                              bool     pi_UTF8String = false);

    };


END_IMAGEPP_NAMESPACE