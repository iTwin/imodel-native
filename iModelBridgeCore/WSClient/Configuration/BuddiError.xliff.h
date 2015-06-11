/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/BuddiError.xliff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <MobileDgn/MobileDgnL10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
MOBILEDGN_TRANSLATABLE_STRINGS_START(BuddiErrorL10N, BuddiError)
    {
    MESSAGE_UrlConfigurationError, // =="URL Configutarion error. Please contact your administrator."==
    };
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define BuddiErrorLocalizedString(K) BuddiErrorL10N::GetString (BuddiErrorL10N::K, #K)