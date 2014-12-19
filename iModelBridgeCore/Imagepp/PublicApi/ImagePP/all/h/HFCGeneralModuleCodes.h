//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCGeneralModuleCodes.h $
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

// HFCException
HFC_ERRORCODE_START_DECLARATION_CLASS2(HFCExceptionBuilder, HFCErrorCodeBuilder, HFCErrorCodeHandler)
HFC_ERRORCODE_DECLARE_BUILDER_EXCEPTION
HFC_ERRORCODE_DECLARE_HANDLER
HFC_ERRORCODE_END_DECLARATION

// HFCObjectNotInFactoryException
HFC_ERRORCODE_START_DECLARATION_CLASS2(HFCObjectNotInFactoryExceptionBuilder, HFCErrorCodeBuilder, HFCErrorCodeHandler)
HFC_ERRORCODE_DECLARE_BUILDER_EXCEPTION
HFC_ERRORCODE_DECLARE_HANDLER
HFC_ERRORCODE_END_DECLARATION


