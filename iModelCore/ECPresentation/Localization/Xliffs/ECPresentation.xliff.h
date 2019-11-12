/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    L10N_STRING(LABEL_Category_General)                  // =="General"==
    L10N_STRING(LABEL_Category_Favorite)                 // =="Favorite"==

    L10N_STRING(ERROR_General_Unknown)                   // =="Unknown error"==
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_ECPRESENTATION_NAMESPACE
