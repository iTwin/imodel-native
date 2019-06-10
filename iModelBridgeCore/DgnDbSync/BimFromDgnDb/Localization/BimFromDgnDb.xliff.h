/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BimFromDgnDb/BimFromDgnDb.h>
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BIM_FROM_DGNDB_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(BimFromDgnDb, bim_import)
    L10N_STRING(TASK_CREATING_THUMBNAIL)            // =="View: %s"==
    L10N_STRING(TASK_IMPORTING_SCHEMA)              // =="%s"==
    L10N_STRING(TASK_IMPORTING_RELATIONSHIP)        // =="ECRelationship: %s"==
    L10N_STRING(TASK_ELEMENT_GROUPS_MEMBERS)        // =="ElementGroupsMembers"==
    L10N_STRING(STEP_GENERATING_THUMBNAILS)         // =="Generating Thumbnails"==
    L10N_STRING(STEP_IMPORTING_SCHEMAS)             // =="Importing Schemas"==
    L10N_STRING(STEP_IMPORTING_ELEMENTS)            // =="Importing Elements"==
BENTLEY_TRANSLATABLE_STRINGS_END

END_BIM_FROM_DGNDB_NAMESPACE