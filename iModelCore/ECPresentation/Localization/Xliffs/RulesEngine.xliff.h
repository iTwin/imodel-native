/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// Localizable string for the rules driven presentation manager.
// @bsiclass                                      
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(RulesEngineL10N, RulesEngine)
    L10N_STRING(LABEL_General_NotSpecified)                         // =="Not specified"==
    L10N_STRING(LABEL_General_Other)                                // =="Other"==
    L10N_STRING(LABEL_General_Varies)                               // =="Varies"==
    L10N_STRING(LABEL_General_MultipleInstances)                    // =="Multiple items"==

    L10N_STRING(ERROR_ECInstanceChangeResult_CantChangeECInstance)  // =="Can not make changes to the selected instance"==
    L10N_STRING(ERROR_ECInstanceChangeResult_ConnectionReadOnly)    // =="Connection is read-only"==
    L10N_STRING(ERROR_ECInstanceChangeResult_NoInstanceToUpdate)    // =="Could not find the updated instance"==
    L10N_STRING(ERROR_ECInstanceChangeResult_ElementLocked)         // =="Element is locked"==
    L10N_STRING(ERROR_ECInstanceChangeResult_InvalidPropertyValue)  // =="Property value is invalid"==
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_ECPRESENTATION_NAMESPACE
