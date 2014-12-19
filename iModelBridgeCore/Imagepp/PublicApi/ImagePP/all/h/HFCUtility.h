//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCUtility.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of HFCExceptionMapping.
//----------------------------------------------------------------------------

#pragma once

#include <Bentley/BeFile.h>
#include "HFCErrorCode.h"

void MapHFCFileExceptionFromBeFileStatus(BeFileStatus    pi_Status,
                                         const WString&  pi_rFileName,
                                         ExceptionID     pi_DefaultException,
                                         ExceptionID*    po_pMappedHFCException);

void MapHFCFileExceptionFromGetLastError(uint32_t       pi_LastError,
                                         const WString&  pi_rFileName,
                                         ExceptionID     pi_DefaultException,
                                         ExceptionID*    po_pMappedHFCException);
