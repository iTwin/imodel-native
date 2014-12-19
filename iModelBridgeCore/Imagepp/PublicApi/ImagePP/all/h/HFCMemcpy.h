//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemcpy.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMemcpy
//-----------------------------------------------------------------------------
#pragma once


/* THE Present code is applicable only on INTEL (tm) processor */
/* since coded in assembler */
#if defined(_WIN32) && !defined(_M_X64)
#   if defined (__cplusplus)
extern "C" {
#   endif

    _HDLLu void*  HFCMemcpy (void*         po_pDst,
                             const void*   pi_pSrc,
                             size_t        pi_Count);

#   if defined (__cplusplus)
    }
#   endif

#   ifdef memcpy
#       undef memcpy
#   endif
#   define memcpy HFCMemcpy

#else /* _WIN32 */

#   ifdef HFCMemcpy
#       undef HFCMemcpy
#   endif
#include <stdlib.h>
#   define HFCMemcpy memcpy
#endif /* _WIN32 */

