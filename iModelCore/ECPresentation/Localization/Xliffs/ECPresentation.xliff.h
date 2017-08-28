/*--------------------------------------------------------------------------------------+
|
|     $Source: Localization/Xliffs/ECPresentation.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// Localizable strings for the ECPresentation layer.
// @bsiclass                                      
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(ECPresentationL10N, ECPresentation)
    L10N_STRING(LABEL_General_DisplayLabel)              // =="Label"==

    L10N_STRING(LABEL_Category_Miscellaneous)            // =="Miscellaneous"==
    L10N_STRING(LABEL_Category_Favorite)                 // =="Favorite"==

    L10N_STRING(ERROR_General_Unknown)                   // =="Unknown error"==
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_ECPRESENTATION_NAMESPACE
