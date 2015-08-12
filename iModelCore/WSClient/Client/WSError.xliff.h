/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSError.xliff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <MobileDgn/MobileDgnL10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
MOBILEDGN_TRANSLATABLE_STRINGS_START (WSErrorL10N, WSError)
    L10N_STRING(MESSAGE_ServerNotSupported)         // =="Server is not supported. Please check server address or contact your server administrator."==
    L10N_STRING(MESSAGE_FunctionalityNotSupported)  // =="Requested functionality is not supported for this server version."==
    L10N_STRING(MESSAGE_UnknownError)               // =="Unknown error. Please contact your server administrator."==
    L10N_STRING(MESSAGE_FileNotFound)               // =="File not found on server. Please contact your server administrator"==
    L10N_STRING(MESSAGE_ClassNotFound)              // =="Class not found on server. Please contact your server administrator"==
    L10N_STRING(MESSAGE_InstanceNotFound)           // =="Item not found on server. Please contact your server administrator"==
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define WSErrorLocalizedString(K) WSErrorL10N::GetString(WSErrorL10N::K())
