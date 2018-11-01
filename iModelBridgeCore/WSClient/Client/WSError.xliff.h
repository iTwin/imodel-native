/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSError.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/L10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START (WSErrorL10N, WSError)
    L10N_STRING(MESSAGE_ServerNotSupported)         // =="Server is not supported. Please check server address or contact your server administrator."==
    L10N_STRING(MESSAGE_FunctionalityNotSupported)  // =="Requested functionality is not supported for this server version."==
    L10N_STRING(MESSAGE_UnknownError)               // =="Unknown error. Please contact your server administrator."==
    L10N_STRING(MESSAGE_FileNotFound)               // =="File not found on server. Please contact your server administrator"==
    L10N_STRING(MESSAGE_ClassNotFound)              // =="Class not found on server. Please contact your server administrator"==
    L10N_STRING(MESSAGE_InstanceNotFound)           // =="Item not found on server. Please contact your server administrator"==
BENTLEY_TRANSLATABLE_STRINGS_END

#define WSErrorLocalizedString(K) WSErrorL10N::GetString(WSErrorL10N::K())
