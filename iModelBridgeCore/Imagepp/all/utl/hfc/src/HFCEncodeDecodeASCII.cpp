//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCEncodeDecodeASCII.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>

//-----------------------------------------------------------------------------
// Static method...
//-----------------------------------------------------------------------------
void HFCEncodeDecodeASCII::EscapeToASCII(string& pio_rString, bool pi_UTF8String)
    {
    // Replace each "%xx" block by the character it represent
    string::size_type Pos = 0;
    size_t            StrLen = pio_rString.size();
    string::size_type EncodedPos;
    string EncodedChar;
    while ((Pos = pio_rString.find('%', Pos)) != string::npos && StrLen > Pos + 2)
        {
        EncodedPos = Pos;
        EncodedChar.erase();
        // Decode all consecutive chars before to convert it to Widechar
        while(pio_rString[Pos] == '%' && StrLen > Pos + 2)
            {
            // The value in hex
            char Char = (pio_rString[Pos + 1] >= 'A' ? ((pio_rString[Pos + 1] & 0xDF) - 'A') + 10 : (pio_rString[Pos + 1] - '0'));
            Char *= 16;
            Char += (pio_rString[Pos + 2] >= 'A' ? ((pio_rString[Pos + 2] & 0xDF) - 'A') + 10 : (pio_rString[Pos + 2] - '0'));

            EncodedChar += Char;
            // skip the %xx
            Pos += 3;
            }

        if (pi_UTF8String)
            {
            WString wideString;
            AString MBString;
            LangCodePage codePage;
            BeStringUtilities::GetCurrentCodePage(codePage);
            BeStringUtilities::Utf8ToWChar(wideString,EncodedChar.c_str());
            BeStringUtilities::WCharToLocaleChar(MBString, codePage, wideString.c_str());

            pio_rString.replace(EncodedPos, EncodedChar.size()*3, MBString.c_str(), MBString.length());
            Pos = EncodedPos + MBString.length();
            }
        else
            {
            // replace the current "%xx" by that character
            pio_rString.replace(EncodedPos, EncodedChar.size()*3, EncodedChar.c_str(), EncodedChar.size());
            Pos = EncodedPos + EncodedChar.size();
            }
        }

#if 0
    // Replace each "%xx" block by the character it represent
    string::size_type Pos = 0;
    string::size_type EncodedPos;
    while ((Pos = pio_rString.find('%', Pos)) != string::npos && pio_rString.size() > Pos + 2)
        {
        EncodedPos = Pos;
        // The value in hex
        char Char = (pio_rString[Pos + 1] >= 'A' ? ((pio_rString[Pos + 1] & 0xDF) - 'A') + 10 : (pio_rString[Pos + 1] - '0'));
        Char *= 16;
        Char += (pio_rString[Pos + 2] >= 'A' ? ((pio_rString[Pos + 2] & 0xDF) - 'A') + 10 : (pio_rString[Pos + 2] - '0'));


        // replace the current "%xx" by that character
        pio_rString.replace(EncodedPos, 3, 1, Char);

        // start the next search one character to the right in case we just inserted '%'
        Pos = EncodedPos + 1;
        }
#endif
    }

