/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/CodePages.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                                    BentleySystems   07/2007
//=======================================================================================
enum class LangCodePage
    {
    Unknown                    =  -1,     //!< unknown code page
    None                       =   0, 
    OEM_US                     =   437,    //!< OEM - United States
    Transparent_ASMO           =   720,    //!< Arabic - Transparent ASMO
    OEM_Greek                  =   737,    //!< OEM - Greek (formerly 437G)
    OEM_Baltic                 =   775,    //!< OEM - Baltic
    OEM_Multilingual           =   850,    //!< OEM - Multilingual Latin I
    OEM_LatinII                =   852,    //!< OEM - Latin II
    OEM_Cryllic                =   855,    //!< OEM - Cyrillic (primarily Russian)
    OEM_Turkish                =   857,    //!< OEM - Turkish
    OEM_LatinI                 =   858,    //!< OEM - Multlingual Latin I + Euro symbol
    OEM_Hebrew                 =   862,    //!< OEM - Hebrew
    OEM_Russian                =   866,    //!< OEM - Russian
    OEM_Thai                   =   874,    //!< ANSI/OEM - Thai (same as 28605, ISO 8859-15)
    Japanese                   =   932,    //!< ANSI/OEM - Japanese, Shift-JIS
    Simplified_Chinese         =   936,    //!< ANSI/OEM - Simplified Chinese (PRC, Singapore)
    Korean                     =   949,    //!< ANSI/OEM - Korean (Unified Hangeul Code)
    Traditional_Chinese        =   950,    //!< ANSI/OEM - Traditional Chinese (Taiwan; Hong Kong SAR, PRC)
    Unicode                    =   1200,   //!< UNICODE
    UNICODE_UCS2_Little_Endian =   1200,   //!< Unicode UCS-2 Little-Endian (BMP of ISO 10646)
    UNICODE_UCS2_Big_Endian    =   1201,   //!< Unicode UCS-2 Big-Endian
    Central_European           =   1250,   //!< ANSI - Central European
    Cyrillic                   =   1251,   //!< ANSI - Cyrillic
    LatinI                     =   1252,   //!< ANSI - Latin I
    Greek                      =   1253,   //!< ANSI - Greek
    Turkish                    =   1254,   //!< ANSI - Turkish
    Hebrew                     =   1255,   //!< ANSI - Hebrew
    Arabic                     =   1256,   //!< ANSI - Arabic
    Baltic                     =   1257,   //!< ANSI - Baltic
    Vietnamese                 =   1258,   //!< ANSI/OEM - Vietnamese
    Johab                      =   1361,   //!< Korean (Johab)
    ISO_8859_1                 =   28591,  //!< ISO 8859-1 Latin I
    ISO_8859_2                 =   28592,  //!< ISO 8859-2 Central Europe
    ISO_8859_3                 =   28593,  //!< ISO 8859-3 Latin 3
    ISO_8859_4                 =   28594,  //!< ISO 8859-4 Baltic
    ISO_8859_5                 =   28595,  //!< ISO 8859-5 Cyrillic
    ISO_8859_6                 =   28596,  //!< ISO 8859-6 Arabic
    ISO_8859_7                 =   28597,  //!< ISO 8859-7 Greek
    ISO_8859_8                 =   28598,  //!< ISO 8859-8 Hebrew
    ISO_8859_9                 =   28599,  //!< ISO 8859-9 Latin 5
    ISO_8859_15                =   28605,  //!< ISO 8859-15 Latin 9
    ISCII_UNICODE_UTF_7        =   65000,  //!< Unicode UTF-7
    ISCII_UNICODE_UTF_8        =   65001,  //!< Unicode UTF-8

    }; // LangCodePage

END_BENTLEY_NAMESPACE

//__PUBLISH_SECTION_END__
