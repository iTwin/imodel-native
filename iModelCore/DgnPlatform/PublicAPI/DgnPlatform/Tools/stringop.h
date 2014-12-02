/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/stringop.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "msstrgop.h"

DGNPLATFORM_EXPORT bool    isSpaceOrTab
(
int     c
);

DGNPLATFORM_EXPORT int     wstrcomp
(
WCharCP     str1,
WCharCP     str2
);


DGNPLATFORM_EXPORT WCharP  wcsnxtchr
(
WCharP   str,
WChar    chr
);


DGNPLATFORM_EXPORT WCharCP  findSubstring
(
WCharCP masterStringP,
WCharCP searchStringP
);

DGNPLATFORM_EXPORT void    wstripTrailingZeros
(
WChar *str
);

DGNPLATFORM_EXPORT void    wstripLeadingZero
(
WChar *str
);

DGNPLATFORM_EXPORT void strutil_wstrpwspc
(
WChar     *chrstr
);

DGNPLATFORM_EXPORT void    wstripSpace
(
WChar *string
);

DGNPLATFORM_EXPORT WChar *wskipBlankTab
(
WChar *stringP
);

DGNPLATFORM_EXPORT WCharP wskipSpace
(
WCharCP string
);

//DGNPLATFORM_EXPORT void    wtrim  removed in graphite - unused
//(
//WChar *field
//);

DGNPLATFORM_EXPORT void    wstrpcpy
(
WChar *result,
WChar **p,
WChar *end
);

DGNPLATFORM_EXPORT WCharP wend_string
(
WCharCP input
);


DGNPLATFORM_EXPORT WChar *wstpblk
(
WCharCP p
);

DGNPLATFORM_EXPORT int     mdlwcscmpi
(
const WChar     *p1,
const WChar     *p2
);

DGNPLATFORM_EXPORT WChar*        mdlwcsupr
(
WChar     *wideStringP
);

DGNPLATFORM_EXPORT WChar*        mdlwcslwr
(
WChar     *wideStringP
);

DGNPLATFORM_EXPORT void  asciiToR50
(
size_t           length,  /* => length of input string to be converted */
CharCP           input,   /* => string to be converted                 */
void*            outputP  /* <= output array                           */
);


DGNPLATFORM_EXPORT void  R50ToWide
(
size_t          length,         /* => maximum length of output string
                                    (excluding terminator). Should be multiple of 3 */
const void*     inputP,         /* => radix 50 to be converted */
WCharP          output         /* <= output string */
);



