//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryModuleCodes.h $
//:>
//:>  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCErrorCodeBuilder.h"
#include "HFCErrorCodeHandler.h"
#include "HFCErrorCodeMacros.h"

// Note that the builder and handlers will be in the same class.
// We will refer to them as builders even though they are also handlers.


// HFC_DEVICE_EXCEPTION
HFC_ERRORCODE_START_DECLARATION_CLASS2(HFCMemoryExceptionBuilder, HFCErrorCodeBuilder, HFCErrorCodeHandler)
HFC_ERRORCODE_DECLARE_BUILDER_EXCEPTION
HFC_ERRORCODE_DECLARE_HANDLER
HFC_ERRORCODE_END_DECLARATION

// HFC_NO_DISK_SPACE_LEFT_EXCEPTION
HFC_ERRORCODE_START_DECLARATION_CLASS(HFCOutOfMemoryExceptionBuilder, HFCMemoryExceptionBuilder)
HFC_ERRORCODE_DECLARE_BUILDER_EXCEPTION
HFC_ERRORCODE_DECLARE_HANDLER
HFC_ERRORCODE_END_DECLARATION

