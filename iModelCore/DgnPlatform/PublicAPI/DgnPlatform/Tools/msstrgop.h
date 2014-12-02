/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/msstrgop.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <stddef.h>

extern "C" {

/*======================================================================+
|                                                                       |
|   Functions                                                           |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
* Increment number value in input string
*
* @param    outStringP              OUT output string with incremented value
* @param    inStringP               IN input string (with embedded number)
* @return   SUCCESS if string was incremented
* @Group    "String Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt   mdlString_incrementNumber
(
WChar *outStringP,
WChar *inStringP,
int     increment
);

/*---------------------------------------------------------------------------------**//**
* Check if string passes specified regular expression
*
* @param    string                  IN string to be matched
* @param    regex                   IN regular expression to match
* @param    start                   IN start character position from which to start the match
* @param    end                     IN end character position at which to end the match
* @return   SUCCESS if string passes specified regular expression
* @Group    "String Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt   mdlString_matchREExtended2
(
WCharCP     lineStart,
WCharCP     line,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end,
int*        stopState
);

/*---------------------------------------------------------------------------------**//**
* Check if string passes specified regular expression
*
* @param    string                  IN string to be matched
* @param    regex                   IN regular expression to match
* @param    start                   IN start character position from which to start the match
* @param    end                     IN end character position at which to end the match
* @return   SUCCESS if string passes specified regular expression
* @Group    "String Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt   mdlString_matchRE
(
WCharCP     string,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end
);

/*---------------------------------------------------------------------------------**//**
* Check if string passes specified regular expression
*
* @param    string                  IN string to be matched
* @param    regex                   IN regular expression to match
* @param    start                   IN start character position from which to start the match
* @param    end                     IN end character position at which to end the match
* @return   SUCCESS if string passes specified regular expression
* @Group    "String Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt   mdlString_matchREExtended
(
WCharCP     string,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end,
int*        stopState
);

/*---------------------------------------------------------------------------------**//**
* Convert ascii string to a radix 50 string
*
* @param    length                  IN length of input string to be converted
* @param    input                   IN ascii string to be converted
* @param    output                  OUT buffer to contain radix 50 string
* @Group    "Conversions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void        mdlCnv_fromAsciiToR50
(
size_t   length,
char    *input, // WIP_CHAR_OK
void    *output
);

/*---------------------------------------------------------------------------------**//**
* Convert radix 50 string to an ascii string
*
* @param    length                  IN length of output string (excluding terminator). Should be multiple of 3
* @param    input                   IN radix 50 string to be converted
* @param    output                  OUT buffer to contain ascii string
* @Group    "Conversions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void        mdlCnv_fromR50ToAscii
(
size_t          length,
const void     *input,
char           *output // WIP_CHAR_OK
);

/*---------------------------------------------------------------------------------**//**
* Convert wide char string to radix 50 string
*
* @param    length                  IN length of input string to be converted
* @param    input                   IN wide char string to be converted
* @param    output                  OUT buffer to contain radix 50 string
* @Group    "Conversions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void        mdlCnv_fromWideToR50
(
size_t           length,
const WChar   *inputP,
void            *output
);

/*---------------------------------------------------------------------------------**//**
* Convert radix 50 string to an ascii string
*
* @param    length                  IN maximum length of output string (excluding terminator). Should be multiple of 3
* @param    input                   IN radix 50 string to be converted
* @param    output                  OUT buffer to contain wide char string
* @Group    "Conversions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void        mdlCnv_fromR50ToWide
(
size_t          length,
const void     *inputP,
WChar         *output
);

}
