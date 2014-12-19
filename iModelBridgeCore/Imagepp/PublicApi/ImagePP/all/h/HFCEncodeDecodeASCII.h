//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCEncodeDecodeASCII.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCEncodeDecodeASCII
//---------------------------------------------------------------------------
#pragma once

class HFCEncodeDecodeASCII
    {
public:

    _HDLLu static void EscapeToASCII(string&   pio_rString,
                                     bool     pi_UTF8String = false);
    _HDLLu static void EscapeToASCII(WString&  pio_rString,
                                     bool     pi_UTF8String = false);

    _HDLLu static void ASCIIToEscape(string&   pio_rString,
                                     bool     pi_UTF8String = false);
    _HDLLu static void ASCIIToEscape(WString&  pio_rString,
                                     bool     pi_UTF8String = false);

    _HDLLu static void ASCIIToEscapeComponent(string&   pio_rString,
                                              bool     pi_UTF8String = false);
    _HDLLu static void ASCIIToEscapeComponent(WString&  pio_rString,
                                              bool     pi_UTF8String = false);

    };


