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

    L10N_STRING(LABEL_StandardCategory_Extended)         // =="Extended"==
    L10N_STRING(LABEL_StandardCategory_General)          // =="General"==
    L10N_STRING(LABEL_StandardCategory_Geometry)         // =="Geometry"==
    L10N_STRING(LABEL_StandardCategory_Groups)           // =="Groups"==
    L10N_STRING(LABEL_StandardCategory_Material)         // =="Material"==
    L10N_STRING(LABEL_StandardCategory_Miscellaneous)    // =="Miscellaneous"==
    L10N_STRING(LABEL_StandardCategory_RawData)          // =="Raw Data"==
    L10N_STRING(LABEL_StandardCategory_Relationships)    // =="Relationships"==
    L10N_STRING(LABEL_StandardCategory_Favorite)         // =="Favorite"==

    L10N_STRING(ERROR_General_Unknown)                   // =="Unknown error"==
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_ECPRESENTATION_NAMESPACE