//-----------------------------------------------------------------------------
// Static method...
//-----------------------------------------------------------------------------
void HFCEncodeDecodeASCII::EscapeToASCII(WString& pio_rString, bool pi_UTF8String)
    {
    // Replace each "%xx" block by the character it represent
    WString::size_type Pos = 0;
    size_t             StrLen = pio_rString.size();
    WString::size_type EncodedPos;
    string EncodedChar;
    while ((Pos = pio_rString.find(L'%', Pos)) != WString::npos && StrLen > Pos + 2)
        {
        EncodedPos = Pos;
        EncodedChar.erase();
        // Decode all consecutive chars before to convert it to Widechar
        while(pio_rString[Pos] == L'%' && StrLen > Pos + 2)
            {
            // The value in hex
            char Char = (char)((pio_rString[Pos + 1] >= L'A' ? ((pio_rString[Pos + 1] & 0x00DF) - L'A') + 10 : (pio_rString[Pos + 1] - L'0')));
            Char *= 16;
            Char += (char)((pio_rString[Pos + 2] >= L'A' ? ((pio_rString[Pos + 2] & 0x00DF) - L'A') + 10 : (pio_rString[Pos + 2] - L'0')));

            EncodedChar += Char;
            // skip the %xx
            Pos += 3;
            }

        size_t NbWideChar;
        WString WideCharStr;

        if (pi_UTF8String)
            BeStringUtilities::Utf8ToWChar( WideCharStr,EncodedChar.c_str());
        else
            BeStringUtilities::CurrentLocaleCharToWChar( WideCharStr,EncodedChar.c_str());

        // replace the current "%xx" by that character
        NbWideChar =  WideCharStr.length();// we don't want to copy the null char
        pio_rString.replace(EncodedPos, EncodedChar.size()*3, WideCharStr.c_str(), NbWideChar); 
        Pos = EncodedPos + NbWideChar;
        }
    }


void HFCEncodeDecodeASCII::ASCIIToEscape(WString& pio_rString, bool pi_UTF8String)
    {
    // Encode all the "," & "%" in the entry name
    WString::size_type Pos = 0;
    while ((Pos = pio_rString.find_first_of(L"%, ", Pos)) != WString::npos)
        {
        // Build the escape sequence
        WChar Temp[5]; // % + 2 hex + \r\n = 5 characters
        BeStringUtilities::Snwprintf(Temp, L"%%%2.2X", pio_rString[Pos]);

        // Replace the character with its escape sequence
        pio_rString.replace(Pos, 1, Temp);

        // Move the position one character to the right so that we
        // do not get the "%" that was just inserted.
        Pos++;
        }
    }

void HFCEncodeDecodeASCII::ASCIIToEscape(string& pio_rString, bool pi_UTF8String)
    {
    // Encode all the "," & "%" in the entry name
    string::size_type Pos = 0;
    while ((Pos = pio_rString.find_first_of("%, ", Pos)) != string::npos)
        {
        // Build the escape sequence
        char Temp[5]; // % + 2 hex + \r\n = 5 characters
        sprintf(Temp, "%%%2.2X", pio_rString[Pos]);

        // Replace the character with its escape sequence
        pio_rString.replace(Pos, 1, Temp);

        // Move the position one character to the right so that we
        // do not get the "%" that was just inserted.
        Pos++;
        }
    }

void HFCEncodeDecodeASCII::ASCIIToEscapeComponent(WString& pio_rString, bool pi_UTF8String)
    {
    // Encode all the "," & "%" in the entry name
    WString::size_type Pos = 0;
    while ((Pos = pio_rString.find_first_of(L"%, ;:/=@&?+", Pos)) != WString::npos)
        {
        // Build the escape sequence
        WChar Temp[5]; // % + 2 hex + \r\n = 5 characters
        BeStringUtilities::Snwprintf(Temp, L"%%%2.2X", pio_rString[Pos]);

        // Replace the character with its escape sequence
        pio_rString.replace(Pos, 1, Temp);

        // Move the position one character to the right so that we
        // do not get the "%" that was just inserted.
        Pos++;
        }
    }

void HFCEncodeDecodeASCII::ASCIIToEscapeComponent(string& pio_rString, bool pi_UTF8String)
    {
    // Encode all the "," & "%" in the entry name
    string::size_type Pos = 0;
    while ((Pos = pio_rString.find_first_of("%, ;:/=@&?+", Pos)) != string::npos)
        {
        // Build the escape sequence
        char Temp[5]; // % + 2 hex + \r\n = 5 characters
        sprintf(Temp, "%%%2.2X", pio_rString[Pos]);

        // Replace the character with its escape sequence
        pio_rString.replace(Pos, 1, Temp);

        // Move the position one character to the right so that we
        // do not get the "%" that was just inserted.
        Pos++;
        }
    }